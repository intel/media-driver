/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file      codec_hw_next.h
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across CODECHAL components. 
//!

#ifndef __CODEC_HW_NEXT_H__
#define __CODEC_HW_NEXT_H__

#include "codechal_common.h"
#include "media_interfaces_mhw_next.h"

#include "mhw_mi_itf.h"
#include "media_sfc_interface.h"
#include "renderhal.h"
#include "codechal_setting.h"
#include "mhw_vdbox_vvcp_itf.h"

#define HUC_DMEM_OFFSET_RTOS_GEMS 0x2000
//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define CODEC_HW_ASSERT(_expr)                                                          \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _expr)

#define CODEC_HW_ASSERTMESSAGE(_message, ...)                                           \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_NORMALMESSAGE(_message, ...)                                           \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_VERBOSEMESSAGE(_message, ...)                                          \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODEC_HW_FUNCTION_ENTER                                                         \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW)

#define CODEC_HW_CHK_STATUS(_stmt)                                                      \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODEC_HW_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODEC_HW_CHK_STATUS_MESSAGE(_stmt, _message, ...)                               \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt, _message, ##__VA_ARGS__)

#define CODEC_HW_CHK_NULL(_ptr)                                                         \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_NULL_NO_STATUS(_ptr)                                               \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODEC_HW_CHK_COND_RETURN(_expr, _message, ...)                                  \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_SURFACE_PITCH_ALIGNMENT 128

class MediaScalability;
class MediaContext;
class MediaCopyBaseState;

