/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file     codechal_memdecomp.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details 
//!

#ifndef __CODECHAL_MEDIAMEMCOMP_H__
#define __CODECHAL_MEDIAMEMCOMP_H__

#include "mhw_render_legacy.h"
#include "mos_os.h"
#include "mediamemdecomp.h"

//!
//! \class MediaMemDecompState
//! \brief Media memory decompression state. This class defines the member fields
//!        functions etc used by memory decompression.
//!
class MediaMemDecompState : public MediaMemDecompBaseState
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaMemDecompState();

    //!
    //! \brief    Copy constructor
    //!
    MediaMemDecompState(const MediaMemDecompState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaMemDecompState& operator=(const MediaMemDecompState&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaMemDecompState();

    //!
    //! \brief    Media memory decompression
    //! \details  Entry point to decompress media memory
    //! \param    targetResource
    //!           [in] The surface will be decompressed
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MemoryDecompress(
        PMOS_RESOURCE targetResource);

    //!
    //! \brief    Initialize memory decompress state
    //! \details  Initialize memory decompress state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(
        PMOS_INTERFACE                  osInterface,
        MhwCpInterface                  *cpInterface,
        MhwMiInterface                  *miInterface,
        MhwRenderInterface              *renderInterface);

    virtual PMOS_INTERFACE GetDecompStateMosInterface()
    {
        return m_osInterface;
    }

protected:

    //!
    //! \enum DecompKernelStateIdx
    //! \brief Decompress kernel state index
    //!
    enum DecompKernelStateIdx
    {
        decompKernelStatePa = 0,
        decompKernelStatePl2,
        decompKernelStateMax
    };

    //!
    //! \enum CopyBindingTableOffset
    //! \brief Decompress copy kernel binding table offset
    //!
    enum CopyBindingTableOffset
    {
        copySurfaceSrcY      = 0,
        copySurfaceSrcU      = 1,
        copySurfaceSrcV      = 2,
        copySurfaceDstY      = 3,
        copySurfaceDstU      = 4,
        copySurfaceDstV      = 5,
        copySurfaceNum       = 6
    };

    //!
    //! \brief    Initialize kernel state
    //! \details  Initialize kernel state
    //! \param    kernelStateIdx
    //!           [in] Kernel state index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState(uint32_t kernelStateIdx);

    //!
    //! \brief    Gets a kernel information for a specific unique kernel identifier
    //! \details  Gets a kernel information for a specific unique kernel identifier 
    //!           from the combined kernel
    //! \param    kernelBase
    //!           [in] The combined kernel
    //! \param    krnUniId
    //!           [in] The unique kernel identifier in the combined kernel
    //! \param    kernelBinary
    //!           [in,out] The binary of the kernel specified by krnUniId
    //! \param    kernelSize
    //!           [in,out] The size of the kernel specified by krnUniId
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetKernelBinaryAndSize(
        uint8_t  *kernelBase,
        uint32_t  krnUniId,
        uint8_t  **kernelBinary,
        uint32_t *kernelSize);

    //!
    //! \brief    Sets up the CURBE data for MMC and loads it into the DSH
    //! \details  Calculates the kernel binary location and size and saves this information to either the
    //!           state heap interface or the provided kernel state
    //! \param    kernelStateIdx
    //!           [in] The type of copy kernel (PA or PL2)
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMediaObjectCopyCurbe(
        DecompKernelStateIdx kernelStateIdx);

    //!
    //! \brief    Set copy kernel states parameters
    //! \details  Set copy kernel states parameters for MMC
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetKernelStateParams();

    //!
    //! \brief    Get resource information
    //! \details  Get resource information for the specifc surface
    //! \param    surface
    //!           [in] Surface pointer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetResourceInfo(PMOS_SURFACE surface);

    //!
    //! \brief    Get the surface width in bytes
    //! \details  Get the suface width in bytes
    //! \param    surface
    //!           [in] Surface pointer
    //! \return   uint32_t
    //!           Output the surface width
    //!
    uint32_t GetSurfaceWidthInBytes(PMOS_SURFACE surface);

    //!
    //! \brief    Updates GPU Sync Tag (H/W Tag) using MI_STORE_DATA_IMM command.
    //! \details  Updates GPU Sync Tag (H/W Tag) using MI_STORE_DATA_IMM command.
    //! \param    cmdBuffer
    //!           [in] Resource to decompress
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WriteSyncTagToResourceCmd(
        PMOS_COMMAND_BUFFER   cmdBuffer);

    static constexpr uint32_t    m_numMemDecompSyncTags  = 8;           //!< Number of memory decompress sync tags

    PMOS_INTERFACE               m_osInterface           = nullptr;     //!< Pointer to Os Inteface
    MhwCpInterface               *m_cpInterface          = nullptr;     //!< Pointer to Cp Interface
    MhwMiInterface              *m_miInterface           = nullptr;     //!< Pointer to MhwMiInterface
    MhwRenderInterface           *m_renderInterface      = nullptr;     //!< Pointer to MhwRenderInterface
    PMHW_STATE_HEAP_INTERFACE    m_stateHeapInterface    = nullptr;     //!< Pointer to PMHW_STATE_HEAP_INTERFACE
    uint8_t                      *m_kernelBase            = nullptr;    //!< Pointer to kernel base address
    MHW_STATE_HEAP_SETTINGS      m_stateHeapSettings;                   //!< State heap setting
    uint32_t                     m_krnUniId[decompKernelStateMax];      //!< Kernel unique ID
    uint8_t                      *m_kernelBinary[decompKernelStateMax]; //!< Kernel binary
    uint32_t                     m_kernelSize[decompKernelStateMax];    //!< Kernel size
    MHW_KERNEL_STATE             m_kernelStates[decompKernelStateMax];  //!< Kernel state
    MOS_GPU_CONTEXT              m_renderContext;                       //!< Render GPU context
    bool                         m_renderContextUsesNullHw = false;     //!< Indicate if render context use null hw or not
    bool                         m_disableDecodeSyncLock   = false;     //!< Indicate if decode sync lock disabled or not
    bool                         m_disableLockForTranscode = false;     //!< Indicate if lock is disabled for transcode or not
    uint32_t                     *m_cmdBufIdGlobal = nullptr;                     //!< Pointer to command buffer global Id
    MOS_RESOURCE                 m_resCmdBufIdGlobal;                   //!< Resource for command buffer global Id
    uint32_t                     m_currCmdBufId;                        //!< Current command buffer Id
};

#endif  //__CODECHAL_MEDIAMEMCOMP_H__
