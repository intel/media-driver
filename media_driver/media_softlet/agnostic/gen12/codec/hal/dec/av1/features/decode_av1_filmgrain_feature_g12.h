/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_av1_filmgrain_feature_g12.h
//! \brief    Defines the film grain feature for decode av1
//!
#ifndef __DECODE_AV1_FILMGRAIN_FEATURE_G12_H__
#define __DECODE_AV1_FILMGRAIN_FEATURE_G12_H__

#include "decode_resource_array.h"
#include "decode_allocator.h"
#include "decode_basic_feature.h"
#include "codec_def_decode_av1.h"
#include "codechal_hw_g12_X.h"
#include "decode_av1_basic_feature_g12.h"

namespace decode
{
//!
//! \enum     FilmGrainKernelStateIdx
//! \brief    Film Grain kernel state index indicating the 4 stages of kernel
//!
enum FilmGrainKernelStateIdx
{
    getRandomValues = 0,  //!< Film Grain kernel index for GetRandomValues
    regressPhase1,        //!< Film Grain kernel index for RegressPhase1
    regressPhase2,        //!< Film Grain kernel index for RegressPhase2
    applyNoise,           //!< Film Grain kernel index for ApplyNoise
    kernelNum             //!< Total kernel number for one Film Grain execution
};

//!
//! \enum PerfTagCallType
//!
enum
{
    PERFTAG_CALL_FILM_GRAIN_KERNEL = 1,
    PERFTAG_CALL_FILM_GRAIN_GRV_KERNEL,
    PERFTAG_CALL_FILM_GRAIN_RP1_KERNEL,
    PERFTAG_CALL_FILM_GRAIN_RP2_KERNEL,
    PERFTAG_CALL_FILM_GRAIN_AN_KERNEL
};

struct CodecKernelHeader
{
    union
    {
        struct
        {
            uint32_t                    : 6;
            uint32_t KernelStartPointer : 26;  // GTT 31:6
        };
        struct
        {
            uint32_t Value;
        };
    };
};

class Av1DecodeFilmGrainG12 : public MediaFeature
{
public:
    Av1DecodeFilmGrainG12(MediaFeatureManager *featureManager, DecodeAllocator *allocator, CodechalHwInterface *hwInterface);

    ~Av1DecodeFilmGrainG12();

    //!
    //! \brief  Init Film Grain feature related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings);

    //!
    //! \brief  Update Film Grain feature related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params);

    //!
    //! \brief    Get common kernel header and size
    //!
    //! \param    [in] binary
    //!           Kernel binary
    //!
    //! \param    [in] index
    //!           kernel stage index
    //!
    //! \param    [in] bitDepthIndicator
    //!           Indication of bit depth, 0-8b, 1-10b
    //!
    //! \param    [in] krnHeader
    //!           Kernel header
    //!
    //! \param    [in] krnSize
    //!           Kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetCommonKernelHeaderAndSize(
        void                            *binary,
        FilmGrainKernelStateIdx         index,
        uint8_t                         bitDepthIndicator,
        void                            *krnHeader,
        uint32_t                        *krnSize);

    //!
    //! \brief    Initialize Film Grain Kernel State
    //! \details  Initialize Film Grain Kernel State & Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeKernelState();

