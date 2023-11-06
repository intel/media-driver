This directory contains a copy of the installed kernel headers
required by the iHD driver to communicate with the kernel.
Whenever driver needs new definitions for new kernel
APIs, these files should be updated.

These files in master should only be updated once the changes have landed
in the drm-xe-next tree (see https://gitlab.freedesktop.org/drm/xe/kernel.git).

You can copy files installed after running this from the kernel
repository, at version the driver require:

$ make headers_install INSTALL_HDR_PATH=/path/to/install

The last update was done at the following kernel commit:

commit 4354e27efb78582ee567ba6264c79d0872a3a4e7 (HEAD -> drm-xe-next, origin/drm-xe-next)
Author: Brian Welty <brian.welty@intel.com>
Date:   Tue Sep 26 13:59:37 2023 -0700

    drm/xe: Simplify xe_res_get_buddy()

    We can remove the unnecessary indirection thru xe->tiles[] to get
    the TTM VRAM manager.  This code can be common for VRAM and STOLEN.

    Signed-off-by: Brian Welty <brian.welty@intel.com>
    Reviewed-by: Matthew Brost <matthew.brost@intel.com>