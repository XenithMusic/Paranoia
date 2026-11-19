#include "terminal.h"
#include "string.h"
#include "pit.h"
#include "ps2.h"
#include "acpi.h"
#include "fail.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

enum DriverReturn {
    SUCCESS,
    FAILURE
};

namespace drivermanager {
    DriverReturn init(ACPITables* rsdt);
}

namespace drivers {
}