    //!
    //! \brief    Send Media VFE cmds
    //! \details  Send Media VFE cmds to setup VFE for media kernel
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] kernelState
    //!           Pointer to MHW kernel state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupMediaVfe(
        PMOS_COMMAND_BUFFER  cmdBuffer,
        MHW_KERNEL_STATE     *kernelState);

    //!
    //! \brief    Allocate fixed size resources
    //! \details  Allocate fixed size resources for film grain kernels
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateFixedSizeSurfaces();

    //!
    //! \brief    Allocate variable size resources
    //! \details  Allocate variable size resources for film grain kernels
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateVariableSizeSurfaces();
    
    //!
    //! \brief    Init scaling function
    //! \details  Calculate scaling LUT according to scaling points
    //! \param    [in] pointValue
    //!           Pointer to scaling point values, corresponding to scaling_points[][0]
    //! \param    [in] pointScaling
    //!           Pointer to point scaling, corresponding to scaling_points[][1]
    //! \param    [in] numPoints
    //!           scaling point number
    //! \param    [out] scalingLUT
    //!           Pointer to scaling LUT
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitScalingFunction(
        uint8_t *pointValue,
        uint8_t *pointScaling,
        uint8_t numPoints,
        int16_t *scalingLUT);

    //!
    //! \brief    Preprocessing scaling points and LUTs
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PreProcScalingPointsAndLUTs();
    
    //!
    //! \brief    Preprocessing AR coefficients for Y/U/V
    //! \param    [out] yCoeff
    //!           Pointer to y coefficients
    //! \param    [out] uCoeff
    //!           Pointer to u coefficients
    //! \param    [out] vCoeff
    //!           Pointer to v coefficients
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PreProcArCoeffs(
        int16_t *yCoeff,
        int16_t *uCoeff,
        int16_t *vCoeff);
    
    //!
    //! \brief    Set frame states per AV1 picture params
    //! \param    [in] picParams
    //!           Pointer to AV1 Decode picture params
    //!
    MOS_STATUS SetFrameStates(
        CodecAv1PicParams *picParams);

    // Parameters passed from application
    CodecAv1SegmentsParams *m_segmentParams         = nullptr;          //!< Pointer to AV1 segments parameter
    CodecAv1TileParams *    m_av1TileParams         = nullptr;          //!< Pointer to AV1 tiles parameter
    bool                    m_filmGrainEnabled      = false;            //!< Per-frame film grain enable flag    

    static const int32_t    m_filmGrainBindingTableCount[kernelNum];    //!< Binding table count for each kernel
    static const int32_t    m_filmGrainCurbeSize[kernelNum];            //!< Curbe size for each kernel
    static const int32_t    m_minLumaLegalRange     = 16;               //!< minimum Luma legal range
    static const int32_t    m_maxLumaLegalRange     = 235;              //!< max Luma legal range
    static const int32_t    m_minChromaLegalRange   = 16;               //!< minimum chroma legal range
    static const int32_t    m_maxChromaLegalRange   = 240;              //!< maximum chroma legal range

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_SURFACE    m_fgOutputSurfList[DecodeBasicFeature::m_maxFrameIndex] = {}; //! \brief fimm grain applied surfaces
#endif

    enum
    {
        fieldTopSrcY        = 0,                                        //!< Binding table offset for Top field input Y
        fieldTopSrcUV       = 1,                                        //!< Binding table offset for Top field input UV
        fieldBotSrcY        = 48,                                       //!< Binding table offset for Bottom field input Y
        fieldBotSrcUV       = 49,                                       //!< Binding table offset for Bottom field input UV
        dstY                = 24,                                       //!< Binding table offset for output Y
        dstUV               = 25,                                       //!< Binding table offset for output UV
        numSurfaces         = 50                                        //!< Number of BT entries for Film Grain
    };

    //combined kernel
    struct FilmGrainCombinedKernelHeader
    {
        int nKernelCount;
        union
        {
            struct
            {
                CodecKernelHeader getRandomValues8b;
                CodecKernelHeader getRandomValues10b;
                CodecKernelHeader regressPhase1;
                CodecKernelHeader regressPhase2For8b;
                CodecKernelHeader regressPhase2For10b;
                CodecKernelHeader applyNoise8b;
                CodecKernelHeader applyNoise10b;
            };
        };
    };

    static const uint32_t           m_maxInputWidth             = 4096;                                 //!< Max input width supported by Film Grain
    static const uint32_t           m_minInputWidth             = 128;                                  //!< Min input width supported by Film Grain
    static const uint32_t           m_maxInputHeight            = 4096;                                 //!< Max input height supported by Film Grain
    static const uint32_t           m_minInputHeight            = 128;                                  //!< Min input height supported by Film Grain

    static const uint32_t           m_initDshSize               = MHW_PAGE_SIZE;                        //!< Init DSH size for Film Grain kernel    
    static const uint32_t           m_numSyncTags               = 16;                                   //!< Sync tags num of state heap settings
    static const float              m_maxScaleRatio;                                                    //!< Maximum scaling ratio for both X and Y directions
    static const float              m_minScaleRatio;                                                    //!< Minimum scaling ratio for both X and Y directions
    static const int32_t            m_bufferPoolDepth            = 8;

    CodechalDecode                  *m_decoder                  = nullptr;                              //!< Pointer to Decode Interface
    MOS_INTERFACE                   *m_osInterface              = nullptr;                              //!< Pointer to OS Interface
    CodechalHwInterface             *m_hwInterface              = nullptr;                              //!< Pointer to HW Interface
    MhwRenderInterface              *m_renderInterface          = nullptr;                              //!< Pointer to Render Interface
    MHW_STATE_HEAP_INTERFACE        *m_stateHeapInterface       = nullptr;                              //!< Pointer to State Heap Interface
    MhwMiInterface                  *m_miInterface              = nullptr;                              //!< Pointer to MI interface.
    uint8_t                         *m_kernelBaseCommon         = nullptr;                              //!< combined kernel base address
    uint32_t                        m_combinedKernelSize        = 0;                                    //!< Combined kernel size
    uint8_t                         m_bitDepthIndicator         = 0;                                    //!< Flag to indicate bit depth, 0-8b, 1-10b

    // Frame Parameters
    CodecAv1PicParams               *m_picParams                 = nullptr;                             //!< Picture Params of AV1
    int16_t                         m_scalingLutY[256]           = {0};                                 //!< Scaling LUT Y
    int16_t                         m_scalingLutCb[256]          = {0};                                 //!< Scaling LUT U
    int16_t                         m_scalingLutCr[256]          = {0};                                 //!< Scaling LUT V
    uint32_t                        m_coordinateSurfaceSize      = 0;                                   //!< Record the existing coordinates random values surface size
    uint16_t                        m_prevRandomSeed             = 0;                                   //!< Previous random seed

    // Surfaces for GetRandomValues
    MOS_BUFFER *                     m_gaussianSequenceSurface          = nullptr;                      //!< Gaussian Sequence surface, 1D buffer, size = 2048 * sizeof(short)
    MOS_SURFACE *                    m_yRandomValuesSurface             = nullptr;                      //!< Y random values 2D surface, size = 70 * 70 * sizeof(short)
    MOS_SURFACE *                    m_uRandomValuesSurface             = nullptr;                      //!< U random values 2D surface, size = 38 * 38 * sizeof(short)
    MOS_SURFACE *                    m_vRandomValuesSurface             = nullptr;                      //!< V random values 2D surface, size = 38 * 38 * sizeof(short)
    MOS_BUFFER *                     m_coordinatesRandomValuesSurface   = nullptr;                      //!< Random values for coordinates, 1D buffer, size = RoundUp(ImageWidth / 64) * RoundUp(ImageHeight / 64) * sizeof(int)

    // Surfaces for RegressPhase1
    MOS_SURFACE *                    m_yDitheringTempSurface            = nullptr;                      //!< First step in generating dithering noise table for Y, 2D surface, size = 70 * 70 * sizeof(short)
    MOS_BUFFER *                     m_yCoefficientsSurface             = nullptr;                      //!< Y Coefficients required for generating dithering noise table, 1D buffer, size = 24 * size(short)
    
    //Surface for RegressionPhase2
    MOS_SURFACE *                    m_yDitheringSurface                = nullptr;                      //!< Y Dithering surface, size = 8 bit: 4 * 64 * 64 * sizeof(char), 10 bit:  4 * 64 * 64 * sizeof(short)
    MOS_SURFACE *                    m_uDitheringSurface                = nullptr;                      //!< U Dithering surface, size = 8 bit:   4 * 32 * 32 * sizeof(char), 10 bit:  4 * 32 * 32 * sizeof(short)
    MOS_SURFACE *                    m_vDitheringSurface                = nullptr;                      //!< V Dithering surface, size = 8 bit:   4 * 32 * 32 * sizeof(char), 10 bit:  4 * 32 * 32 * sizeof(short)
    MOS_BUFFER *                     m_yCoeffSurface                    = nullptr;                      //!< Input Y Coeff surface, size = 32 * sizeof(short), 1D buffer
    MOS_BUFFER *                     m_uCoeffSurface                    = nullptr;                      //!< Input U Coeff surface, size = 32 * sizeof(short), 1D buffer
    MOS_BUFFER *                     m_vCoeffSurface                    = nullptr;                      //!< Input V Coeff surface, size = 32 * sizeof(short), 1D buffer

    // Surfaces for ApplyNoise
    MOS_BUFFER *                     m_yGammaLUTSurface                 = nullptr;                      //!< Input Y Gamma LUT surface, size = 256 * sizeof(short), 1D buffer
    MOS_BUFFER *                     m_uGammaLUTSurface                 = nullptr;                      //!< Input U Gamma LUT surface, size = 256 * sizeof(short), 1D buffer
    MOS_BUFFER *                     m_vGammaLUTSurface                 = nullptr;                      //!< Input V Gamma LUT surface, size = 256 * sizeof(short), 1D buffer

    uint8_t                         *m_kernelBinary[kernelNum];                                         //!< Kernel binary
    uint32_t                        m_kernelUID[kernelNum];                                             //!< Kernel unique ID
    uint32_t                        m_kernelSize[kernelNum];                                            //!< Kernel size
    MHW_KERNEL_STATE                m_kernelStates[kernelNum];                                          //!< Kernel state
    uint32_t                        m_dshSize[kernelNum];                                               //!< DSH size
    MOS_RESOURCE                    m_syncObject = {};                                                  //!< Sync Object

    //!
    //! \brief    Initialize state heap settings and kernel params
    //! \details  Initialize Film Grain Kernel State heap settings & params
    //! \param    [in] hwInterface
    //!           Pointer to HW Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitInterfaceStateHeapSetting();

    //!
    //! \brief    Initialize state heap
    //! \details  Initialize state heap for film grain
    //! \param    [in] hwInterface
    //!           Pointer to HW Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateStateHeap(
        CodechalHwInterface               *hwInterface);