//!  Codec hw next interface
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal
*/
class CodechalHwInterfaceNext
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceNext(
        PMOS_INTERFACE     osInterface,
        CODECHAL_FUNCTION  codecFunction,
        MhwInterfacesNext  *mhwInterfacesNext,
        bool               disableScalability = false);
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceNext(
        PMOS_INTERFACE osInterface,
        bool           disableScalability = false);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceNext();

    static CodechalHwInterfaceNext* Create(
        PMOS_INTERFACE     osInterface,
        CODECHAL_FUNCTION  codecFunction,
        MhwInterfacesNext* mhwInterfacesNext,
        bool               disableScalability);

    //!
    //! \brief    Initialize the codechal hw interface
    //! \details  Initialize the interface before using
    //! 
    //! \param    [in] settings
    //!           Settings for initialization
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(
        CodechalSetting *settings);

    MOS_STATUS GetVdencPictureSecondLevelCommandsSize(
        uint32_t  mode,
        uint32_t *commandsSize);

    //! \brief    Create media copy
    //! \details  Create media copy instance.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \return   MediaCopyBaseState*
    //!           Pointer to MediaCopyBaseState
    //!
    virtual MediaCopyBaseState *CreateMediaCopy(PMOS_INTERFACE mosInterface)
    {
        return nullptr;
    }

    //!
    //! \brief    Get Cacheability Settings
    //! \details  Get Cacheability Settings in codechal hw interface
    //!
    //! \return   [out] MHW_MEMORY_OBJECT_CONTROL_PARAMS*
    //!           Cachebility Settings got.
    //!
    inline MHW_MEMORY_OBJECT_CONTROL_PARAMS *GetCacheabilitySettings()
    {
        return m_cacheabilitySettings;
    }

    //!
    //! \brief    Get platform
    //! \details  Get platform in codechal hw interface
    //!
    //! \return   [out] PLATFORM
    //!           Platform got.
    //!
    inline PLATFORM GetPlatform()
    {
        return m_platform;
    }
    
    //!
    //! \brief    Get Wa table
    //! \details  Get Wa table in codechal hw interface 
    //!
    //! \return   [out] MEDIA_WA_TABLE
    //!           Wa table got.
    //!
    inline MEDIA_WA_TABLE *GetWaTable()
    {
        return m_waTable;
    }

    //!
    //! \brief    Get SFC interface
    //! \details  Get SFC interface in codechal hw interface
    //!
    //! \return   [out] MhwSfcInterface*
    //!           Interface got.
    //!
    inline MhwSfcInterface *GetSfcInterface()
    {
        return m_sfcInterface;
    }

    //!
    //! \brief    Get renderHal interface
    //! \details  Get renderHal interface in codechal hw interface
    //!
    //! \return   [out] RENDERHAL_INTERFACE*
    //!           Interface got.
    //!
    inline RENDERHAL_INTERFACE *GetRenderHalInterface()
    {
        return m_renderHal;
    }

    //!
    //! \brief    Get mfx interface
    //! \details  Get mfx interface in codechal hw interface next
    //!
    //! \return    pointer to new MFX interface
    //!
    inline std::shared_ptr<mhw::vdbox::mfx::Itf> GetMfxInterfaceNext()
    {
        return m_mfxItf;
    }

    //!
    //! \brief    Get avp interface
    //! \details  Get avp interface in codechal hw interface next
    //!
    //! \return    pointer to new AVP interface
    //!
    inline std::shared_ptr<mhw::vdbox::avp::Itf> GetAvpInterfaceNext()
    {
        return m_avpItf;
    }

    //!
    //! \brief    Get vdenc interface
    //! \details  Get vdenc interface in codechal hw interface next
    //!
    //! \return    pointer to new VDENC interface
    //!
    inline std::shared_ptr<mhw::vdbox::vdenc::Itf> GetVdencInterfaceNext()
    {
        return m_vdencItf;
    }
    
    //! \brief    Get huc interface
    //! \details  Get huc interface in codechal hw interface next
    //!
    //! \return    pointer to new HUC interface
    //!
    inline std::shared_ptr<mhw::vdbox::huc::Itf> GetHucInterfaceNext()
    {
        return m_hucItf;
    }

    //!
    //! \brief    Get mi interface
    //! \details  Get mi interface in codechal hw interface next
    //!
    //! \return    pointer to new MI interface
    //!
    inline std::shared_ptr<mhw::mi::Itf> GetMiInterfaceNext()
    {
        return m_miItf;
    }

    //!
    //! \brief    Get hcp interface
    //! \details  Get hcp interface in codechal hw interface next
    //!
    //! \return    pointer to new HCP interface
    //!
    inline std::shared_ptr<mhw::vdbox::hcp::Itf> GetHcpInterfaceNext()
    {
        return m_hcpItf;
    }
    
    //!
    //! \brief    Get render interface
    //! \details  Get render interface in codechal hw interface next
    //!
    //! \return    pointer to new HCP interface
    //!
    inline std::shared_ptr<mhw::render::Itf> GetRenderInterfaceNext()
    {
        return m_renderItf;
    }

    //!
    //! \brief    Get vvcp interface
    //! \details  Get vvcp interface in codechal hw interface next
    //!
    //! \return    pointer to new VVCP interface
    //!
    inline std::shared_ptr<mhw::vdbox::vvcp::Itf> GetVvcpInterfaceNext()
    {
        return m_vvcpItf;
    }

    //!
    //! \brief    Get Os interface
    //! \details  Get Os interface in codechal hw interface
    //!
    //! \return   [out] PMOS_INTERFACE
    //!           Interface got.
    //!
    inline PMOS_INTERFACE GetOsInterface()
    {
        return m_osInterface;
    }

    //!
    //! \brief    Get Sku table
    //! \details  Get Sku table in codechal hw interface
    //!
    //! \return   [out] MEDIA_FEATURE_TABLE *
    //!           Sku table got.
    //!
    inline MEDIA_FEATURE_TABLE *GetSkuTable()
    {
        return m_skuTable;
    }

    //!
    //! \brief    Get cp interface
    //! \details  Get cp interface in codechal hw interface
    //!
    //! \return   [out] MhwCpInterface*
    //!           Interface got.
    //!
    inline MhwCpInterface *GetCpInterface()
    {
        return m_cpInterface;
    }

    inline std::shared_ptr<MediaSfcInterface> GetMediaSfcInterface()
    {
        return m_mediaSfcItf;
    }

    inline void SetMediaSfcInterface(std::shared_ptr<MediaSfcInterface> itf)
    {
        m_mediaSfcItf = itf;
    }

    //!
    //! \brief    Init Cacheability Control Settings
    //! \details  Init Cacheability Control Settings in codechal hw interface 
    //!
    //! \param    [in] codecFunction
    //!           codec function used to judge how to setup cacheability control settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitCacheabilityControlSettings(
        CODECHAL_FUNCTION codecFunction);

    //!
    //! \brief    Get memory object of GMM Cacheability control settings
    //! \details  Internal function to get memory object of GMM Cacheability control settings
    //! 
    //! \param    [in] mosUsage
    //!           Codec usages in mos type
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CachePolicyGetMemoryObject(
        MOS_HW_RESOURCE_DEF mosUsage);

    uint32_t ComposeSurfaceCacheabilityControl(
        uint32_t cacheabiltySettingIdx,
        uint32_t cacheabilityTypeRequested)
    {
        CODEC_HW_FUNCTION_ENTER;

        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheSetting = m_cacheabilitySettings[cacheabiltySettingIdx];

        return (uint32_t)m_cacheabilitySettings[cacheabiltySettingIdx].Value;
    }

    //!
    //! \brief    Calculates the maximum size for AVP picture level commands
    //! \details  Client facing function to calculate the maximum size for AVP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAvpStateCommandSize(
            uint32_t                        mode,
            uint32_t                        *commandsSize,
            uint32_t                        *patchListSize,
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for AVP tile level commands
    //! \details  Client facing function to calculate maximum size for AVP tile level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAvpPrimitiveCommandSize(
            uint32_t                        mode,
            uint32_t                        *commandsSize,
            uint32_t                        *patchListSize);

    //!
    //! \brief    Set Cacheability Settings
    //! \details  Set Cacheability Settings in sub interfaces in codechal hw interface 
    //!
    //! \param    [in] cacheabilitySettings
    //!           cacheability Settings to set into sub mhw intefaces in hw interface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]);

    //!
    //! \brief    Calculates the maximum size for Huc picture level commands
    //! \details  Client facing function to calculate the maximum size for Huc picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucStateCommandSize(
            uint32_t mode,
            uint32_t* commandsSize,
            uint32_t* patchListSize,
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for all picture level commands
    //! \details  Client facing function to calculate maximum size for all slice/MB level commands in mfx pipeline
    //!
    //! \param    [in] mode
    //!           Codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] modeSpecific
    //!           Indicate the long or short format for decoder or single take phase for encoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific);

    //!
    //! \brief    Calculates maximum size for all slice/MB level commands
    //! \details  Client facing function to calculate maximum size for all slice/MB level commands in mfx pipeline
    //!
    //! \param    [in] mode
    //!           Codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] modeSpecific
    //!           Indicate the long or short format for decoder or single take phase for encoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific);

    //!
    //! \brief    Calculates maximum size for HCP slice/MB level commands
    //! \details  Client facing function to calculate maximum size for HCP slice/MB level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] modeSpecific
    //!           Indicate the long or short format
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific);

    //!
    //! \brief    Calculates the maximum size for VVCP picture level commands
    //! \details  Client facing function to calculate the maximum size for VVCP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVvcpStateCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates the size for VVCP slice level commands
    //! \details  Client facing function to calculate the maximum size for VVCP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] sliceCommandsSize
    //!           The maximum command buffer size for slice
    //! \param    [out] slicePatchListSize
    //!           The maximum command patch list size for slice
    //! \param    [out] tileCommandsSize
    //!           The maximum command buffer size for tile
    //! \param    [out] tilePatchListSize
    //!           The maximum command patch list size for tile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVvcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *sliceCommandsSize,
        uint32_t *slicePatchListSize,
        uint32_t *tileCommandsSize,
        uint32_t *tilePatchListSize);

    MOS_STATUS GetVvcpSliceLvlCmdSize(uint32_t *sliceLvlCmdSize);

    //!
    //! \brief    Add Huc stream out copy cmds
    //! \details  Prepare and add Huc stream out copy cmds
    //!
    //! \param    [in,out] cmdBuffer
    //!           Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHucDummyStreamOut(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Perform Huc stream out copy
    //! \details  Implement the copy using huc stream out
    //!
    //! \param    [in] hucStreamOutParams
    //!           Huc stream out parameters
    //! \param    [in,out] cmdBuffer
    //!           Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PerformHucStreamOut(
        CodechalHucStreamoutParams  *hucStreamOutParams,
        PMOS_COMMAND_BUFFER         cmdBuffer);
    
    //! \brief    Read HCP status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for HCP status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadHcpStatus(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read HCP specific image status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
   //! \param    params
    //!           [in] the parameters for HCP IMG status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatusForHcp(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    MHW_VDBOX_NODE_IND GetMaxVdboxIndex()
    {
        return MEDIA_IS_SKU(m_skuTable, FtrVcs2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;
    };

    //!
    //! \brief    Check if disable scalability by default
    //! \return   bool
    //!           True if it is to disable scalability by default, else it is not.
    //!
    bool IsDisableScalability()
    {
        return m_disableScalability;
    }

    //!
    //! \brief    Resizes the cmd buffer and patch list
    //! \details  Resizes the buffer to be used for rendering GPU commands
    //!
    //! \param    [in] requestedCommandBufferSize
    //!           Requested resize command buffer size
    //! \param    [in] requestedPatchListSize
    //!           Requested resize patchlist size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResizeCommandBufferAndPatchList(
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize)
    {
        CODEC_HW_FUNCTION_ENTER;

        if (m_osInterface->bUsesCmdBufHeaderInResize)
        {
            return ResizeCommandBufferAndPatchListCmd(requestedCommandBufferSize, requestedPatchListSize);
        }
        else
        {
            return ResizeCommandBufferAndPatchListOs(requestedCommandBufferSize, requestedPatchListSize);
        }
    }


    MediaScalability *m_singlePipeScalability = nullptr;
    MediaScalability *m_multiPipeScalability  = nullptr;
    MOS_STATUS(*pfnCreateDecodeSinglePipe)
    (void *hwInterface, MediaContext *mediaContext, uint8_t componentType) = nullptr;
    MOS_STATUS(*pfnCreateDecodeMultiPipe)
    (void *hwInterface, MediaContext *mediaContext, uint8_t componentType) = nullptr;

    //!
    //! \brief    Select Vdbox by index and get MMIO register
    //! \details  Uses input parameters to Select VDBOX from KMD and get MMIO register
    //! \param    index
    //!           [in] vdbox index interface
    //! \param    pCmdBuffer
    //!           [in] get mos vdbox id from cmd buffer
    //! \return   MmioRegistersMfx
    //!           return the vdbox mmio register
    //!
    MmioRegistersMfx *SelectVdAndGetMmioReg(
        MHW_VDBOX_NODE_IND  index,
        PMOS_COMMAND_BUFFER pCmdBuffer);

    //!
    //! \brief    Init L3 Cache Settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitL3CacheSettings();
    
    //!
    //! \brief    Set Rowstore Cache offsets
    //! \details  Set Rowstore Cache offsets in sub interfaces in codechal hw interface 
    //!
    //! \param    [in] rowstoreParams
    //!           parameters to set rowstore cache offsets
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRowstoreCachingOffsets(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

    bool UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard)
    {
        if (codecFunction == CODECHAL_FUNCTION_ENC ||
            (codecFunction == CODECHAL_FUNCTION_ENC_PAK) ||
            codecFunction == CODECHAL_FUNCTION_HYBRIDPAK ||
            ((codecFunction == CODECHAL_FUNCTION_DECODE) && (standard == CODECHAL_VC1)) ||
            codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK ||
            codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC ||
            codecFunction == CODECHAL_FUNCTION_FEI_ENC ||
            codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK)
        {
            return true;
        }

        return false;
    }

    //!
    //! \brief    Send mi atomic dword cmd with indirect data (non-inline) mode
    //! \details  Send mi atomic dword cmd for sync perpose
    //!
    //! \param    [in] resource
    //!           Reource used in mi atomic dword cmd
    //! \param    [in] opCode
    //!           Operation code
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMiAtomicDwordIndirectDataCmd(
        PMOS_RESOURCE               resource,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;
        CODEC_HW_CHK_NULL_RETURN(m_miItf);

        auto &params             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwDataSize        = sizeof(uint32_t);
        params.Operation         = (mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE) opCode;
        params.bInlineData       = false;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer);

        return eStatus;
    }

    //!
    //! \brief    Send mi atomic dword cmd
    //! \details  Send mi atomic dword cmd for sync perpose
    //!
    //! \param    [in] resource
    //!           Reource used in mi atomic dword cmd
    //! \param    [in] immData
    //!           Immediate data
    //! \param    [in] opCode
    //!           Operation code
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMiAtomicDwordCmd(
        PMOS_RESOURCE               resource,
        uint32_t                    immData,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;
        CODEC_HW_CHK_NULL_RETURN(m_miItf);

        auto &params             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwDataSize        = sizeof(uint32_t);
        params.Operation         = (mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE) opCode;
        params.bInlineData       = true;
        params.dwOperand1Data[0] = immData;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer);

        return eStatus;
    }

    //!
    //! \brief    Send hw semphore wait cmd
    //! \details  Send hw semphore wait cmd for sync perpose
    //!
    //! \param    [in] semaMem
    //!           Reource of Hw semphore
    //! \param    [in] semaData
    //!           Data of Hw semphore
    //! \param    [in] opCode
    //!           Operation code
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendHwSemaphoreWaitCmd(
        PMOS_RESOURCE                             semaMem,
        uint32_t                                  semaData,
        MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION opCode,
        PMOS_COMMAND_BUFFER                       cmdBuffer,
        uint32_t                                  semaMemOffset = 0)
    {
        CODEC_HW_FUNCTION_ENTER;
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_CHK_NULL_RETURN(m_miItf);
        auto &params            = m_miItf->MHW_GETPAR_F(MI_SEMAPHORE_WAIT)();
        params                  = {};
        params.presSemaphoreMem = semaMem;
        params.bPollingWaitMode = true;
        params.dwSemaphoreData  = semaData;
        params.CompareOperation = (mhw::mi::MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION) opCode;
        params.dwResourceOffset = semaMemOffset;
        eStatus                 = m_miItf->MHW_ADDCMD_F(MI_SEMAPHORE_WAIT)(cmdBuffer);

        return eStatus;
    }

    //!
    //! \brief    Check if simulation/emulation is active
    //! \return   bool
    //!           True if simulation/emulation is active, else false.
    //!
    bool IsSimActive()
    {
        return m_osInterface ? m_osInterface->bSimIsActive : false;
    }

    //!
    //! \brief    Send mi store data imm cmd
    //! \param    [in] resource
    //!           Reource used in mi store data imm cmd
    //! \param    [in] immData
    //!           Immediate data
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMiStoreDataImm(
        PMOS_RESOURCE       resource,
        uint32_t            immData,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        CODEC_HW_CHK_NULL_RETURN(resource);
        CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

        CODEC_HW_CHK_NULL_RETURN(m_miItf);
        auto &params            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        params                  = {};
        params.pOsResource      = resource;
        params.dwResourceOffset = 0;
        params.dwValue          = immData;
        eStatus                 = m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer);

        return eStatus;
    }

    // Hardware dependent parameters
    bool     m_turboMode                         = false;  //!> Turbo mode info to pass in cmdBuf
    bool     m_isVdencSuperSliceEnabled          = false;  //!> Flag indicating Vdenc super slice is enabled
    uint16_t m_sizeOfCmdBatchBufferEnd           = 0;      //!> Size of batch buffer end cmd
    uint16_t m_sizeOfCmdMediaReset               = 0;      //!> Size of media reset cmd
    uint32_t m_vdencBrcImgStateBufferSize        = 0;      //!> vdenc brc img state buffer size
    uint32_t m_vdencBatchBuffer1stGroupSize      = 0;      //!> vdenc batch buffer 1st group size
    uint32_t m_vdencBatchBuffer2ndGroupSize      = 0;      //!> vdenc batch buffer 2nd group size
    uint32_t m_vdencReadBatchBufferSize          = 0;      //!> vdenc read batch buffer size for group1 and group2
    uint32_t m_vdenc2ndLevelBatchBufferSize      = 0;      //!> vdenc 2nd level batch buffer size
    uint32_t m_vdencBatchBufferPerSliceConstSize = 0;      //!> vdenc batch buffer per slice const size
    uint32_t m_HucStitchCmdBatchBufferSize       = 0;      //!> huc stitch cmd 2nd level batch buffer size
    uint32_t m_pakIntTileStatsSize               = 0;      //!> Size of combined statistics across all tiles
    uint32_t m_pakIntAggregatedFrameStatsSize    = 0;      //!> Size of HEVC/ VP9 PAK Stats, HEVC Slice Streamout, VDEnc Stats
    uint32_t m_tileRecordSize                    = 0;      //!> Tile record size
    uint32_t m_hucCommandBufferSize              = 0;      //!> Size of a single HuC command buffer

      // Slice/Sub-slice/EU Shutdown Parameters
    uint32_t m_numRequestedEuSlices  = 0;      //!> Number of requested Slices
    uint32_t m_numRequestedSubSlices = 0;      //!> Number of requested Sub-slices
    uint32_t m_numRequestedEus       = 0;      //!> Number of requested EUs
    bool     m_enableCodecMmc        = false;  //!> Flag to indicate if enable codec MMC by default or not

    //! \brief    default disable the get vdbox node by UMD, decided by MHW and MOS
    bool m_getVdboxNodeByUMD = false;

    void *legacyHwInterface = nullptr;

