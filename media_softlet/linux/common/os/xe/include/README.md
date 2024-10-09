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

commit f2881dfdaaa9ec873dbd383ef5512fc31e576cbb (HEAD -> drm-xe-next, origin/drm-xe-next)

drm/xe/oa/uapi: Make bit masks unsigned

The infrastructure to query GuC firmware version is already in place. It
is extended with a new micro-controller type to query the HuC firmware
version. It can be used from user space to know if HuC is running.

Cc: John Harrison <John.C.Harrison@Intel.com>
Cc: Francois Dugast <francois.dugast@intel.com>
Cc: Lucas De Marchi <lucas.demarchi@intel.com>
Signed-off-by: Francois Dugast <francois.dugast@intel.com>
Signed-off-by: José Roberto de Souza <jose.souza@intel.com>
Reviewed-by: Rodrigo Vivi <rodrigo.vivi@intel.com>
Reviewed-by: John Harrison <John.C.Harrison@Intel.com>
Link: https://patchwork.freedesktop.org/patch/msgid/20240208183539.185095-2-jose.souza@intel.com
