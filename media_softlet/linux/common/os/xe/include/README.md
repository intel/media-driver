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

commit 8ea438d53bbde68d8b0aedd1690f4b1644a1eb5e (HEAD -> drm-xe-next, origin/drm-xe-next)
drm/xe/uapi: Remove DRM_XE_VM_BIND_FLAG_ASYNC comment left over
This is a comment left over of commit d3d76739
("drm/xe/uapi: Remove sync binds").

Fixes: d3d76739

 ("drm/xe/uapi: Remove sync binds")
Reviewed-by: default avatarRodrigo Vivi <rodrigo.vivi@intel.com>
Cc: Matthew Brost <matthew.brost@intel.com>
Signed-off-by: default avatarJosé Roberto de Souza <jose.souza@intel.com>