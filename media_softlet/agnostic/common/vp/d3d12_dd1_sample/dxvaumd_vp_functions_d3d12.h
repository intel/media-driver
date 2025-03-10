/*===================== begin_copyright_notice ==================================

INTEL CONFIDENTIAL
Copyright 2020 - 2022
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and confidential
information of Intel or its suppliers and licensors. The Material is protected by
worldwide copyright and trade secret laws and treaty provisions. No part of the
Material may be used, copied, reproduced, modified, published, uploaded, posted,
transmitted, distributed, or disclosed in any way without Intel's prior express
written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel
or otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

File Name: dxvaumd_vp_functions_d3d12.h

Abstract:
    DXVA User Mode Media Function VP Part d3d12

Environment:
    Windows RS

Notes:

======================= end_copyright_notice ==================================*/
#ifndef __DXVAUMD_VP_FUNCTIONS_D3D12_H__
#define __DXVAUMD_VP_FUNCTIONS_D3D12_H__

#include "dxvaumd_media_functions_d3d12.h"
#include "dxva_d3d12.h"
#include "dxvaumd_d3d12.h"
#include "vphal_ddi_d3d11.h"

class DdiVPFunctionsD3D12 :public DdiMediaFunctionsD3D12
{
public:

    DdiVPFunctionsD3D12() { registerCapsFunctions(); };

    //!
    //! \brief  Register VideoProcessing caps functions
    //! \return void
    //!
    void registerCapsFunctions();

    //!
    //! \brief  Video Process Frame
    //! \param  [in]  drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in]  drvVideoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \param  [in]  outputParameters
    //!         Pointer to output parameters
    //! \param  [in]  inputStreamParameters
    //!         Pointer to input parameters
    //! \param  [in]  numInputStreams
    //!         Count of input streams
    //! \return void
    //!
    virtual VOID VideoProcessFrame(
        D3D12DDI_HCOMMANDLIST                                         drvCommandList,
        D3D12DDI_HVIDEOPROCESSOR_0020                                 drvVideoProcessor,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032  *outputParameters,
        const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032   *inputStreamParameters,
        uint32_t                                                      numInputStreams);

    //!
    //! \brief  Video Process Frame
    //! \param  [in]  drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in]  drvVideoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \param  [in]  outputParameters
    //!         Pointer to output parameters
    //! \param  [in]  inputStreamParameters
    //!         Pointer to input parameters
    //! \param  [in]  numInputStreams
    //!         Count of input streams
    //! \return void
    //!
    virtual VOID VideoProcessFrame(
        D3D12DDI_HCOMMANDLIST                                         drvCommandList,
        D3D12DDI_HVIDEOPROCESSOR_0020                                 drvVideoProcessor,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032  *outputParameters,
        const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043   *inputStreamParameters,
        uint32_t                                                      numInputStreams);

    //!
    //! \brief  Calculate Video Processor Size
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032
    //! \return SIZE_T
    //!
    virtual SIZE_T CalcPrivateVideoProcessorSize(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032 *createVideoProcessor);

    //!
    //! \brief  Calculate Video Processor Size
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043
    //! \return SIZE_T
    //!
    virtual SIZE_T CalcPrivateVideoProcessorSize(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043 *createVideoProcessor);

    //!
    //! \brief  Calculate Video Processor Size
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072
    //! \return SIZE_T
    //!
    virtual SIZE_T CalcPrivateVideoProcessorSize(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072 *createVideoProcessor);

    //!
    //! \brief  Create Video Processor
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032
    //! \param  [in]  videoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \return HRESULT
    //!
    virtual HRESULT CreateVideoProcessor(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032 *createVideoProcessor,
        D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor);

    //!
    //! \brief  Create Video Processor
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043
    //! \param  [in]  videoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \return HRESULT
    //!
    virtual HRESULT CreateVideoProcessor(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043 *createVideoProcessor,
        D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor);

    //!
    //! \brief  Create Video Processor
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072
    //! \param  [in]  videoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \return HRESULT
    //!
    virtual HRESULT CreateVideoProcessor(
        D3D12DDI_HDEVICE                              deviceD3D12,
        const D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072 *createVideoProcessor,
        D3D12DDI_HVIDEOPROCESSOR_0020                 videoProcessor);

