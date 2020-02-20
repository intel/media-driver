/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_context_next.h
//! \brief    Container for parameters shared across different GPU contextNexts of the same device instance
//!

#ifndef __MOS_CONTEXT_NEXT_H__
#define __MOS_CONTEXT_NEXT_H__

#include "mos_os_next.h"
#include "mos_cmdbufmgr_next.h" 
#include "mos_gpucontextmgr_next.h"

class OsContextNext
{
protected:
    //!
    //! \brief Constructor for the OsContextNext
    //!
    OsContextNext(){};

public:
    //!
    //! \brief Destructor for the OsContextNext
    //!
    virtual ~OsContextNext(){};

    //!
    //! \brief  Initialzie the OS ContextNext Object
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    virtual MOS_STATUS Init(DDI_DEVICE_CONTEXT osDriverContextNext) = 0;

private:
    //!
    //! \brief  Destory the OS ContextNext Object, internal function, called by cleanup
    //!
    virtual void Destroy() = 0;

public:
    //!
    //! \brief  Static entrypoint, get the OS ContextNext Object
    //! \return the os specific object for OS contextNext
    //!
    static class OsContextNext *GetOsContextObject();

    //!
    //! \brief  Clean up the smartptr
    //!
    void CleanUp();

    //!
    //! \brief  Get the skuTable
    //! \return The active SKU Table
    //!
    MEDIA_FEATURE_TABLE *GetSkuTable() { return &m_skuTable; };

    //!
    //! \brief  Get the waTable
    //! \return The active WA table
    //!
    MEDIA_WA_TABLE *GetWaTable() { return &m_waTable; };

    //!
    //! \brief  Get the platform information string
    //! \return value of m_platformInfo
    //!
    PLATFORM *GetPlatformInfo() { return &m_platformInfo; };

    //!
    //! \brief  Get the gtSystemInfo string
    //! \return value of m_gtSystemInfo
    //!
    MEDIA_SYSTEM_INFO *GetGtSysInfo() { return &m_gtSystemInfo; };

    //!
    //! \brief  Get MemDecompState
    //! \return pointer to m_mediaMemDecompState
    //!
    void*  GetMemDecompState() { return m_mediaMemDecompState; };

    //!
    //! \brief  Check the platform is Atom or not
    //! \return true on Atom platform, false on none Atom platform
    //!
    bool IsAtomSoc() { return m_isAtomSOC; };

    //!
    //! \brief  Set the OS ContextNext valid flag
    //! \param   [in] isOsContextNextValid
    //!          Flag to indicate if the os contextNext is valid. 
    //!
    void SetOsContextValid(bool isOsContextValid) { m_osContextValid = isOsContextValid; };

    //!
    //! \brief  Return the OS ContextNext valid flag
    //! \return true if the OS contextNext is valid, false if not valid
    //!
    bool GetOsContextValid() { return m_osContextValid; };

    //!
    //! \brief  Set slice count to shared memory and KMD
    //! \param  [in,out] pSliceCount
    //!         pointer to the slice count. Input the slice count for current
    //!         contextNext, output the ruling slice count shared by all contextNexts.
    //!
    virtual void SetSliceCount(uint32_t *pSliceCount) { MOS_UNUSED(pSliceCount); };

    //!
    //! \brief  Get GPU context manager of the device
    //! \return GPU context manager
    //!
    GpuContextMgrNext *GetGpuContextMgr() { return m_gpuContextMgr; }

    //!
    //! \brief  Get Cmd buffer manager of the device
    //! \return Cmd buffer manager
    //!
    CmdBufMgrNext *GetCmdBufferMgr() { return m_cmdBufMgr; }

    //!
    //! \brief  Get GMM client context of the device
    //! \return Cmd buffer manager
    //!
    GMM_CLIENT_CONTEXT *GetGmmClientContext() { return m_gmmClientContext; }

    static const uint32_t m_cmdBufAlignment = 16;   //!> Cmd buffer alignment

protected:
    GpuContextMgrNext              *m_gpuContextMgr = nullptr;   //!> GPU context manager of the device
    CmdBufMgrNext                  *m_cmdBufMgr     = nullptr;   //!> Cmd buffer manager of the device
    GMM_CLIENT_CONTEXT             *m_gmmClientContext = nullptr; //!> GMM client context of the device

    //! \brief  Platform string including product family, chipset family, etc
    PLATFORM                        m_platformInfo = {};

    //! \brief  sku table
    MEDIA_FEATURE_TABLE             m_skuTable = {};

     //! \brief  wa table
    MEDIA_WA_TABLE                  m_waTable = {};

     //! \brief  GT system information, like EU counter, thread count, etc.
    MEDIA_SYSTEM_INFO               m_gtSystemInfo = {};

    //! \brief  Whether the processor is Atom
    bool                            m_isAtomSOC = false;

    //! \brief  Internal media state for memory decompression
    void*                           m_mediaMemDecompState = nullptr;

    //! \brief  Flag to mark whether the os context is valid
    bool                            m_osContextValid =  false;

    //! \brief  Whether the current driver is of 64 bit
    bool                            m_64bit = false;

    //! \brief  Whether or not need deallocation on exit
    bool                            m_deallocateOnExit = false;

    //! \brief  need KMD to track the media frame or not
    bool                            m_enableKmdMediaFrameTracking = false;

    //! \brief  need KMD to assist the command buffer parsing
    bool                            m_noParsingAssistanceInKmd = false;

    //! \brief  how many bytes of the Nal Unit need be included
    uint32_t                        m_numNalUnitBytesIncluded = 0;

    //! \brief   For GPU Reset Statistics, rest counter
    uint32_t                        m_gpuResetCount = 0;

    //! \brief   For GPU Reset Statistics, the active batch
    uint32_t                        m_gpuActiveBatch = 0;

    //! \brief   For GPU Reset Statistics, the pending batch
    uint32_t                        m_gpuPendingBatch = 0;

    //! \brief   For Resource addressing, whether patch list mode is active
    bool                            m_usesPatchList = false;

    //! \brief   For Resource addressing, whether GPU address mode is active
    bool                            m_usesGfxAddress = false;

    //! \brief   For limited GPU VA resource can not be mapped during creation
    bool                            m_mapOnCreate = false;

    //! \brief  check whether use inline codec status update or seperate BB
    bool                            m_inlineCodecStatusUpdate = false;

    //! \brief   Component info
    MOS_COMPONENT                   m_component = COMPONENT_UNKNOWN;

    //! \brief   Flag to indicate if HAS is enabled
    bool                            m_simIsActive = false;

    //! \brief   Flag to indicate if implicit Tile is needed
    bool                            m_implicitTileNeeded = false;
};
#endif // #ifndef __MOS_CONTEXTNext_NEXT_H__
