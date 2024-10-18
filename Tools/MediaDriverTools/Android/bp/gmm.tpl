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

package {
    default_applicable_licenses: ["external_gmmlib_license"],
}

license {
    name: "external_gmmlib_license",
    visibility: [":__subpackages__"],
    license_text: [
        "LICENSE.md",
    ],
}

cc_library_shared {
    name: "libigdgmm_android",
    vendor: true,
    srcs: [
@LOCAL_SRC_FILES
    ],

    cflags: [
@LOCAL_CFLAGS
        "-fvisibility=hidden",
        "-fno-omit-frame-pointer",
        "-march=corei7",
        "-Werror",
        "-Wno-logical-op-parentheses",
        "-Wno-shift-negative-value",
        "-Wno-unused-parameter",
    ],

    cppflags: [
        "-Wno-implicit-fallthrough",
        "-Wno-missing-braces",
        "-Wno-unknown-pragmas",
        "-Wno-parentheses",
        "-Wno-pragma-pack",
        "-fexceptions",
        "-std=c++11",
        "-fvisibility-inlines-hidden",
        "-fno-use-cxa-atexit",
        "-fno-rtti",
        "-fcheck-new",
        "-pthread",
    ],

    local_include_dirs: [
@LOCAL_C_INCLUDES
    ],

    enabled: false,
    arch: {
        x86_64: {
            enabled: true,
        },
    },
}

cc_library_headers {
    name: "libigdgmm_headers",
    vendor: true,
    export_include_dirs: [
        "Source/GmmLib/inc",
        "Source/inc",
        "Source/inc/common",
    ],

    enabled: false,
    arch: {
        x86_64: {
            enabled: true,
        },
    },
}
