/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     media_blt_copy_next.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_NEXT_H__
#define __MEDIA_BLT_COPY_NEXT_H__

#include "media_interfaces_mhw_next.h"
#include "mhw_blt.h"
#include "mhw_mi.h"
#include "mhw_cp_interface.h"
#include "mos_os.h"
#include "media_copy.h"
#include "media_copy_common.h"

class BltStateNext
{
public:
    //!
    //! \brief    BltStateNext constructor
    //! \details  Initialize the BltStateNext members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltStateNext(PMOS_INTERFACE     osInterface);
    BltStateNext(PMOS_INTERFACE    osInterface, MhwInterfacesNext* mhwInterfaces);

    virtual ~BltStateNext();

    //!
    //! \brief    BltStateNext initialize
    //! \details  Initialize the BltStateNext, create BLT context.
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
        PMOS_SURFACE dst);

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

    //!
    //! \brief    Setup blt copy parameters
    //! \details  Setup blt copy parameters for BLT Engine
    //! \param    mhwParams
    //!           [in/out] Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    inputSurface
    //!           [in] Pointer to input surface
    //! \param    outputSurface
    //!           [in] Pointer to output surface
    //! \param    planeIndex
    //!           [in] Pointer to YUV(RGB) plane index
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupBltCopyParam(
      PMHW_FAST_COPY_BLT_PARAM mhwParams,
      PMOS_RESOURCE            inputSurface,
      PMOS_RESOURCE            outputSurface,
      int                      planeIndex);

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
    //! \brief    Get Block copy color depth.
    //! \details  get different format's color depth.
    //! \param    Gmm format and bits per Pixel
    //!           [in] Gmm format, Bits per Pixel;
    //! \return   color depth
    //!           Return color depth
    //!
    uint32_t GetBlkCopyColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            Pixel);

    //!
    //! \brief    Get plane's byte per texel
    //! \details  Get plane's byte per texel
    //! \param    MOS_FORMAT format
    //!           [in] GMM resource format
    //! \return   int
    //!           return the scaling ratio;
    //!
    int GetBytesPerTexelScaling(MOS_FORMAT format);

    //!
    //! \brief    Get plane number
    //! \details  Get plane number
    //! \param    MOS_FORMAT format
    //!           [in] GMM resource format
    //! \return   int
    //!           return the plane number
    //!
    int GetPlaneNum(MOS_FORMAT format);


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
    MOS_STATUS SetupCtrlSurfCopyBltParam(
        PMHW_CTRL_SURF_COPY_BLT_PARAM pMhwBltParams,
        PMOS_SURFACE                  inputSurface,
        PMOS_SURFACE                  outputSurface,
        uint32_t                      flag);

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

    //!
    //! \brief    SetPrologParamsforCmdbuffer
    //! \details  Set PrologParams for Cmdbuffer
    //! \param    PMOS_COMMAND_BUFFER
    //!           [in] Pointer to PMOS_COMMAND_BUFFER
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetPrologParamsforCmdbuffer(PMOS_COMMAND_BUFFER cmdBuffer);

     //!
    //! \brief    Set BCS_SWCTR cmd
    //! \details  Set BCS_SWCTR for Cmdbuffer
    //! \param    PMOS_COMMAND_BUFFER
    //!           [in] Pointer to PMOS_COMMAND_BUFFER
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetBCSSWCTR(MOS_COMMAND_BUFFER *cmdBuffer);

 public:
    PMOS_INTERFACE     m_osInterface      = nullptr;
    MhwInterfacesNext *m_mhwInterfaces    = nullptr;
    MhwCpInterface    *m_cpInterface      = nullptr;

    std::shared_ptr<mhw::mi::Itf>   m_miItf  = nullptr;
    std::shared_ptr<mhw::blt::Itf>  m_bltItf = nullptr;
    
protected:
    bool         initialized = false;
    bool         allocated   = false;
    PMOS_SURFACE tempSurface = nullptr;
    PMOS_SURFACE tempAuxSurface = nullptr;
    uint32_t     surfaceSize = 0;
    uint32_t     auxSize     = 0;
    void*        pMainSurface = nullptr;
    void*        pAuxSurface  = nullptr;

    MEDIA_CLASS_DEFINE_END(BltStateNext)
};

#endif // __MEDIA_BLT_COPY_H__
