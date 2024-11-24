#include "terminal.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


void setError(int errno);
int getError();
void clearError();
void fault(int errno,char* string);
void fault(int errno);



// DEPRECATED

void panic(int errno,char* string);
// introduced in indev-2024-11-23
// deprecated in indev-2024-11-23
// superceded by fault(int, char*)

void panic(int errno);
// introduced in indev-2024-11-23
// deprecated in indev-2024-11-23
// superceded by fault(int, char*)
