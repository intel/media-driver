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

commit 0f1d88f2786458a8986920669bd8fb3fec6e618d (HEAD -> drm-xe-next, origin/drm-xe-next)
drm/xe/uapi: Kill exec_queue_set_property

All the properties should be immutable and set upon exec_queue creation
using the existent extension. So, let's kill this useless and dangerous
uapi.

Cc: Francois Dugast <francois.dugast@intel.com>
Cc: José Roberto de Souza <jose.souza@intel.com>
Cc: Matthew Brost <matthew.brost@intel.com>
Signed-off-by: default avatarRodrigo Vivi <rodrigo.vivi@intel.com>
Reviewed-by: default avatarJosé Roberto de Souza <jose.souza@intel.com>
Signed-off-by: default avatarFrancois Dugast <francois.dugast@intel.com>