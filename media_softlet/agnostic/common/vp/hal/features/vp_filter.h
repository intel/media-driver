/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     vp_filter.h
//! \brief    Defines the common interface for vp filters
//!           this file is for the base interface which is shared by all features.
//!
#ifndef __VP_FILTER_H__
#define __VP_FILTER_H__

#include <map>
#include "mos_defs.h"
#include "vp_pipeline_common.h"
#include "vp_sfc_common.h"
#include "vp_render_common.h"
#include "vp_utils.h"
#include "sw_filter.h"
#include "vp_feature_caps.h"
#include "vp_render_fc_types.h"

namespace vp {

#define ESR_LAYER_NUM       10
#define VPHAL_MAX_HDR_INPUT_LAYER 8
#define VPHAL_HDR_EOTF_1DLUT_POINT_NUMBER 256
#define VPHAL_HDR_OETF_1DLUT_POINT_NUMBER 256
#define VPHAL_HDR_OETF_1DLUT_WIDTH 16
#define VPHAL_HDR_OETF_1DLUT_HEIGHT 16

class VpCmdPacket;

class VpFilter
{
public:
    VpFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    virtual ~VpFilter() {};

    //!
    //! \brief  Initialize the media filter, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() = 0;

    //!
    //! \brief  Prepare the parameters for filter generation
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() = 0;

    //!
    //! \brief  Destroy the media Filter and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() = 0;

    //!
    //! \brief  Get execute caps for this filter
    //! \return VP_EXECUTE_CAPS
    //!         return the caps of filters
    //!
    VP_EXECUTE_CAPS GetExecuteCaps()
    {
        return m_executeCaps;
    }

    //!
    //! \brief  Get current associated media Packet
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    VpCmdPacket * GetActivePacket()
    {
        return m_packet;
    }

    //!
    //! \brief  Set current associated media Packet
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    void SetPacket(VpCmdPacket* packet)
    {
        m_packet = packet;
    }

    //!
    //! \brief  Get current associated media Packet
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    PVP_MHWINTERFACE GetMhwInterface()
    {
        return m_pvpMhwInterface;
    }

protected:

    PVP_MHWINTERFACE      m_pvpMhwInterface = nullptr;   // vp HW interfaces
    VP_EXECUTE_CAPS       m_executeCaps = {};        // Filter executed caps
    PVPHAL_SURFACE        m_tempSurface = nullptr;   // Inter-Media surface for Filter temp output

    VpCmdPacket         * m_packet = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpFilter)
};

struct _SFC_SCALING_PARAMS
{
    // Scaling parameters
    uint32_t                        dwOutputFrameHeight;                        // Output Frame Height
    uint32_t                        dwOutputFrameWidth;                         // Output Frame Width
    uint32_t                        dwInputFrameHeight;                         // Input Frame Height
    uint32_t                        dwInputFrameWidth;                          // Input Frame Width
    MOS_FORMAT                      inputFrameFormat;                           // Input Frame Format

    bool                            bBilinearScaling;                           // true if bilinear scaling, otherwise avs scaling.
    uint32_t                        dwSourceRegionHeight;                       // Source/Crop region height
    uint32_t                        dwSourceRegionWidth;                        // Source/Crop region width
    uint32_t                        dwSourceRegionVerticalOffset;               // Source/Crop region vertical offset
    uint32_t                        dwSourceRegionHorizontalOffset;             // Source/Crop region horizontal offset
    uint32_t                        dwScaledRegionHeight;                       // Scaled region height
    uint32_t                        dwScaledRegionWidth;                        // Scaled region width
    uint32_t                        dwScaledRegionVerticalOffset;               // Scaled region vertical offset
    uint32_t                        dwScaledRegionHorizontalOffset;             // Scaled region horizontal offset
    uint32_t                        dwTargetRectangleStartHorizontalOffset;     // Target rectangle start horizontal offset
    uint32_t                        dwTargetRectangleEndHorizontalOffset;       // Target rectangle end horizontal offset
    uint32_t                        dwTargetRectangleStartVerticalOffset;       // Target rectangle start vertical offset
    uint32_t                        dwTargetRectangleEndVerticalOffset;         // Target rectangle end vertical offset
    bool                            bRectangleEnabled;                          // Target rectangle enabled
    float                           fAVSXScalingRatio;                          // X Scaling Ratio
    float                           fAVSYScalingRatio;                          // Y Scaling Ratio

