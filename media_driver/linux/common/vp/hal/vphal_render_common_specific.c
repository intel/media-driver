/*
* Copyright (c) 2009-2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     vphal_render_common_specific.c
//! \brief    The header file of common struct/macro definitions shared by low level renderers
//! \details  Common struct/macro for different renderers, e.g. DNDI or Comp
//!
#include "vphal.h"
#include "mos_os.h"

//!
//! \brief    Determine if the Batch Buffer End is needed to add in the end
//! \details  Detect platform OS and return the flag whether the Batch Buffer End is needed to add in the end
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \return   bool
//!           The flag of adding Batch Buffer End
//!
bool VpHal_RndrCommonIsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface)
{
#ifdef WDDM_LINUX
    bool needed = true;
#else
    bool needed = false;
#endif

    VPHAL_RENDER_ASSERT(pOsInterface);

    if (pOsInterface->osCpInterface &&
            pOsInterface->osCpInterface->IsHMEnabled())
    {
        needed = true;
    }

    return needed;
}