protected:
    DecodeAllocator *    m_allocator         = nullptr;
    Av1BasicFeatureG12 * m_basicFeature      = nullptr;
    bool                 m_resourceAllocated = false;

    // Surfaces arrayfor GetRandomValues
    BufferArray *                     m_coordinatesRandomValuesSurfaceArray   = nullptr;                      //!< Random values for coordinates, 1D buffer, size = RoundUp(ImageWidth / 64) * RoundUp(ImageHeight / 64) * sizeof(int)

    // Surfaces array for RegressPhase1
    BufferArray *                     m_yCoefficientsSurfaceArray             = nullptr;                      //!< Y Coefficients required for generating dithering noise table, 1D buffer, size = 24 * size(short)
    
    //Surface array for RegressionPhase2
    SurfaceArray *                    m_yDitheringSurfaceArray                = nullptr;                      //!< Y Dithering surface, size = 8 bit: 4 * 64 * 64 * sizeof(char), 10 bit:  4 * 64 * 64 * sizeof(short)
    SurfaceArray *                    m_uDitheringSurfaceArray                = nullptr;                      //!< U Dithering surface, size = 8 bit:   4 * 32 * 32 * sizeof(char), 10 bit:  4 * 32 * 32 * sizeof(short)
    SurfaceArray *                    m_vDitheringSurfaceArray                = nullptr;                      //!< V Dithering surface, size = 8 bit:   4 * 32 * 32 * sizeof(char), 10 bit:  4 * 32 * 32 * sizeof(short)
    BufferArray *                     m_yCoeffSurfaceArray                    = nullptr;                      //!< Input Y Coeff surface, size = 32 * sizeof(short), 1D buffer
    BufferArray *                     m_uCoeffSurfaceArray                    = nullptr;                      //!< Input U Coeff surface, size = 32 * sizeof(short), 1D buffer
    BufferArray *                     m_vCoeffSurfaceArray                    = nullptr;                      //!< Input V Coeff surface, size = 32 * sizeof(short), 1D buffer
    
    // Surfaces array for ApplyNoise
    BufferArray *                     m_yGammaLUTSurfaceArray                 = nullptr;                      //!< Input Y Gamma LUT surface, size = 256 * sizeof(short), 1D buffer
    BufferArray *                     m_uGammaLUTSurfaceArray                 = nullptr;                      //!< Input U Gamma LUT surface, size = 256 * sizeof(short), 1D buffer
    BufferArray *                     m_vGammaLUTSurfaceArray                 = nullptr;                      //!< Input V Gamma LUT surface, size = 256 * sizeof(short), 1D buffer
MEDIA_CLASS_DEFINE_END(decode__Av1DecodeFilmGrainG12)
};

}  // namespace decode

#endif  // !__DECODE_AV1_FILMGRAIN_FEATURE_G12_H__