    SFC_COLORFILL_PARAMS            sfcColorfillParams;                         // Colorfill Params

    VPHAL_SCALING_MODE              sfcScalingMode;                             // Bilinear, Nearest, AVS and future extension (configured by AVS coefficients)
    //Interlaced scaling parameters
    uint32_t                        interlacedScalingType;
    VPHAL_SAMPLE_TYPE               srcSampleType;
    VPHAL_SAMPLE_TYPE               dstSampleType;
    bool                            isDemosaicNeeded;                           // 0: demosaic is not needed; 1: demosaic is needed
    bool                            b1stPassOfSfc2PassScaling;                  // 1st Pass of Sfc 2Pass Scaling
};

struct _SFC_CSC_PARAMS
{
    bool                            bCSCEnabled;                                 // CSC Enabled
    bool                            isInputColorSpaceRGB;                        // 0: YUV color space, 1:RGB color space
    bool                            bIEFEnable;                                  // IEF Enabled
    bool                            bChromaUpSamplingEnable;                     // ChromaUpSampling
    bool                            b8tapChromafiltering;                        // Enables 8 tap filtering for Chroma Channels
    bool                            isDitheringNeeded;                           // 0: dithering is not needed; 1: dithering is needed
    VPHAL_CSPACE                    inputColorSpace;                             // Input Color Space
    MOS_FORMAT                      inputFormat;                                 // SFC Input Format
    MOS_FORMAT                      outputFormat;                                // SFC Output Format
    PVPHAL_IEF_PARAMS               iefParams;                                   // Vphal Params
    uint32_t                        sfcSrcChromaSiting;                          // SFC Source Chroma Siting location
    uint32_t                        chromaDownSamplingVerticalCoef;              // Chroma DownSampling Vertical Coeff
    uint32_t                        chromaDownSamplingHorizontalCoef;            // Chroma DownSampling Horizontal Coeff
    bool                            isFullRgbG10P709;                            // Whether output colorspace is DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709
    bool                            isDemosaicNeeded;                            // 0: demosaic is not needed; 1: demosaic is needed       
};

struct _SFC_ROT_MIR_PARAMS
{
    VPHAL_ROTATION                  rotationMode;                               // Rotation mode -- 0, 90, 180 or 270
    uint32_t                        mirrorType;                               // Mirror Type -- vert/horiz
    bool                            bMirrorEnable;                              // Mirror mode -- enable/disable
};

struct _VEBOX_DN_PARAMS
{
    bool                            bDnEnabled;
    bool                            bChromaDenoise;                             // bEnableChroma && bEnableLuma
    bool                            bAutoDetect;
    float                           fDenoiseFactor;
    VPHAL_NOISELEVEL                NoiseLevel;
    bool                            bEnableHVSDenoise;
    VPHAL_HVSDENOISE_PARAMS         HVSDenoise;
    bool                            bProgressive;
};

struct _VEBOX_STE_PARAMS
{
    bool                            bEnableSTE;                                 // STE Enabled
    uint32_t                        dwSTEFactor;

    bool                            bEnableSTD;                                 // STD alone Enabled
    VPHAL_STD_PARAMS                STDParam;
};

