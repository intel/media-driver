/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     vphal_common_tools.h
//! \brief    vphal tools interface clarification
//! \details  vphal tools interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_COMMON_TOOLS_H__
#define __VPHAL_COMMON_TOOLS_H__

// max size of status table, this value must be power of 2 such as 256, 512, 1024, etc.
// so VPHAL_STATUS_TABLE_MAX_SIZE-1 can form a one-filled mask to wind back a VPHAL_STATUS_TABLE ring table.
#define VPHAL_STATUS_TABLE_MAX_SIZE    512

//!
//! Structure VPHAL_STATUS_ENTRY
//! \brief Pre-Processing - Query status struct
//!
typedef struct _VPHAL_STATUS_ENTRY
{
    uint32_t        StatusFeedBackID;
    MOS_GPU_CONTEXT GpuContextOrdinal;
    uint32_t        dwTag;          // software tag, updated by driver for every command submit.
    uint32_t        dwStatus;       // 0:OK; 1:Not Ready; 2:Not Available; 3:Error;
    uint16_t        streamIndex;    // stream index corresponding to the gpucontext
    bool            isStreamIndexSet;
 } VPHAL_STATUS_ENTRY, *PVPHAL_STATUS_ENTRY;

//!
//! \brief Structure to VPHAL Status table
//!
typedef struct _VPHAL_STATUS_TABLE
{
    VPHAL_STATUS_ENTRY  aTableEntries[VPHAL_STATUS_TABLE_MAX_SIZE];
    uint32_t            uiHead;
    uint32_t            uiCurrent;
} VPHAL_STATUS_TABLE, *PVPHAL_STATUS_TABLE;

//!
//! Structure STATUS_TABLE_UPDATE_PARAMS
//! \brief Pre-Processing - params for updating status report table
//!
typedef struct _STATUS_TABLE_UPDATE_PARAMS
{
    bool                bReportStatus;
    bool                bSurfIsRenderTarget;
    PVPHAL_STATUS_TABLE pStatusTable;
    uint32_t            StatusFeedBackID;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                bTriggerGPUHang;
#endif
    bool                bUpdateStreamIndex;
 } STATUS_TABLE_UPDATE_PARAMS, *PSTATUS_TABLE_UPDATE_PARAMS;

//!
//! \brief Structure to query status params from application
//! be noted that the structure is defined by app (msdk) so we cannot reorder its entries or size
//!
typedef struct _QUERY_STATUS_REPORT_APP
{
    uint32_t StatusFeedBackID;
    uint32_t dwStatus          : 8;  //!< 0: OK; 1: Not Ready; 2: Not Available; 3: Error;
    uint32_t                   : 24; //!< Reserved
    uint32_t dwReserved[4];          //!< keep this to align what application (msdk lib) defined
} QUERY_STATUS_REPORT_APP, *PQUERY_STATUS_REPORT_APP;

//!
//! \brief VPreP status
//!
typedef enum _VPREP_STATUS
{
    VPREP_OK           = 0,
    VPREP_NOTREADY     = 1,
    VPREP_NOTAVAILABLE = 2,
    VPREP_ERROR        = 3
} VPREP_STATUS;

