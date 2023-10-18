/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file      codechal_hw.h 
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across CODECHAL components. 
//!

#ifndef __CODECHAL_HW_H__
#define __CODECHAL_HW_H__

#include "codechal.h"
#include "renderhal_legacy.h"
#include "mhw_mi.h"
#include "mhw_render_legacy.h"
#include "mhw_state_heap.h"
#include "mhw_vdbox.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "mhw_cp_interface.h"
#include "media_copy.h"
#include "media_blt_copy.h"

#include "mhw_vdbox_mfx_interface.h"
#include "mhw_vdbox_hcp_interface.h"
#include "mhw_vdbox_avp_interface.h"
#include "mhw_vdbox_huc_interface.h"
#include "mhw_vdbox_vdenc_interface.h"
#include "mhw_vdbox_hcp_itf.h"

#include "media_interfaces_mhw.h"
#include "media_sfc_interface.h"

#include "gfxmacro.h"

#include "codec_hw_next.h"

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define CODECHAL_HW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _expr)

#define CODECHAL_HW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODECHAL_HW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODECHAL_HW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define CODECHAL_HW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW)

#define CODECHAL_HW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODECHAL_HW_CHK_STATUS_RETURN(_stmt)                                            \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define CODECHAL_HW_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_HW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODECHAL_HW_CHK_NULL_RETURN(_ptr)                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODECHAL_HW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _ptr)

#define CODECHAL_HW_CHK_COND_RETURN(_expr, _message, ...)                           \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_SURFACE_PITCH_ALIGNMENT        128

#define CODECHAL_MEDIA_WALKER_MAX_COLORS        16      // 4 Bits for color field gives max 16 colors

#define CODECHAL_VDIRECTION_FRAME               2
#define CODECHAL_VDIRECTION_TOP_FIELD           1
#define CODECHAL_VDIRECTION_BOT_FIELD           3
#define CODECHAL_VLINESTRIDE_FRAME              0
#define CODECHAL_VLINESTRIDE_FIELD              1
#define CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD    0
#define CODECHAL_VLINESTRIDEOFFSET_BOT_FIELD    1

#define CODECHAL_MAX_DEPENDENCY_COUNT  8

#define CODECHAL_INVALID_BINDING_TABLE_IDX  0xFFFFFFFF

//!
//! \enum     MoTargetCache
//! \brief    Mo target cache
//!
enum MoTargetCache
{
    CODECHAL_MO_TARGET_CACHE_ELLC = 0x0,
    CODECHAL_MO_TARGET_CACHE_LLC = 0x1,
    CODECHAL_MO_TARGET_CACHE_LLC_ELLC = 0x2,
    CODECHAL_MO_TARGET_CACHE_L3_LLC_ELLC = 0x3
};

//!
//! \enum     CodechalCacheabilityType
//! \brief    Codechal cacheability type
//!
enum CodechalCacheabilityType
{
    codechalUncacheable    = 0,
    codechalLLC            = 1,
    codechalL3             = 2,
    codechalUncacheableWa  = 8
};

//!
//! \enum     CodechalWalkingPattern
//! \brief    Codechal walking pattern
//!
enum CodechalWalkingPattern
{
    codechalHorizontal26DegreeScan           = 0,
    codechalHorizontal45DegreeScan           = 1,
    codechalHorizontal26DegreeScanMbaff      = 2,
    codechalHorizontalRasterScan             = 3,
    codechalVerticalRasterScan               = 4
};

typedef enum _CODECHAL_SLICE_STATE
{
    CODECHAL_SLICE_SHUTDOWN_DEFAULT     = 0,
    CODECHAL_SLICE_SHUTDOWN_ONE_SLICE   = 1,
    CODECHAL_SLICE_SHUTDOWN_TWO_SLICES  = 2
} CODECHAL_SLICE_STATE;

//!
//! \struct    CodechalQpStatusCount
//! \brief     Codechal qp status count
//!
struct CodechalQpStatusCount
{
    union{
        struct{
            uint32_t   cumulativeQP : 24;
            uint32_t   cumulativeQPAdjust : 8;
        };

        struct
        {
            // DW0
            uint32_t   hcpCumulativeQP : 24;
            uint32_t                   : 8;

            // DW1
            uint32_t   hcpFrameMinCUQp : 6;
            uint32_t   hcpFrameMaxCUQp : 6;
            uint32_t                   : 20;
        };

        struct
        {
            uint32_t value[2];
        };
    };
};