protected:

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS InitL3ControlUserFeatureSettings(
        mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG   *l3CacheConfig,
        mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *l3Overrides);
#endif

    //!
    //! \brief    Send conditional batch buffer end cmd
    //! \details  Send conditional batch buffer end cmd
    //!
    //! \param    [in] resource
    //!           Reource used in conditional batch buffer end cmd
    //! \param    [in] offset
    //!           Reource offset used in mi atomic dword cmd
    //! \param    [in] compData
    //!           Compare data
    //! \param    [in] disableCompMask
    //!           Indicate disabling compare mask
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendCondBbEndCmd(
        PMOS_RESOURCE       resource,
        uint32_t            offset,
        uint32_t            compData,
        bool                disableCompMask,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        if (!Mos_ResourceIsNull(&m_conditionalBbEndDummy))
        {
            auto &miFlushDwParams             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            miFlushDwParams                   = {};
            miFlushDwParams.postSyncOperation = 1;
            miFlushDwParams.pOsResource       = &m_conditionalBbEndDummy;
            miFlushDwParams.dwDataDW1         = 0;
            CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));
        }

        auto &miConditionalBatchBufferEndParams               = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams                     = {};
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = resource;
        miConditionalBatchBufferEndParams.dwOffset            = offset;
        miConditionalBatchBufferEndParams.dwValue             = compData;
        miConditionalBatchBufferEndParams.bDisableCompareMask = disableCompMask;
        CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(cmdBuffer));

        return eStatus;
    }

    MOS_STATUS ResizeCommandBufferAndPatchListCmd(
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        CODEC_HW_CHK_NULL_RETURN(m_miItf);

        MOS_COMMAND_BUFFER cmdBuffer;

        CODEC_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
        CODEC_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&cmdBuffer, nullptr));
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
        CODEC_HW_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(m_osInterface, requestedCommandBufferSize, requestedPatchListSize, 0));

        return eStatus;
    }

    MOS_STATUS ResizeCommandBufferAndPatchListOs(
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(m_osInterface, requestedCommandBufferSize, requestedPatchListSize, 0));

        return eStatus;
    }

