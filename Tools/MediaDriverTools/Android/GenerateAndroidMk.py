#! /usr/bin/env python3
"""
* Copyright (c) 2018, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
"""

"""
the basic idea from this script are:
1. generate linux make file using cmake
2. get source files using "grep -ir \\.o:$ Makefile  |sed s/.c.o:/.c/ |sed s/.cpp.o:/.cpp/ | sed s,^,\\t, | sed s/$/' \\'/"
3. get c/cpp defines using "grep CXX_DEFINES flags.make | sed s/"CXX_DEFINES = "/""/g | sed s/" "/"\n"/g | sed s/"^"/"\t"/g | sed 's/$/ \\/'"
4. get includes directories using "grep CXX_INCLUDES flags.make | sed s/"CXX_DEFINES = "/""/g | sed s/" "/"\n"/g | sed s/"^"/"\t"/g | sed 's/$/ \\/'"
5. replace related text in template
"""

import os
import os.path as path
import re

INDENT = "    "
TOOL_DIR = "Tools/MediaDriverTools/Android/"
TEMPLATE_DIR = path.join(TOOL_DIR, "mk")


def remove(f):
    cmd = "rm " + f + "> /dev/null 2>&1"
    os.system(cmd)

def getDriverName(root):
    driver = "mod"
    if not path.exists(path.join(root, driver)):
        driver = "media-driver"
    if not path.exists(path.join(root, driver)):
        raise Exception("driver path " + driver +" not existed")
    return driver

class Generator:
    def __init__(self, src, root, makefile=None):
        #where to put the Android makefile
        self.makefile = makefile if makefile else src

        #where is the major source file
        self.src = src

        #driver name
        driver = getDriverName(root)
        #where to put build file and template
        self.tool = path.join(root, driver, TOOL_DIR)

        """where to put the template"""
        self.templ = path.join(root, driver, TEMPLATE_DIR)

    #major function
    def generate(self):

        if path.exists(self.src) == False:
            raise Exception(self.src + "not existed")
        self.generateMakefile()

        mk = path.join(self.makefile, "Android.mk")
        #remove old Android.mk
        remove(mk)

        #create new Android.mk
        with open(self.getTemplatePath()) as f:
            tpl = f.read()
        tpl = tpl.replace("@LOCAL_SRC_FILES", self.getSources())
        tpl = tpl.replace("@LOCAL_CFLAGS", self.getDefines("CXX_DEFINES"))
        tpl = tpl.replace("@LOCAL_C_INCLUDES", self.getIncludes())
        with open(mk, "w") as f:
            f.write(tpl)
        print("generated " + self.getName() + " to " + self.makefile)

    #virtuall functions
    def getTemplate(self):
        raise Exception("pure virtul function")

    def getFlagsfile(self):
        raise Exception("pure virtul function")

    def getMakefile(self):
        return "Makefile"

    def getName(self):
        return self.getTemplate().split('.')[0]

    def getTemplatePath(self):
        return path.join(self.templ, self.getTemplate())

    def getBuildDir(self):
        return path.join(self.tool, 'build', self.getName())

    def adjustSources(self, lines):
        #print(lines)
        return lines

    def adjustIncludes(self, lines):
        #print(lines)
        return lines

    def getCmakeCmd(self):
        return "cmake " + self.src

    def generateMakefile(self, debug=False):
        #Win env can help us debug the script
        #but we do not want generate makefile on Win-system
        if os.name == "nt":
            return
        verbose = ";" if debug else "> /dev/null 2>&1;"
        build = self.getBuildDir()
        cmd = "rm " + path.join(build, "CMakeCache.txt") + verbose
        cmd += "mkdir -p " + build + verbose
        cmd += "cd " + build
        cmd += "&& " + self.getCmakeCmd() + verbose
        os.system(cmd)

    def getIncludes(self):
        text = self.getDefines("CXX_INCLUDES")
        includes = []
        lines = text.split("\n")
        for l in lines:
            #normpath will make sure we did not refer outside.
            p = path.normpath(l)
            j = p.find(self.src)
            if j != -1:
                includes.append(p[j:].replace(self.src, "$(LOCAL_PATH)"))
        return INDENT + ("\n" + INDENT).join(includes) if includes else ""

    def getDefines(self, name):
        flags = path.join(self.getBuildDir(), self.getFlagsfile())
        with open(flags) as f:
            text = f.read()
        line = re.findall(name + " =.*\n", text)[0]
        if not line:
            return ""

        line = line.replace(name + " = ", "").strip()
        return INDENT + line.replace(" ", " \\\n" + INDENT) if line else ""

    def getSources(self):
        makefile = path.join(self.getBuildDir(), self.getMakefile())
        with open(makefile) as f:
            text = f.read()
        lines = re.findall(".*?\\.o:\\n", text)
        lines = [l.replace(".o:\n", " \\") for l in lines]
        self.adjustSources(lines)
        #make source pretty
        return INDENT + ("\n" + INDENT).join(lines)


