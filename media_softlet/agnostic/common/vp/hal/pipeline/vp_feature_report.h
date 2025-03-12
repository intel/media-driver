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
//! \file     vp_feature_report.h
//! \brief    vp feature report
//! \details  vp feature report class inlcuding:
//!           features, functions
//!

#ifndef __VP_FEATURE_REPORT_H__
#define __VP_FEATURE_REPORT_H__

#include <stdint.h>
#include "vp_common.h"
#include "vp_common_tools.h"
#include "media_class_trace.h"
#include "vp_common_hdr.h"

//!
//! Class VphalFeatureReport
//! \brief    Vphal Feature Report Class
//!
class VpFeatureReport
{
public:
    struct VP_FEATURES
    {
        bool                          iecp                = false;                        //!< IECP enable/disable
        bool                          ief                 = false;                        //!< Enhancement filter
        bool                          denoise             = false;                        //!< Denoise
        bool                          chromaDenoise       = false;                        //!< Chroma Denoise
        VPHAL_DI_REPORT_MODE          deinterlaceMode     = VPHAL_DI_REPORT_PROGRESSIVE;  //!< Deinterlace mode
        VPHAL_SCALING_MODE            scalingMode         = VPHAL_SCALING_NEAREST;        //!< Scaling mode
        VPHAL_OUTPUT_PIPE_MODE        outputPipeMode      = VPHAL_OUTPUT_PIPE_MODE_COMP;  //!< Output Pipe
        bool                          vpMMCInUse          = false;                        //!< MMC enable flag
        bool                          rtCompressible      = false;                        //!< RT MMC Compressible flag
        uint8_t                       rtCompressMode      = 0;                            //!< RT MMC Compression mode
        bool                          ffdiCompressible    = false;                        //!< FFDI MMC Compressible flag
        uint8_t                       ffdiCompressMode    = 0;                            //!< FFDI MMC Compression mode
        bool                          ffdnCompressible    = false;                        //!< FFDN MMC Compressible flag
        uint8_t                       ffdnCompressMode    = 0;                            //!< FFDN MMC Compression mode
        bool                          stmmCompressible    = false;                        //!< STMM MMC Compressible flag
        uint8_t                       stmmCompressMode    = 0;                            //!< STMM MMC Compression mode
        bool                          scalerCompressible  = false;                        //!< Scaler MMC Compressible flag for Gen10
        uint8_t                       scalerCompressMode  = 0;                            //!< Scaler MMC Compression mode for Gen10
        bool                          primaryCompressible = false;                        //!< Input Primary Surface Compressible flag
        uint8_t                       primaryCompressMode = 0;                            //!< Input Primary Surface Compression mode
        VPHAL_COMPOSITION_REPORT_MODE compositionMode     = VPHAL_NO_COMPOSITION;         //!< Inplace/Legacy Compostion flag
        bool                          veFeatureInUse      = false;                        //!< If any VEBOX feature is in use, excluding pure bypass for SFC
        bool                          diScdMode           = false;                        //!< Scene change detection
        VPHAL_HDR_MODE                hdrMode             = VPHAL_HDR_MODE_NONE;          //!< HDR mode
        bool                          packetReused        = false;                        //!< true if packet reused.
        uint8_t                       rtCacheSetting      = 0;                            //!< Render Target cache usage
#if (_DEBUG || _RELEASE_INTERNAL)
        uint8_t                       rtOldCacheSetting   = 0;                            //!< Render Target old cache usage
        bool                          isOcl3DLut          = false;
        bool                          isOclFC             = false;
        uint32_t                      diffLogOclFC        = 0;
        uint32_t                      featureLogOclFC     = 0;
        bool                          isLegacyFCInUse     = false;
        bool                          fallbackScalingToRender8K = false;
#endif
        bool                          VeboxScalability    = false;                        //!< Vebox Scalability flag
        bool                          VPApogeios          = false;                        //!< VP Apogeios flag
        bool                          sfcLinearOutputByTileConvert = false;               //!< enableSFCLinearOutputByTileConvert
    };

    virtual ~VpFeatureReport(){};

    //!
    //! \brief    VphalFeatureReport Constructor
    //! \details  Creates instance of VphalFeatureReport
    //!
    VpFeatureReport(void *owner = nullptr)
    {
        this->owner = owner;
    };

    //!
    //! \brief    initialize VphalFeatureReport value
    //! \details  initialize VphalFeatureReport value, can use it to reset report value
    //!
    virtual void InitReportValue();

    //!
    //! \brief    set VphalFeatureReport value
    //! \details  set VphalFeatureReport value
    //!
    virtual void SetConfigValues(
        PVP_CONFIG configValues,
        bool       traceEvent = true);

    VP_FEATURES &GetFeatures()
    {
        return m_features;
    }

    void *owner = nullptr;  //!< Pointer to object creating the report

protected:
    VP_FEATURES m_features;

MEDIA_CLASS_DEFINE_END(VpFeatureReport)
};
#endif // __VP_FEATURE_REPORT_H__
