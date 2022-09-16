/*
* Copyright (c) 2021, Intel Corporation
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

#include "codechal.h"
#include "media_interfaces_mhw_next.h"

// Remove legacy header files
#include "mhw_vdbox_mfx_interface.h"
#include "mhw_vdbox_hcp_interface.h"
#include "mhw_vdbox_vdenc_interface.h"
#include "mhw_vdbox_huc_interface.h"
#include "mhw_mi_itf.h"
#include "media_sfc_interface.h"
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
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceNext() {
        CODEC_HW_FUNCTION_ENTER;
        m_osInterface->pfnFreeResource(m_osInterface, &m_hucDmemDummy);
        m_osInterface->pfnFreeResource(m_osInterface, &m_dummyStreamIn);
        m_osInterface->pfnFreeResource(m_osInterface, &m_dummyStreamOut);
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
        return MHW_VDBOX_NODE_1;
    }

    //!
    //! \brief    Init L3 Cache Settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitL3CacheSettings();

protected:

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS InitL3ControlUserFeatureSettings(
        mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG   *l3CacheConfig,
        mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *l3Overrides);
#endif

protected:
    std::shared_ptr<mhw::vdbox::avp::Itf>    m_avpItf   = nullptr;      //!< Pointer to Mhw avp interface
    std::shared_ptr<mhw::vdbox::vdenc::Itf>  m_vdencItf = nullptr;      //!< Pointer to Mhw vdenc interface
    std::shared_ptr<mhw::vdbox::huc::Itf>    m_hucItf   = nullptr;      //!< Pointer to Mhw huc interface
    std::shared_ptr<mhw::mi::Itf>            m_miItf    = nullptr;      //!< Pointer to Mhw mi interface
    std::shared_ptr<mhw::vdbox::hcp::Itf>    m_hcpItf   = nullptr;      //!< Pointer to Mhw hcp interface
    std::shared_ptr<mhw::vdbox::mfx::Itf>    m_mfxItf   = nullptr;      //!< Pointer to Mhw mfx interface
    std::shared_ptr<mhw::render::Itf>        m_renderItf   = nullptr;      //!< Pointer to render interface
    std::shared_ptr<MediaSfcInterface>       m_mediaSfcItf = nullptr;      //!< Pointer to Media sfc interface

    // States
    PMOS_INTERFACE       m_osInterface;  //!< Pointer to OS interface

    // Auxiliary
    MEDIA_FEATURE_TABLE *m_skuTable = nullptr;  //!< Pointer to SKU table
    MEDIA_WA_TABLE      *m_waTable  = nullptr;  //!< Pointer to WA table

    // HuC WA implementation
    MOS_RESOURCE                m_dummyStreamIn;          //!> Resource of dummy stream in
    MOS_RESOURCE                m_dummyStreamOut;         //!> Resource of dummy stream out
    MOS_RESOURCE                m_hucDmemDummy;           //!> Resource of Huc DMEM for dummy streamout WA
    uint32_t                    m_dmemBufSize = 0;        //!>

    // Next: remove legacy Interfaces
    MhwCpInterface                  *m_cpInterface = nullptr;         //!< Pointer to Mhw cp interface
    MhwVdboxMfxInterface            *m_mfxInterface = nullptr;        //!< Pointer to Mhw mfx interface
    MhwVdboxVdencInterface          *m_vdencInterface = nullptr;      //!< Pointer to Mhw vdenc interface

MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceNext)
};

#endif  // __CODEC_HW_NEXT_H__
