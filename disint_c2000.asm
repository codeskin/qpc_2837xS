; * Copyright 2018 by CodeSkin LLC, www.codeskin.com.
; *
; * This program is free software: you can redistribute it and/or modify
; * it under the terms of the GNU General Public License as published by
; * the Free Software Foundation, either version 3 of the License, or
; * (at your option) any later version.

; * This program is distributed in the hope that it will be useful,
; * but WITHOUT ANY WARRANTY; without even the implied warranty of
; * ERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; * GNU General Public License for more details.

; * You should have received a copy of the GNU General Public License
; * along with this program.  If not, see <http://www.gnu.org/licenses/>.

   .def _DisableInt
   .def _RestoreInt

_DisableInt:
    PUSH  ST1
    SETC  INTM,DBGM
    MOV   AL, *--SP
    LRETR

_RestoreInt:
    MOV   *SP++, AL
    POP   ST1
    LRETR


   