    //!
    //! \brief  Destroy Video Processor
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  videoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \return void
    //!
    virtual VOID DestroyVideoProcessor(
        D3D12DDI_HDEVICE              deviceD3D12,
        D3D12DDI_HVIDEOPROCESSOR_0020 videoProcessor);

    //!
    //! \brief  Check whether the specified format can be used as videoprocessor output format
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  format
    //!         output format
    //! \param  [out]  formatCaps
    //!         Pointer of format supporting flag
    //! \return void
    //!
    virtual void CheckFormatSupport(
        D3D12DDI_HDEVICE deviceD3D12,
        DXGI_FORMAT      format,
        uint32_t         *formatCaps);

    //!
    //! \brief  Implementation of VideoProcessCapsSupport
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  data
    //!         Pointer to D3D12DDI_VIDEO_PROCESS_SUPPORT_DATA_0032
    //! \param  [in]  dataSize
    //!         Data size
    //! \return HRESULT
    //!
    static HRESULT VideoProcessCapsSupport(
        D3D12DDI_HDEVICE deviceD3D12,
        void             *data,
        uint32_t         dataSize);

    //!
    //! \brief  Implementation of VideoProcessCapsMaxInputStreams
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  data
    //!         Pointer to D3D12DDI_VIDEO_PROCESS_MAX_INPUT_STREAMS_DATA_0020
    //! \param  [in]  dataSize
    //!         Data size
    //! \return HRESULT
    //!
    static HRESULT VideoProcessCapsMaxInputStreams(
        D3D12DDI_HDEVICE deviceD3D12,
        void             *data,
        uint32_t         dataSize);

    //!
    //! \brief  Implementation of VideoProcessCapsReferenceInfo
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  data
    //!         Pointer to D3D12DDI_VIDEO_PROCESS_REFERENCE_INFO_DATA_0020
    //! \param  [in]  dataSize
    //!         Data size
    //! \return HRESULT
    //!
    static HRESULT VideoProcessCapsReferenceInfo(
        D3D12DDI_HDEVICE deviceD3D12,
        void             *data,
        uint32_t         dataSize);

private:
    //!
    //! \brief  Calculate Video Processor Size
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \return SIZE_T
    //!
    SIZE_T CalcPrivateVideoProcessorSize(
        D3D12DDI_HDEVICE deviceD3D12);

    //!
    //! \brief  Implementation of CreateVideoProcessor
    //! \param  [in]  deviceD3D12
    //!         Handle of D3D12 context
    //! \param  [in]  createVideoProcessor
    //!         Pointer to template D3D12DDIARG_CREATE_VIDEO_PROCESSOR
    //! \param  [in]  videoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \return HRESULT
    //!
    template <class T>
    HRESULT CreateVideoProcessorT(
        D3D12DDI_HDEVICE              deviceD3D12,
        const T                       *createVideoProcessor,
        D3D12DDI_HVIDEOPROCESSOR_0020 videoProcessor);

    BOOL CheckMemoryBlockNotZero(char *addr, uint32_t count);

    //!
    //! \brief  This function converts color space
    //! \param  [in]  colorSpace
    //!         DXGI_COLOR_SPACE_TYPE value
    //! \return D3D11_1DDI_STREAM_STATE_COLOR_SPACE
    //!
    D3D11_1DDI_STREAM_STATE_COLOR_SPACE ConvertColorSpace(
        DXGI_COLOR_SPACE_TYPE colorSpace);

    //!
    //! \brief  This function converts color space to ChromaSiting
    //! \param  [in]  colorSpace
    //!         DXGI_COLOR_SPACE_TYPE value
    //! \return uint32_t
    //!
    uint32_t ColorSpace2ToChromaSiting(
        DXGI_COLOR_SPACE_TYPE colorSpace);

    //!
    //! \brief  This function converts color space to GammaValue
    //! \param  [in]  colorSpace
    //!         DXGI_COLOR_SPACE_TYPE value
    //! \param  [in]  hdrState
    //!         pointer to VPE_HDR_STATE
    //! \return VPHAL_GAMMA_VALUE
    //!
    VPHAL_GAMMA_VALUE ColorSpace2ToGammaValue(
        DXGI_COLOR_SPACE_TYPE colorSpace,
        VPE_HDR_STATE         *hdrState);

