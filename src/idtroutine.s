/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.



Defines:

setIDT(limit, base)
// introduced in indev-2024-11-24

setGDT(limit, base)
// introduced in indev-2024-11-24

*/

# setIDT(limit, base)

.idtr:
	.word 0 # For limit storage (16-bit)
	.long 0 # For base storage (32-bit)

.section .text
.global setIDT

setIDT:
   MOV   4(%esp), %ax # [esp + 4]
   MOV   %ax, .idtr # [idtr]
   MOV   8(%esp), %eax # [ESP + 8]
   MOV   %eax, .idtr + 2 # [idtr + 2]
   LIDT  .idtr
   RET


# setGDT(limit, base)

.gdtr:
	.word 0 # For limit storage
   .long 0 # For base storage

.section .text
.global setGDT

setGDT:
   MOV   4(%esp), %ax # [esp + 4]
   MOV   %ax, .gdtr
   MOV   8(%esp), %eax # [ESP + 8]
   MOV   .gdtr + 2, %eax # [gdtr + 2]
   LGDT  .gdtr
   RET
