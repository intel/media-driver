/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_hdrlite_render_filter.h
//! \brief    Defines the common interface for Hdr
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#ifndef __VP_HDRLITE_RENDER_FILTER_H__
#define __VP_HDRLITE_RENDER_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"
#include "kernel_args/igvpHdrRender_args.h"
#include "vp_allocator.h"

namespace vp
{
//!
//! \brief HDR Format Descriptor enum
//!
typedef enum _HDR_FORMAT_DESCRIPTOR
{
    HDR_FORMAT_DESCRIPTOR_UNKNOW            = -1,
    HDR_FORMAT_R16G16B16A16_FLOAT           = 44,
    HDR_FORMAT_DESCRIPTOR_R16G16_UNORM      = 60,
    HDR_FORMAT_DESCRIPTOR_R16_UNORM         = 70,
    HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM = 89,
    HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM    = 101,
    HDR_FORMAT_DESCRIPTOR_YUY2              = 201,
    HDR_FORMAT_DESCRIPTOR_NV12              = 220,
    HDR_FORMAT_DESCRIPTOR_P010              = 222,
    HDR_FORMAT_DESCRIPTOR_P016              = 223
} HDR_FORMAT_DESCRIPTOR;

//!
//! \brief HDR Chroma Siting enum
//!
typedef enum _HDR_CHROMA_SITING
{
    HDR_CHROMA_SITTING_A = 0,  // Sample even index at even line
    HDR_CHROMA_SITTING_B,      // Sample even index at odd line
    HDR_CHROMA_SITTING_AC,     // Average consistent even index and odd index at even line
    HDR_CHROMA_SITTING_BD,     // Average consistent even index and odd index at odd line
    HDR_CHROMA_SITTING_AB,     // Average even index of even line and even index of odd line
    HDR_CHROMA_SITTING_ABCD    // Average even and odd index at even line and odd line
} HDR_CHROMA_SITING;

//!
//! \brief HDR Rotation enum
//!
typedef enum _HDR_ROTATION
{
    HDR_LAYER_ROTATION_0 = 0,  // 0 degree rotation
    HDR_LAYER_ROTATION_90,     // 90 degree CW rotation
    HDR_LAYER_ROTATION_180,    // 180 degree rotation
    HDR_LAYER_ROTATION_270,    // 270 degree CW rotation
    HDR_LAYER_MIRROR_H,        // 0 degree rotation then mirror horizontally
    HDR_LAYER_ROT_90_MIR_H,    // 90 degree CW rotation then mirror horizontally
    HDR_LAYER_MIRROR_V,        // 180 degree rotation then mirror horizontally (vertical mirror)
    HDR_LAYER_ROT_90_MIR_V     // 270 degree CW rotation then mirror horizontally (90 degree CW rotation then vertical mirror)
} HDR_ROTATION;

//!
//! \brief Two Layer Option enum
//!
typedef enum _HDR_TWO_LAYER_OPTION
{
    HDR_TWO_LAYER_OPTION_SBLEND = 0,  // Source Blending
    HDR_TWO_LAYER_OPTION_CBLEND,      // Constant Blending
    HDR_TWO_LAYER_OPTION_PBLEND,      // Partial Blending
    HDR_TWO_LAYER_OPTION_CSBLEND,     // Constant Source Blending
    HDR_TWO_LAYER_OPTION_CPBLEND,     // Constant Partial Blending
    HDR_TWO_LAYER_OPTION_COMP         // Composition
} HDR_TWO_LAYER_OPTION;

struct HDRLITE_LAYER_PARAM
{
    bool                  enabled     = false;
    VPHAL_SCALING_MODE    scalingMode = VPHAL_SCALING_NEAREST;
    VPHAL_ROTATION        rotation    = VPHAL_ROTATION_IDENTITY;
    VPHAL_BLENDING_PARAMS blending    = {};
    HDR_PARAMS            hdrParams   = {};
    HDRStageEnables       stageEnable = {};