struct _VEBOX_DI_PARAMS
{
    bool                            bDiEnabled;                                 // DI Enabled
    VPHAL_SAMPLE_TYPE               sampleTypeInput;
    bool                            b60fpsDi;
    VPHAL_DI_MODE                   diMode;                                     //!< DeInterlacing mode
    bool                            enableFMD;                                  //!< FMD
    bool                            bSCDEnabled;                                //!< Scene change detection
    bool                            bHDContent;
    bool                            bEnableQueryVariance;                       //!< Query variance enable
};

struct _VEBOX_ACE_PARAMS
{
    bool                            bEnableACE;                                 // ACE Enabled
    bool                            bAceLevelChanged;
    uint32_t                        dwAceLevel;
    uint32_t                        dwAceStrength;
    bool                            bAceHistogramEnabled;
    bool                            bEnableLACE;
};

struct _VEBOX_TCC_PARAMS
{
    bool                            bEnableTCC;                                 // TCC Enabled
    uint8_t                         Red;
    uint8_t                         Green;
    uint8_t                         Blue;
    uint8_t                         Cyan;
    uint8_t                         Magenta;
    uint8_t                         Yellow;
};

struct _VEBOX_CGC_PARAMS
{
    bool                                bEnableCGC;                                 // CGC Enabled
    bool                                bBt2020ToRGB;                               // Bt2020 convert to sRGB
    VPHAL_CSPACE                        inputColorSpace;
    VPHAL_CSPACE                        outputColorSpace;
    MOS_FORMAT                          inputFormat;
    MOS_FORMAT                          outputFormat;
    bool                                bExtendedSrcGamut;
    bool                                bExtendedDstGamut;
    VPHAL_GAMUT_MODE                    GCompMode;
    uint32_t                            dwAttenuation;
    float                               displayRGBW_x[4];
    float                               displayRGBW_y[4];
};

struct _VEBOX_PROCAMP_PARAMS
{
    bool                            bEnableProcamp;                            // Procamp Enabled
    float                           fBrightness;
    float                           fContrast;
    float                           fHue;
    float                           fSaturation;
};

enum class VEBOX_CSC_BLOCK_TYPE
{
    DEFAULT,
    BACK_END,
    FRONT_END
};

struct _VEBOX_CSC_PARAMS
{
    bool                            bCSCEnabled;                                 // CSC Enabled
    VPHAL_CSPACE                    inputColorSpace;                             // Input Color Space
    VPHAL_CSPACE                    outputColorSpace;                            // Input Color Space
    MOS_FORMAT                      inputFormat;                                 // Input Format
    MOS_FORMAT                      outputFormat;                                // Output Format
    PVPHAL_ALPHA_PARAMS             alphaParams;                                 // Output Alpha Params
    bool                            bypassCUS;                                   // Bypass Chroma up sampling
    bool                            bypassCDS;                                   // Bypass Chroma down sampling
    uint32_t                        chromaUpSamplingVerticalCoef;                // Chroma UpSampling Vertical Coeff
    uint32_t                        chromaUpSamplingHorizontalCoef;              // Chroma UpSampling Horizontal Coeff
    uint32_t                        chromaDownSamplingVerticalCoef;              // Chroma DownSampling Vertical Coeff
    uint32_t                        chromaDownSamplingHorizontalCoef;            // Chroma DownSampling Horizontal Coeff
    VEBOX_CSC_BLOCK_TYPE            blockType;                                   // Use Back End CSC or Front End CSC
};

struct _RENDER_CSC_PARAMS
{
    uint32_t                        layer;
    bool                            bCSCEnabled;                                 // CSC Enabled
    VPHAL_CSPACE                    inputColorSpcase;                            // Input Color Space
    VPHAL_CSPACE                    outputColorSpcase;                           // Input Color Space
    MOS_FORMAT                      inputFormat;                                 // Input Format
    MOS_FORMAT                      outputFormat;                                // Output Format
    PVPHAL_ALPHA_PARAMS             alphaParams;                                 // Output Alpha Params
    uint32_t                        inputChromaSetting;                          // Chroma setting
};

