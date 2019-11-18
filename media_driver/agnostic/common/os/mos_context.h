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
//! \file     mos_context.h
//! \brief    Container for parameters shared across different GPU contexts of the same device instance
//!

#ifndef __MOS_CONTEXT_H__
#define __MOS_CONTEXT_H__

#include "mos_os.h"

class OsContext
{
public:
    enum MOS_S3D_CHANNEL
    {
        MOS_S3D_NONE,        //!< Not in 3D mode
        MOS_S3D_LEFT,        //!< Left eye channel
        MOS_S3D_RIGHT        //!< Right eye channel
    };

    struct MOS_PLANE_OFFSET
    {
        int    iSurfaceOffset;              //!< Plane surface offset
        int    iXOffset;                    //!< Tile X offset
        int    iYOffset;                    //!< Tile Y offset
        int    iLockSurfaceOffset;          //!< Offset in Locked Surface
    };

protected:
    //!
    //! \brief Constructor for the OsContext
    //!
    OsContext(){};

public:
    //!
    //! \brief Destructor for the OsContext
    //!
    virtual ~OsContext(){};

    //!
    //! \brief  Initialzie the OS Context Object
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    virtual MOS_STATUS Init(MOS_CONTEXT* osDriverContext) = 0;

private:
    //!
    //! \brief  Destory the OS Context Object, internal function, called by cleanup
    //!
    virtual void Destroy() = 0;

public:
    //!
    //! \brief  Static entrypoint, get the OS Context Object
    //! \return the os specific object for OS context
    //!
    static class OsContext* GetOsContextObject();

    //!
    //! \brief  Clean up the smartptr
    //!
    void CleanUp();

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    //!
    //! \brief    Unified dump command buffer initialization
    //! \details  check if dump command buffer was enabled and create the output directory
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS CommandBufferDumpInit();
#endif

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
    PLATFORM GetPlatformInfo() { return m_platformInfo; };

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
    //! \brief  Set the OS Context valid flag
    //! \param   [in] isOsContextValid
    //!          Flag to indicate if the os context is valid. 
    //!
    void SetOsContextValid(bool isOsContextValid) { m_osContextValid = isOsContextValid; };

    //!
    //! \brief  Return the OS Context valid flag
    //! \return true if the OS context is valid, false if not valid
    //!
    bool GetOsContextValid() { return m_osContextValid; };

    //!
    //! \brief  Set slice count to shared memory and KMD
    //! \param  [in,out] pSliceCount
    //!         pointer to the slice count. Input the slice count for current
    //!         context, output the ruling slice count shared by all contexts.
    //!
    virtual void SetSliceCount(uint32_t *pSliceCount) { MOS_UNUSED(pSliceCount); };

    //! \brief   Flag to indicate if implicit tile setting is needed
    bool m_implicitTileNeeded = false;

protected:
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

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    //! \brief   Command buffer dump.
    //! \brief   Flag to indicate if Dump command buffer is enabled
    bool                            m_dumpCommandBuffer = false;

    //! \brief   Indicates that the command buffer should be dumped to a file
    bool                            m_dumpCommandBufferToFile = false;

    //! \brief   Indicates that the command buffer should be dumped via MOS normal messages
    bool                            m_dumpCommandBufferAsMessages = false;

    //! \brief   Platform name - maximum 4 bytes length
    char                            m_platformName[MOS_COMMAND_BUFFER_PLATFORM_LEN] = {0};
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED
};
#endif // #ifndef __MOS_CONTEXT_H__
