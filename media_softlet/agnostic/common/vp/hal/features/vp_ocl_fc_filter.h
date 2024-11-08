/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_ocl_fc_filter.h
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all ocl fc in driver.

#ifndef __VP_OCL_FC_FILTER_H__
#define __VP_OCL_FC_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"
#include "vp_fc_filter.h"

namespace vp
{

struct OCL_FC_LUMA_KEY_PARAMS
{
    bool                 enabled = false;
    VPHAL_LUMAKEY_PARAMS params  = {};
};

struct OCL_FC_DI_PARAMS
{
    bool            enabled = false;
    VPHAL_DI_PARAMS params  = {};
};

struct OCL_FC_LAYER_PARAM
{
    VP_SURFACE            *surf                             = nullptr;
    uint32_t               layerID                          = 0;
    uint32_t               layerIDOrigin                    = 0;  //!< Origin layerID before layerSkipped, which can be used to reference surfaces in SurfaceGroup.
    VPHAL_SCALING_MODE     scalingMode                      = VPHAL_SCALING_NEAREST;
    VPHAL_ROTATION         rotation                         = VPHAL_ROTATION_IDENTITY;
    OCL_FC_LUMA_KEY_PARAMS lumaKey                          = {};
    VPHAL_BLENDING_PARAMS  blendingParams                   = {};
    VPHAL_PROCAMP_PARAMS   procampParams                    = {};
    OCL_FC_DI_PARAMS       diParams                         = {};
    bool                   needIntermediaSurface            = false;
    bool                   needSepareateIntermediaSecPlane  = false;
    MOS_FORMAT             intermediaFormat                 = Format_Any;
    MOS_FORMAT             separateIntermediaSecPlaneFormat = Format_Any;
};

struct OCL_FC_COMP_PARAM
{
    OCL_FC_LAYER_PARAM     inputLayersParam[8]   = {};
    VPHAL_CSPACE           mainCSpace            = CSpace_Any;
    uint32_t               layerNumber           = 0;
    OCL_FC_LAYER_PARAM     outputLayerParam      = {};
    bool                   enableColorFill       = false;
    VPHAL_COLORFILL_PARAMS colorFillParams       = {};     //!< ColorFill - BG only
    VPHAL_ALPHA_PARAMS     compAlpha             = {};     //!< Alpha for composited surface
    bool                   bAlphaCalculateEnable = false;  //!< Alpha Calculation flag
};

class VpOclFcFilter : public VpFilter
{
public:
    VpOclFcFilter(PVP_MHWINTERFACE vpMhwInterface);

    ~VpOclFcFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe   *executingPipe,
        VP_EXECUTE_CAPS vpExecuteCaps);

    MOS_STATUS            CalculateEngineParams();
    PRENDER_OCL_FC_PARAMS GetFcParams()
    {
        return m_renderOclFcParams;
    }

protected:
    //Kernel Curbe Struct
    typedef struct _OCL_FC_KRN_RECT
    {
        int32_t left;
        int32_t right;
        int32_t top;
        int32_t bottom;
    } OCL_FC_KRN_RECT;

    typedef struct _OCL_FC_KRN_CSC_MATRIX
    {
        float s0123[4];
        float s4567[4];
        float s89AB[4];
        float sCDEF[4];
    } OCL_FC_KRN_CSC_MATRIX;

    typedef struct _OCL_FC_KRN_SCALE_PARAM
    {
        OCL_FC_KRN_RECT trg;
        struct
        {
            float startX;
            float startY;
            float strideX;
            float strideY;
        } src;
        uint32_t rotateIndices[2];
    } OCL_FC_KRN_SCALE_PARAM;

    typedef struct _OCL_FC_KRN_COORD_SHIFT_PARAM
    {
        float commonShiftX;
        float commonShiftY;
        float chromaShiftX;
        float chromaShiftY;
    } OCL_FC_KRN_COORD_SHIFT_PARAM;

    typedef struct _OCL_FC_KRN_INPUT_CONTROL_PARAM
    {
        uint8_t samplerType;
        uint8_t isChromaShift;
        uint8_t ignoreSrcPixelAlpha;
        uint8_t ignoreDstPixelAlpha;
    } OCL_FC_KRN_INPUT_CONTROL_PARAM;