class GmmGeneator(Generator):
    def __init__(self, root):
        src = path.join(root, "gmmlib")
        super(GmmGeneator, self).__init__(src, root)

    def getTemplate(self):
        return "gmm.tpl"

    def getMakefile(self):
        return "Source/GmmLib/Makefile"
    def getFlagsfile(self):
        return "Source/GmmLib/CMakeFiles/igfx_gmmumd_dll.dir/flags.make"

    def adjustSources(self, lines):
        for i, l in enumerate(lines):
            j = l.find("__/")
            if j == -1:
                lines[i] = path.join("Source/GmmLib", l)
            else:
                lines[i] = path.join("Source", l[j+3:])


class CmrtGeneator(Generator):
    def __init__(self, root):
        src = path.join(root, getDriverName(root), "cmrtlib")
        super(CmrtGeneator, self).__init__(src, root)

    def getTemplate(self):
        return "cmrt.tpl"

    def getMakefile(self):
        return "linux/Makefile"

    def getFlagsfile(self):
        return "linux/CMakeFiles/igfxcmrt.dir/flags.make"

    def adjustSources(self, lines):
        for i, l in enumerate(lines):
            j = l.find("__/")
            if j == -1:
                lines[i] = path.join("linux", l)
            else:
                lines[i] = l[j+3:]


class DriverGeneator(Generator):
    def __init__(self, root):
        src = path.join(root, getDriverName(root), "media_driver")
        super(DriverGeneator, self).__init__(src, root)

    def getCmakeCmd(self):
        wd = path.join(self.src, "..")
        cmd = 'cmake ' + wd +' -DCMAKE_INSTALL_PREFIX=/usr -DMEDIA_VERSION="2.0.0"'
        cmd += ' -DBUILD_ALONG_WITH_CMRTLIB=1 -DBS_DIR_GMMLIB=' + path.join(wd, '../gmmlib/Source/GmmLib/')
        cmd += ' -DBS_DIR_COMMON=' + path.join(wd, '../gmmlib/Source/Common/')
        cmd += ' -DBS_DIR_INC=' + path.join(wd, '../gmmlib/Source/inc/')
        cmd += ' -DBS_DIR_MEDIA=' + wd
        return cmd

    def getTemplate(self):
        return "media_driver.tpl"

    def getMakefile(self):
        return "media_driver/Makefile"

    def getFlagsfile(self):
        return "media_driver/CMakeFiles/iHD_drv_video.dir/flags.make"

    def adjustSources(self, lines):
        lines[:] = [l for l in lines if "media_libva_putsurface_linux.cpp" not in l]

class Main:

    def run(self):
        tool = path.dirname(__file__)
        root = path.abspath(path.join(tool, "../../../../"))
        print("root = "+root)
        gens = [GmmGeneator(root), CmrtGeneator(root), DriverGeneator(root)]
        for g in gens:
            g.generate()


m = Main()
m.run()
