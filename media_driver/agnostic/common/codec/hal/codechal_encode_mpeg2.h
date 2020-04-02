/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_mpeg2.h
//! \brief    Defines base class for MPEG2 dual-pipe encoder.
//!

#ifndef __CODECHAL_ENCODE_MPEG2_H__
#define __CODECHAL_ENCODE_MPEG2_H__

#include "codechal_encoder_base.h"

class CodechalKernelHme;

//!
//! \class   CodechalEncodeMpeg2
//! \brief   MPEG2 dual-pipe encoder base class
//! \details This class defines the base class for MPEG2 dual-pipe encoder, it includes
//!          common member fields, functions, interfaces etc shared by all GENs.
//!          Gen specific definitions, features should be put into their corresponding classes.
//!          To create a MPEG2 dual-pipe encoder instance, client needs to new the instance in media interfaces
//!
class CodechalEncodeMpeg2 : public CodechalEncoderState
{
public:

    //!
    //! \brief    Copy construtor
    //!
    CodechalEncodeMpeg2(const CodechalEncodeMpeg2&) = delete;


    //!
    //! \brief    Copy assignment construtor
    //!
    CodechalEncodeMpeg2& operator=(const CodechalEncodeMpeg2&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeMpeg2();

    //!
    //! \brief    Allocate resources for encoder instance
    //! \details  It is invoked when initializing encoder instance and it would call #AllocateEncResources(), #AllocateBrcResources()
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    //!
    //! \brief    Free encoder resources
    //! \details  It is invoked when destorying encoder instance and it would call #FreeEncResources(), #FreeBrcResources()
    //!           and FreePakResources()
    //!
    //! \return   void
    //!
    void FreeResources() override;

    //!
    //! \brief    Initialize encoder at picture level
    //!
    //! \param    [in] params
    //!           Picture encoding parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializePicture(const EncoderParams& params) override;

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteKernelFunctions() override;

    //!
    //! \brief    Encode command at picture level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecutePictureLevel() override;

    //!
    //! \brief    Encode command at slice level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteSliceLevel() override;

    //!
    //! \brief    Copy skip frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeCopySkipFrame() override;

    //!
    //! \brief    Initialize encoder instance
    //! \details  When GEN specific derived class implements this function to do its own initialization,
    //            it is required that the derived class calls #CodechalEncodeMpeg2::Initialize() first
    //            which would do common initialization for all GENs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(CodechalSetting * codecHalSettings) override;

    //!
    //! \brief  Inserts the generic prologue command for a command buffer
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] frameTracking
    //!         Indicate if frame tracking requested
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    CodecEncodeMpeg2PictureParams          *m_picParams = nullptr;        //!< Pointer to picture parameter
    PCODEC_REF_LIST                        m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2];  //!< Pointer to reference list

    // Codec to define its own GetStatusReport
    MOS_STATUS GetStatusReport(
        EncodeStatus       *encodeStatus,
        EncodeStatusReport *encodeStatusReport) override { return MOS_STATUS_SUCCESS; }

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;

