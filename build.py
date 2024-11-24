import os,datetime,math

def escape_shell_string(input_str):
    # Escape single quotes and double quotes by adding backslashes
    return input_str.replace('"', '\\"')

os.system(f"make -Bs")

with open("Makefile","r") as f:
	data = f.read().split("\n")

constants = {
	"CONST_VERSION":"\"indev-2024-11-23\""
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

os.system(f"make -B {constants} && qemu-system-i386 -cdrom paranoia.iso -boot d")