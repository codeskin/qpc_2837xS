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

#include "qpc.h"
#include "bsp.h"

#include "F2837xS_Device.h"
#include "F2837xS_Gpio.h"
#include "F2837xS_Gpio_defines.h"
#include "F2837xS_cputimervars.h"
#include "F2837xS_GlobalPrototypes.h"
#include "F2837xS_Pie_defines.h"

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

// function prototypes
extern void DevInit(Uint16 clock_source, Uint16 imult, Uint16 fmult);
extern void MemCopy(Uint16 *SourceAddr, Uint16 *SourceEndAddr, Uint16 *DestAddr);
extern void InitFlashHz(Uint32 clkHz);

// linker addresses, needed to copy code from flash to ram
extern Uint16 RamfuncsLoadStart, RamfuncsLoadEnd, RamfuncsRunStart;

static interrupt void SysTick(){
	CpuTimer0Regs.TCR.bit.TIF = 1;  // clear interrupt flag (seems to be optional)
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // acknowledge interrupt to PIE
	IER |= M_INT1;

	QF_TICK_X(0U, (void *)0); /* process time events for rate 0 */
}

void BSP_init(void){
	// low level hardware configuration
	DevInit(PLL_SRC, PLL_IMULT, PLL_FMULT);
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	InitFlashHz(SYSCLK_HZ); // this assumes that clock is exact, use 3% safety margin for INT OSC

	// for blinking
	InitGpio();
	EALLOW;
	GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;
	EDIS;

	// configure CPU timer
	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, SYSCLK_HZ/1000000L, 1000000L/(BSP_TICKS_PER_SEC));
	StartCpuTimer0();

	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	CpuTimer0Regs.TCR.bit.TIE = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge interrupt to PIE
	IER |= M_INT1;

	EALLOW;
	PieVectTable.TIMER0_INT = &SysTick;
	EDIS;
}

void BSP_ledOff(void){
	GpioDataRegs.GPACLEAR.bit.GPIO12 = 1;
}

void BSP_ledOn(void){
	GpioDataRegs.GPASET.bit.GPIO12 = 1;
}

void QF_onStartup(void){
	// enable interrupts
	EINT;   // global
	ERTM;   // real-time
}

void QF_onCleanup(void){
}

void QV_onIdle(void){
    QF_INT_ENABLE(); // CAUTION: called with interrupts DISABLED
}

void Q_onAssert(char const *module, int loc){
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
	asm(" ESTOP0");
	for(;;);
}
