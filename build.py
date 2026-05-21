import os,datetime,math,subprocess,argparse,sys

"""

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

"""

parser = argparse.ArgumentParser()
parser.add_argument("-k","--keepfs",action="store_true",default=False,
                    help="Prevents recreation of the filesystem.")
parser.add_argument("-D","--debug",action="store_true",default=False,
                    help="Enables debugging mode. More verbose logging.")
args = parser.parse_args()

COLOR_RED = "\033[31m"
COLOR_RESET = "\033[0m"

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

if args.debug:
     constants["CONST_DEBUGGING"] = "true"

# dynamic constants

constants["CONST_COMPDATE"] = "\"" + datetime.datetime.now().strftime("%a. %B %d, %Y @ %I:%M %p") + "\""
constants["CONST_KERNELSIZE"] = str(round((os.path.getsize("bin/paranoia.bin")*2)/512))

# manipulate constants

constants = [f"-D{i}='{v}'" for i,v in constants.items()]
constants = " ".join(constants)
constants = "CONST=\"" + escape_shell_string(constants) + "\""

print("\n\n\nCONSTANTS:",constants,"\n\n\n")
print()

# make iso
proc = subprocess.run(f"make -B {constants}",shell=True)
if proc.returncode != 0:
    print(f"""
{COLOR_RED}*************
Build Failed!
*************{COLOR_RESET}
""")
    exit(proc.returncode);
# && qemu-img create -f qcow2 -b paranoia.iso -o backing_fmt=raw paranoia.img 512M && \
# qemu-system-i386 -m 256M -s -drive file=paranoia.img,format=qcow2,media=disk -boot c")

# make filesystem
if not args.keepfs:
    print("[build] Making filesystem...")
    os.system("sudo ./build_filesystem")

# run
print("[build] Running!")
os.system("qemu-system-i386 -m 256M -s -drive file=filesystem.img,format=raw,media=disk -boot c")