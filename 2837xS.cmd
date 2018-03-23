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

MEMORY
{
PAGE 0 :  /* Program Memory */
   BEGIN           	: origin = 0x080000, length = 0x000002
   RAMLS0          	: origin = 0x008000, length = 0x000800
   RAMLS1          	: origin = 0x008800, length = 0x000800
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3      		: origin = 0x009800, length = 0x000800
   RAMGS14_         : origin = 0x01A000, length = 0x001000  /* non secure, only available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS15_         : origin = 0x01B000, length = 0x001000  /* non secure, only available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RESET           	: origin = 0x3FFFC0, length = 0x000002

   /* Flash sectors */
   FLASHABC           : origin = 0x080002, length = 0x005FFE	/* on-chip Flash */

   //FLASHA_           : origin = 0x080002, length = 0x001FFE	/* on-chip Flash */
   //FLASHB_           : origin = 0x082000, length = 0x002000	/* on-chip Flash */
   //FLASHC_           : origin = 0x084000, length = 0x002000	/* on-chip Flash */
   FLASHD_           : origin = 0x086000, length = 0x002000	/* on-chip Flash */
   FLASHE_           : origin = 0x088000, length = 0x008000	/* on-chip Flash */
   FLASHF_           : origin = 0x090000, length = 0x008000	/* on-chip Flash */
   FLASHG_           : origin = 0x098000, length = 0x008000	/* on-chip Flash */
   FLASHH_           : origin = 0x0A0000, length = 0x008000	/* on-chip Flash */
   FLASHI_           : origin = 0x0A8000, length = 0x008000	/* on-chip Flash */
   FLASHJ_           : origin = 0x0B0000, length = 0x008000	/* on-chip Flash */
   FLASHK_           : origin = 0x0B8000, length = 0x002000	/* on-chip Flash */
   FLASHL_           : origin = 0x0BA000, length = 0x002000	/* on-chip Flash */
   FLASHM_           : origin = 0x0BC000, length = 0x002000	/* on-chip Flash */
   FLASHN_           : origin = 0x0BE000, length = 0x002000	/* on-chip Flash */

PAGE 1 : /* Data Memory */
   BOOT_RSVD_       : origin = 0x000002, length = 0x000120	/* Part of M0, BOOT rom will use this for stack */
   RAMM0_           : origin = 0x000122, length = 0x0002DE	/* non secure */
   RAMM1_           : origin = 0x000400, length = 0x000400	/* non secure, on-chip RAM block M1 */
   RAMD0           	: origin = 0x00B000, length = 0x000800
   RAMD1          	: origin = 0x00B800, length = 0x000800

   RAMLS4      		: origin = 0x00A000, length = 0x000800
   RAMLS5      		: origin = 0x00A800, length = 0x000800

   RAMGS0_      	: origin = 0x00C000, length = 0x001000	/* non secure */
   RAMGS1_      	: origin = 0x00D000, length = 0x001000	/* non secure */
   RAMGS2_      	: origin = 0x00E000, length = 0x001000	/* non secure */
   RAMGS3_      	: origin = 0x00F000, length = 0x001000	/* non secure */
   RAMGS4_      	: origin = 0x010000, length = 0x001000	/* non secure */
   RAMGS5_      	: origin = 0x011000, length = 0x001000	/* non secure */
   RAMGS6_      	: origin = 0x012000, length = 0x001000	/* non secure */
   RAMGS7_      	: origin = 0x013000, length = 0x001000	/* non secure */
   RAMGS8_11_   	: origin = 0x014000, length = 0x004000	/* non secure */
   RAMGS12_     	: origin = 0x018000, length = 0x001000   /* non secure, only available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS13_     	: origin = 0x019000, length = 0x001000   /* non secure, only available  on F28379D, F28377D, F28375D devices. Remove line on other devices. */
}

SECTIONS
{
    /* Allocate program areas: */
   .cinit              : > FLASHABC      	PAGE = 0, ALIGN(4)
   .pinit              : > FLASHABC      	PAGE = 0, ALIGN(4)
   .text               : > FLASHABC     	PAGE = 0, ALIGN(4)
   codestart           : > BEGIN      		PAGE = 0, ALIGN(4), { __APP_ENTRY = .;}


   /* Allocate uninitalized data sections: */
   .stack              : > RAMD0 		PAGE = 1
   .ebss               : >> RAMLS4 | RAMLS5     PAGE = 1
   .esysmem            : >> RAMLS4 | RAMLS5     PAGE = 1

   /* Initalized sections go in Flash */
   .econst             : > FLASHABC      PAGE = 0, ALIGN(4)
   .switch             : > FLASHABC      PAGE = 0, ALIGN(4)

   .reset              : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */

   GROUP : LOAD = FLASHABC,
                         RUN = RAMLS0 | RAMLS1 | RAMLS2 |RAMLS3,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(4)
   {
  	ramfuncs
#ifdef __TI_COMPILER_VERSION__
#if __TI_COMPILER_VERSION__ >= 15009000
  	.TI.ramfunc
#endif
#endif
   }
}

