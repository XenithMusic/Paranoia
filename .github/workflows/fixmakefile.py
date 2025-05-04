with open("././Makefile","r") as f:
    makefile = f.read()
with open("././Makefile","w") as f:
    f.write(makefile.replace("i686","./tools/bin/i686"))