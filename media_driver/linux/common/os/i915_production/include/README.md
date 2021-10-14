This directory contains a copy of the installed kernel headers
required by the iHD driver to communicate with the kernel **for
the set of particular platforms**. The set of platforms is:

* DG1

This directory contains version of the kernel headers which is **not yet
available upstream**. By default media-driver build **does not use these
headers**. To enable driver build with these not upstreamed header files,
use -DENABLE_PRODUCTION_KMD=ON cmake build option.

Whenever driver needs new definitions for new kernel
APIs, these files should be updated.

These files in master should only be updated once the changes have landed
in the intel-gpu kernel tree (see https://github.com/intel-gpu/kernel).

You can copy files installed after running this from the kernel
repository, at version the driver requires:

$ make headers_install INSTALL_HDR_PATH=/path/to/install

The last update was done at the following kernel commit :

commit 3534f483a2e14dcbd7f4916de1de69ae5470dc48
Author: Fei Yang <fei.yang@intel.com>
Date:   Tue Jan 26 12:00:19 2021 -0800

    PROD_DG1_201210.0

    7fdc9667f80c (HEAD, tag: PROD_DG1_201210.0, origin/prod/5.4/dg1) drm/i915/dg1: WA GPU hang at RCC
    8dfaf41edcd6 (tag: PROD_DG1_201201.0, k/dg1-201201.0) drm/i915: Use ABI engine class in error state ecode
    d1c6201160ae drm/i915: Improve record of hung engines in error state
    3132f1ac9511 drm/i915: add extra slice common debug registers
    5dfc60bb1510 drm/i915: retry on set_cpu_domain for lmem obj migration and set 0
    6297c3c4f89b drm/i915: removal of trylock for req creation
    e4061f79d155 drm/i915/gem: Unpin idle contexts from kswapd reclaim

    Signed-off-by: Fei Yang <fei.yang@intel.com>

