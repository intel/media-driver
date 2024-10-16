// Copyright (c) 2024, Intel Corporation

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

cc_library_shared {
    name: "libigfxcmrt",

    srcs: [
@LOCAL_SRC_FILES
    ],

    local_include_dirs: [
@LOCAL_C_INCLUDES
    ],

    cflags: [
        "-Werror",
        "-Wno-unused-variable",
        "-Wno-unused-parameter",
        "-Wno-unused-private-field",
        "-Wno-non-virtual-dtor",
        "-Wno-implicit-fallthrough",
@LOCAL_CFLAGS
    ],

    header_libs: [
        "libva_headers",
    ],

    shared_libs: [
        "libc",
        "libdl",
        "libcutils",
        "liblog",
        "libutils",
        "libm",
        "libva",
        "libva-android",
    ],
    vendor: true,
    enabled: false,
    arch: {
        x86_64: {
            enabled: true,
        },
    },
}

cc_library_headers {
    name: "libcmrt_headers",
    export_include_dirs: [
        "linux/hardware",
    ],

    vendor: true,
    enabled: false,
    arch: {
        x86_64: {
            enabled: true,
        },
    },
}