struct _VEBOX_HDR_PARAMS
{
    uint32_t                        uiMaxDisplayLum;       //!< Maximum Display Luminance
    uint32_t                        uiMaxContentLevelLum;  //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE                  hdrMode;
    VPHAL_CSPACE                    srcColorSpace;
    VPHAL_CSPACE                    dstColorSpace;
    MOS_FORMAT                      dstFormat;
    HDR_STAGE                       stage;
    uint32_t                        lutSize;
    bool                            isFp16Enable;
    PVPHAL_3DLUT_PARAMS             external3DLutParams;
};

using SFC_SCALING_PARAMS    = _SFC_SCALING_PARAMS;
using PSFC_SCALING_PARAMS   = SFC_SCALING_PARAMS * ;
using SFC_CSC_PARAMS        = _SFC_CSC_PARAMS;
using PSFC_CSC_PARAMS       = SFC_CSC_PARAMS * ;
using SFC_ROT_MIR_PARAMS    = _SFC_ROT_MIR_PARAMS;
using PSFC_ROT_MIR_PARAMS   = SFC_ROT_MIR_PARAMS * ;
using VEBOX_DN_PARAMS       = _VEBOX_DN_PARAMS;
using PVEBOX_DN_PARAMS      = VEBOX_DN_PARAMS *;
using VEBOX_STE_PARAMS      = _VEBOX_STE_PARAMS;
using PVEBOX_STE_PARAMS     = VEBOX_STE_PARAMS *;
using VEBOX_DI_PARAMS       = _VEBOX_DI_PARAMS;
using PVEBOX_DI_PARAMS      = VEBOX_DI_PARAMS *;
using VEBOX_ACE_PARAMS      = _VEBOX_ACE_PARAMS;
using PVEBOX_ACE_PARAMS     = VEBOX_ACE_PARAMS *;
using VEBOX_TCC_PARAMS      = _VEBOX_TCC_PARAMS;
using PVEBOX_TCC_PARAMS     = VEBOX_TCC_PARAMS *;
using VEBOX_CGC_PARAMS      = _VEBOX_CGC_PARAMS;
using PVEBOX_CGC_PARAMS     = VEBOX_CGC_PARAMS *;
using VEBOX_PROCAMP_PARAMS  = _VEBOX_PROCAMP_PARAMS;
using PVEBOX_PROCAMP_PARAMS = VEBOX_PROCAMP_PARAMS *;
using VEBOX_CSC_PARAMS      = _VEBOX_CSC_PARAMS;
using PVEBOX_CSC_PARAMS     = VEBOX_CSC_PARAMS *;

using KERNEL_ARGS = std::vector<KRN_ARG>;
using KERNEL_BTIS = std::map<uint32_t,uint32_t>;
using KERNEL_INDEX_ARG_MAP              = std::map<uint32_t,KRN_ARG>;
using MULTI_LAYERS_KERNEL_INDEX_ARG_MAP = std::map<uint32_t,KERNEL_INDEX_ARG_MAP>;

struct _VEBOX_UPDATE_PARAMS
{
    VEBOX_DN_PARAMS                 denoiseParams;
    VP_EXECUTE_CAPS                 veboxExecuteCaps;
    VpKernelID                      kernelId;
};

using VEBOX_UPDATE_PARAMS                 = _VEBOX_UPDATE_PARAMS;
using PVEBOX_UPDATE_PARAMS                = VEBOX_UPDATE_PARAMS *;
using VEBOX_HDR_PARAMS                    = _VEBOX_HDR_PARAMS;
using PVEBOX_HDR_PARAMS                   = VEBOX_HDR_PARAMS *;