protected:
    std::shared_ptr<mhw::vdbox::avp::Itf>    m_avpItf   = nullptr;      //!< Pointer to Mhw avp interface
    std::shared_ptr<mhw::vdbox::vdenc::Itf>  m_vdencItf = nullptr;      //!< Pointer to Mhw vdenc interface
    std::shared_ptr<mhw::vdbox::huc::Itf>    m_hucItf   = nullptr;      //!< Pointer to Mhw huc interface
    std::shared_ptr<mhw::mi::Itf>            m_miItf    = nullptr;      //!< Pointer to Mhw mi interface
    std::shared_ptr<mhw::vdbox::hcp::Itf>    m_hcpItf   = nullptr;      //!< Pointer to Mhw hcp interface
    std::shared_ptr<mhw::vdbox::mfx::Itf>    m_mfxItf   = nullptr;      //!< Pointer to Mhw mfx interface
    std::shared_ptr<mhw::render::Itf>        m_renderItf   = nullptr;      //!< Pointer to render interface
    std::shared_ptr<MediaSfcInterface>       m_mediaSfcItf = nullptr;      //!< Pointer to Media sfc interface
    std::shared_ptr<mhw::vdbox::vvcp::Itf>   m_vvcpItf     = nullptr;
    // States
    PMOS_INTERFACE       m_osInterface;  //!< Pointer to OS interface

    MediaUserSettingSharedPtr   m_userSettingPtr = nullptr;
    
    // Auxiliary
    PLATFORM             m_platform;  //!< Platform information

    MHW_STATE_HEAP_SETTINGS                 m_stateHeapSettings;               //!< State heap Mhw settings
    MhwCpInterface                         *m_cpInterface          = nullptr;  //!< Pointer to Mhw cp interface
    RENDERHAL_INTERFACE                    *m_renderHal            = nullptr;  //!< RenderHal interface
    MhwCpInterface                         *m_renderHalCpInterface = nullptr;  //!< Pointer to RenderHal cp interface
    MhwVeboxInterface                      *m_veboxInterface       = nullptr;  //!< Pointer to Mhw vebox interface
    MhwSfcInterface                        *m_sfcInterface         = nullptr;  //!< Pointer to Mhw sfc interface
    MHW_MEMORY_OBJECT_CONTROL_PARAMS        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC];  //!< Cacheability Settings list
    bool                                    m_disableScalability                  = false;   //!> Flag to indicate if disable scalability by default
    CODECHAL_SSEU_SETTING const            *m_ssEuTable          = nullptr;                              //!< Pointer to the default SSEU settings table
    uint16_t                                m_numMediaStates     = CODECHAL_NUM_MEDIA_STATES;            //!< number of media states
    uint32_t                                m_maxKernelLoadCmdSize                = 0;                                    //!> Max kernel load cmd size
