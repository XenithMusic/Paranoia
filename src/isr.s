/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

    .globl irq1_assembly
    .extern irq1_handler
    .globl basic_eoi_assembly_low
    .extern basic_eoi_handler_low
    .globl basic_eoi_assembly_high
    .extern basic_eoi_handler_high

basic_eoi_assembly_low:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call basic_eoi_handler_low
    add $8, %esp
    popal
    pop %ds
    iret
basic_eoi_assembly_high:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call basic_eoi_handler_high
    add $8, %esp
    popal
    pop %ds
    iret
irq1_assembly:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call irq1_handler
    add $8, %esp
    popal
    pop %ds
    iret