    //!
    //! \brief  Initialize Vphal at ddi layer
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  createVideoProcessor
    //!         pointer to D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0032/D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0043/D3D12DDIARG_CREATE_VIDEO_PROCESSOR_0072
    //! \param  [in]  adapterInfo
    //!         pointer to ADAPTER_INFO
    //! \return HRESULT
    //!
    template <class T>
    HRESULT InitDdiVphal(
        PDXVA_VP_DATA vpData,
        const T       *createVideoProcessor,
        PADAPTER_INFO adapterInfo);

    //!
    //! \brief  Set field type
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  inputStreams
    //!         pointer to D3D12DDI_VIDEO_PROCESSOR_INPUT_STREAM_DESC_0032
    //! \param  [in]  stream
    //!         stream index
    //! \return void
    //!
    void SetFieldType(
        PDXVA_VP_DATA                                         vpData,
        const D3D12DDI_VIDEO_PROCESSOR_INPUT_STREAM_DESC_0032 *inputStreams,
        uint32_t                                              stream);

    //!
    //! \brief  Set field type
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  inputStreams
    //!         pointer to D3D12DDI_VIDEO_PROCESSOR_INPUT_STREAM_DESC_0043
    //! \param  [in]  stream
    //!         stream index
    //! \return void
    //!
    template <class T>
    void SetFieldType(
        PDXVA_VP_DATA vpData,
        const T       *inputStreams,
        uint32_t      stream);

    //!
    //! \brief  Set Input Stream States field type
    //! \param  [in/out]  vpData
    //!         pointer to  DXVA_VP_DATA
    //! \param  [in]  istream
    //!         input streams
    //! \param  [in]  inputStreamParameters
    //!         pointer to  D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \return void
    //!
    void SetFieldType(
        PDXVA_VP_DATA                                               vpData,
        const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 *inputStreamParameters,
        uint32_t                                                    istream);