struct _RENDER_HDR_3DLUT_CAL_PARAMS
{
    uint32_t                        maxDisplayLum;       //!< Maximum Display Luminance
    uint32_t                        maxContentLevelLum;  //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE                  hdrMode;
    VpKernelID                      kernelId;
    uint32_t                        threadWidth;
    uint32_t                        threadHeight;
    uint32_t                        localWidth;
    uint32_t                        localHeight;
    KERNEL_ARGS                     kernelArgs;
    void                            Init();
};

using RENDER_HDR_3DLUT_CAL_PARAMS  = _RENDER_HDR_3DLUT_CAL_PARAMS;
using PRENDER_HDR_3DLUT_CAL_PARAMS = RENDER_HDR_3DLUT_CAL_PARAMS *;

struct _RENDER_DN_HVS_CAL_PARAMS
{
    uint32_t                        qp;
    uint32_t                        strength;
    VPHAL_HVSDN_MODE                hvsMode;
    VpKernelID                      kernelId;
    uint32_t                        threadWidth;
    uint32_t                        threadHeight;
    KERNEL_ARGS                     kernelArgs;
};

using RENDER_DN_HVS_CAL_PARAMS  = _RENDER_DN_HVS_CAL_PARAMS;
using PRENDER_DN_HVS_CAL_PARAMS = RENDER_DN_HVS_CAL_PARAMS *;

struct _STATE_COPY_PARAMS
{
    bool needed;
    VpKernelID                      kernelId;
};

using STATE_COPY_PARAMS = _STATE_COPY_PARAMS;
using PSTATE_COPY_PARAMS = STATE_COPY_PARAMS*;

struct SR_LAYER_PARAMS
{
    uint32_t                                          uLayerID;
    KERNEL_ARGS                                       kernelArgs;
    SurfaceIndex                                      outputSurface;
    uint32_t                                          uWidth;
    uint32_t                                          uHeight;
    MOS_FORMAT                                        format;
    std::string                                       sKernelName;
    VpKernelID                                        kernelId;
    SurfaceIndex                                      weightBuffer;
    uint32_t                                          uWeightBufferSize;
    uint32_t                                          uOutChannels;
    uint32_t                                          uInChannels;
    uint32_t                                          uWeightsPerChannel;

    SurfaceIndex                                      biasBuffer;
    uint32_t                                          uBiasBufferSize;
    SurfaceIndex                                      reluBuffer;
    uint32_t                                          uReluBufferSize;

    uint32_t                                          uThreadWidth;
    uint32_t                                          uThreadHeight;

    uint16_t                                          imgDim[2];
    uint16_t                                          channelDim[2];

    float                                             reluValue;
    uint16_t                                          relu;
    uint16_t                                          addToOutput;
};

struct CHROMA_LAYER_PARAMS
{
    uint32_t                                          uLayerID;
    KERNEL_ARGS                                       kernelArgs;
    SurfaceIndex                                      inputSRYSurface;
    SurfaceIndex                                      inputUSurface;
    SurfaceIndex                                      inputVSurface;
    SurfaceIndex                                      outputSurface;
    uint16_t                                          kernelFormat;
    float                                             fDeltaU;
    float                                             fDeltaV;
    float                                             fShiftU;
    float                                             fShiftV;
    float                                             fScaleX;
    float                                             fScaleY;
    float                                             fChromaScaleX;
    float                                             fChromaScaleY;
    float                                             original_x;
    float                                             original_y;
    float                                             fScaleRatioX;
    float                                             fScaleRatioY;

    std::string                                       sKernelName;
    VpKernelID                                        kernelId;

    uint32_t                                          uThreadWidth;
    uint32_t                                          uThreadHeight;
};

struct _RENDER_FC_PARAMS
{
    VpKernelID              kernelId;
    VP_COMPOSITE_PARAMS     compParams;
};
using RENDER_FC_PARAMS  = _RENDER_FC_PARAMS;
using PRENDER_FC_PARAMS = RENDER_FC_PARAMS *;


struct OCL_FC_KERNEL_CONFIG
{
    VPHAL_PERFTAG perfTag = VPHAL_NONE;
};