protected:

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeMpeg2(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Help function to allocate a 1D buffer
    //!
    //! \param    [in,out] buffer
    //!           Pointer to allocated buffer
    //! \param    [in] bufSize
    //!           Buffer size
    //! \param    [in] name
    //!           Buffer name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer(
        PMOS_RESOURCE           buffer,
        uint32_t                bufSize,
        PCCHAR                  name);

    //!
    //! \brief    Help function to allocate a generic 2D surface
    //!
    //! \param    [in,out] surface
    //!           Pointer to allocated surface
    //! \param    [in] surfWidth
    //!           Surface width
    //! \param    [in] surfHeight
    //!           Surface height
    //! \param    [in] name
    //!           Surface name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer2D(
        PMOS_SURFACE         surface,
        uint32_t             surfWidth,
        uint32_t             surfHeight,
        PCCHAR               name);

    //!
    //! \brief    Help function to allocate a 1D buffer
    //!
    //! \param    [in,out] batchBuffer
    //!           Pointer to allocated batch buffer
    //! \param    [in] bufSize
    //!           Buffer size
    //! \param    [in] name
    //!           Batch buffer name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBatchBuffer(
        PMHW_BATCH_BUFFER            batchBuffer,
        uint32_t                     bufSize,
        PCCHAR                       name);

    //!
    //! \brief    Allocate resources for ENC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateEncResources();

    //!
    //! \brief    Allocate BRC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBrcResources();

    //!
    //! \brief    Free BRC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeBrcResources();

    //!
    //! \brief    Free ENC resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeEncResources();

    //!
    //! \brief    Check profile and level
    //! \details  Check if the required profile and level are supported by driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckProfileAndLevel();

    //!
    //! \brief    Setup/configure encoder based on sequence parameter set
    //! \details  It is invoked when the encoder receives a new sequence parameter set and it would
    //!           set up and configure the encoder state that used for the sequence
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Setup/configure encoder based on picture parameter set
    //! \details  It is invoked for every picture and it would set up and configure the
    //            encoder state that used for current picture
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPictureStructs();

    //!
    //! \brief    Setup/configure Slice Group
    //! \details  It is invoked for every picture and it would set up and configure the
    //            encoder state that used for current picture
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSliceGroups();

    //!
    //! \brief    Get current byte offset
    //! \details  Get current byte offset for the bit stream buffer
    //! \param    bsBuffer
    //!           [in] Bit stream buffer
    //! \return   uint32_t
    //!           Byte offset
    //!
    uint32_t GetCurByteOffset(BSBuffer* bsBuffer);

    //!
    //! \brief    Pack display sequence extension
    //! \details  Pack display sequence extension, MPEG2 Spec 6.2.2.4
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackDisplaySeqExtension();

    //!
    //! \brief    Pack sequence extension
    //! \details  Pack sequence extension, MPEG2 Spec 6.2.2.3
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackSeqExtension();

    //!
    //! \brief    Pack sequence header
    //! \details  Pack sequence extension
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackSeqHeader();

    //!
    //! \brief    Pack sequence parameters
    //! \details  Pack sequence parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackSequenceParams();

    //!
    //! \brief    Pack picture coding extension
    //! \details  Pack picture coding extension, MPEG2 Spec 6.2.3.1
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackPicCodingExtension();

    //!
    //! \brief    Pack picture user data
    //! \details  Pack picture user data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackPicUserData();

    //!
    //! \brief    Pack picture header
    //! \details  Pack picture header, MPEG2 Spec 6.2.3
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackPicHeader();

    //!
    //! \brief    Pack group of pictures header
    //! \details  Pack group of pictures header, MPEG2 Spec 6.2.2.6
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackGroupOfPicHeader();

    //!
    //! \brief    Pack picture paramters
    //! \details  Pack icture paramters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackPictureParams();

    //!
    //! \brief    Pack Picture Header
    //! \details  Function to Pack Picture Header
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackPictureHeader();

    //!
    //! \brief    Pack skip slice data
    //! \details  Function to pack skip slice data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackSkipSliceData();

    //!
    //! \brief    Pack skipped MB
    //! \details  Function to pack skipped MB
    //! \param    [in]  mbIncrement
    //!           Number MBs for slice
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS PackSkippedMB(uint32_t mbIncrement);

    //!
    //! \brief    Invoke ME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeMeKernel();

    //!
    //! \brief    Calculate frame rate value
    //!
    //! \param    [in] frameRateCode
    //!           Frame rate code
    //! \param    [in] factor
    //!           factor
    //!
    //! \return   uint32_t
    //!           Frame rate value
    //!
    uint32_t CalcFrameRateValue(uint16_t frameRateCode, uint32_t factor);

    //!
    //! \brief    Setup Curbe for BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcInitReset();

    //!
    //! \brief    Send surfaces BRC Init/Reset kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcInitResetSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Invoke BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcInitResetKernel();

    //!
    //! \brief    Top level function for invoking MBenc kernel
    //!
    //! \param    [in]  mbEncIFrameDistEnabled
    //!           Indicate if MbEnc I-Frame distortion is enabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeMbEncKernel(bool mbEncIFrameDistEnabled);

    //!
    //! \brief    Send surfaces for BRC Update kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for BRC Update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcUpdate();

    //!
    //! \brief    Initialize for BRC constant buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitBrcConstantBuffer();

    //!
    //! \brief    Invoke BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcUpdateKernel();

    //!
    //! \brief    Send Slice parameters
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  params
    //!           Pointer to PMHW_VDBOX_MPEG2_SLICE_STATE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSliceParams(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_MPEG2_SLICE_STATE    params);

    //!
    //! \brief    Send Surfaces for MbEnc kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  mbEncIFrameDistEnabled
    //!           Indicate if MbEnc I-Frame distortion is enabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMbEncSurfaces(
        PMOS_COMMAND_BUFFER  cmdBuffer,
        bool mbEncIFrameDistEnabled);

    //!
    //! \brief    Initialize kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState() = 0;

    //!
    //! \brief    Get maximum BT count
    //!
    //! \return   uint32_t
    //!           Maximum BT count
    //!
    virtual uint32_t GetMaxBtCount();

    //!
    //! \brief    Prepare the Curbe for ME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeMe()
    {
        // No operations when m_hmeKernel exists
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Send surfaces to the ME kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMeSurfaces(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        // No operations when m_hmeKernel exists
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup Curbe for MbEnc Kernels
    //!
    //! \param    [in]  mbEncIFrameDistEnabled
    //!           Indicate if MbEnc I-Frame distortion is enabled
    //! \param    [in]  mbQpDataEnabled
    //!           Indicate if MB QP data is enabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeMbEnc(
        bool mbEncIFrameDistEnabled,
        bool mbQpDataEnabled) = 0;

    //!
    //! \brief    Initialize BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Update the slice count according to the DymanicSliceShutdown policy
    //!
    virtual void UpdateSSDSliceCount();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpSeqParams(
        CodecEncodeMpeg2SequenceParams *seqParams);

    MOS_STATUS DumpPicParams(
        CodecEncodeMpeg2PictureParams *picParams);

    MOS_STATUS DumpSliceParams(
        CodecEncodeMpeg2SliceParmas *sliceParams);

    MOS_STATUS DumpVuiParams(
        CodecEncodeMpeg2VuiParams *vuiParams);
#endif
    //!
    //! \enum  MbEncKernelStateIdx
    //! \brief MbEnc kernel index
    //!
    enum MbEncKernelStateIdx
    {
        mbEncKernelIdxI = 0,
        mbEncKernelIdxP,
        mbEncKernelIdxB,
        mbEncKernelIdxNum,
    };

    //!
    //! \struct BindingTableMbEnc
    //! \brief  MbEnc binding table r structure
    //!
    struct BindingTableMbEnc
    {
        uint32_t m_mbEncPakObj;
        uint32_t m_mbEncPakObjPrev;
        uint32_t m_mbEncCurrentY;
        uint32_t m_mbEncBrcDistortionSurface;
        uint32_t m_mbEncCurrentPic;
        uint32_t m_mbEncForwardPic;
        uint32_t m_mbEncBackwardPic;
        uint32_t m_mbEncInterlaceFrameCurrentPic;
        uint32_t m_mbEncInterlaceFrameBackwardPic;
        uint32_t m_mbEncMbControl;
        uint32_t m_mbEncScoreboard;
    };

    static const uint32_t                  m_numSyncTags                  = 36;                 //!< Number of kernels: per frame & kernel workload
    static const uint32_t                  m_initDshSize                  = MHW_PAGE_SIZE * 2;  //!< Perfomance tuning might be needed depending on curbe size
    static const uint32_t                  m_frameRateDenom               = 100;                //!< Frame rate denom
    static const uint32_t                  m_frameThresholdArraySize      = 64;                 //!< Frame threadold array size
    static const uint32_t                  m_distQpAdjustmentArraySize    = 96;                 //!< QP adjustemnt array size
    static const uint32_t                  m_brcConstantSurfaceWidth      = 64;                 //!< BRC constant surface width
    static const uint32_t                  m_brcPicHeaderSurfaceSize      = 1024;               //!< BRC picture header surface size
    static const uint32_t                  m_brcHistoryBufferSize         = 576;                //!< BRC history buffer size
    static const uint32_t                  m_targetUsageNum               = 8;                  //!< Target usage number
    static const uint32_t                  m_maxVmvr                      = 128 * 4;            //!< Max VMVR
    static const uint32_t                  m_mvCostTableOffset            = 52;                 //!< MV cost table offset

    static const uint8_t                   m_qpAdjustmentDistThresholdMaxFrameThresholdI[m_frameThresholdArraySize];//!< QP adjustment threashold array for I frame
    static const uint8_t                   m_qpAdjustmentDistThresholdMaxFrameThresholdP[m_frameThresholdArraySize];//!< QP adjustment threashold array for P frame
    static const uint8_t                   m_qpAdjustmentDistThresholdMaxFrameThresholdB[m_frameThresholdArraySize];//!< QP adjustment threashold array for B frame
    static const uint8_t                   m_distQpAdjustmentI[m_distQpAdjustmentArraySize];                        //!< QP adjustment array for I frame
    static const uint8_t                   m_distQpAdjustmentP[m_distQpAdjustmentArraySize];                        //!< QP adjustment array for P frame
    static const uint8_t                   m_distQpAdjustmentB[m_distQpAdjustmentArraySize];                        //!< QP adjustment array for B frame
    static const uint8_t                   m_targetUsageToKernelMode[m_targetUsageNum];                             //!< Table for target usage to kernel mode convert

    static const uint32_t                  m_vmeLutXyP[2];                                      //!< vme LUT XY table for P frame
    static const uint32_t                  m_vmeLutXyB[2];                                      //!< vme LUT XY table for B frame
    static const uint32_t                  m_vmeSPathP0[16];                                    //!< vme search path table 0 for P frame
    static const uint32_t                  m_vmeSPathP1[16];                                    //!< vme search path table 1 for P frame
    static const uint32_t                  m_vmeSPathB0[16];                                    //!< vme search path table 0 for B frame
    static const uint32_t                  m_vmeSPathB1[16];                                    //!< vme search path table 1 for B frame

    CodecEncodeMpeg2SequenceParams         *m_seqParams = nullptr;                              //!< Pointer to sequence parameter
    CodecEncodeMpeg2VuiParams              *m_vuiParams = nullptr;                              //!< Pointer to vui parameter
    CodecEncodeMpeg2SliceParmas            *m_sliceParams = nullptr;                            //!< Pointer to slice parameter
    CodecEncodeMpeg2QmatixParams           *m_qMatrixParams = nullptr;                          //!< Pointer to qmatrix parameter
    CODEC_PIC_ID                           m_picIdx[CODEC_MAX_NUM_REF_FRAME_NON_AVC];           //!< Picture index

    uint8_t*                               m_kernelBinary = nullptr;                            //!< Pointer to the kernel binary
    uint32_t                               m_combinedKernelSize = 0;                            //!< Combined kernel binary size

    bool                                   m_sliceStateEnable = true;                           //!< Indicate if slice state is enabled

    // BRC
    bool                                   m_brcInit = true;                                    //!< Indicate if BRC is initilized
    bool                                   m_mbEncCurbeSetInBrcUpdate = false;                  //!< Indicatd if Mbenc curbe is set
    bool                                   m_brcEnabled = false;                                //!< Indicate if BRC is enabled
    bool                                   m_brcReset = false;                                  //!< Indicate if BRC is reset
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                                   m_brcDistortionBufferSupported = false;              //!< Indicate if BRC distorion buffer is supported
#endif
    uint16_t                               m_avbrAccuracy = 0;                                  //!< AVBR Accuracy
    uint16_t                               m_avbrConvergence = 0;                               //!< AVBR Convergence
    uint32_t                               m_picHeaderDataBufferSize = 0;                       //!< Picture header buffer size
    uint32_t                               m_qScaleTypeByteOffse = 0;                           //!< Offset for QScle
    uint32_t                               m_vbvDelayOffset = 0;                                //!< Offset for Vbv delay
    uint32_t                               m_intraDcPrecisionOffset = 0;                        //!< Offset for Intra DC precision
    MHW_KERNEL_STATE                       m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_NUM];      //!< BRC kernel state
    EncodeBrcBuffers                       m_brcBuffers;                                        //!< BRC buffers
    double                                 m_brcInitCurrentTargetBufFullInBits = 0;             //!< BRC init buffer full
    double                                 m_brcInitResetInputBitsPerFrame = 0;                 //!< BRC init reset input bits per frame
    double                                 m_brcInitResetBufSizeInBits = 0;                     //!< BRC init reset buffer size

    // MbEnc
    bool                                   m_mbQpDataEnabled = false;                           //!< Mb Qp data flag
    MOS_SURFACE                            m_mbQpDataSurface;                                   //!< MOS_SURFACE of Mb Qp data surface
    uint32_t                               m_frameNumB     = 0;                                 //!< The num of the successive B frames
    uint32_t                               m_prevMBCodeIdx = 0;                                 //!< Previous MB Code index                                                                                    // MbEnc
    uint8_t                                m_mbEncForcePictureCodingType = 0;                   //!< force I, P, or B for MbEnc kernel only
    MHW_KERNEL_STATE                       m_mbEncKernelStates[mbEncKernelIdxNum];              //!< MbEnc kernel state
    BindingTableMbEnc                      m_mbEncBindingTable;                                 //!< MbEnc binding table

    // ME
    CodechalKernelHme                      *m_hmeKernel = nullptr;                              //!< ME kernel object
    bool                                   m_hmeEnabled = false;                                //!< HME enable flag
    MOS_SURFACE                            m_4xMEMVDataBuffer;                                  //!< 4xME mv data buffer
    MHW_BATCH_BUFFER                       m_batchBufForMEDistBuffer[NUM_ENCODE_BB_TYPE];       //!< ME Distortion batch buffer for ME call
    uint32_t                               m_memvBottomFieldOffset = 0;                         //!< MEMV bottom filed offset
    MOS_SURFACE                            m_4xMEDistortionBuffer;                              //!< MOS_SURFACE of ME distortion surface
    uint32_t                               m_meDistortionBottomFieldOffset = 0;                 //!< ME distortion bottom filed offset

private:
    //!
    //! \brief    Walker function
    //!
    void MBWalker(uint16_t, uint16_t, uint16_t*);
    void MBWalker45Degree(uint16_t, uint16_t, uint16_t*);
    void MBWalkerMBAFF(uint16_t, uint16_t, uint16_t*);
    void MBWalkerRasterScan(uint16_t, uint16_t, uint16_t*);
    void MBWalkerVerticalScan(uint16_t, uint16_t, uint16_t*);
};

#endif  // __CODECHAL_ENCODE_MPEG2_H__