    //!
    //! \brief  Set filter flags
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  filterFlags
    //!         D3D12DDI_VIDEO_PROCESS_FILTER_FLAGS_0020 value
    //! \param  [in]  adapterInfo
    //!         pointer to ADAPTER_INFO
    //! \param  [in]  stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetFilterFlags(
        PDXVA_VP_DATA                            vpData,
        D3D12DDI_VIDEO_PROCESS_FILTER_FLAGS_0020 filterFlags,
        PADAPTER_INFO                            adapterInfo,
        uint32_t                                 stream);

    //!
    //! \brief  Set back ground color
    //! \param  [out]  bltStateBackgroundColor
    //!         INTEL_BLT_STATE_BACKGROUND_COLOR_DATA pointer
    //! \param  [in]  outputStream
    //!          D3D12DDI_VIDEO_PROCESS_OUTPUT_STREAM_DESC_0032 pointer
    //! \return void
    //!
    void SetBackGroundColor(
        INTEL_BLT_STATE_BACKGROUND_COLOR_DATA                *bltStateBackgroundColor,
        const D3D12DDI_VIDEO_PROCESS_OUTPUT_STREAM_DESC_0032 *outputStream);

    //!
    //! \brief  Set stereo enabling
    //! \param  [in]  enableStereo
    //!         enable Stereo
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  adapterInfo
    //!         pointer to ADAPTER_INFO
    //! \return HRESULT
    //!
    HRESULT SetStereoEnabling(
        int           enableStereo,
        PDXVA_VP_DATA vpData,
        PADAPTER_INFO adapterInfo);

    //!
    //! \brief  This function initializes vpda from RegistryKey
    //! \param  [in]  userSettingPtr
    //!         pointer to user setting
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  mediaResetCount
    //!         pointer
    //! \return void
    //!
    void InitVpdataFromRegistryKey(
        MediaUserSettingSharedPtr userSettingPtr,
        PDXVA_VP_DATA             vpData,
        PDWORD                    mediaResetCount);

    //!
    //! \brief  Video Process Frame
    //! \param  [in]  drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in]  drvVideoProcessor
    //!         Handle of D3D12DDI_HVIDEOPROCESSOR_0020
    //! \param  [in]  outputParameters
    //!         Pointer to output parameters
    //! \param  [in]  inputStreamParameters
    //!         Pointer to template input parameters
    //! \param  [in]  numInputStreams
    //!         Count of input streams
    //! \return void
    //!
    template <class T>
    void VideoProcessFrameT(
        D3D12DDI_HCOMMANDLIST                                        drvCommandList,
        D3D12DDI_HVIDEOPROCESSOR_0020                                drvVideoProcessor,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputParameters,
        const T                                                      *inputStreamParameters,
        uint32_t                                                     numInputStreams);

    //!
    //! \brief  Check if format is RGB
    //! \param  [in]  colorSpace
    //!         DXGI_COLOR_SPACE_TYPE value
    //! \return bool
    //!
    bool CheckRGBFormat(DXGI_COLOR_SPACE_TYPE colorSpace);

    //!
    //! \brief  Set VpData Stream States Informations
    //! \param  [in/out]  vpData
    //!         pointer to  DXVA_VP_DATA
    //! \param  [in]  numInputStreams
    //!         input streams counts
    //! \param  [in]  inputStreamParameters
    //!         pointer to  D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 / D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \return void
    //!
    template <class T>
    void SetVpDataStreamStateInfos(
        PDXVA_VP_DATA vpData,
        const T       *inputStreamParameters,
        uint32_t      numInputStreams);

    //!
    //! \brief  Set VphalRenderParams Prediction And Marker params
    //! \param  [out]  vpData
    //!         pointer to  PDXVA_VP_DATA
    //! \param  [in]  commandList
    //!         DXVA12_COMMAND_LIST pointer
    //! \param  [in]  dxvaDevice
    //!         pointer to  DXVA_DEVICE_CONTEXT
    //! \param  [in]  resPredicationDdiResource
    //!         DXVAUMD_RESOURCE pointer
    //! \param  [in]  resSetMarkerDdiResource
    //!         DXVAUMD_RESOURCE pointer
    //! \param  [in]  osInterface
    //!         PMOS_INTERFACE pointer
    //! \return HRESULT
    //!
    HRESULT SetPredictionAndMarker(
        PDXVA_VP_DATA        vpData,
        DXVA12_COMMAND_LIST  *commandList,
        PMOS_CONTEXT         dxvaDevice,
        MOS_RESOURCE         &resPredication,
        MOS_RESOURCE         &resSetMarker,
        PMOS_INTERFACE       osInterface);

    //!
    //! \brief  Set alpha parameters (from App)
    //! \param  [in]  compAlpha
    //!         pointer to  VPHAL_ALPHA_PARAMS
    //! \param  [in]  stateAlphaFillData
    //!         struct of INTEL_BLT_STATE_ALPHA_FILL_DATA
    //! \param  [in]  videoIndex
    //!         index
    //! \return HRESULT
    //!
    HRESULT SetAlphaParams(
        PVPHAL_ALPHA_PARAMS             &compAlpha,
        INTEL_BLT_STATE_ALPHA_FILL_DATA stateAlphaFillData,
        int                             videoIndex);

    //!
    //! \brief  Set background color params
    //! \param  [in]  vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in]  renderParams
    //!          VPHAL_RENDER_PARAMS_EXT pointer
    //! \return void
    //!
    HRESULT SetBackGroundColorParams(
        PDXVA_VP_DATA           vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams);

    //!
    //! \brief  Set rotation and mirror params
    //! \param  [in]  stmStateRotation
    //!         INTEL_STREAM_STATE_ROTATION_DATA
    //! \param  [in]  stmStateMirror
    //!         INTEL_STREAM_STATE_MIRROR_DATA
    //! \param  [in/out]  rotation
    //!         rotation value
    //! \return void
    //!
    void SetRotationAndMirror(
        INTEL_STREAM_STATE_ROTATION_DATA stmStateRotation,
        INTEL_STREAM_STATE_MIRROR_DATA   stmStateMirror,
        VPHAL_ROTATION                   &rotation);

    //!
    //! \brief  Set deinterlacing flags
    //! \param  [in]  stmStateFrameFormat
    //!         INTEL_VIDEO_FRAME_FORMAT
    //! \param  [in]  auxSample
    //!         COMPOSITING_SAMPLE_D3D11_1
    //! \param  [in]  rtTarget
    //!         REFERENCE_TIME
    //! \param  [in]  surface
    //!         pointer to VPHAL_SURFACE_EXT
    //! \param  [in]  outputIndex
    //!         index value
    //! \return void
    //!
    void Setdeinterlacingflags(
        INTEL_VIDEO_FRAME_FORMAT   stmStateFrameFormat,
        COMPOSITING_SAMPLE_D3D11_1 &auxSample,
        VPHAL_SURFACE_EXT          *surface,
        REFERENCE_TIME             &rtTarget,
        uint32_t                   outputIndex);

    //!
    //! \brief  Adopt the frame rate and deinterlace mode of primary video
    //! \param  [in]  vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in]  renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in]  stream
    //!         index value
    //! \return void
    //!
    void GetFrameRateandDeinterlaceMode(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        uint32_t                stream);

    //!
    //! \brief  Set stereo params
    //! \param  [in]  vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in]  renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] inputStreamParameters
    //!         D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032/ D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         input streams count
    //! \return HRESULT
    //!
    template <class T>
    HRESULT SetStereoParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        const T                 *inputStreamParameters,
        VPHAL_SURFACE_EXT       *surface,
        uint32_t                stream);

    //!
    //! \brief  Set the rendering parameters for vphal
    //! \param  [in] drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] outputStreamParameters
    //!         D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 pointer
    //! \param  [in] inputStreamParameters
    //!         D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032/ D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 pointer
    //! \param  [in] streamCount
    //!         input streams count
    //! \return HRESULT
    //!
    template <class T>
    HRESULT DdiSetVpHalRenderingParams(
        D3D12DDI_HCOMMANDLIST                                           drvCommandList,
        PDXVA_VP_DATA                                                   vpData,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032    *outputStreamParameters,
        const T                                                         *inputStreamParameters,
        uint32_t                                                        streamCount);

    //!
    //! \brief  Checks if Screen Capture Defense is enabled for input and output views
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  outputStreamParameters
    //!         D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 pointer
    //! \param  [in]  streamCount
    //!         stream count
    //! \param  [in]  inputStreamParameters
    //!         Porinter to D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032/ D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \return bool
    //!
    template <class T>
    bool IsValidViewsForSCD(
        DXVA_DEVICE_CONTEXT                                          *dxvaDevice,
        DXVA_VP_DATA                                                 *vpData,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
        uint32_t                                                     streamCount,
        const T                                                      *inputStreamParameters);

