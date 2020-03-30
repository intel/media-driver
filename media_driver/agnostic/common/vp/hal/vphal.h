/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     vphal.h
//! \brief    vphal interface clarification
//! \details  vphal interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_H__
#define __VPHAL_H__

#include "vphal_common.h"
#include "vphal_common_tools.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"

//*-----------------------------------------------------------------------------
//| DEFINITIONS
//*-----------------------------------------------------------------------------
// Incremental size for allocating/reallocating resource
#define VPHAL_BUFFER_SIZE_INCREMENT     128

// YUV input ranges
#define YUV_RANGE_16_235                1
#define YUV_RANGE_0_255                 2
#define YUV_RANGE_FROM_DDI              3

// RGB input ranges
#define RGB_RANGE_16_235                1
#define RGB_RANGE_0_255                 0

// Media Features width
#define VPHAL_RNDR_8K_WIDTH (7680)

// Media Features height
#define VPHAL_RNDR_2K_HEIGHT  1080
// The reason that the definition is not (VPHAL_RNDR_2K_HEIGHT*2) is because some 4K clips have 1200 height.
#define VPHAL_RNDR_4K_HEIGHT  1200
#define VPHAL_RNDR_6K_HEIGHT  (VPHAL_RNDR_2K_HEIGHT*3)
#define VPHAL_RNDR_8K_HEIGHT  (VPHAL_RNDR_2K_HEIGHT*4)
#define VPHAL_RNDR_10K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*5)
#define VPHAL_RNDR_12K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*6)
#define VPHAL_RNDR_14K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*7)
#define VPHAL_RNDR_16K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*8)
#define VPHAL_RNDR_18K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*9)
#define VPHAL_RNDR_20K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*10)
#define VPHAL_RNDR_22K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*11)
#define VPHAL_RNDR_24K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*12)
#define VPHAL_RNDR_26K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*13)
#define VPHAL_RNDR_28K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*14)

//------------------------------------------------------------------------------
// Simplified macros for debug message, Assert, Null check and MOS eStatus check
// within VPhal without the need to explicitly pass comp and sub-comp name
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define VPHAL_HW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _expr)

#define VPHAL_HW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW)

#define VPHAL_HW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _stmt)

#define VPHAL_HW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

#define VPHAL_HW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_PUBLIC sub-comp
//------------------------------------------------------------------------------
#define VPHAL_PUBLIC_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _expr)

#define VPHAL_PUBLIC_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC)

#define VPHAL_PUBLIC_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define VPHAL_DEBUG_ASSERT(_expr)                                                    \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _expr)

#define VPHAL_DEBUG_ASSERTMESSAGE(_message, ...)                                     \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_NORMALMESSAGE(_message, ...)                                     \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_VERBOSEMESSAGE(_message, ...)                                    \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_FUNCTION_ENTER                                                   \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VPHAL_DEBUG_FUNCTION_EXIT                                                    \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VPHAL_DEBUG_CHK_STATUS(_stmt)                                                \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _stmt)

#define VPHAL_DEBUG_CHK_NULL(_ptr)                                                   \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

#define VPHAL_DEBUG_CHK_NULL_NO_STATUS(_ptr)                                         \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define VPHAL_RENDER_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define VPHAL_RENDER_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define VPHAL_RENDER_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DDI sub-comp
//------------------------------------------------------------------------------
#define VP_DDI_ASSERT(_expr)                                                         \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _expr)

#define VP_DDI_ASSERTMESSAGE(_message, ...)                                          \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_NORMALMESSAGE(_message, ...)                                          \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_VERBOSEMESSAGE(_message, ...)                                         \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_FUNCTION_ENTER                                                        \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI)

#define VP_DDI_CHK_STATUS(_stmt)                                                     \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt)

#define VP_DDI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                              \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

#define VP_DDI_CHK_NULL(_ptr)                                                        \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VP_DDI_CHK_NULL_NO_STATUS(_ptr)                                              \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_HR(_ptr)                                                       \
    MOS_CHK_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_NULL_WITH_HR(_ptr)                                             \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

//!
//! \brief Base VP kernel list
//!
enum VpKernelID
{
    // FC
    kernelCombinedFc = 0,

