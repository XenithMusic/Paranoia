/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" {
    void exception_handler();
    void divzero_handler();
    void genprotfault_handler();
    void irq1_assembly();
    void irq1_handler();
    void basic_eoi_assembly_low();
    void basic_eoi_handler_low();
    void basic_eoi_assembly_high();
    void basic_eoi_handler_high();
    void syscall();
    void syscall_c(int id,int arg1,int arg2,int arg3);
}