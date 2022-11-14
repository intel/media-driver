/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     vphal_g9.h
//! \brief    vphal interface clarification for GEN9
//! \details  vphal interface clarification for GEN9 inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_G9_H__
#define __VPHAL_G9_H__

#include "vphal.h"

//!
//! Class VphalStateG9
//! \brief VPHAL class definition for GEN9
//!
class VphalStateG9 : public VphalState
{
public:
    //!
    //! \brief    VphalState Constructor
    //! \details  Creates instance of VphalState
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be NULL
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] peStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalStateG9(
        PMOS_INTERFACE          pOsInterface,
        MOS_STATUS              *peStatus) :
        VphalState(pOsInterface, peStatus)
    {
    }

    //!
    //! \brief    VphalState Destuctor
    //! \details  Destroys VPHAL and all internal states and objects
    //!
    virtual ~VphalStateG9()
    {
    }

protected:
    //!
    //! \brief    Create instance of VphalRenderer
    //! \details  Create instance of VphalRenderer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CreateRenderer();
};

#endif  // __VPHAL_G9_H__