    typedef struct _OCL_FC_KRN_TARGET_CONTROL_PARAM
    {
        uint8_t isColorFill;
        uint8_t alphaLayerIndex;
        uint8_t hitSecPlaneFactorX;
        uint8_t hitSecPlaneFactorY;
    } OCL_FC_KRN_TARGET_CONTROL_PARAM;

    //common path params
    typedef struct _OCL_FC_KRN_IMAGE_PARAM
    {
        OCL_FC_KRN_CSC_MATRIX          csc;
        uint32_t                       inputChannelIndices[4];
        OCL_FC_KRN_SCALE_PARAM         scale;
        OCL_FC_KRN_COORD_SHIFT_PARAM   coordShift;
        uint32_t                       inputPlaneNum;
        OCL_FC_KRN_INPUT_CONTROL_PARAM controlSetting;
        float                          constAlphs;
        struct OCL_FC_KRN_LUMA_PARAM
        {
            float low;
            float high;
        } lumaKey;
    } OCL_FC_KRN_IMAGE_PARAM;

    typedef struct _OCL_FC_KRN_TARGET_PARAM
    {
        OCL_FC_KRN_RECT                 targetROI;
        uint32_t                        dynamicChannelIndices[4];
        float                           background[4];
        float                           chromaSitingFactor[4];
        uint32_t                        planeNumber;
        OCL_FC_KRN_TARGET_CONTROL_PARAM controlSetting;
        float                           alpha;
        uint32_t                        reserved;
    } OCL_FC_KRN_TARGET_PARAM;

    //fast path params
    typedef struct _OCL_FC_FP_KRN_IMAGE_PARAM
    {
        OCL_FC_KRN_CSC_MATRIX          csc;
        uint32_t                       inputChannelIndices[4];
        OCL_FC_KRN_SCALE_PARAM         scaleParam;
        OCL_FC_KRN_COORD_SHIFT_PARAM   coordShift;
        uint32_t                       inputPlaneNum;
        OCL_FC_KRN_INPUT_CONTROL_PARAM controlSetting;
    } OCL_FC_FP_KRN_IMAGE_PARAM;

    typedef struct _OCL_FC_FP_KRN_TARGET_PARAM
    {
        OCL_FC_KRN_RECT targetROI;
        uint32_t        dynamicChannelIndices[4];
        float           background[4];
        float           chromaSitingFactor[4];
        uint32_t        combineChannelIndices[2];
        uint32_t        planeNumber;
        struct
        {
            uint16_t x;
            uint16_t y;
        } alignedTrgRectStart;
        struct
        {
            uint16_t width;
            uint16_t height;
        } alignedTrgRectSize;
        OCL_FC_KRN_TARGET_CONTROL_PARAM controlSetting;
        float                           alpha;
        uint32_t                        reserved;
    } OCL_FC_FP_KRN_TARGET_PARAM;

protected:
    //common func
    MOS_STATUS InitKrnParams(OCL_FC_KERNEL_PARAMS &krnParam, SwFilterPipe &executingPipe);
    MOS_STATUS InitLayer(SwFilterPipe &executingPipe, bool isInputPipe, int index, VPHAL_SCALING_MODE defaultScalingMode, OCL_FC_LAYER_PARAM &layer);
    MOS_STATUS InitCompParam(SwFilterPipe &executingPipe, OCL_FC_COMP_PARAM &compParam);
    MOS_STATUS SetPerfTag(OCL_FC_COMP_PARAM &compParam, VPHAL_PERFTAG &perfTag);

    //OCL FC common kernel
    MOS_STATUS GenerateFcCommonKrnParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS SetupSingleFcCommonKrnArg(uint32_t layerNum, std::vector<OCL_FC_KRN_IMAGE_PARAM> &imageParams, OCL_FC_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit);
    MOS_STATUS SetupSingleFcCommonBti(uint32_t uIndex, const OCL_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit);
    
