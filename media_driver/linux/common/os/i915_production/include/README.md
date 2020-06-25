This directory contains a copy of the installed kernel headers required by
the iHD driver to communicate with the kernel **for the set of particular
platforms**. The set of platforms is:
* DG1

This directory contains version of the kernel headers which is **not yet
available upstream**. By default media-driver build **does not use these
headers**. To enable driver build with these not upstreamed header files,
use `-DENABLE_PRODUCTION_KMD=ON` cmake build option.

Whenever driver needs new definitions for new kernel APIs, these files
should be updated.

These files in master should only be updated once the changes have landed
at https://repositories.intel.com/graphics/kernel-api/index.html

The last update was done with https://repositories.intel.com/graphics/kernel-api/drm-headers-20200624.tgz
