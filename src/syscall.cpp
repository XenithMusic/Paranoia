#include "fail.h"
#include "memory.h"
#include "types.h"
#include "pit.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" {
	void handleSyscall(enum syscall_id id, int arg1, int arg2, int arg3) {
		// switch (id) {
		// 	case SCALL_SLEEP: {
		// 		// busy_sleep((double)arg1);
		// 	}
		// }
	}
}