    VPHAL_HDR_LUT_MODE LUTMode   = VPHAL_HDR_LUT_MODE_NONE;
    VPHAL_GAMMA_TYPE   EOTFGamma = VPHAL_GAMMA_NONE;     //!< EOTF
    VPHAL_GAMMA_TYPE   OETFGamma = VPHAL_GAMMA_NONE;     //!< OETF
    VPHAL_HDR_MODE     HdrMode   = VPHAL_HDR_MODE_NONE;  //!< Hdr Mode
    VPHAL_HDR_CCM_TYPE CCM       = VPHAL_HDR_CCM_NONE;   //!< CCM Mode
    VPHAL_HDR_CCM_TYPE CCMExt1   = VPHAL_HDR_CCM_NONE;   //!< CCM Ext1 Mode
    VPHAL_HDR_CCM_TYPE CCMExt2   = VPHAL_HDR_CCM_NONE;   //!< CCM Ext2 Mode
    VPHAL_HDR_CSC_TYPE PriorCSC  = VPHAL_HDR_CSC_NONE;   //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE PostCSC   = VPHAL_HDR_CSC_NONE;   //!< Post CSC Mode
};

struct HDRLITE_HAL_PARAM
{
    uint16_t               layer                                      = 0;
    uint32_t               uiMaxDisplayLum                            = 0;
    HDRLITE_LAYER_PARAM       inputLayerParam[VPHAL_MAX_HDR_INPUT_LAYER] = {};
    bool                   enableColorFill                            = false;
    VPHAL_COLORFILL_PARAMS colorFillParams                            = {};
    HDR_PARAMS             targetHdrParams                            = {};
};


// VpHdrLiteKrnStructedData structure
struct VpHdrLiteKrnStructedData
{
    float horizontalFrameOrigin[8];
    float verticalFrameOrigin[8];
    float horizontalScalingStepRatio[8];
    float verticalScalingStepRatio[8];

    struct {
        uint16_t left;
        uint16_t top;
    } topLeft[8];

    struct {
        uint16_t right;
        uint16_t bottom;
    } bottomRight[8];

    struct LayerInfo {
        uint32_t formatDescriptor       : 8;
        uint32_t chromaSitting          : 3;
        uint32_t channelSwap            : 1;
        uint32_t iefBypass              : 1;
        uint32_t rotation               : 3;
        uint32_t samplerIndexPlane1     : 4;
        uint32_t samplerIndexPlane2_3   : 4;
        uint32_t ccmExtEnable           : 1;
        uint32_t toneMappingEnable      : 1;
        uint32_t priorCscEnable         : 1;
        uint32_t eotfEnable             : 1;
        uint32_t ccmEnable              : 1;
        uint32_t oetfEnable             : 1;
        uint32_t postCscEnable          : 1;
        uint32_t lut3dEnable            : 1;
    } layerInfo[8];
    
    uint8_t constantAlpha[8];
    
    uint8_t twoLayerOperation[8];
    
    struct {
        uint16_t rvChannel;
        uint16_t gyChannel;
        uint16_t buChannel;
        uint16_t alphaChannel;
    } fillColor;
    
    uint16_t dstWidth;
    
    uint16_t dstHeight;
    
    uint16_t layerNum;
    
    struct {
        uint16_t formatDescriptor   : 8;
        uint16_t chromaSitting      : 3;
        uint16_t channelSwap        : 1;
        uint16_t dstCscEnable       : 1;
        uint16_t reserved           : 1;
        uint16_t ditherRoundEnable  : 2;
    } dstInfo;
};

class VpHdrLiteRenderFilter : public VpFilter
{
public:
    VpHdrLiteRenderFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    virtual ~VpHdrLiteRenderFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe   *executedPipe,
        VP_EXECUTE_CAPS vpExecuteCaps);

    MOS_STATUS CalculateEngineParams(
        FeatureParamHdr &HdrParams,
        VP_EXECUTE_CAPS  vpExecuteCaps);

    PRENDER_HDRLITE_PARAMS GetRenderParams()
    {
        return &m_renderHdrParams;
    }

protected:
    // Helper functions for CURBE data handling
    static HDR_FORMAT_DESCRIPTOR GetFormatDescriptor(MOS_FORMAT format);
    static HDR_CHROMA_SITING GetHdrChromaSiting(uint32_t chromaSiting);
    static HDR_ROTATION GetHdrRotation(VPHAL_ROTATION rotation);
    
    MOS_STATUS PopulateHdrLiteCurbeData(
        VpHdrLiteKrnStructedData                     &curbeData,
        const HDRLITE_HAL_PARAM                 &param,
        const VP_SURFACE_GROUP &surfaceGroup);
    
    MOS_STATUS GenerateHdrKrnParam(
        const HDRLITE_HAL_PARAM                 &param,
        const VP_SURFACE_GROUP &surfaceGroup,
        RENDER_HDRLITE_KERNEL_PARAM             &renderParam);
    
    MOS_STATUS SetupHdrKrnArg(
        const VpHdrLiteKrnStructedData &curbeData,
        KRN_ARG               &krnArg,
        bool                  &bInit);
    
    void PrintHdrLiteCurbeData(const VpHdrLiteKrnStructedData &curbeData);

    MOS_STATUS SetupHdrStatefulSurface(
        uint32_t                                  uIndex,
        const HDRLITE_HAL_PARAM                     &param,
        const VP_SURFACE_GROUP &surfaceGroup,
        SURFACE_PARAMS                           &surfaceParam);

