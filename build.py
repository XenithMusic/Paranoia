#!/usr/bin/env python
import os,datetime,math,subprocess,argparse,sys,struct

def embed(path):
    data = b""
    with open(path,"rb") as f:
        data = f.read()
    data = list(data)
    return ",".join(str(x) for x in data)

def fail(ret):
    print(f"""
{COLOR_RED}*************
Build Failed!
*************{COLOR_RESET}
""")
    exit(ret);

def makeTemplates():
    for i in os.listdir("templates"):
        with open(f"templates/{i}","r") as f:
            with open(f"src/{i}","w") as g:
                lines = f.read().split("\n")
                out = []
                for i in lines:
                    if i.startswith("/*$$"):
                        i = i[4:]
                        i = eval(i)
                    out.append(i)
                g.write("\n".join(out))
"""

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

"""

print(sys.argv)
parser = argparse.ArgumentParser()
parser.add_argument("-k","--keepfs",action="store_true",default=False,
                    help="Prevents recreation of the filesystem.")
parser.add_argument("-D","--debug",action="store_true",default=False,
                    help="Enables debugging mode. More verbose logging.")
parser.add_argument("-q","--qemuflags",default="",
                    help="Passes extra flags to QEMU.")
parser.add_argument("-s","--skip",action="store_true",default=False,
                    help="Skip compilation, only run.")
parser.add_argument("-u","--uefi",action="store_true",default=False,
                    help="(experimental) Make a UEFI system instead of a BIOS system.")
parser.add_argument("-S","--strip",action="count",default=0,
                    help="Prevent generation of extra debug symbols. Vastly reduces file size. Pass twice to strip the remaining debug symbols.")
args = parser.parse_args()
QFLAGS = args.qemuflags
QEMUPLATFORM = "i386"
BIOSMODE = "bios"
if args.uefi:
    QFLAGS += " -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,file=$HOME/.local/share/qemu/OVMF_VARS_4M.fd"
    QEMUPLATFORM = "x86_64"
    BIOSMODE = "uefi"
MAKEFLAGS = f"STRIP={args.strip}"


COLOR_RED = "\033[31m"
COLOR_RESET = "\033[0m"

def escape_shell_string(input_str):
    # Escape single quotes and double quotes by adding backslashes
    return input_str.replace('"', '\\"')

constants = {
	"CONST_VERSION":"\"early-0.1.0\"",
    "BITS32":"true",
}

if args.debug:
     constants["CONST_DEBUGGING"] = "true"

# dynamic constants

constants["CONST_COMPDATE"] = "\"" + datetime.datetime.now().strftime("%a. %B %d, %Y @ %I:%M %p") + "\""

# manipulate constants

constants = [f"-D{i}='{v}'" for i,v in constants.items()]
constants = " ".join(constants)
constants = "CONST=\"" + escape_shell_string(constants) + "\""

print("\n\n\nCONSTANTS:",constants,"\n\n\n")
print()

if not args.skip:
    # make templates
    makeTemplates()

    # make iso
    proc = subprocess.run(f"make -B {constants} {MAKEFLAGS}",shell=True)
    if proc.returncode != 0:
        fail(proc.returncode)
    # && qemu-img create -f qcow2 -b paranoia.iso -o backing_fmt=raw paranoia.img 512M && \
    # qemu-system-i386 -m 256M -s -drive file=paranoia.img,format=qcow2,media=disk -boot c")

    with open("./filesystem/sys/state/compiled/kernelsize","wb") as f:
        f.write(struct.pack("d",float(os.path.getsize("bin/paranoia.bin"))))

    # make filesystem
    if not args.keepfs:
        print("[build] Making filesystem...")
        proc = subprocess.run(f"sudo ./build_filesystem {BIOSMODE}",shell=True)
        if proc.returncode != 0:
            fail(proc.returncode)

# run
print("[build] Running!")
os.system(f"qemu-system-{QEMUPLATFORM} -m 256M -s {QFLAGS} -drive file=filesystem.img,format=raw,media=disk -boot c")