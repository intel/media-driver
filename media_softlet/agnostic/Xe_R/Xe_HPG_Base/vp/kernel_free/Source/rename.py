import os
import sys

### This scrpit is used to rename files. *Dscale* -> *DScale*

def change_mistake(path, upperFileName):
    upperFile = os.path.join(path, upperFileName)
    print("upperFile: " + upperFile)
    if(os.path.exists(upperFile)):
        print("Upper file" + upperFile + " exists! No need to rename!")
    else:
        lowerFile = upperFile.replace('DScale', 'Dscale')
        os.rename(lowerFile, upperFile)
        print(lowerFile + "replaced!")

change_mistake(sys.argv[1], sys.argv[2])
if(len(sys.argv) == 4):
    change_mistake(sys.argv[1], sys.argv[3])