public:

    //!
    //! \brief  Set Procamp params in VPHAL
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalProcampParams(
        PDXVA_VP_DATA        vpData,
        PVPHAL_RENDER_PARAMS renderParams,
        PVPHAL_SURFACE       surface,
        uint32_t             stream);

    //!
    //! \brief  Set NLAS params in VPHAL
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalNLASParams(
        PDXVA_VP_DATA            vpData,
        VPHAL_RENDER_PARAMS_EXT  *renderParams,
        VPHAL_SURFACE_EXT        *surface,
        uint32_t                 stream);

    //!
    //! \brief  Set denoise params in VPHAL
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \param  [in] resource
    //!         DXVAUMD_RESOURCE pointer
    //! \return HRESULT
    //!
    HRESULT SetVpHalDenoiseParams(
        PDXVA_VP_DATA        vpData,
        PVPHAL_RENDER_PARAMS renderParams,
        PVPHAL_SURFACE       surface,
        uint32_t             stream,
        DXVAUMD_RESOURCE     resource);

    //!
    //! \brief  Set istab in VPHAL
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalISParams(
        PDXVA_VP_DATA           vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        uint32_t                stream);

    //!
    //! \brief  Set ief params in VPHAL
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalIEFParams(
        PDXVA_VP_DATA        vpData,
        PVPHAL_RENDER_PARAMS renderParams,
        PVPHAL_SURFACE       surface,
        uint32_t             stream);

    //!
    //! \brief  Set colorpipe in VPHAL
    //! \param  [in] context
    //!         device context pointer
    //! \param  [in] vpData
    //!         PDXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalColorPipeParams(
        DXVA_DEVICE_CONTEXT  *context,
        PDXVA_VP_DATA        vpData,
        PVPHAL_RENDER_PARAMS renderParams,
        PVPHAL_SURFACE_EXT   surface,
        uint32_t             stream);

    //!
    //! \brief  Set Gamut compression in VPHAL
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \return HRESULT
    //!
    HRESULT SetVpHalGamutCompressionParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        VPHAL_SURFACE_EXT       *surface,
        uint32_t                stream);

    //!
    //! \brief  Set FDFB params in VPHAL
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \param  [in] contentWidth
    //!         content width
    //! \param  [in] contentHeight
    //!         content height
    //! \return MOS_STATUS
    //!
    HRESULT SetVpHalFDFBParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        VPHAL_SURFACE_EXT       *surface,
        uint32_t                stream,
        DWORD                   contentWidth,
        DWORD                   contentHeight);

    //!
    //! \brief  Set Super Resolution params in VPHAL
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \return HRESULT
    //!
    HRESULT SetVpHalSuperResolutionParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        VPHAL_SURFACE_EXT       *surface);

    //!
    //! \brief  Set Vpe Scaling Mode
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  renderParams
    //!         pointer to VPHAL_RENDER_PARAMS_EXT
    //! \param  [in/out]  surface
    //!         pointer to VPHAL_SURFACE_EXT
    //! \param  [in]  stream
    //!         stream index
    //! \return HRESULT
    //!
    void SetVpeScalingtMode(
        DXVA_DEVICE_CONTEXT     *dxvaDevice,
        PDXVA_VP_DATA           vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        VPHAL_SURFACE_EXT       *surface,
        uint32_t                stream);

    //!
    //! \brief  Set FRC params in VPHAL
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  renderParams
    //!         pointer to VPHAL_RENDER_PARAMS_EXT
    //! \param  [in]  streams
    //!         pointer to D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 or D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \param  [in]  stream
    //!         stream index
    //! \return HRESULT
    //!
    template <class T>
    HRESULT SetVpHalFrcParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        const T                 *streams,
        uint32_t                stream);

    //!
    //! \brief  Set Capture Pipe params in VPHAL
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] renderParams
    //!         VPHAL_RENDER_PARAMS_EXT pointer
    //! \param  [in] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \return HRESULT
    //!
    HRESULT SetVpHalCapPipeParams(
        DXVA_VP_DATA            *vpData,
        VPHAL_RENDER_PARAMS_EXT *renderParams,
        VPHAL_SURFACE_EXT       *surface);

