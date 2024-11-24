#define PIT_FREQUENCY 1193182.0

#include "assemblyUtils.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


extern "C" {
	void set_pit_count(unsigned count);
	unsigned get_pit_count(void);
	double get_pit_seconds(void);
	void sleep(double seconds);
}