struct OCL_FC_KERNEL_PARAM
{
    KERNEL_ARGS                  kernelArgs;
    std::string                  kernelName;
    VpKernelID                   kernelId;
    uint32_t                     threadWidth;
    uint32_t                     threadHeight;
    uint32_t                     localWidth;
    uint32_t                     localHeight;
    KERNEL_ARG_INDEX_SURFACE_MAP kernelStatefulSurfaces;
    OCL_FC_KERNEL_CONFIG         kernelConfig;
    void                         Init();
};

using OCL_FC_KERNEL_PARAMS = std::vector<OCL_FC_KERNEL_PARAM>;
struct _RENDER_OCL_FC_PARAMS
{
    OCL_FC_KERNEL_PARAMS fc_kernelParams = {};
    void                 Init();
};
using RENDER_OCL_FC_PARAMS  = _RENDER_OCL_FC_PARAMS;
using PRENDER_OCL_FC_PARAMS = RENDER_OCL_FC_PARAMS *;

struct AI_KERNEL_CONFIG
{
    VPHAL_PERFTAG            perfTag     = VPHAL_NONE;
};

struct AI_KERNEL_PARAM
{
    KERNEL_ARGS                  kernelArgs;
    std::string                  kernelName;
    uint32_t                     threadWidth;
    uint32_t                     threadHeight;
    uint32_t                     localWidth;
    uint32_t                     localHeight;
    KERNEL_ARG_INDEX_SURFACE_MAP kernelStatefulSurfaces;
    void                         Init();
};

using AI_KERNEL_PARAMS = std::vector<AI_KERNEL_PARAM>;
struct _RENDER_AI_PARAMS
{
    AI_KERNEL_PARAMS ai_kernelParams = {};
    AI_KERNEL_CONFIG ai_kernelConfig = {};
    void             Init();
};
using RENDER_AI_PARAMS  = _RENDER_AI_PARAMS;
using PRENDER_AI_PARAMS = RENDER_AI_PARAMS *;

struct _RENDER_HDR_PARAMS
{
    VpKernelID              kernelId;
    VP_COMPOSITE_PARAMS     compParams;
    uint32_t                uiMaxDisplayLum;       //!< Maximum Display Luminance
    uint32_t                uiMaxContentLevelLum;  //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE          hdrMode;
    VPHAL_CSPACE            srcColorSpace;
    VPHAL_CSPACE            dstColorSpace;
    uint32_t                threadWidth;
    uint32_t                threadHeight;


    VPHAL_HDR_CACHE_CNTL            SurfMemObjCtl;                  //!< Cache attributes
    uint32_t                        uiSplitFramePortions = 1;       //!< Split Frame flag
    bool                            bForceSplitFrame     = false;

    uint32_t                        uSourceCount;                                               //!< Number of sources
    uint32_t                        uTargetCount;                                               //!< Number of targets
    uint32_t                        uSourceBindingTableIndex[8];        //!< Binding Table Index
    uint32_t                        uTargetBindingTableIndex[8];       //!< Binding Table Index
    uint32_t                        dwSurfaceWidth;                                             //!< Record the input surface width of last HDR render
    uint32_t                        dwSurfaceHeight;                                            //!< Record the input surface height of last HDR render
    PVPHAL_COLORFILL_PARAMS         pColorFillParams;                                           //!< ColorFill - BG only

    VPHAL_HDR_LUT_MODE              LUTMode[VPHAL_MAX_HDR_INPUT_LAYER];         //!< LUT Mode
    VPHAL_HDR_LUT_MODE              GlobalLutMode;                              //!< Global LUT mode control for debugging purpose

    uint32_t                        dwOetfSurfaceWidth;             //!< Gamma 1D LUT surface
    uint32_t                        dwOetfSurfaceHeight;            //!< Gamma 1D LUT surface
    uint32_t                        dwUpdateMask;                   //!< Coefficients Update Mask
    uint32_t                        Cri3DLUTSize;                   //!< CRI 3D LUT surface