    // 2 VEBOX KERNELS
    kernelVeboxSecureBlockCopy,
    kernelVeboxUpdateDnState,

    // User Ptr
    kernelUserPtr,
    // Fast 1toN
    kernelFast1toN,

    // HDR
    kernelHdrMandatory,
    kernelHdrPreprocess,

    baseKernelMaxNumID
};

//!
//! \brief VPHAL SS/EU setting
//!
struct VphalSseuSetting
{
    uint8_t   numSlices;
    uint8_t   numSubSlices;
    uint8_t   numEUs;
    uint8_t   reserved;       // Place holder for frequency setting
};

//-----------------------------------------------------------------------------
// VPHAL-DDI RENDERING INTERFACE
//
//      Params that may apply to more than one layer are part of VPHAL_SURFACE
//      DDI layers must set this interface before calling pfnRender
//-----------------------------------------------------------------------------
//!
//! Structure VphalSettings
//! \brief VPHAL Settings - controls allocation of internal resources in VPHAL
//!
struct VphalSettings
{
    //!
    //! \brief    VphalSettings Constructor
    //! \details  Creates instance of VphalSettings
    //!
    VphalSettings() :
        maxPhases(0),
        mediaStates(0),
        sameSampleThreshold(0),
        disableDnDi(0),
        kernelUpdate(0),
        disableHdr(0),
        veboxParallelExecution(0)
    {
    };

    int32_t                maxPhases;
    int32_t                mediaStates;
    int32_t                sameSampleThreshold;
    uint32_t               disableDnDi;                                          //!< For validation purpose
    uint32_t               kernelUpdate;                                         //!< For VEBox Copy and Update kernels
    uint32_t               disableHdr;                                           //!< Disable Hdr
    uint32_t               veboxParallelExecution;                               //!< Control VEBox parallel execution with render engine
};

#pragma pack(push)
#pragma pack(1)

//!
//! Structure VphalFeatureReport
//! \brief    Vphal Feature Report Structure
//!
struct VphalFeatureReport
{
    //!
    //! \brief    VphalFeatureReport Constructor
    //! \details  Creates instance of VphalFeatureReport
    //!
    VphalFeatureReport()
    {
        // call InitReportValue() to initialize report value
        InitReportValue();
    };

    //!
    //! \brief    initialize VphalFeatureReport value
    //! \details  initialize VphalFeatureReport value, can use it to reset report value
    //!
    void InitReportValue();

    bool                            IECP;               //!< IECP enable/disable
    bool                            IEF;                //!< Enhancement filter
    bool                            Denoise;            //!< Denoise
    bool                            ChromaDenoise;      //!< Chroma Denoise
    VPHAL_DI_REPORT_MODE            DeinterlaceMode;    //!< Deinterlace mode
    VPHAL_SCALING_MODE              ScalingMode;        //!< Scaling mode
    VPHAL_OUTPUT_PIPE_MODE          OutputPipeMode;     //!< Output Pipe
    bool                            VPMMCInUse;         //!< MMC enable flag
    bool                            RTCompressible;     //!< RT MMC Compressible flag
    uint8_t                         RTCompressMode;     //!< RT MMC Compression mode
    bool                            FFDICompressible;   //!< FFDI MMC Compressible flag
    uint8_t                         FFDICompressMode;   //!< FFDI MMC Compression mode
    bool                            FFDNCompressible;   //!< FFDN MMC Compressible flag
    uint8_t                         FFDNCompressMode;   //!< FFDN MMC Compression mode
    bool                            STMMCompressible;   //!< STMM MMC Compressible flag
    uint8_t                         STMMCompressMode;   //!< STMM MMC Compression mode
    bool                            ScalerCompressible; //!< Scaler MMC Compressible flag for Gen10
    uint8_t                         ScalerCompressMode; //!< Scaler MMC Compression mode for Gen10
    bool                            PrimaryCompressible;//!< Input Primary Surface Compressible flag
    uint8_t                         PrimaryCompressMode;//!< Input Primary Surface Compression mode
    VPHAL_COMPOSITION_REPORT_MODE   CompositionMode;    //!< Inplace/Legacy Compostion flag
    bool                            VEFeatureInUse;     //!< If any VEBOX feature is in use, excluding pure bypass for SFC
    bool                            DiScdMode;          //!< Scene change detection
    VPHAL_HDR_MODE                  HDRMode;            //!< HDR mode
};