    //OCL FC 420 PL3 input kernel
    MOS_STATUS GenerateFc420PL3InputParam(OCL_FC_LAYER_PARAM &inputLayersParam, uint32_t index, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS SetupSingleFc420PL3InputBti(uint32_t uIndex, uint32_t layIndex, SURFACE_PARAMS &surfaceParam, bool &bInit);
    MOS_STATUS SetupSingleFc420PL3InputKrnArg(uint32_t srcSurfaceWidth, uint32_t srcSurfaceHeight, uint32_t lumaChannelIndices, uint32_t chromaChannelIndices[4], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit);
    MOS_STATUS ConvertInputOutputSingleChannelIndexToKrnParam(MOS_FORMAT format, uint32_t &inputChannelIndex);

    //OCL FC 420 PL3 output kernel
    MOS_STATUS GenerateFc420PL3OutputParam(OCL_FC_LAYER_PARAM &outputLayersParam, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS SetupSingleFc420PL3OutputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, bool &bInit);
    MOS_STATUS SetupSingleFc420PL3OutputKrnArg(uint32_t srcSurfaceWidth, uint32_t srcSurfaceHeight, uint32_t lumaChannelIndices, uint32_t chromaChannelIndices[2], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit);

    //OCL FC 422HV/411P input kernel
    MOS_STATUS GenerateFc422HVInputParam(OCL_FC_LAYER_PARAM &inputLayersParam, uint32_t index, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS SetupSingleFc422HVInputBti(uint32_t uIndex, uint32_t layIndex, SURFACE_PARAMS &surfaceParam, bool &bInit);
    MOS_STATUS SetupSingleFc422HVInputKrnArg(uint32_t srcSurfaceWidthPL1, uint32_t srcSurfaceHeightPL1, uint32_t channelIndex, uint32_t chromaChannelIndices[4], uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit);
    
    //OCL FC 444 PL3 input kernel
    MOS_STATUS GenerateFc444PL3InputParam(OCL_FC_LAYER_PARAM &layer, uint32_t layerNumber, OCL_FC_KERNEL_PARAM &param, uint32_t layerIndex);
    MOS_STATUS SetupSingleFc444PL3InputKrnArg(uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit, uint32_t inputChannelIndices[4], uint32_t outputChannelIndices[4], uint32_t planeChannelIndices);
    MOS_STATUS SetupSingleFc444PL3InputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, uint32_t layerIndex, bool &bInit);
    
    //OCL FC 444 PL3 output kernel
    MOS_STATUS GenerateFc444PL3OutputParam(OCL_FC_LAYER_PARAM &outputLayersParam, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS SetupSingleFc444PL3OutputKrnArg(uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit, uint32_t inputChannelIndices[4], uint32_t outputChannelIndices[4]);
    MOS_STATUS SetupSingleFc444PL3OutputBti(uint32_t uIndex, SURFACE_PARAMS &surfaceParam, bool &bInit);

    MOS_STATUS GetDefaultScalingMode(VPHAL_SCALING_MODE &defaultScalingMode, SwFilterPipe &executedPipe);
    MOS_STATUS GetChromaSitingFactor(MOS_FORMAT format, uint8_t &hitSecPlaneFactorX, uint8_t &hitSecPlaneFactorY);
    MOS_STATUS GetBitNumber(MOS_FORMAT format, uint8_t *pOriginBitNumber, uint8_t *pStoredBitNumber, uint8_t *pAlphaBitNumber);

    //OCL FC common kernel input image parameter generate
    MOS_STATUS GenerateInputImageParam(OCL_FC_LAYER_PARAM &layer, VPHAL_CSPACE mainCSpace, OCL_FC_KRN_IMAGE_PARAM &imageParam);
    MOS_STATUS ConvertProcampAndCscToKrnParam(VPHAL_CSPACE srcColorSpace, VPHAL_CSPACE dstColorSpace, OCL_FC_KRN_CSC_MATRIX &csc, VPHAL_PROCAMP_PARAMS &procampParams);
    MOS_STATUS GenerateProcampCscMatrix(VPHAL_CSPACE srcColorSpace, VPHAL_CSPACE dstColorSpace, float *cscMatrix, VPHAL_PROCAMP_PARAMS &procampParams);
    MOS_STATUS ConvertScalingRotToKrnParam(RECT &rcSrc, RECT &rcDst, VPHAL_SCALING_MODE scalingMode, uint32_t inputWidth, uint32_t inputHeight, VPHAL_ROTATION rotation, OCL_FC_KRN_SCALE_PARAM &scaling, uint8_t &samplerType, OCL_FC_KRN_COORD_SHIFT_PARAM &coordShift);
    MOS_STATUS ConvertRotationToKrnParam(VPHAL_ROTATION rotation, float strideX, float strideY, float startLeft, float startRight, float startTop, float startBottom, OCL_FC_KRN_SCALE_PARAM &scaling);
    MOS_STATUS ConvertChromaUpsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, VPHAL_SCALING_MODE scalingMode, uint32_t inputWidth, uint32_t inputHeight, float &chromaShiftX, float &chromaShiftY, uint8_t &isChromaShift);
    MOS_STATUS ConvertInputChannelIndicesToKrnParam(MOS_FORMAT format, MOS_FORMAT separateIntermediaSecPlaneFormat, uint32_t *inputChannelIndices);
    MOS_STATUS ConvertPlaneNumToKrnParam(MOS_FORMAT format, bool needSeparateIntermediaSecPlane, bool isInput, uint32_t &planeNum);
    MOS_STATUS ConvertBlendingToKrnParam(VPHAL_BLENDING_PARAMS &blend, uint8_t &ignoreSrcPixelAlpha, uint8_t &ignoreDstPixelAlpha, float &constAlpha);

    //OCL FC common kernel output parameter
    MOS_STATUS GenerateTargetParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KRN_TARGET_PARAM &targetParam);
    MOS_STATUS ConvertOutputChannelIndicesToKrnParam(MOS_FORMAT format, uint32_t *dynamicChannelIndices);
    MOS_STATUS ConvertTargetRoiToKrnParam(RECT &outputRcDst, uint32_t outputWidth, uint32_t outputHeight, OCL_FC_KRN_RECT &targetROI);
    MOS_STATUS ConvertChromaDownsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, float *chromaSitingFactor, uint8_t &hitSecPlaneFactorX, uint8_t &hitSecPlaneFactorY);
    MOS_STATUS ConvertAlphaToKrnParam(bool bAlphaCalculateEnable, VPHAL_ALPHA_PARAMS &compAlpha, float colorFillAlpha, uint8_t &alphaLayerIndex, float &alpha);
    MOS_STATUS ConvertColorFillToKrnParam(bool enableColorFill, VPHAL_COLORFILL_PARAMS &colorFillParams, MEDIA_CSPACE dstCspace, uint8_t &isColorFill, float *background);

    void PrintCompParam(OCL_FC_COMP_PARAM &compParam);
    void PrintCompLayerParam(uint32_t index, bool isInput, OCL_FC_LAYER_PARAM &layerParam);
    void PrintKrnParam(std::vector<OCL_FC_KRN_IMAGE_PARAM> &imageParams, OCL_FC_KRN_TARGET_PARAM &targetParam);
    void PrintKrnImageParam(uint32_t index, OCL_FC_KRN_IMAGE_PARAM &imageParam);
    void PrintKrnTargetParam(OCL_FC_KRN_TARGET_PARAM &targetParam);
    void ReportDiffLog(const OCL_FC_COMP_PARAM &compParam, bool isFastExpressSupported);
    void ReportFeatureLog(const OCL_FC_COMP_PARAM &compParam);

    //OCL FC FastExpress
    bool       FastExpressConditionMeet(const OCL_FC_COMP_PARAM &compParam);
    MOS_STATUS GenerateFcFastExpressKrnParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_KERNEL_PARAM &param);
    MOS_STATUS GenerateFastExpressInputOutputParam(OCL_FC_COMP_PARAM &compParam, OCL_FC_FP_KRN_IMAGE_PARAM &imageParam, OCL_FC_FP_KRN_TARGET_PARAM &targetParam);
    MOS_STATUS SetupSingleFcFastExpressKrnArg(uint32_t layerNum, OCL_FC_FP_KRN_IMAGE_PARAM &imageParams, OCL_FC_FP_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], uint32_t globalSize[3], KRN_ARG &krnArg, bool &bInit);
    MOS_STATUS SetupSingleFcFastExpressBti(uint32_t uIndex, const OCL_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit);
    MOS_STATUS ConvertAlignedTrgRectToKrnParam(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, bool enableColorFill, OCL_FC_FP_KRN_TARGET_PARAM &targetParam);
    void       PrintFastExpressKrnParam(OCL_FC_FP_KRN_IMAGE_PARAM &imageParam, OCL_FC_FP_KRN_TARGET_PARAM &targetParam);


    SwFilterPipe         *m_executingPipe     = nullptr;
    PRENDER_OCL_FC_PARAMS m_renderOclFcParams = nullptr;

    //to avoid duplicate allocate and free
    KERNEL_INDEX_ARG_MAP              m_fcCommonKrnArgs;
    KERNEL_INDEX_ARG_MAP              m_fcFastExpressKrnArgs;
    MULTI_LAYERS_KERNEL_INDEX_ARG_MAP m_fc420PL3InputMultiLayersKrnArgs;
    MULTI_LAYERS_KERNEL_INDEX_ARG_MAP m_fc444PL3InputMultiLayersKrnArgs;
    MULTI_LAYERS_KERNEL_INDEX_ARG_MAP m_fc422HVInputMultiLayersKrnArgs;
    KERNEL_INDEX_ARG_MAP              m_fc420PL3OutputKrnArgs;
    KERNEL_INDEX_ARG_MAP              m_fc444PL3OutputKrnArgs;

    MEDIA_CLASS_DEFINE_END(vp__VpOclFcFilter)
};