    VPHAL_SURFACE                   OETF1DLUTSurface[VPHAL_MAX_HDR_INPUT_LAYER];        //!< OETF 1D LUT surface
    VPHAL_SURFACE                   CoeffSurface;                                       //!< CSC CCM Coeff surface

    uint16_t                        OetfSmpteSt2084[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER];      //!< EOTF 1D LUT SMPTE ST2084

    uint8_t*                        pInput3DLUT;                                             //!< Input 3DLUT address for GPU generate 3DLUT

    const uint16_t                  *pHDRStageConfigTable = nullptr;

    HDRStageEnables                 StageEnableFlags[VPHAL_MAX_HDR_INPUT_LAYER];

    VPHAL_GAMMA_TYPE                EOTFGamma[VPHAL_MAX_HDR_INPUT_LAYER]; //!< EOTF
    VPHAL_GAMMA_TYPE                OETFGamma[VPHAL_MAX_HDR_INPUT_LAYER]; //!< OETF
    VPHAL_HDR_MODE                  HdrMode[VPHAL_MAX_HDR_INPUT_LAYER];   //!< Hdr Mode
    VPHAL_HDR_CCM_TYPE              CCM[VPHAL_MAX_HDR_INPUT_LAYER];       //!< CCM Mode
    VPHAL_HDR_CCM_TYPE              CCMExt1[VPHAL_MAX_HDR_INPUT_LAYER];   //!< CCM Ext1 Mode
    VPHAL_HDR_CCM_TYPE              CCMExt2[VPHAL_MAX_HDR_INPUT_LAYER];   //!< CCM Ext2 Mode
    VPHAL_HDR_CSC_TYPE              PriorCSC[VPHAL_MAX_HDR_INPUT_LAYER];  //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE              PostCSC[VPHAL_MAX_HDR_INPUT_LAYER];   //!< Post CSC Mode

    HDR_PARAMS                      HDRFrameTargetParams;
    HDR_PARAMS                      HDRLastFrameSourceParams[VPHAL_MAX_HDR_INPUT_LAYER];
    HDR_PARAMS                      HDRLastFrameTargetParams;

    STATUS_TABLE_UPDATE_PARAMS      StatusTableUpdateParams;                   //!< Status table, Video Pre-Processing Only

    bool                            bNeed3DSampler;                       //!< indicate whether 3D should neede by force considering AVS removal etc.

   VPHAL_ROTATION                   Rotation   = VPHAL_ROTATION_IDENTITY;  //!<  0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degreee
   VPHAL_SCALING_MODE               ScalingMode = VPHAL_SCALING_NEAREST;    //!<  Scaling Mode
   PVPHAL_IEF_PARAMS                pIEFParams;
   HDR_PARAMS                       srcHDRParams[VPHAL_MAX_HDR_INPUT_LAYER];
   HDR_PARAMS                       targetHDRParams[VPHAL_MAX_HDR_OUTPUT_LAYER];
   bool                             bUsingAutoModePipe;  //!< Hdr Auto Mode pipe flag
   uint16_t                         InputSrc[VPHAL_MAX_HDR_INPUT_LAYER] = {};  // Input Surface
   uint16_t                         Target[VPHAL_MAX_HDR_OUTPUT_LAYER]  = {};  // Target Surface
   bool                             bGpuGenerate3DLUT;                         //!< Flag for per frame GPU generation of 3DLUT
   float                            f3DLUTNormalizationFactor;                 //!< Normalization factor for 3DLUT
   bool                             bDisableAutoMode;                          //!< Force to disable Hdr auto mode tone mapping for debugging purpose
   bool                             coeffAllocated     = false;
   bool                             OETF1DLUTAllocated = false;
   bool                             Cri3DLUTAllocated  = false;
   PVPHAL_BLENDING_PARAMS           pBlendingParams    = nullptr;              //!< Blending parameters
};
using RENDER_HDR_PARAMS  = _RENDER_HDR_PARAMS;
using PRENDER_HDR_PARAMS = RENDER_HDR_PARAMS *;

