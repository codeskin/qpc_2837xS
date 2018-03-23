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

#ifndef BSP_H_
#define BSP_H_

#define InitFlashHz InitFlash_Bank0_Hz

#if !defined(_CODESKIN_) || !defined(CPU1)
#error Incorrect project settings!
#endif

#define SYSCLK_HZ (200000000l)
#define LSPCLK_HZ (SYSCLK_HZ / 4l)

#define PLL_SRC 1 // 1=XTAL, 2=INT
#define PLL_FMULT 0
#define PLL_IMULT 40 // assuming 10 MHz crystal (28377S LaunchPad)

#define BSP_TICKS_PER_SEC 10000U

void BSP_init(void);

void BSP_ledOn(void);
void BSP_ledOff(void);

#endif /* BSP_H_ */