struct HW_FILTER_OCL_FC_PARAM : public HW_FILTER_PARAM
{
    SwFilterPipe *executingPipe;
};

class HwFilterOclFcParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_OCL_FC_PARAM &param, FeatureType featureType);
    HwFilterOclFcParameter(FeatureType featureType);
    virtual ~HwFilterOclFcParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_OCL_FC_PARAM &param);

private:
    HW_FILTER_OCL_FC_PARAM m_Params = {};

    MEDIA_CLASS_DEFINE_END(vp__HwFilterOclFcParameter)
};

class VpRenderOclFcParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_OCL_FC_PARAM &param);
    VpRenderOclFcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderOclFcParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_OCL_FC_PARAM &params);

    VpOclFcFilter m_fcFilter;

    MEDIA_CLASS_DEFINE_END(vp__VpRenderOclFcParameter)
};

class PolicyOclFcFeatureHandler : public PolicyFeatureHandler
{
public:
    PolicyOclFcFeatureHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
    {
        m_Type = FeatureTypeFc;
    }
    virtual ~PolicyOclFcFeatureHandler()
    {
    }
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
    {
        // Not create hwFilters for single FC features.
        return false;
    }
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
    {
        return nullptr;
    }
    virtual MOS_STATUS        UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        return nullptr;
    }
    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

    MEDIA_CLASS_DEFINE_END(vp__PolicyOclFcFeatureHandler)
};

class PolicyOclFcHandler : public PolicyFcHandler
{
public:
    PolicyOclFcHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyOclFcHandler();
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface) override;
    static VpPacketParameter  *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeFcOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for FC!");
            return nullptr;
        }

        HW_FILTER_OCL_FC_PARAM *fcParam = (HW_FILTER_OCL_FC_PARAM *)(&param);
        return VpRenderOclFcParameter::Create(*fcParam);
    }

    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps) override;

private:
    MOS_STATUS AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode, int index, VP_SURFACE &input, SwFilterSubPipe &pipe, VP_SURFACE &output, VP_EXECUTE_CAPS &caps, int32_t &resLayers);

    PacketParamFactory<VpRenderOclFcParameter> m_PacketOclParamFactory;

    MEDIA_CLASS_DEFINE_END(vp__PolicyOclFcHandler)
};

}  // namespace vp

#endif