//!
//! \struct    CodechalDataCopyParams
//! \brief     Codechal data copy parameters
//!
struct CodechalDataCopyParams
{
    // Src params
    PMOS_RESOURCE   srcResource;
    uint32_t        srcSize;
    uint32_t        srcOffset;

    // Dst params
    PMOS_RESOURCE   dstResource;
    uint32_t        dstSize;
    uint32_t        dstOffset;
};

//!
//! \struct    BrcPakStatsReadParams
//! \brief     Read brc pak states parameters
//!
struct BrcPakStatsReadParams
{
    PMOS_RESOURCE           presBrcPakStatisticBuffer;
    uint32_t                bitstreamBytecountFrameOffset;
    uint32_t                bitstreamBytecountFrameNoHeaderOffset;
    uint32_t                imageStatusCtrlOffset;

    PMOS_RESOURCE           presStatusBuffer;
    uint32_t                dwStatusBufNumPassesOffset;
    uint8_t                 ucPass;
    MOS_GPU_CONTEXT         VideoContext;
};

//!  Codechal hw interface
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal
*/
class CodechalHwInterface
{
protected: 
    // Slice Shutdown Threshold
    static const uint32_t m_sliceShutdownAvcTargetUsageThreshold = 2;         //!< slice shutdown AVC target usage threshold
    static const uint32_t m_sliceShutdownAvcResolutionThreshold = 2073600;    //!< slice shutdown AVC resolution threshold: 1080p - 1920x1080
    static const uint32_t m_sliceShutdownMpeg2ResolutionThreshold = 8294400;  //!< slice shutdown MPEG2 resolution threshold: 4K - 3840x2160

    // States
    PMOS_INTERFACE                  m_osInterface;                    //!< Pointer to OS interface

    // Auxiliary
    PLATFORM                        m_platform;                       //!< Platform information
    MEDIA_FEATURE_TABLE             *m_skuTable = nullptr;             //!< Pointer to SKU table
    MEDIA_WA_TABLE                  *m_waTable = nullptr;              //!< Pointer to WA table

    MHW_STATE_HEAP_SETTINGS         m_stateHeapSettings;              //!< State heap Mhw settings
    MhwMiInterface                  *m_miInterface = nullptr;         //!< Pointer to Mhw mi interface
    MhwCpInterface                  *m_cpInterface = nullptr;         //!< Pointer to Mhw cp interface
    MhwRenderInterface              *m_renderInterface = nullptr;     //!< Pointer to Mhw render interface
    RENDERHAL_INTERFACE             *m_renderHal = nullptr;           //!< RenderHal interface
    MhwCpInterface                  *m_renderHalCpInterface = nullptr;//!< Pointer to RenderHal cp interface
    MhwVeboxInterface               *m_veboxInterface = nullptr;      //!< Pointer to Mhw vebox interface
    MhwSfcInterface                 *m_sfcInterface = nullptr;        //!< Pointer to Mhw sfc interface
    MhwVdboxMfxInterface            *m_mfxInterface = nullptr;        //!< Pointer to Mhw mfx interface
    MhwVdboxHcpInterface            *m_hcpInterface = nullptr;        //!< Pointer to Mhw hcp interface
    MhwVdboxAvpInterface            *m_avpInterface = nullptr;        //!< Pointer to Mhw avp interface
    MhwVdboxHucInterface            *m_hucInterface = nullptr;        //!< Pointer to Mhw huc interface
    MhwVdboxVdencInterface          *m_vdencInterface = nullptr;      //!< Pointer to Mhw vdenc interface
    std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpItf   = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
    std::shared_ptr<MediaSfcInterface> m_mediaSfcItf = nullptr;
    std::shared_ptr<mhw::render::Itf>       m_renderItf             = nullptr;

    CODECHAL_SSEU_SETTING const         *m_ssEuTable = nullptr;       //!< Pointer to the default SSEU settings table
    uint16_t                            m_numMediaStates = CODECHAL_NUM_MEDIA_STATES;  //!< number of media states

