import os
import sys

def change_mistake(path):
    for filename in os.listdir(path):
        fp = os.path.join(path, filename)
        if os.path.isfile(fp):
            if filename.find('Dscale') > -1 and filename.find('.dat') > -1:
                filename1 = filename.replace('Dscale', 'DScale')
                newfp = os.path.join(path, filename1)
                os.rename(fp, newfp)
            elif (filename.find('Dscale') > -1) and (filename.find('.fcpatch') > -1):
                filename1 = filename.replace('Dscale', 'DScale')
                newfp = os.path.join(path, filename1)
                os.rename(fp, newfp)
        else:
            change_mistake(fp)

change_mistake(sys.argv[1])