#if (_DEBUG || _RELEASE_INTERNAL)
    bool     m_numRequestedOverride          = false;  //!> Flag to indicate whether these params are set by Reg
    uint32_t m_numRequestedEuSlicesOverride  = 0;      //!> Number of requested Slices set by Reg
    uint32_t m_numRequestedSubSlicesOverride = 0;      //!> Number of requested Sub-slices set by Reg
    uint32_t m_numRequestedEusOverride       = 0;      //!> Number of requested EUs set by Reg
#endif

    uint32_t m_sizeOfCmdMediaObject     = 0;  //!> Size of media object cmd
    uint32_t m_sizeOfCmdMediaStateFlush = 0;  //!> Size of media state flush cmd
    // COND BBE WA
    MOS_RESOURCE m_conditionalBbEndDummy;  //!> Dummy Resource for conditional batch buffer end WA


    // Auxiliary
    MEDIA_FEATURE_TABLE *m_skuTable = nullptr;  //!< Pointer to SKU table
    MEDIA_WA_TABLE      *m_waTable  = nullptr;  //!< Pointer to WA table

    // HuC WA implementation
    MOS_RESOURCE                m_dummyStreamIn;          //!> Resource of dummy stream in
    MOS_RESOURCE                m_dummyStreamOut;         //!> Resource of dummy stream out
    MOS_RESOURCE                m_hucDmemDummy;           //!> Resource of Huc DMEM for dummy streamout WA
    uint32_t                    m_dmemBufSize = 0;        //!>

    bool         m_checkBankCount   = false;  //!> Used to check L3 cache enable

MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceNext)
};

#endif  // __CODEC_HW_NEXT_H__
