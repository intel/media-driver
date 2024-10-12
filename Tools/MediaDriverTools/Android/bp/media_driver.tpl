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
    default_applicable_licenses: ["external_intel_media_driver_license"],
}

license {
    name: "external_intel_media_driver_license",
    visibility: [":__subpackages__"],
    license_text: [
        "LICENSE.md",
    ],
}

//####################COMMON CFLAGS######################
common_cflags = [
    "-Wreorder",
    "-Wsign-promo",
    "-Wno-invalid-offsetof",
    "-fno-use-cxa-atexit",
    "-fexceptions",
    "-fcheck-new",
    "-Werror",
    "-Wno-unknown-pragmas",
    "-Wno-comments",
    "-Wno-attributes",
    "-Wno-narrowing",
    "-Wno-overflow",
    "-Wno-parentheses",
    "-Wno-delete-incomplete",
    "-Wno-overloaded-virtual",
    "-std=c++14",
    "-DNO_EXCEPTION_HANDLING",
    "-Wno-ignored-qualifiers",
    "-Wno-missing-braces",
    "-Wno-implicit-fallthrough",
    "-DGMM_LIB_DLL",
@LOCAL_CFLAGS
]

cc_library_shared {
    name: "iHD_drv_video",

    srcs: [
@LOCAL_SRC_FILES
    ],

    header_libs: [
        "libva_headers",
        "libigdgmm_headers",
        "libcmrt_headers",
    ],

    shared_libs: [
        "libsync",
        "libutils",
        "libdrm",
        "libva",
        "liblog",
        "libigdgmm_android",
    ],

    cflags: common_cflags,
    cppflags: common_cflags,
    conlyflags: [
        "-xc++",
        "-std=c++14",
    ],

    local_include_dirs: [
@LOCAL_C_INCLUDES
    ],

    rtti: true,
    enabled: false,
    arch: {
        x86_64: {
            enabled: true,
        },
    },

    vendor: true,
}
