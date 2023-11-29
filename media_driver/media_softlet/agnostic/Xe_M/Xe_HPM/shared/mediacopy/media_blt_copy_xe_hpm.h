/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_blt_copy_xe_hpm.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_XE_HPM_H__
#define __MEDIA_BLT_COPY_XE_HPM_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "mos_resource_defs.h"
#include "media_blt_copy.h"
#include "mhw_blt_xe_hp_base.h"
class MhwInterfaces;

class BltState_Xe_Hpm: virtual public BltState
{
public:
    //!
    //! \brief    BltState_Xe_Hpm constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltState_Xe_Hpm(PMOS_INTERFACE     osInterface);
    BltState_Xe_Hpm(PMOS_INTERFACE    osInterface, MhwInterfaces *mhwInterfaces);

    virtual ~BltState_Xe_Hpm();

    //!
    //! \brief    BltState_Xe_Hpm initialize
    //! \details  Initialize the BltState_Xe_Hpm, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    CopyProtectResource
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [out] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyProtectResource(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

    //!
    //! \brief    MapTileType
    //! \param    flags
    //!           [in] GMM resource flags
    //! \param    type
    //!           [in] GMM tile type
    //! \return   MOS_TILE_TYPE
    //!           Return MOS tile type for given GMM resource and tile flags
    //!
    MOS_TILE_TYPE MapTileType(GMM_RESOURCE_FLAG flags, GMM_TILE_TYPE type);

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
    //! \brief    CopyProtectSurface
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination buffer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyProtectSurface(
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

    //!
    //! \brief    Block copy buffer
    //! \details  BLT engine will copy source buffer to destination buffer
    //! \param    pBltStateParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS BlockCopyBuffer(
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
MEDIA_CLASS_DEFINE_END(BltState_Xe_Hpm)
};

#endif // __MEDIA_BLT_COPY_XE_HPM_H__