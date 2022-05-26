/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_blt_copy_xe_xpm_plus.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_XE_XPM_PLUS_H__
#define __MEDIA_BLT_COPY_XE_XPM_PLUS_H__

#include "media_blt_copy_xe_xpm_base.h"
#include "mhw_blt_xe_hpc.h"

class BltStateXe_Xpm_Plus: virtual public BltState
{
public:
    //!
    //! \brief    BltStateXe_Xpm_Plus constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltStateXe_Xpm_Plus(PMOS_INTERFACE     osInterface);
    BltStateXe_Xpm_Plus(PMOS_INTERFACE    osInterface, MhwInterfaces *mhwInterfaces);

    virtual ~BltStateXe_Xpm_Plus();

    //!
    //! \brief    BltStateXe_Xpm_Plus initialize
    //! \details  Initialize the BltStateXe_Xpm_Plus, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface 
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_SURFACE src,
        PMOS_SURFACE dst)
    {
        return BltState::CopyMainSurface(src, dst);
    }

    virtual MOS_STATUS CopyMainSurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

    //!
    //! \brief    Get control surface
    //! \details  BLT engine will copy aux data of source surface to destination
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination buffer which is created for aux data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetCCS(
        PMOS_SURFACE src,
        PMOS_SURFACE dst);

    //!
    //! \brief    Put control surface
    //! \details  BLT engine will copy aux data in source buffer to CCS of destination surface
    //! \param    src
    //!           [in] Pointer to source buffer which store aux data
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS PutCCS(
        PMOS_SURFACE src,
        PMOS_SURFACE dst);

    //!
    //! \brief    dump surface
    //! \details  dump surface to get main surface and aux data
    //! \param    pSrcSurface
    //!           [in] Pointer to source surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise error code
    //!
    virtual MOS_STATUS LockSurface(
        PMOS_SURFACE pSurface);

    //!
    //! \brief    free surface
    //! \details  Free resource created by lockSurface, must be called once call LockSurface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise error code
    //!
    virtual MOS_STATUS UnLockSurface();

    //!
    //! \brief    Write compressed surface
    //! \details  Write compressed surface data from system memory to GPU memory
    //! \param    pSysMemory
    //!           [in] Pointer to system memory
    //! \param    dataSize
    //!           [in] data size, including main surface data and aux data
    //! \param    pSurface
    //!           [in] Pointer to the destination surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise error code
    //!
    virtual MOS_STATUS WriteCompressedSurface(
        void*        pSysMemory,
        uint32_t     dataSize,
        PMOS_SURFACE pSurface);

    //!
    //! \brief    Get main surface size
    //! \details  Get the size of main surface
    //! \return   uint32_t
    //!           Retrun data size
    //!
    uint32_t GetMainSurfaceSize()
    {
        return surfaceSize;
    }

    //!
    //! \brief    Get aux data size
    //! \details  Get the size of aux
    //! \return   uint32_t
    //!           Retrun data size
    //!
    uint32_t GetAuxSize()
    {
        return auxSize;
    }

    //!
    //! \brief    Get main surface data
    //! \details  Get the data of main surface
    //! \return   void*
    //!           Retrun the pointer to main surface data
    //!
    void* GetMainSurfaceData()
    {
        return pMainSurface;
    }

    //!
    //! \brief    Get aux data
    //! \details  Get the data of aux
    //! \return   void*
    //!           Retrun the pointer to aux data
    //!
    void* GetAuxData()
    {
        return pAuxSurface;
    }

    //!
    //! \brief    Get color depth.
    //! \details  get different format's color depth.
    //! \param    surface
    //!           [in] input or output surface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    uint32_t GetColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            BytesPerTexel);

protected:
    //!
    //! \brief    Allocate resource
    //! \details  Allocate internel resource
    //! \param    pSrcSurface
    //!           [in] Pointer to source surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise error code
    //!
    virtual MOS_STATUS AllocateResource(
        PMOS_SURFACE pSurface);

    //!
    //! \brief    Free resource
    //! \details  Free internel resource, must be called once call AllocateResource
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise error code
    //!
    virtual MOS_STATUS FreeResource();

    //!
    //! \brief    Setup control surface copy parameters
    //! \details  Setup control surface copy parameters for BLT Engine
    //! \param    mhwParams
    //!           [in/out] Pointer to MHW_CTRL_SURF_COPY_BLT_PARAM
    //! \param    inputSurface
    //!           [in] Pointer to input surface
    //! \param    outputSurface
    //!           [in] Pointer to output surface
    //! \param    flag
    //!           [in] Flag for read/write CCS
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupCtrlSurfCopyBltParam(
        PMHW_CTRL_SURF_COPY_BLT_PARAM pMhwBltParams,
        PMOS_SURFACE                  inputSurface,
        PMOS_SURFACE                  outputSurface,
        uint32_t                      flag);

    //!
    //! \brief    Submit command
    //! \details  Submit BLT command
    //! \param    pBltStateParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD(
        PBLT_STATE_PARAM pBltStateParam);

private:
    bool         initialized = false;
    bool         allocated   = false;
    PMOS_SURFACE tempSurface = nullptr;
    PMOS_SURFACE tempAuxSurface = nullptr;
    uint32_t     surfaceSize = 0;
    uint32_t     auxSize     = 0;
    void*        pMainSurface = nullptr;
    void*        pAuxSurface  = nullptr;
MEDIA_CLASS_DEFINE_END(BltStateXe_Xpm_Plus)
};

#endif // __MEDIA_BLT_COPY_XE_XPM_PLUS_H__

