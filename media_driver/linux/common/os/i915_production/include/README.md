This directory contains a copy of the installed kernel headers
required by the iHD driver to communicate with the kernel **for
the set of particular platforms**. The set of platforms is:

* DG2, XEHP_SDV, PVC, ATS-M

This directory contains version of the kernel headers which is **not yet
available upstream**. These files are taken from https://github.com/intel-gpu/drm-uapi-helper/tree/master/drm-uapi. 
By default media-driver build **does not use these
headers**. To enable driver build with these not upstreamed header files,
use -DENABLE_PRODUCTION_KMD=ON cmake build option.

Whenever driver needs new definitions for new kernel
APIs, these files should be updated.

The last update was done at the following kernel commit:

```
Repo:      https://github.com/intel-gpu/drm-uapi-helper/tree/master/drm-uapi
Author:    Ashutosh Dixit <ashutosh.dixit@intel.com>
Date:      Thu Jul 21 13:02:21 2022 -0700
Commit ID: 014dbfd6f934a16b6665957692ccfc5d862aa6fe
Tag:       v2.0-rc14
```