#pragma pack(pop)


//!
//! Class VphalState
//! \brief VPHAL class definition
//!
class VphalState
{
public:
    // factory function
    static VphalState* VphalStateFactory(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    VphalState Constructor
    //! \details  Creates instance of VphalState
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be nullptr
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] peStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalState(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    Copy constructor
    //!
    VphalState(const VphalState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    VphalState& operator=(const VphalState&) = delete;

    //!
    //! \brief    VphalState Destuctor
    //! \details  Destroys VPHAL and all internal states and objects
    //!
    virtual ~VphalState();

    //!
    //! \brief    Allocate VPHAL Resources
    //! \details  Allocate VPHAL Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Allocate(
        const VphalSettings     *pVpHalSettings);

    //!
    //! \brief    Performs VP Rendering
    //! \details  Performs VP Rendering
    //!           - call default render of video
    //! \param    [in] pcRenderParams
    //!           Pointer to Render Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    //!
    //! \brief    Get feature reporting from renderer
    //! \details  Get feature reporting from renderer
    //! \return   VphalFeatureReport*
    //!           Pointer to VPHAL_FEATURE_REPOR: rendering features reported
    //!
    virtual VphalFeatureReport*       GetRenderFeatureReport();

    //!
    //! \brief    Get Status Report
    //! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
    //! \param    [out] pQueryReport
    //!           Pointer to pQueryReport, the status query report array.
    //! \param    [in] wStatusNum
    //!           The size of array pQueryReport.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    virtual MOS_STATUS GetStatusReport(
        PQUERY_STATUS_REPORT_APP        pQueryReport,
        uint16_t                        numStatus);

    //!
    //! \brief    Get Status Report's entry length from head to tail
    //! \details  Get Status Report's entry length from head to tail
    //! \param    [out] puiLength
    //!           Pointer to the entry length
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS GetStatusReportEntryLength(
        uint32_t                         *puiLength);

    MEDIA_FEATURE_TABLE*          GetSkuTable()
    {
        return m_skuTable;
    };

    PMOS_INTERFACE              GetOsInterface()
    {
        return m_osInterface;
    };

    VphalRenderer*             GetRenderer()
    {
        return m_renderer;
    };

    void SetMhwVeboxInterface(MhwVeboxInterface* veboxInterface)
    {
        if (veboxInterface == nullptr)
        {
            return;
        }

        if (m_veboxInterface != nullptr)
        {
            MOS_STATUS eStatus = m_veboxInterface->DestroyHeap();
            MOS_Delete(m_veboxInterface);
            m_veboxInterface = nullptr;
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
            }
        }

        m_veboxInterface = veboxInterface;
    }

    void SetMhwSfcInterface(MhwSfcInterface* sfcInterface)
    {
        if (sfcInterface == nullptr)
        {
            return;
        }

        if (m_sfcInterface != nullptr)
        {
            MOS_Delete(m_sfcInterface);
            m_sfcInterface = nullptr;
        }

        m_sfcInterface = sfcInterface;
    }

    HANDLE                      m_gpuAppTaskEvent;

protected:
    // Internals
    PLATFORM                    m_platform;
    MEDIA_FEATURE_TABLE         *m_skuTable;
    MEDIA_WA_TABLE              *m_waTable;

    // States
    PMOS_INTERFACE              m_osInterface;
    PRENDERHAL_INTERFACE        m_renderHal;
    PMHW_VEBOX_INTERFACE        m_veboxInterface;
    MhwCpInterface              *m_cpInterface;
    PMHW_SFC_INTERFACE          m_sfcInterface;
    VphalRenderer               *m_renderer;

    // Render GPU context/node
    MOS_GPU_NODE                m_renderGpuNode;
    MOS_GPU_CONTEXT             m_renderGpuContext;

    // StatusTable indicating if command is done by gpu or not
    VPHAL_STATUS_TABLE          m_statusTable = {};

    //!
    //! \brief    Create instance of VphalRenderer
    //! \details  Create instance of VphalRenderer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CreateRenderer() = 0;
};

#endif  // __VPHAL_H__
