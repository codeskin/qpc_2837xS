/*
 * Copyright 2018 by CodeSkin LLC, www.codeskin.com.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "F2837xS_device.h"
#include "F2837xS_Examples.h"

// Functions that will be run from RAM need to be assigned to
// a different section.  This section will then be mapped to a load and
// run address using the linker cmd file.
#pragma CODE_SECTION(InitFlash_Bank0_Hz, "ramfuncs");
#pragma CODE_SECTION(InitFlash_Bank1_Hz, "ramfuncs");

// Function prototypes
void ISR_ILLEGAL(void);
void DisableDog(void);

static void PieCntlInit(void);
static void PieVectTableInit(void);
static void EnableUnbondedIOPullups();

#pragma diag_suppress 112 // ASSERT(0) in switch statements
#define ASSERT(x) do {\
		if(!(x)){\
			asm("        ESTOP0");\
			for(;;);\
		}\
} while(0)

void DevInit(Uint16 clock_source, Uint16 imult, Uint16 fmult)
{
	DisableDog();
	DINT;			// Global Disable all Interrupts
	IER = 0x0000;	// Disable CPU interrupts
	IFR = 0x0000;	// Clear all CPU interrupt flags

	// Initialise interrupt controller and Vector Table
	// to defaults for now. Application ISR mapping done later.
	PieCntlInit();
	PieVectTableInit();

	//
	//      *IMPORTANT*
	//
	// The Device_cal function, which copies the ADC & oscillator calibration
	// values from TI reserved OTP into the appropriate trim registers, occurs
	// automatically in the Boot ROM. If the boot ROM code is bypassed during
	// the debug process, the following function MUST be called for the ADC and
	// oscillators to function according to specification. The clocks to the
	// ADC MUST be enabled before calling this function.
	//
	// See the device data manual and/or the ADC Reference Manual for more
	// information.
	//
	EALLOW;

	//
	// Enable pull-ups on unbonded IOs as soon as possible to reduce power
	// consumption.
	//
	EnableUnbondedIOPullups();

	CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
	CpuSysRegs.PCLKCR13.bit.ADC_B = 1;
	CpuSysRegs.PCLKCR13.bit.ADC_C = 1;
	CpuSysRegs.PCLKCR13.bit.ADC_D = 1;

	// this does not seem to work!
	//(*Device_cal)();

	//
	// Check if device is trimmed
	//
	if(*((Uint16 *)0x5D1B6) == 0x0000){
		//
		// Device is not trimmed--apply static calibration values
		//
		AnalogSubsysRegs.ANAREFTRIMA.all = 31709;
		AnalogSubsysRegs.ANAREFTRIMB.all = 31709;
		AnalogSubsysRegs.ANAREFTRIMC.all = 31709;
		AnalogSubsysRegs.ANAREFTRIMD.all = 31709;
	}

	CpuSysRegs.PCLKCR13.bit.ADC_A = 0;
	CpuSysRegs.PCLKCR13.bit.ADC_B = 0;
	CpuSysRegs.PCLKCR13.bit.ADC_C = 0;
	CpuSysRegs.PCLKCR13.bit.ADC_D = 0;
	EDIS;

	//
	// Initialize the PLL control: SYSPLLMULT and SYSCLKDIVSEL.
	//
	// Defined options to be passed as arguments to this function are defined
	// in F2807x_Examples.h.
	//
	// Note: The internal oscillator CANNOT be used as the PLL source if the
	// PLLSYSCLK is configured to frequencies above 194 MHz.
	//
	//  PLLSYSCLK = (XTAL_OSC) * (IMULT + FMULT) / (PLLSYSCLKDIV)
	//
	ASSERT((clock_source == 1) || (clock_source == 2)); // only XTAL and OSC2 supported
	InitSysPll(clock_source, imult, fmult, PLLCLK_BY_2);
	EDIS;
}

void DisableDog(void)
{
	volatile Uint16 temp;

	//
	// Grab the clock config first so we don't clobber it
	//
	EALLOW;
	temp = WdRegs.WDCR.all & 0x0007;
	WdRegs.WDCR.all = 0x0068 | temp;
	EDIS;
}

//
// InitSysPll - This function initializes the PLL registers.
//
// Note: The internal oscillator CANNOT be used as the PLL source if the
// PLLSYSCLK is configured to frequencies above 194 MHz.
//
// Note: This function uses the Watchdog as a monitor for the PLL. The user
// watchdog settings will be modified and restored upon completion.
//
void InitSysPll(Uint16 clock_source, Uint16 imult, Uint16 fmult, Uint16 divsel)
{
	if((clock_source == ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL)    &&
			(imult         == ClkCfgRegs.SYSPLLMULT.bit.IMULT)           &&
			(fmult         == ClkCfgRegs.SYSPLLMULT.bit.FMULT)           &&
			(divsel        == ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV))
	{
		//everything is set as required, so just return
		return;
	}

	if(clock_source != ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL)
	{
		switch (clock_source)
		{
		case INT_OSC2:
			EALLOW;
			ClkCfgRegs.CLKSRCCTL1.bit.INTOSC2OFF=0;         // Turn on INTOSC2
			ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 0;     // Clk Src = INTOSC2
			EDIS;
			break;

		case XTAL_OSC:
			EALLOW;
			ClkCfgRegs.CLKSRCCTL1.bit.XTALOFF=0;            // Turn on XTALOSC
			ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 1;     // Clk Src = XTAL
			EDIS;
			break;

		default:
			ASSERT(0);
			break;
		}
	}

	EALLOW;
	// first modify the PLL multipliers
	if(imult != ClkCfgRegs.SYSPLLMULT.bit.IMULT || fmult != ClkCfgRegs.SYSPLLMULT.bit.FMULT)
	{
		// Bypass PLL and set dividers to /1
		ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0;
		ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = 0;

		// Program PLL multipliers
		Uint32 temp_syspllmult = ClkCfgRegs.SYSPLLMULT.all;
		ClkCfgRegs.SYSPLLMULT.all = ((temp_syspllmult & ~(0x37FU)) |
				((fmult << 8U) | imult));

		ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 1;            // Enable SYSPLL

		// Wait for the SYSPLL lock
		while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1)
		{
			// Uncomment to service the watchdog
			// ServiceDog();
		}

		// Write a multiplier again to ensure proper PLL initialization
		// This will force the PLL to lock a second time
		ClkCfgRegs.SYSPLLMULT.bit.IMULT = imult;        // Setting integer multiplier

		// Wait for the SYSPLL re-lock
		while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1)
		{
			// Uncomment to service the watchdog
			// ServiceDog();
		}
	}

	// Set divider to produce slower output frequency to limit current increase
	if(divsel != PLLCLK_BY_126)
	{
		ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = divsel + 1;
	}else
	{
		ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = divsel;
	}

	// Enable PLLSYSCLK is fed from system PLL clock
	ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;

	// Small 100 cycle delay
	asm(" RPT #100 || NOP");

	// Set the divider to user value
	ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = divsel;
	EDIS;
}

// This function initializes the PIE control registers to a known state.
//
static void PieCntlInit(void)
{
	//
	// Disable Interrupts at the CPU level:
	//
	DINT;

	//
	// Disable the PIE
	//
	PieCtrlRegs.PIECTRL.bit.ENPIE = 0;

	//
	// Clear all PIEIER registers:
	//
	PieCtrlRegs.PIEIER1.all = 0;
	PieCtrlRegs.PIEIER2.all = 0;
	PieCtrlRegs.PIEIER3.all = 0;
	PieCtrlRegs.PIEIER4.all = 0;
	PieCtrlRegs.PIEIER5.all = 0;
	PieCtrlRegs.PIEIER6.all = 0;
	PieCtrlRegs.PIEIER7.all = 0;
	PieCtrlRegs.PIEIER8.all = 0;
	PieCtrlRegs.PIEIER9.all = 0;
	PieCtrlRegs.PIEIER10.all = 0;
	PieCtrlRegs.PIEIER11.all = 0;
	PieCtrlRegs.PIEIER12.all = 0;

	//
	// Clear all PIEIFR registers:
	//
	PieCtrlRegs.PIEIFR1.all = 0;
	PieCtrlRegs.PIEIFR2.all = 0;
	PieCtrlRegs.PIEIFR3.all = 0;
	PieCtrlRegs.PIEIFR4.all = 0;
	PieCtrlRegs.PIEIFR5.all = 0;
	PieCtrlRegs.PIEIFR6.all = 0;
	PieCtrlRegs.PIEIFR7.all = 0;
	PieCtrlRegs.PIEIFR8.all = 0;
	PieCtrlRegs.PIEIFR9.all = 0;
	PieCtrlRegs.PIEIFR10.all = 0;
	PieCtrlRegs.PIEIFR11.all = 0;
	PieCtrlRegs.PIEIFR12.all = 0;
}


static void PieVectTableInit(void)
{
	Uint16  i;
	Uint32  *Source  =  (void  *)  &ISR_ILLEGAL;
	Uint32  *Dest  =  (void  *)  &PieVectTable;

	//
	// Do not write over first 3 32-bit locations (these locations are
	// initialized by Boot ROM with boot variables)
	//
	Dest  =  Dest  +  3;

	EALLOW;
	for(i  =  0;  i  <  221;  i++)
	{
		*Dest++  =  *Source;
	}
	EDIS;

	//
	// Enable the PIE Vector Table
	//
	PieCtrlRegs.PIECTRL.bit.ENPIE  =  1;
}

interrupt void ISR_ILLEGAL(void)   // Illegal operation TRAP
{
	ASSERT(0);
}

// This function initializes the Flash Control registers

//                   CAUTION
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results
void InitFlash_Bank0_Hz(Uint32 clkHz)
{
	EALLOW;

	// set VREADST to the proper value for the
	// flash banks to power up properly
	// This sets the bank power up delay
	Flash0CtrlRegs.FBAC.bit.VREADST = 0x14;

	//At reset bank and pump are in sleep
	//A Flash access will power up the bank and pump automatically
	//After a Flash access, bank and pump go to low power mode (configurable in FBFALLBACK/FPAC1 registers)-
	//if there is no further access to flash
	//Power up Flash bank and pump and this also sets the fall back mode of flash and pump as active
	Flash0CtrlRegs.FPAC1.bit.PMPPWR = 0x1;
	Flash0CtrlRegs.FBFALLBACK.bit.BNKPWR0 = 0x3;

	//Disable Cache and prefetch mechanism before changing wait states
	Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 0;
	Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 0;

	//Set waitstates according to frequency
	//                CAUTION
	//Minimum waitstates required for the flash operating
	//at a given CPU rate must be characterized by TI.
	//Refer to the datasheet for the latest information.
	uint16_t clkMHz = (uint16_t)(clkHz / 1000000L);
	if(clkMHz > 150){
		Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x3;
	} else if(clkMHz > 100){
		Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x2;
	} else if(clkMHz > 50){
		Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x1;
	} else {
		Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x0;
	}

	//Enable Cache and prefetch mechanism to improve performance
	//of code executed from Flash.
	Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 1;
	Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 1;

	//At reset, ECC is enabled
	//If it is disabled by application software and if application again wants to enable ECC
	Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;

	EDIS;

	//Force a pipeline flush to ensure that the write to
	//the last register configured occurs before returning.

	__asm(" RPT #7 || NOP");

}

void InitFlash_Bank1_Hz(Uint32 clkHz)
{
	EALLOW;

	// set VREADST to the proper value for the
	// flash banks to power up properly
	// This sets the bank power up delay
	Flash1CtrlRegs.FBAC.bit.VREADST = 0x14;

	//At reset bank and pump are in sleep
	//A Flash access will power up the bank and pump automatically
	//After a Flash access, bank and pump go to low power mode (configurable in FBFALLBACK/FPAC1 registers)-
	//if there is no further access to flash
	//Power up Flash bank and pump and this also sets the fall back mode of flash and pump as active
	Flash1CtrlRegs.FPAC1.bit.PMPPWR = 0x1;
	Flash1CtrlRegs.FBFALLBACK.bit.BNKPWR0 = 0x3;

	//Disable Cache and prefetch mechanism before changing wait states
	Flash1CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 0;
	Flash1CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 0;

	//Set waitstates according to frequency
	//                CAUTION
	//Minimum waitstates required for the flash operating
	//at a given CPU rate must be characterized by TI.
	//Refer to the datasheet for the latest information.
	uint16_t clkMHz = (uint16_t)(clkHz / 1000000L);
	if(clkMHz > 150){
		Flash1CtrlRegs.FRDCNTL.bit.RWAIT = 0x3;
	} else if(clkMHz > 100){
		Flash1CtrlRegs.FRDCNTL.bit.RWAIT = 0x2;
	} else if(clkMHz > 50){
		Flash1CtrlRegs.FRDCNTL.bit.RWAIT = 0x1;
	} else {
		Flash1CtrlRegs.FRDCNTL.bit.RWAIT = 0x0;
	}


	//Enable Cache and prefetch mechanism to improve performance
	//of code executed from Flash.
	Flash1CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 1;
	Flash1CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 1;

	//At reset, ECC is enabled
	//If it is disabled by application software and if application again wants to enable ECC
	Flash1EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;

	EDIS;

	//Force a pipeline flush to ensure that the write to
	//the last register configured occurs before returning.

	__asm(" RPT #7 || NOP");

}

// This function will copy the specified memory contents from
// one location to another.
//
//	Uint16 *SourceAddr        Pointer to the first word to be moved
//                          SourceAddr < SourceEndAddr
//	Uint16* SourceEndAddr     Pointer to the last word to be moved
//	Uint16* DestAddr          Pointer to the first destination word
//
// No checks are made for invalid memory locations or that the
// end address is > then the first start address.

void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr)
{
	while(SourceAddr < SourceEndAddr)
	{
		*DestAddr++ = *SourceAddr++;
	}
	return;
}

static void EnableUnbondedIOPullupsFor176Pin()
{
	EALLOW;
	GpioCtrlRegs.GPCPUD.all = ~0x80000000;  //GPIO 95
	GpioCtrlRegs.GPDPUD.all = ~0xFFFFFFF7;  //GPIOs 96-127
	GpioCtrlRegs.GPEPUD.all = ~0xFFFFFFDF;  //GPIOs 128-159 except for 133
	GpioCtrlRegs.GPFPUD.all = ~0x000001FF;  //GPIOs 160-168
	EDIS;
}

static void EnableUnbondedIOPullupsFor100Pin()
{
	EALLOW;
	GpioCtrlRegs.GPAPUD.all = ~0xFFC003E3;  //GPIOs 0-1, 5-9, 22-31
	GpioCtrlRegs.GPBPUD.all = ~0x03FFF1FF;  //GPIOs 32-40, 44-57
	GpioCtrlRegs.GPCPUD.all = ~0xE10FBC18;  //GPIOs 67-68, 74-77, 79-83, 93-95
	GpioCtrlRegs.GPDPUD.all = ~0xFFFFFFF7;  //GPIOs 96-127
	GpioCtrlRegs.GPEPUD.all = ~0xFFFFFFFF;  //GPIOs 128-159
	GpioCtrlRegs.GPFPUD.all = ~0x000001FF;  //GPIOs 160-168
	EDIS;
}

//
// GPIO_EnableUnbondedIOPullups - Enable IO pullups for specific packages
//
static void EnableUnbondedIOPullups()
{
	unsigned char pin_count = ((DevCfgRegs.PARTIDL.all & 0x00000700) >> 8) ;

	//
	// 5 = 100 pin
	// 6 = 176 pin
	// 7 = 337 pin
	//
	if(pin_count == 5)
	{
		EnableUnbondedIOPullupsFor100Pin();
	}
	else if (pin_count == 6)
	{
		EnableUnbondedIOPullupsFor176Pin();
	}
	else
	{
		//
		//do nothing - this is 337 pin package
		//
	}
}

//===========================================================================
// End of file.
//===========================================================================
