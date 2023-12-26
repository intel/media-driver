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

commit eb787b7ded88f3ea20bbbb5929931c3c11887341 (HEAD -> drm-xe-next, origin/drm-xe-next)
drm/xe/uapi: Be more specific about the vm_bind prefetch region

Let's bring a bit of clarity on this 'region' field that is
part of vm_bind operation struct. Rename and document to make
it more than obvious that it is a region instance and not a
mask and also that it should only be used with the prefetch
operation itself.

Signed-off-by: default avatarRodrigo Vivi <rodrigo.vivi@intel.com>
Signed-off-by: default avatarFrancois Dugast <francois.dugast@intel.com>
Reviewed-by: default avatarMatt Roper <matthew.d.roper@intel.com>