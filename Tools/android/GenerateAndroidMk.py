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
2. get soruce files using "grep -ir \\.o:$ Makefile  |sed s/.c.o:/.c/ |sed s/.cpp.o:/.cpp/ | sed s,^,\\t, | sed s/$/' \\'/"
3. get c/cpp defines using "grep CXX_DEFINES flags.make | sed s/"CXX_DEFINES = "/""/g | sed s/" "/"\n"/g | sed s/"^"/"\t"/g | sed 's/$/ \\/'"
4. get includes directories using "grep CXX_INCLUDES flags.make | sed s/"CXX_DEFINES = "/""/g | sed s/" "/"\n"/g | sed s/"^"/"\t"/g | sed 's/$/ \\/'"
5. replace related text in template
"""

import os
import os.path as path
import re

INDENT = "    "


class Generator:
    def __init__(self, src, templ):
        self.src_ = src
        self.templ_ = templ
        self.build_ = path.join(templ, "build")

    #major function
    def generate(self):
        if path.exists(self.src_) == False:
            raise Exception(self.src_ + "not existed")
        self.generateMakefile()
        with open(self.getTemplatePath()) as f:
            tpl = f.read()
        tpl = tpl.replace("@LOCAL_SRC_FILES", self.getSources())
        tpl = tpl.replace("@LOCAL_CFLAGS", self.getDefines("CXX_DEFINES"))
        tpl = tpl.replace("@LOCAL_C_INCLUDES", self.getIncludes())
        with open(path.join(self.src_, "Android.mk"), "w") as f:
            f.write(tpl)

    #virtuall functions
    def getTemplatePath(self):
        raise Exception("pure virtul function")

    def getMakefile(self):
        raise Exception("pure virtul function")

    def getFlagsfile(self):
        raise Exception("pure virtul function")

    def adjustSources(self, lines):
        print(lines)
        return lines

    def adjustIncludes(self, lines):
        print(lines)
        return lines

    def getCmakeCmd(self):
        return "cmake " + self.src_

    def generateMakefile(self):
        #windows can help us debug the script
        #but we do not want generate makefile on widnows
        if os.name == "nt":
            return
        cmd = "rm " + path.join(self.build_, "CMakeCache.txt") + "> /dev/null 2>&1;"
        cmd += "mkdir " + self.build_ + "> /dev/null 2>&1;"
        cmd += "cd " + self.build_
        cmd += "&& " + self.getCmakeCmd() + "> /dev/null 2>&1"
        os.system(cmd)

    def getIncludes(self):
        text = self.getDefines("CXX_INCLUDES")
        includes = []
        lines = text.split("\n")
        for l in lines:
            #normpath will make sure we did not refer outside.
            p = path.normpath(l)
            j = p.find(self.src_)
            if j != -1:
                includes.append(p[j:].replace(self.src_, "$(LOCAL_PATH)"))
        return INDENT + ("\n" + INDENT).join(includes)

    def getDefines(self, name):
        flags = self.getFlagsfile()
        with open(flags) as f:
            text = f.read()
        line = re.findall(name + " =.*\n", text)[0]
        line = line.replace(name + " = ", "")
        line = INDENT + line.replace(" ", " \\\n" + INDENT)
        return line

    def getSources(self):
        makefile = self.getMakefile()
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
        templ = path.join(src,  "Tools/android")
        super(GmmGeneator, self).__init__(src, templ)

    def getTemplatePath(self):
        return path.join(self.templ_, "gmm.Android.mk.template")

    def getMakefile(self):
        return path.join(self.build_, "Source/GmmLib/Makefile")

    def getFlagsfile(self):
        return path.join(self.build_, "Source/GmmLib/CMakeFiles/gmm_umd.dir/flags.make")

    def adjustSources(self, lines):
        for i, l in enumerate(lines):
            j = l.find("__/")
            if j == -1:
                lines[i] = path.join("Source/GmmLib", l)
            else:
                lines[i] = path.join("Source", l[j+3:])


class CmrtGeneator(Generator):
    def __init__(self, root):
        src = path.join(root, "media-driver/cmrtlib")
        templ = path.join(src,  "android")
        super(CmrtGeneator, self).__init__(src, templ)

    def getTemplatePath(self):
        return path.join(self.templ_, "cmrt.Android.mk.template")

    def getMakefile(self):
        return path.join(self.build_, "linux/Makefile")

    def getFlagsfile(self):
        return path.join(self.build_, "linux/CMakeFiles/igfxcmrt.dir/flags.make")

    def adjustSources(self, lines):
        for i, l in enumerate(lines):
            j = l.find("__/")
            if j == -1:
                lines[i] = path.join("linux", l)
            else:
                lines[i] = l[j+3:]


class DriverGeneator(Generator):
    def __init__(self, root):
        src = path.join(root, "media-driver/media_driver")
        templ = path.join(src,  "android")
        super(DriverGeneator, self).__init__(src, templ)

    def getCmakeCmd(self):
        """
        cmake ../../../../media-driver -DCMAKE_INSTALL_PREFIX=/usr -DMEDIA_VERSION="2.0.0" \
        -DBUILD_ALONG_WITH_CMRTLIB=1 -DBS_DIR_GMMLIB=`pwd`/../../../../gmmlib/Source/GmmLib/ \
        -DBS_DIR_COMMON=`pwd`/../../../../gmmlib/Source/Common/ -DBS_DIR_INC=`pwd`/../../../../gmmlib/Source/inc/ \
        -DBS_DIR_MEDIA=`pwd`/../../../../media-driver
        """
        cmd = 'cmake ../../../../media-driver -DCMAKE_INSTALL_PREFIX=/usr -DMEDIA_VERSION="2.0.0" '
        cmd += "-DBUILD_ALONG_WITH_CMRTLIB=1 -DBS_DIR_GMMLIB=`pwd`/../../../../gmmlib/Source/GmmLib/ "
        cmd += "-DBS_DIR_COMMON=`pwd`/../../../../gmmlib/Source/Common/ -DBS_DIR_INC=`pwd`/../../../../gmmlib/Source/inc/ "
        cmd += "-DBS_DIR_MEDIA=`pwd`/../../../../media-driver"
        return cmd

    def getTemplatePath(self):
        return path.join(self.templ_, "media_driver.Android.mk.template")

    def getMakefile(self):
        return path.join(self.build_, "media_driver/Makefile")

    def getFlagsfile(self):
        return path.join(self.build_, "media_driver/CMakeFiles/iHD_drv_video.dir/flags.make")

    def adjustSources(self, lines):
        lines[:] = [l for l in lines if l.find(
            "media_libva_putsurface_linux.cpp") == -1]


class Main:

    def run(self):
        templ = path.dirname(__file__)
        root = path.abspath(path.join(templ, "../../.."))

        print("+++++++++++generate Android.mk for gmm++++++++++++")
        gmm = GmmGeneator(root)
        gmm.generate()

        print("+++++++++++generate Android.mk for cmrt++++++++++++")
        cmrt = CmrtGeneator(root)
        cmrt.generate()

        print("+++++++++++generate Android.mk for meida_drver++++++++++++")
        driver = DriverGeneator(root)
        driver.generate()


m = Main()
m.run()