class SwFilterPipe;
class HwFilter;
class PacketParamFactoryBase;

/////////////////////////////HwFilter Parameters///////////////////////////////////
class HwFilterParameter
{
public:
    HwFilterParameter(FeatureType featureType);
    virtual ~HwFilterParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter) = 0;

    FeatureType GetFeatureType()
    {
        return m_FeatureType;
    }

private:
    FeatureType m_FeatureType = FeatureTypeInvalid;

MEDIA_CLASS_DEFINE_END(vp__HwFilterParameter)
};

/////////////////////////////Packet Parameters///////////////////////////////////

class VpPacketParameter
{
public:
    VpPacketParameter(PacketParamFactoryBase *packetParamFactory);
    virtual ~VpPacketParameter();

    static void Destory(VpPacketParameter *&p);

    virtual bool SetPacketParam(VpCmdPacket *pPacket) = 0;

private:
    PacketParamFactoryBase *m_packetParamFactory = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpPacketParameter)
};

/////////////////////////////Policy Feature Handler//////////////////////////////

class PolicyFeatureHandler
{
public:
    PolicyFeatureHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyFeatureHandler();
    virtual bool IsFeatureEnabled(SwFilterPipe &swFilterPipe);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    FeatureType GetType();
    HwFilterParameter *GetHwFeatureParameterFromPool();
    virtual MOS_STATUS ReleaseHwFeatureParameter(HwFilterParameter *&pParam);
protected:
    FeatureType m_Type = FeatureTypeInvalid;
    std::vector<HwFilterParameter *> m_Pool;
    VP_HW_CAPS  &m_hwCaps;

MEDIA_CLASS_DEFINE_END(vp__PolicyFeatureHandler)
};

class PacketParamFactoryBase
{
public:
    PacketParamFactoryBase();
    virtual ~PacketParamFactoryBase();
    virtual VpPacketParameter *GetPacketParameter(PVP_MHWINTERFACE pHwInterface) = 0;
    void ReturnPacketParameter(VpPacketParameter *&p);
protected:
    std::vector<VpPacketParameter *> m_Pool;

MEDIA_CLASS_DEFINE_END(vp__PacketParamFactoryBase)
};

template<class T>
class PacketParamFactory : public PacketParamFactoryBase
{
public:
    PacketParamFactory()
    {
    }
    virtual ~PacketParamFactory()
    {
    }
    virtual VpPacketParameter *GetPacketParameter(PVP_MHWINTERFACE pHwInterface)
    {
        if (nullptr == pHwInterface)
        {
            return nullptr;
        }
        if (m_Pool.empty())
        {
            T *p = MOS_New(T, pHwInterface, this);
            if (nullptr == p)
            {
                return nullptr;
            }

            VpPacketParameter *pBase = dynamic_cast<VpPacketParameter *>(p);

            if (nullptr == pBase)
            {
                MOS_Delete(p);
            }
            return pBase;
        }
        else
        {
            VpPacketParameter *p = m_Pool.back();
            m_Pool.pop_back();
            return p;
        }
    }

MEDIA_CLASS_DEFINE_END(vp__PacketParamFactory)
};

struct HW_FILTER_PARAM
{
    FeatureType             type;
    PVP_MHWINTERFACE        pHwInterface;
    VP_EXECUTE_CAPS         vpExecuteCaps;
    PacketParamFactoryBase *pPacketParamFactory                     = nullptr;
    VpPacketParameter*    (*pfnCreatePacketParam)(HW_FILTER_PARAM&) = nullptr;
};
}

#endif // !__VP_FILTER_H__