    MHW_MEMORY_OBJECT_CONTROL_PARAMS    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC];  //!< Cacheability Settings list

    // HuC WA implementation
    MOS_RESOURCE                m_dummyStreamIn;          //!> Resource of dummy stream in
    MOS_RESOURCE                m_dummyStreamOut;         //!> Resource of dummy stream out
    MOS_RESOURCE                m_hucDmemDummy;           //!> Resource of Huc DMEM for dummy streamout WA
    uint32_t                    m_dmemBufSize = 0;        //!>

    // COND BBE WA
    MOS_RESOURCE                m_conditionalBbEndDummy;  //!> Dummy Resource for conditional batch buffer end WA

    uint32_t                    m_maxKernelLoadCmdSize = 0;  //!> Max kernel load cmd size
    bool                        m_checkTargetCache = false;  //!> Used to check L3 cache enable
    bool                        m_checkBankCount = false;    //!> Used to check L3 cache enable

    uint32_t                    m_sizeOfCmdMediaObject = 0;      //!> Size of media object cmd
    uint32_t                    m_sizeOfCmdMediaStateFlush = 0;  //!> Size of media state flush cmd

    bool                        m_noSeparateL3LlcCacheabilitySettings = false;   // No separate L3 LLC cacheability settings

    bool                        m_disableScalability                  = false;   //!> Flag to indicate if disable scalability by default

public:
    // Hardware dependent parameters
    bool                        m_turboMode = false;                            //!> Turbo mode info to pass in cmdBuf
    bool                        m_isVdencSuperSliceEnabled = false;             //!> Flag indicating Vdenc super slice is enabled
    bool                        m_noHuC = false;                                //!> This flag to indicate HuC present on Linux
    uint16_t                    m_sizeOfCmdBatchBufferEnd = 0;                  //!> Size of batch buffer end cmd
    uint16_t                    m_sizeOfCmdMediaReset = 0;                      //!> Size of media reset cmd
    uint32_t                    m_vdencBrcImgStateBufferSize = 0;               //!> vdenc brc img state buffer size
    uint32_t                    m_vdencBatchBuffer1stGroupSize = 0;             //!> vdenc batch buffer 1st group size
    uint32_t                    m_vdencBatchBuffer2ndGroupSize = 0;             //!> vdenc batch buffer 2nd group size
    uint32_t                    m_vdencReadBatchBufferSize = 0;                 //!> vdenc read batch buffer size for group1 and group2
    uint32_t                    m_vdencGroup3BatchBufferSize = 0;               //!> vdenc read batch buffer size for group3
    uint32_t                    m_vdencCopyBatchBufferSize = 0;                 //!> vdenc copy batch buffer size
    uint32_t                    m_vdenc2ndLevelBatchBufferSize = 0;             //!> vdenc 2nd level batch buffer size
    uint32_t                    m_vdencBatchBufferPerSliceConstSize = 0;        //!> vdenc batch buffer per slice const size
    uint32_t                    m_HucStitchCmdBatchBufferSize = 0;              //!> huc stitch cmd 2nd level batch buffer size
    uint32_t                    m_mpeg2BrcConstantSurfaceWidth = 64;            //!> mpeg2 brc constant surface width
    uint32_t                    m_mpeg2BrcConstantSurfaceHeight = 43;           //!> mpeg2 brc constant surface height
    uint32_t                    m_avcMbStatBufferSize = 0;                      //!> AVC Mb status buffer size
    uint32_t                    m_pakIntTileStatsSize = 0;                      //!> Size of combined statistics across all tiles
    uint32_t                    m_pakIntAggregatedFrameStatsSize = 0;           //!> Size of HEVC/ VP9 PAK Stats, HEVC Slice Streamout, VDEnc Stats
    uint32_t                    m_tileRecordSize = 0;                           //!> Tile record size
    uint32_t                    m_hucCommandBufferSize = 0;                     //!> Size of a single HuC command buffer

    // Slice/Sub-slice/EU Shutdown Parameters
    uint32_t                    m_numRequestedEuSlices = 0;                     //!> Number of requested Slices
    uint32_t                    m_numRequestedSubSlices = 0;                    //!> Number of requested Sub-slices
    uint32_t                    m_numRequestedEus = 0;                          //!> Number of requested EUs
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                        m_numRequestedOverride = false;                 //!> Flag to indicate whether these params are set by Reg
    uint32_t                    m_numRequestedEuSlicesOverride = 0;             //!> Number of requested Slices set by Reg
    uint32_t                    m_numRequestedSubSlicesOverride = 0;            //!> Number of requested Sub-slices set by Reg
    uint32_t                    m_numRequestedEusOverride = 0;                  //!> Number of requested EUs set by Reg
