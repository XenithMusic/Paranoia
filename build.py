import os,datetime,math

"""

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

"""

def escape_shell_string(input_str):
    # Escape single quotes and double quotes by adding backslashes
    return input_str.replace('"', '\\"')

os.system(f"make -Bs")

with open("Makefile","r") as f:
	data = f.read().split("\n")

constants = {
	"CONST_VERSION":"\"indev-2025-01-24\"",
    "CONST_DEBUGGING":"false",
    "BITS32":"true",
}

# dynamic constants

constants["CONST_COMPDATE"] = "\"" + datetime.datetime.now().strftime("%a. %B %d, %Y @ %I:%M %p") + "\""
constants["CONST_KERNELSIZE"] = str(round((os.path.getsize("bin/paranoia.bin")*2)/512))


# manipulate constants

constants = [f"-D{i}='{v}'" for i,v in constants.items()]
constants = " ".join(constants)
constants = "CONST=\"" + escape_shell_string(constants) + "\""

print("\n\n\nCONSTANTS:",constants,"\n\n\n")
print()

os.system(f"make -B {constants} && qemu-system-i386 -m 256M -s -cdrom paranoia.iso -boot d ")