    // mathmatic functions
    typedef float (*pfnOETFFunc)(float radiance);
    static MOS_STATUS       CalculateCscMatrix(VPHAL_HDR_CSC_TYPE cscType, bool is3DLutPath, float *outMatrix);
    static MOS_STATUS       CalculateCCMWithMonitorGamut(VPHAL_HDR_CCM_TYPE ccmType, HDR_PARAMS &targetHdrParam, std::array<float, 12> &outMatrix);
    static MOS_STATUS       CaluclateToneMapping3DLut(VPHAL_HDR_MODE hdrMode, double inputX, double inputY, double inputZ, double &outputX, double &outputY, double &outputZ);
    static CSC_COEFF_FORMAT ConvertDouble2RegisterForamt(double input);
    static double           ConvertRegister2DoubleFormat(CSC_COEFF_FORMAT regVal);
    MOS_STATUS              Generate2SegmentsOETFLUT(float fStretchFactor, pfnOETFFunc oetfFunc, uint16_t *lut);
    MOS_STATUS              GenerateColorTransfer3dLut(HDRLITE_LAYER_PARAM &param, HDR_PARAMS &targetHdrParam, MOS_FORMAT inputFormat, float fInputX, float fInputY, float fInputZ, uint16_t &outputX, uint16_t &outputY, uint16_t &outputZ);
    MOS_STATUS              GenerateH2HPWLFCoeff(HDR_PARAMS &srcHdrParam, HDR_PARAMS &targetHdrParam, float *pivotPoint, uint16_t *slopeIntercept);

protected:
    MOS_STATUS InitHalParam(SwFilterPipe &executingPipe, FeatureParamHdr &hdrParams, HDRLITE_HAL_PARAM &param);
    MOS_STATUS InitLayerParam(SwFilterPipe &executingPipe, uint32_t index, HDR_PARAMS &targetHdrParam, VPHAL_CSPACE targetCSpace, MOS_FORMAT targetFormat, HDRLITE_LAYER_PARAM &param, bool &needUpdate);
    MOS_STATUS InitLayerOETF1DLUT(VpAllocator &allocator, HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, PVP_SURFACE oetfSurface);
    MOS_STATUS InitLayerCri3DLut(VpAllocator &allocator, HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, PVP_SURFACE inputSurface, PVP_SURFACE cri3DLutSurface);
    MOS_STATUS InitCoeff(VpAllocator &allocator, HDRLITE_HAL_PARAM &param, VP_SURFACE_GROUP &surfGroup);
    MOS_STATUS InitLayerCoeffBase(HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, uint64_t line, uint32_t maxDisplayLum, float *dst);
    MOS_STATUS InitLayerCoeffCCMExt(HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, uint64_t line, float *dst);

protected:
    RENDER_HDRLITE_PARAMS m_renderHdrParams = {};
    SwFilterPipe      *m_executedPipe    = nullptr;

    HDR_PARAMS m_lastFrameSourceParams[VPHAL_MAX_HDR_INPUT_LAYER] = {};
    HDR_PARAMS m_lastFrameTargetParams                            = {};

    VPHAL_HDR_LUT_MODE m_globalLutMode = VPHAL_HDR_LUT_MODE_NONE; //For debug purpose
    
    KERNEL_INDEX_ARG_MAP m_hdrMandatoryKrnArgs = {};

    MEDIA_CLASS_DEFINE_END(vp__VpHdrLiteRenderFilter)
};

struct HW_FILTER_HDRLITE_RENDER_PARAM : public HW_FILTER_PARAM
{
    FeatureParamHdr hdrParams;
    SwFilterPipe   *executedPipe;
};

class HwFilterHdrLiteRenderParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_HDRLITE_RENDER_PARAM &param, FeatureType featureType);
    HwFilterHdrLiteRenderParameter(FeatureType featureType);
    virtual ~HwFilterHdrLiteRenderParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_HDRLITE_RENDER_PARAM &param);

private:
    HW_FILTER_HDRLITE_RENDER_PARAM m_Params = {};

    MEDIA_CLASS_DEFINE_END(vp__HwFilterHdrLiteRenderParameter)
};

class VpRenderHdrLiteParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_HDRLITE_RENDER_PARAM &param);
    VpRenderHdrLiteParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderHdrLiteParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_HDRLITE_RENDER_PARAM &params);

    VpHdrLiteRenderFilter m_HdrFilter;

    MEDIA_CLASS_DEFINE_END(vp__VpRenderHdrLiteParameter)
};

class PolicyRenderHdrLiteHandler : public PolicyFeatureHandler
{
public:
    PolicyRenderHdrLiteHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyRenderHdrLiteHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeHdrOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for VEBOX Hdr!");
            return nullptr;
        }

        HW_FILTER_HDRLITE_RENDER_PARAM *HdrParam = (HW_FILTER_HDRLITE_RENDER_PARAM *)(&param);
        return VpRenderHdrLiteParameter::Create(*HdrParam);
    }

    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps);

private:
    PacketParamFactory<VpRenderHdrLiteParameter> m_PacketParamFactory;

    MEDIA_CLASS_DEFINE_END(vp__PolicyRenderHdrLiteHandler)
};

}  // namespace vp
#endif
