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

commit 41a97c4a12947c2786a1680d6839bb72d1c57cec (HEAD -> drm-xe-next, origin/drm-xe-next)

drm/xe/pxp/uapi: Add API to mark a BO as using PXP

The driver needs to know if a BO is encrypted with PXP to enable the
display decryption at flip time.
Furthermore, we want to keep track of the status of the encryption and
reject any operation that involves a BO that is encrypted using an old
key. There are two points in time where such checks can kick in:

1 - at VM bind time, all operations except for unmapping will be
    rejected if the key used to encrypt the BO is no longer valid. This
    check is opt-in via a new VM_BIND flag, to avoid a scenario where a
    malicious app purposely shares an invalid BO with a non-PXP aware
    app (such as a compositor). If the VM_BIND was failed, the
    compositor would be unable to display anything at all. Allowing the
    bind to go through means that output still works, it just displays
    garbage data within the bounds of the illegal BO.

2 - at job submission time, if the queue is marked as using PXP, all
    objects bound to the VM will be checked and the submission will be
    rejected if any of them was encrypted with a key that is no longer
    valid.

Note that there is no risk of leaking the encrypted data if a user does
not opt-in to those checks; the only consequence is that the user will
not realize that the encryption key is changed and that the data is no
longer valid.

v2: Better commnnts and descriptions (John), rebase

v3: Properly return the result of key_assign up the stack, do not use
xe_bo in display headers (Jani)

v4: improve key_instance variable documentation (John)

Signed-off-by: default avatarDaniele Ceraolo Spurio <daniele.ceraolospurio@intel.com>
Cc: Matthew Brost <matthew.brost@intel.com>
Cc: Thomas Hellström <thomas.hellstrom@linux.intel.com>
Cc: John Harrison <John.C.Harrison@Intel.com>
Cc: Jani Nikula <jani.nikula@intel.com>
Reviewed-by: default avatarJohn Harrison <John.C.Harrison@Intel.com>
Link: https://patchwork.freedesktop.org/patch/msgid/20250129174140.948829-11-daniele.ceraolospurio@intel.com

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