#endif

    uint32_t                    m_ssdResolutionThreshold = 0;                   //!> Slice shutdown resolution threshold
    uint32_t                    m_ssdTargetUsageThreshold = 0;                  //!> Slice shutdown target usage threshold
    uint32_t                    m_mpeg2SSDResolutionThreshold = 0;              //!> Slice shutdown resolution threshold for MPEG2

    bool                        m_slicePowerGate = false;                       //!> Slice power gate

    bool                        m_enableCodecMmc = false;                       //!> Flag to indicate if enable codec MMC by default or not

    //!
    //! \brief    Constructor
    //!
    CodechalHwInterface(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces,
        bool              disableScalability = false);

    static CodechalHwInterface *Create(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces,
        bool              disableScalability);

    //!
    //! \brief    Get avp interface
    //! \details  Get avp interface in codechal hw interface next
    //!
    //! \return    pointer to new AVP interface
    //!
    inline std::shared_ptr<mhw::vdbox::avp::Itf> GetAvpInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetAvpInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }

    //!
    //! \brief    Get vdenc interface
    //! \details  Get vdenc interface in codechal hw interface next
    //!
    //! \return    pointer to new Vdenc interface
    //!
    inline std::shared_ptr<mhw::vdbox::vdenc::Itf> GetVdencInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetVdencInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }

    //! \brief    Get huc interface
    //! \details  Get huc interface in codechal hw interface next
    //!
    //! \return    pointer to new HUC interface
    //!
    inline std::shared_ptr<mhw::vdbox::huc::Itf> GetHucInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetHucInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }

    //!
    //! \brief    Get mi interface
    //! \details  Get mi interface in codechal hw interface next
    //!
    //! \return    pointer to new MI interface
    //!
    inline std::shared_ptr<mhw::mi::Itf> GetMiInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetMiInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }

    //!
    //! \brief    Get hcp interface
    //! \details  Get hcp interface in codechal hw interface next
    //!
    //! \return    pointer to new HCP interface
    //!
    inline std::shared_ptr<mhw::vdbox::hcp::Itf> GetHcpInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetHcpInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }

    //!
    //! \brief    Get mfx interface
    //! \details  Get mfx interface in codechal hw interface next
    //!
    //! \return    pointer to new MFX interface
    //!
    inline std::shared_ptr<mhw::vdbox::mfx::Itf> GetMfxInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetMfxInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }
    //!
    //! \brief    Get render interface
    //! \details  Get render interface in codechal hw interface next
    //!
    //! \return    pointer to new HCP interface
    //!
    inline std::shared_ptr<mhw::render::Itf> GetRenderInterfaceNext()
    {
        if (m_hwInterfaceNext)
        {
            return m_hwInterfaceNext->GetRenderInterfaceNext();
        }
        else
        {
            return nullptr;
        }
    }
    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterface(const CodechalHwInterface&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterface& operator=(const CodechalHwInterface&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterface()
    {
        CODECHAL_HW_FUNCTION_ENTER;

        if (MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable) && m_osInterface)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_hucDmemDummy);
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_dummyStreamIn);
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_dummyStreamOut);
        }

        if (m_osInterface)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_conditionalBbEndDummy);

            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }

        if (m_miInterface)
        {
            MOS_Delete(m_miInterface);
            m_miInterface = nullptr;
        }

        if (m_renderInterface)
        {
            MOS_Delete(m_renderInterface);
            m_renderInterface = nullptr;
        }

        if (m_veboxInterface)
        {
            m_veboxInterface->DestroyHeap();
            MOS_Delete(m_veboxInterface);
            m_veboxInterface = nullptr;
        }

        if (m_mfxInterface)
        {
            MOS_Delete(m_mfxInterface);
            m_mfxInterface = nullptr;
        }
        if (m_hcpInterface)
        {
            MOS_Delete(m_hcpInterface);
            m_hcpInterface = nullptr;
        }
        if (m_hucInterface)
        {
            MOS_Delete(m_hucInterface);
            m_hucInterface = nullptr;
        }
        if (m_vdencInterface)
        {
            MOS_Delete(m_vdencInterface);
            m_vdencInterface = nullptr;
        }

        if (m_sfcInterface)
        {
            MOS_Delete(m_sfcInterface);
            m_sfcInterface = nullptr;
        }

        if (m_hwInterfaceNext)
        {
            MOS_Delete(m_hwInterfaceNext);
            m_hwInterfaceNext = nullptr;
        }

    #if MHW_HWCMDPARSER_ENABLED
        mhw::HwcmdParser::DestroyInstance();
    #endif
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

    inline std::shared_ptr<MediaSfcInterface> GetMediaSfcInterface()
    {
        return m_mediaSfcItf;
    }

    //!
    //! \brief    Get State Heap Settings
    //! \details  Get State Heap Settings in codechal hw interface 
    //!
    //! \return   [out] MHW_STATE_HEAP_SETTINGS*
    //!           Settings got.
    //!
    inline MHW_STATE_HEAP_SETTINGS *GetStateHeapSettings()
    {
        return &m_stateHeapSettings;
    }

    //!
    //! \brief    Get mi interface
    //! \details  Get mi interface in codechal hw interface 
    //!
    //! \return   [out] MhwMiInterface*
    //!           Interface got.
    //!
    inline MhwMiInterface          *GetMiInterface()
    {
        return m_miInterface;
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

    //!
    //! \brief    Get render interface
    //! \details  Get render interface in codechal hw interface 
    //!
    //! \return   [out] MhwRenderInterface*
    //!           Interface got.
    //!
    inline MhwRenderInterface *GetRenderInterface()
    {
        return m_renderInterface;
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
    //! \brief    Get vebox interface
    //! \details  Get vebox interface in codechal hw interface 
    //!
    //! \return   [out] MhwVeboxInterface*
    //!           Interface got.
    //!
    inline MhwVeboxInterface *GetVeboxInterface()
    {
        return m_veboxInterface;
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
    //! \brief    Get mfx interface
    //! \details  Get mfx interface in codechal hw interface 
    //!
    //! \return   [out] MhwVdboxMfxInterface*
    //!           Interface got.
    //!
    inline MhwVdboxMfxInterface *GetMfxInterface()
    {
        return m_mfxInterface;
    }

    //!
    //! \brief    Get hcp interface
    //! \details  Get hcp interface in codechal hw interface 
    //!
    //! \return   [out] MhwVdboxHcpInterface*
    //!           Interface got.
    //!
    inline MhwVdboxHcpInterface *GetHcpInterface()
    {
        return m_hcpInterface;
    }

    //!
    //! \brief    Get avp interface
    //! \details  Get avp interface in codechal hw interface
    //!
    //! \return   [out] MhwVdboxAvpInterface*
    //!           Interface got.
    //!
    MhwVdboxAvpInterface *GetAvpInterface()
    {
        return m_avpInterface;
    }

    //!
    //! \brief    Get huc interface
    //! \details  Get huc interface in codechal hw interface 
    //!
    //! \return   [out] MhwVdboxHucInterface*
    //!           Interface got.
    //!
    inline MhwVdboxHucInterface *GetHucInterface()
    {
        return m_hucInterface;
    }

    //!
    //! \brief    Get vdenc interface
    //! \details  Get vdenc interface in codechal hw interface 
    //!
    //! \return   [out] MhwVdboxVdencInterface*
    //!           Interface got.
    //!
    inline MhwVdboxVdencInterface *GetVdencInterface()
    {
        return m_vdencInterface;
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

    //! \brief    Create media copy
    //! \details  Create media copy instance.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \return   MediaCopyBaseState*
    //!           Pointer to MediaCopyBaseState
    //!
    virtual MediaCopyBaseState* CreateMediaCopy(PMOS_INTERFACE mosInterface)
    {
        return nullptr;
    }

    //! \brief    Get blt state
    //! \details  Get blt interface in codechal hw interface
    //!
    //! \return   [out] BltState*
    //!           Interface got.
    //!
    virtual BltState *GetBltState()
    {
        return nullptr;
    }

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
    //! \brief    Init L3 Cache Settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitL3CacheSettings();

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

    //!
    //! \brief    Calculates the maximum size for all picture level commands
    //! \details  Client facing function to calculate the maximum size for all picture level commands in mfx pipline
    //!
    //! \param    [in] mode
    //!           codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] shortFormat
    //!           True if short format, false long format
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        bool                            shortFormat);

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
    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        bool                            modeSpecific);

    //!
    //! \brief    Calculates the maximum size for HCP/HUC picture level commands
    //! \details  Client facing function to calculate the maximum size for HCP/HUC picture level commands
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
    MOS_STATUS GetHxxStateCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for HCP/HUC slice/MB level commands
    //! \details  Client facing function to calculate maximum size for HCP/HUC slice/MB level commands
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
    MOS_STATUS GetHxxPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        bool                            modeSpecific);

    //!
    //! \brief    Calculates the maximum size for HCP picture level commands
    //! \details  Client facing function to calculate the maximum size for HCP picture level commands
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
    MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t *                      commandsSize,
        uint32_t *                      patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

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
    MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific);

    //!
    //! \brief    Calculates the maximum size for Huc picture level commands
    //! \details  Client facing function to calculate the maximum size for HUC picture level commands
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
    MOS_STATUS GetHucStateCommandSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    //!
    //! \brief    Calculates maximum size for Huc slice/MB level commands
    //! \details  Client facing function to calculate maximum size for Huc slice/MB level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetHucPrimitiveCommandSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize);

    //!
    //! \brief    Calculates the maximum size for Vdenc state level commands
    //! \details  Client facing function to calculate the maximum size for Vdenc state level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVdencStateCommandsDataSize(
        uint32_t                    mode,
        uint32_t                   *commandsSize,
        uint32_t                   *patchListSize);

    //!
    //! \brief    Calculates maximum size for all slice level VDEnc commands
    //! \details  Client facing function to calculate the maximum size for Vdenc slice level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVdencPrimitiveCommandsDataSize(
        uint32_t                    mode,
        uint32_t                   *commandsSize,
        uint32_t                   *patchListSize);

    //!
    //! \brief    Calculates the maximum size for Vdenc picture 2nd level commands
    //! \details  Client facing function to calculate the maximum size for Vdenc picture 2nd level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVdencPictureSecondLevelCommandsSize(
        uint32_t                    mode,
        uint32_t                   *commandsSize);

    //!
    //! \brief    Calculates the maximum size for Huc Streamout commands
    //! \details  Client facing function to calculate the maximum size for Huc Streamout commands
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStreamoutCommandSize(
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize);

    //!
    //! \brief    Get max vdbox index
    //!
    //! \return   MHW_VDBOX_NODE_IND
    //!           max vdbox index got
    //!
    inline MHW_VDBOX_NODE_IND GetMaxVdboxIndex()
    {
        return MEDIA_IS_SKU(m_skuTable, FtrVcs2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;
    }

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

    //!
    //! \brief    Get meida object buffer size
    //! \details  Calculate the 2nd level BB size for media object BBs used with kernels
    //! 
    //! \param    [in] numMbs
    //!           Number of MBs for calculation
    //! \param    [in] inlineDataSize
    //!           Inline data size used
    //!
    //! \return   uint32_t
    //!           Buffer size got.
    //!
    uint32_t GetMediaObjectBufferSize(
        uint32_t numMbs,
        uint32_t inlineDataSize);

    //!
    //! \brief    Add vdenc brc img buffer
    //! \details  Add vdenc bitrate control image buffer into cmdbuffer
    //! 
    //! \param    [in] vdencBrcImgBuffer
    //!           Resource of vdenc brc img buffer
    //! \param    [in] params
    //!           parameter used for AVC img parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencBrcImgBuffer(
        PMOS_RESOURCE               vdencBrcImgBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS   params);

    //!
    //! \brief    Add vdenc sfd img buffer
    //! \details  Add vdenc static frame detection image buffer into cmdbuffer
    //! 
    //! \param    [in] vdencSfdImgBuffer
    //!           Resource of vdenc brc img buffer
    //! \param    [in] params
    //!           parameter used for AVC img parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencSfdImgBuffer(
        PMOS_RESOURCE               vdencSfdImgBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS   params);

    //!
    //! \brief    Calculates the maximum size for all kernel loading commands
    //! \details  Client facing function to calculate the maximum size for all kernel loading commands
    //! \param    [in] maxNumSurfaces
    //!           maximum surface number
    //! \return   uint32_t
    //!           Returns the maximum size for all kernel loading commands
    //!
    uint32_t GetKernelLoadCommandSize(
        uint32_t maxNumSurfaces);

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
        uint32_t                    requestedCommandBufferSize,
        uint32_t                    requestedPatchListSize);

    //!
    //! \brief    Resizes the cmd buffer and patch list with cmd buffer header
    //! \details  Resizes the buffer to be used for rendering GPU commands with cmd buffer header
    //!
    //! \param    [in] requestedCommandBufferSize
    //!           Requested resize command buffer size
    //! \param    [in] requestedPatchListSize
    //!           Requested resize patchlist size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResizeCommandBufferAndPatchListCmd(
        uint32_t                    requestedCommandBufferSize,
        uint32_t                    requestedPatchListSize);

    //!
    //! \brief    Resizes the cmd buffer and patch list without cmd buffer header
    //! \details  Resizes the buffer to be used for rendering GPU commands without cmd buffer header
    //!
    //! \param    [in] requestedCommandBufferSize
    //!           Requested resize command buffer size
    //! \param    [in] requestedPatchListSize
    //!           Requested resize patchlist size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResizeCommandBufferAndPatchListOs(
        uint32_t                    requestedCommandBufferSize,
        uint32_t                    requestedPatchListSize);

    // Synchronization Functions

    //!
    //! \brief    Write sync tag to resource
    //!
    //! \param    [in,out] cmdBuffer
    //!           command buffer used
    //! \param    [in] syncParams
    //!           sync parameters used to add tag
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WriteSyncTagToResource(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMOS_SYNC_PARAMS            syncParams);

    //!
    //! \brief    Helper function composes SURFACE_STATE cacheability settings
    //! \details  Helper function composes SURFACE_STATE cacheability settings with LLC and L3 values
    //!
    //! \param    [in] cacheabiltySettingIdx
    //!           which module needs to be set caching control
    //! \param    [in] cacheabilityTypeRequested
    //!           cacheability type
    //!
    //! \return   uint32_t
    //!           cacheability settings result
    //!
    uint32_t ComposeSurfaceCacheabilityControl(
        uint32_t                cacheabiltySettingIdx,
        uint32_t                cacheabilityTypeRequested);

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
    virtual MOS_STATUS AddHucDummyStreamOut(
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

    //!
    //! \brief    Update the number of Slices, Sub-slices and EUs in the command buffer
    //! \details  Update the number of Slices, Sub-slices and EUs in the command buffer
    //!           with the final value from the HwInterface structure
    //!
    //! \param    [in,out] cmdBuffer
    //!           Command buffer
    //! \param    [in] singleTaskPhaseSupported
    //!           Indicate if single task phase supported
    //! \param    [in] lastTaskInPhase
    //!           Indicate if it is last task phase
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateSSEuForCmdBuffer(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        bool                                singleTaskPhaseSupported,
        bool                                lastTaskInPhase);

    //!
    //! \brief    Set the default number of Slice, Sub-slice, EUs
    //! \details  Set the default number of Slice, Sub-slice, EUs recommended for 
    //!           the given kernel type in the HwInterface structure
    //!
    //! \param    [in] mediaStateType
    //!           Media state type
    //! \param    [in] setRequestedSlices
    //!           Slices requested to set
    //! \param    [in] setRequestedSubSlices
    //!           SubSlices requested to set
    //! \param    [in] setRequestedEus
    //!           Eu numbers requested to set
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetDefaultSSEuSetting(
        CODECHAL_MEDIA_STATE_TYPE           mediaStateType,
        bool                                setRequestedSlices,
        bool                                setRequestedSubSlices,
        bool                                setRequestedEus);

    //!
    //! \brief    Copy data source with drv
    //!
    //! \param    [in] dataCopyParams
    //!           Parameters for data copy
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyDataSourceWithDrv(
        CodechalDataCopyParams          *dataCopyParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Initialize L3 control user feature settings
    //!
    //! \param    [in] l3CacheConfig
    //!           L3 cache configuration
    //! \param    [in] l3Overrides
    //!           L3 overrides
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitL3ControlUserFeatureSettings(
        MHW_RENDER_ENGINE_L3_CACHE_CONFIG   *l3CacheConfig,
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *l3Overrides);
#endif // _DEBUG || _RELEASE_INTERNAL

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
        uint32_t                                  semaMemOffset = 0);

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
        PMOS_COMMAND_BUFFER         cmdBuffer);

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
        PMOS_RESOURCE              resource,
        uint32_t                   offset,
        uint32_t                   compData,
        bool                       disableCompMask,
        PMOS_COMMAND_BUFFER        cmdBuffer);

    //!
    //! \brief    Loads kernel data into the ISH
    //! \details  Uses the data described in the kernel state to assign an ISH block and load the kernel data into it
    //! \param    stateHeapInterface
    //!           [in] State heap interface
    //! \param    kernelState
    //!           [in] Kernel state describing the kernel data to be loaded
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS MhwInitISH(
        PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
        PMHW_KERNEL_STATE           kernelState);

    //!
    //! \brief    Assigns space in both DSH and SSH to the kernel state
    //! \details  Uses input parameters to assign DSH/SSH regions to the kernel state
    //! \param    stateHeapInterface
    //!           [in] State heap interface
    //! \param    kernelState
    //!           [in] The kernel state to assign the new DSH/ISH regions
    //! \param    noDshSpaceRequested
    //!           [in] No DSH space should be assigned in this call
    //! \param    forcedDshSize
    //!           [in] The size of the DSH space required for this kernel state.
    //!                If this value is 0, the size is calculated from the kernel state.
    //! \param    noSshSpaceRequested
    //!           [in] No SSH space should be assigned in this call
    //! \param    currCmdBufId
    //!           [in] Command buffer Id to keep track of the state heap resource
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS AssignDshAndSshSpace(
        PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
        PMHW_KERNEL_STATE           kernelState,
        bool                        noDshSpaceRequested,
        uint32_t                    forcedDshSize,
        bool                        noSshSpaceRequested,
        uint32_t                    currCmdBufId);
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
    MmioRegistersMfx * SelectVdboxAndGetMmioRegister(
                       MHW_VDBOX_NODE_IND index,
                       PMOS_COMMAND_BUFFER pCmdBuffer);

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
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read MFC status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for Mfc status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadMfcStatus(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read Image status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for Image status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatus(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read BRC PAK statistics for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for BRC PAK statistics specific
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadBrcPakStatistics(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const BrcPakStatsReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
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

    //!
    //! \brief    Read HCP specific BRC PAK statistics for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for BRC PAK statistics specific
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadBrcPakStatisticsForHcp(
        MHW_VDBOX_NODE_IND vdboxIndex,
        const BrcPakStatsReadParams &params,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Set the status tag(start/end) for status report by PIPE contol command,
    //!           this function is for render engine.
    //! \param    osResource
    //!           [in] Reource used in the cmd
    //! \param    offset
    //!           [in] Reource offset used the cmd
    //! \param    tag
    //!           [in] queryStart/queryEnd defined in the media_status_report.h
    //! \param    needFlushCache
    //!           [in] whether need to flush the cache or not. For queryStart, need to flush cache, otherwise
    //!                don't need.
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetStatusTagByPipeCtrl(
        PMOS_RESOURCE osResource,
        uint32_t offset,
        uint32_t tag,
        bool needFlushCache,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Set the status tag(start/end) for status report by MI command
    //!           this function is for vdbox.
    //! \param    osResource
    //!           [in] Reource used in the cmd
    //! \param    offset
    //!           [in] Reource offset used the cmd
    //! \param    tag
    //!           [in] queryStart/queryEnd defined in the media_status_report.h
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetStatusTagByMiCommand(
        MOS_RESOURCE *osResource,
        uint32_t offset,
        uint32_t tag,
        PMOS_COMMAND_BUFFER cmdBuffer);

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
    //! \brief    Check if disable scalability by default
    //! \return   bool
    //!           True if it is to disable scalability by default, else it is not.
    //!
    bool IsDisableScalability()
    {
        return m_disableScalability;
    }

    virtual bool UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard);

    virtual bool Uses2PlanesInputSurfaceFilmGrain()
    {
        return false;
    }

    //!
    //! \brief    Get film grain kernel info
    //! \details  Get kernel base and size
    //!
    //! \param    [out] kernelBase
    //!           base addr of film grain kernels
    //!
    //! \param    [out] kernelSize
    //!           size of film grain kernels
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetFilmGrainKernelInfo(
        uint8_t*    &kernelBase,
        uint32_t    &kernelSize);

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
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    };

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
            uint32_t                        *patchListSize)
    {
        return MOS_STATUS_SUCCESS;
    };


    //! \brief    default disable vdbox balancing by UMD
    bool bEnableVdboxBalancingbyUMD = false;
    
    //! \brief    default disable the get vdbox node by UMD, decided by MHW and MOS
    bool m_getVdboxNodeByUMD = false;

    operator CodechalHwInterfaceNext &()
    {
        if (m_hwInterfaceNext == nullptr)
        {
            CODECHAL_HW_ASSERTMESSAGE("Conversion cannot succeed due to null pointer of m_hwInterfaceNext.");
        }
        return *m_hwInterfaceNext;
    }

    operator CodechalHwInterfaceNext *()
    {
        return m_hwInterfaceNext;
    }

    CodechalHwInterfaceNext *m_hwInterfaceNext = nullptr;

};

extern const MOS_SYNC_PARAMS                        g_cInitSyncParams;

#endif // __CODECHAL_HW_H__
