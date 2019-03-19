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
//! \file     mos_os_cp_interface_specific.h
//! \brief    OS specific implement for CP related functions
//!

#ifndef __MOS_OS_CP_INTERFACE_SPECIFIC_H__
#define __MOS_OS_CP_INTERFACE_SPECIFIC_H__

#include "mos_defs.h"
#include "mos_os_hw.h"
#include "mos_util_debug.h"
#include "cplib.h"

class MosCpInterface
{
public:
    MosCpInterface() {}

    virtual ~MosCpInterface() {}

    //!
    //! \brief    Registers CP patch entry for CP Mode
    //! \details  The function registers parameters for CP mode tracking.
    //! \param    [in] pPatchAddress
    //!           Address to patch location in GPU command buffer
    //! \param    [in] bWrite
    //!           is write operation
    //! \param    [in] HwCommandType
    //!           the cmd that the cpCmdProps assocites with
    //! \param    [in] forceDwordOffset
    //!           override of uint32_t offset to Data parameter
    //! \param    [in] plResource
    //!           hAllocation - Allocation handle to notify GMM about possible 
    //!           changes in protection flag for display surface tracking
    //!           Parameter is void to mos_os OS agnostic
    //!           pGmmResourceInfo - GMM resource information to notify GMM about 
    //!           possible changes in protection flag for display surface tracking
    //!           Parameter is void to mos_os OS agnostic
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterPatchForHM(
        uint32_t       *pPatchAddress,
        uint32_t       bWrite,
        MOS_HW_COMMAND HwCommandType,
        uint32_t       forceDwordOffset,
        void           *plResource,
        void           *pPatchLocationList);

    //!
    //! \brief    UMD level patching
    //! \details  The function performs command buffer patching
    //! \param    virt
    //!           [in] virtual address to be patched
    //! \param    pvCurrentPatch
    //!           [in] pointer to current patch address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS PermeatePatchForHM(
        void        *virt,
        void        *pvCurrentPatch,
        void        *resource);

    //!
    //! \brief    Determines if this UMD Context is running with CP enabled
    //! \return   bool
    //!           true if CP enabled, false otherwise
    //!
    virtual bool IsCpEnabled();

    //!
    //! \brief    transist UMD Context running with/without CP enabled
    //! \param    [in] isCpInUse
    //!           
    //! \return   void
    //!
    virtual void SetCpEnabled(bool bIsCpEnabled);

    //!
    //! \brief    Determines if this UMD Context is running in CP Mode
    //! \return   bool
    //!           true if CP Mode, false otherwise
    //!
    virtual bool IsHMEnabled();

    //!
    //! \brief    Determines if this UMD Context is running in Isolated Decode Mode
    //! \return   bool
    //!           true if Isolated Decode Mode, false otherwise
    //!
    virtual bool IsIDMEnabled();

    //!
    //! \brief    Determines if this UMD Context is running in Stout Mode
    //! \return   bool
    //!           true if Stout Mode, false otherwise
    //!
    virtual bool IsSMEnabled();

    //!
    //! \brief    Determines if tear down happened
    //! \return   bool
    //!           true if tear down happened, false otherwise
    //!
    virtual bool IsTearDownHappen();

    virtual MOS_STATUS SetResourceEncryption(
        void        *pResource,
        bool        bEncryption);

    virtual MOS_STATUS PrepareResources(
        void        *source[],
        uint32_t    sourceCount,
        void        *target[],
        uint32_t    targetCount);

    //!
    //! \brief    Determines whether or not render has access to content.
    //! \details  Render does not have access to content when Stout
    //!           or Isolated Decode is in use.
    //! \return   bool
    //!           True if render does not have access, false otherwise
    //!
    virtual bool RenderBlockedFromCp();

    //!
    //! \brief    Used by video processor to request the cached version of transcrypted and authenticated kernels
    //! \param    [out] **ppTKs
    //!           Cached version of transcrypted kernels
    //! \param    [out] *pTKsSize
    //!           Sized in bytes of the cached transcrypted kernels
    //! \param    [out] *pTKsUpdateCnt
    //!           The update count of the cached transcrypted kernels.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetTK(
        uint32_t                    **ppTKs,
        uint32_t                    *pTKsSize,
        uint32_t                    *pTKsUpdateCnt);

    //!
    //! \brief    Read Counter Nounce Register
    //! \param    [in] readCtr0
    //!           Read counter 0 register
    //! \param    [out] pCounter
    //!           Pointer to hw counter
    //!
    MOS_STATUS ReadCtrNounceRegister(bool readCtr0, uint32_t *pCounter);
};

//!
//! \brief    Create MosCpInterface Object according CPLIB loading status
//!           Must use Delete_MosCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \param    [in] pvOsInterface
//!           void*
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
MosCpInterface* Create_MosCpInterface(void* pvOsInterface);

//!
//! \brief    Delete the MhwCpInterface Object according CPLIB loading status
//!
//! \param    [in] pMosCpInterface 
//!           MosCpInterface
//!
void Delete_MosCpInterface(MosCpInterface* pMosCpInterface);

#endif  // __MOS_OS_CP_SPECIFIC_H__