private:
    //!
    //! \brief  Detect if any of the input streams is protected.
    //! \param  [in]  vpData
    //!         pointer to  DXVA_VP_DATA
    //! \param  [in]  streamCount
    //!         input streams count
    //! \param  [in]  streams
    //!         pointer to  D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 / D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \return bool
    //!
    template <class T>
    bool IsVpInputViewsProtected(
        DXVA_VP_DATA *vpData,
        uint32_t     streamCount,
        const T      *streams);

    //!
    //! \brief  Detect if input view is protected.
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in]  inputTexture
    //!         struct of D3D12DDI_HRESOURCE
    //! \return bool
    //!
    bool IsInputViewProtected(
        DXVA_VP_DATA            *vpData,
        D3D12DDI_HRESOURCE      inputTexture);

    //!
    //! \brief  Set Backward Reference
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] streams
    //!         D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 or D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 pointer
    //! \param  [in] rightFrame
    //!         right frame
    //! \param  [in/out] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \param  [in] rtTarget
    //!         Time stamp for render target
    //! \param  [in] diParams
    //!         di params pointer
    //! \param  [in] invalidCaps
    //!         Represent unsupported capabilities for this surface type
    //! \param  [in] yuvRange
    //!         Requested YUV Range
    //! \param  [in] sample
    //!         COMPOSITING_SAMPLE_D3D11_1 pointer
    //! \return HRESULT
    //!
    template <class T>
    HRESULT SetVpHalBackwardRefs(
        DXVA_VP_DATA               *vpData,
        const T                    *streams,
        bool                       rightFrame,
        PVPHAL_SURFACE_EXT         surface,
        uint32_t                   istream,
        REFERENCE_TIME             rtTarget,
        VPHAL_DI_PARAMS            *diParams,
        DWORD                      invalidCaps,
        DWORD                      yuvRange,
        COMPOSITING_SAMPLE_D3D11_1 *sample);

    //!
    //! \brief  Set Forward Reference
    //! \param  [in] vpData
    //!         DXVA_VP_DATA pointer
    //! \param  [in] streams
    //!         D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 or D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 pointer
    //! \param  [in] rightFrame
    //!         right frame
    //! \param  [in/out] surface
    //!         VPHAL_SURFACE_EXT pointer
    //! \param  [in] stream
    //!         stream index
    //! \param  [in] rtTarget
    //!         Time stamp for render target
    //! \param  [in] diParams
    //!         di params pointer
    //! \param  [in] invalidCaps
    //!         Represent unsupported capabilities for this surface type
    //! \param  [in] yuvRange
    //!         Requested YUV Range
    //! \param  [in] sample
    //!         COMPOSITING_SAMPLE_D3D11_1 pointer
    //! \return HRESULT
    //!
    template <class T>
    HRESULT SetVpHalForwardRefs(
        DXVA_VP_DATA               *vpData,
        const T                    *streams,
        BOOL                       rightFrame,
        PVPHAL_SURFACE_EXT         surface,
        uint32_t                   iStream,
        REFERENCE_TIME             rtTarget,
        VPHAL_DI_PARAMS            *diParams,
        DWORD                      invalidCaps,
        DWORD                      yuvRange,
        COMPOSITING_SAMPLE_D3D11_1 *sample);

    //!
    //! \brief  Initialize the resource infomration of input & output streams
    //! \param  [in]  drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  outputStreamParameters
    //!         Pointer to output parameters
    //! \param  [in]  inputStreamParameters
    //!         Pointer to template input parameters
    //! \param  [in]  streamCount
    //!         Count of input streams
    //! \return HRESULT
    //!
    template <class T>
    HRESULT InitializeDxvaResourceInfo(
        D3D12DDI_HCOMMANDLIST                                        drvCommandList,
        DXVA_VP_DATA                                                 *vpData,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
        const T                                                      *inputStreamParameters,
        uint32_t                                                     streamCount);

    //!
    //! \brief  Destroy the resource infomration of input & output streams
    //! \param  [in]  drvCommandList
    //!         Handle of D3D12DDI_HCOMMANDLIST
    //! \param  [in]  vpData
    //!         pointer to DXVA_VP_DATA
    //! \param  [in]  outputStreamParameters
    //!         Pointer to output parameters
    //! \param  [in]  inputStreamParameters
    //!         Pointer to template input parameters
    //! \param  [in]  streamCount
    //!         Count of input streams
    //! \return void
    //!
    template <class T>
    void DestroyDxvaResourceInfo(
        D3D12DDI_HCOMMANDLIST                                        drvCommandList,
        DXVA_VP_DATA                                                 *vpData,
        const D3D12DDIARG_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS_0032 *outputStreamParameters,
        const T                                                      *inputStreamParameters,
        uint32_t                                                     streamCount);

    //!
    //! \brief  Destroy the DxvaResourceInfo of resource
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  inputTexture
    //!         resource
    //! \return HRESULT
    //!
    HRESULT InitDXVAResourceInfo(
        DXVA_DEVICE_CONTEXT *dxvaDevice,
        D3D12DDI_HRESOURCE  inputTexture);

    //!
    //! \brief  Destroy the DxvaResourceInfo of resource
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  inputTexture
    //!         resource
    //! \return void
    //!
    void DestroyDXVAResourceInfo(
        DXVA_DEVICE_CONTEXT *dxvaDevice,
        D3D12DDI_HRESOURCE  inputTexture);

    //!
    //! \brief  This function sets command lists and pool
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  inputStreamParameters
    //!         pointer to D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032
    //! \param  [in/out]  commandList
    //!         pointer to DXVA12_COMMAND_LIST
    //! \param  [in]  osInterface
    //!         pointer to PMOS_INTERFACE
    //! \return HRESULT
    //!
    HRESULT SetupCmdListAndPool(
        PMOS_CONTEXT                                                dxvaDevice,
        const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0032 *inputStreamParameters,
        DXVA12_COMMAND_LIST                                         *commandList,
        PMOS_INTERFACE                                              osInterface);

    //!
    //! \brief  This function sets command lists and pool
    //! \param  [in]  dxvaDevice
    //!         pointer to DXVA_DEVICE_CONTEXT
    //! \param  [in]  inputStreamParameters
    //!         pointer to D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043
    //! \param  [in/out]  commandList
    //!         pointer to DXVA12_COMMAND_LIST
    //! \param  [in]  osInterface
    //!         pointer to PMOS_INTERFACE
    //! \return HRESULT
    //!
    HRESULT SetupCmdListAndPool(
        PMOS_CONTEXT                                                dxvaDevice,
        const D3D12DDIARG_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS_0043 *inputStreamParameters,
        DXVA12_COMMAND_LIST                                         *commandList,
        PMOS_INTERFACE                                              osInterface);

protected:
    static const uint32_t m_maxinputstreams;

MEDIA_CLASS_DEFINE_END(DdiVPFunctionsD3D12)
};
#endif