//!
//! \brief Internal Override/Reporting Video Processing Configuration Values
//!
typedef struct _VP_CONFIG
{
    bool       bVpComponentReported;       // Vp Component has been reported
    uint32_t   dwVpPath;                   // Video Processing path
    uint32_t   dwVpComponent;              // Video Processing Component
    uint32_t   dwCreatedDeinterlaceMode;   // Created Deinterlace mode
    uint32_t   dwCurrentDeinterlaceMode;   // Current Deinterlace mode
    uint32_t   dwReportedDeinterlaceMode;  // Reported Deinterlace mode
    uint32_t   dwCreatedScalingMode;       // Created Scaling mode
    uint32_t   dwCurrentScalingMode;       // Current Scaling mode
    uint32_t   dwReportedScalingMode;      // Reported Scaling mode
    uint32_t   dwReportedFastCopyMode;     // Reported FastCopy mode
    uint32_t   dwCurrentXVYCCState;        // Current xvYCC State
    uint32_t   dwReportedXVYCCState;       // Reported xvYCC state
    uint32_t   dwCurrentOutputPipeMode;    // Current Output Pipe Mode
    uint32_t   dwReportedOutputPipeMode;   // Reported Ouput Pipe Mode
    uint32_t   dwCurrentVEFeatureInUse;    // Current VEFeatureInUse
    uint32_t   dwReportedVEFeatureInUse;   // Reported VEFeatureInUse
    uint32_t   dwCurrentFrcMode;           // Current Frame Rate Conversion Mode
    uint32_t   dwReportedFrcMode;          // Reported Frame Rate Conversion Mode
    uint32_t   dwVPMMCInUse;               // Memory compression enable flag
    uint32_t   dwVPMMCInUseReported;       // Reported Memory compression enable flag
    uint32_t   dwRTCompressible;           // RT MMC Compressible flag
    uint32_t   dwRTCompressibleReported;   // RT MMC Reported compressible flag
    uint32_t   dwRTCompressMode;           // RT MMC Compression Mode
    uint32_t   dwRTCompressModeReported;   // RT MMC Reported Compression Mode
    uint32_t   dwFFDICompressible;         // FFDI Compressible flag
    uint32_t   dwFFDICompressMode;         // FFDI Compression mode
    uint32_t   dwFFDNCompressible;         // FFDN Compressible flag
    uint32_t   dwFFDNCompressMode;         // FFDN Compression mode
    uint32_t   dwSTMMCompressible;         // STMM Compressible flag
    uint32_t   dwSTMMCompressMode;         // STMM Compression mode
    uint32_t   dwScalerCompressible;       // Scaler Compressible flag for Gen10
    uint32_t   dwScalerCompressMode;       // Scaler Compression mode for Gen10
    uint32_t   dwPrimaryCompressible;      // Input Primary Surface Compressible flag
    uint32_t   dwPrimaryCompressMode;      // Input Primary Surface Compression mode
    uint32_t   dwFFDICompressibleReported; // FFDI Reported Compressible flag
    uint32_t   dwFFDICompressModeReported; // FFDI Reported Compression mode
    uint32_t   dwFFDNCompressibleReported; // FFDN Reported Compressible flag
    uint32_t   dwFFDNCompressModeReported; // FFDN Reported Compression mode
    uint32_t   dwSTMMCompressibleReported; // STMM Reported Compressible flag
    uint32_t   dwSTMMCompressModeReported; // STMM Reported Compression mode
    uint32_t   dwScalerCompressibleReported;   // Scaler Reported Compressible flag for Gen10
    uint32_t   dwScalerCompressModeReported;   // Scaler Reported Compression mode for Gen10
    uint32_t   dwPrimaryCompressibleReported;  // Input Primary Surface Reported Compressible flag
    uint32_t   dwPrimaryCompressModeReported;  // Input Primary Surface Reported Compression mode
    uint32_t   dwCapturePipeInUse;         // Capture pipe
    uint32_t   dwCapturePipeInUseReported; // Reported Capture pipe
    uint32_t   dwCurrentCompositionMode;   // In Place or Legacy Composition
    uint32_t   dwReportedCompositionMode;  // Reported Composition Mode
    uint32_t   dwCurrentHdrMode;           // Current Hdr Mode
    uint32_t   dwReportedHdrMode;          // Reported Hdr Mode
    uint32_t   dwCurrentScdMode;           // Current Scd Mode
    uint32_t   dwReportedScdMode;          // Reported Scd Mode
    uint32_t   dwTCCPreprocessInUse;                // Vebox TCC Pre-process for HDR
    uint32_t   dwTCCPreprocessInUseReported;        // Reported Vebox TCC Pre-process for HDR
    uint32_t   dwIEFPreprocessInUse;                // Vebox IEF Pre-process for HDR
    uint32_t   dwIEFPreprocessInUseReported;        // Reported Vebox IEF Pre-process for HDR
    bool       bAdvancedScalingInUse;              // Advanced Scaling Enabled
    bool       bAdvancedScalingInUseReported;      // Reported Advanced Scaling Enabled

    // Configurations for cache control
    uint32_t   dwDndiReferenceBuffer;
    uint32_t   dwDndiOutputBuffer;
    uint32_t   dwIecpOutputBuffer;
    uint32_t   dwDnOutputBuffer;
    uint32_t   dwStmmBuffer;
    uint32_t   dwPhase2RenderTarget;
    uint32_t   dwPhase2Source;
    uint32_t   dwPhase1Source;

    // For Deinterlace Mode - the flags reflect the content size and SKU,
    // should not be changed after initialized.
    bool       bFFDI;

    //Debug enhancement to force color fill
    //FALSE(0): no force color fill, TRUE(1): force color fill with default color,
    //ELSE(other non-zero value): force color fill with color info from dwForceColorFill
    uint32_t   dwForceColorFill;

    //VEBOX perf is not enough for 8K@60fps processing
    //add config to switch 8K resolution on VEBOX or render
    //default is use render for 8k
    uint32_t   dwUseVeboxFor8K;
} VP_CONFIG, *PVP_CONFIG;

//!
//! \brief status query param
//!
typedef struct _VPHAL_STATUS_PARAM
{
    uint32_t            FrameId;
    VPREP_STATUS        BltStatus;
} VPHAL_STATUS_PARAM, *PVPHAL_STATUS_PARAM;

//!
//! Structure VPHAL_QUERYVARIANCE_PARAMS
//! \brief Query Variance Parameters
//!
typedef struct _VPHAL_QUERYVARIANCE_PARAMS
{
    uint32_t            dwFrameNumber;
    void*               pVariances;
} VPHAL_QUERYVARIANCE_PARAMS, *PVPHAL_QUERYVARIANCE_PARAMS;

//!
//! \brief Query Multiple Variance Parameters
//!
typedef struct _VPHAL_BATCHQUERYVARIANCE_PARAMS
{
    uint32_t            FrameCount;
    uint32_t            BufferSize;
    void                *pBuffer;
} VPHAL_BATCHQUERYVARIANCE_PARAMS, *PVPHAL_BATCHQUERYVARIANCE_PARAMS;


#endif  // __VPHAL_COMMON_TOOLS_H__
