/*
* Copyright (c) 2016-2017, Intel Corporation
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
//! \file     codechal_encode_hevc.h
//! \brief    Defines base class for HEVC dual-pipe encoder.
//!

#ifndef __CODECHAL_ENCODE_HEVC_H__
#define __CODECHAL_ENCODE_HEVC_H__

#include "codechal_encode_hevc_base.h"
#include "codechal_kernel_hme.h"

#define  HUC_CMD_LIST_MODE 1
#define  HUC_BATCH_BUFFER_END 0x05000000

//! QP type
enum {
    QP_TYPE_CONSTANT = 0,
    QP_TYPE_FRAME,
    QP_TYPE_CU_LEVEL
};

//!
//! \struct HucCommandData
//! \brief  The struct of Huc commands data
//!
struct HucCommandData
{
    uint32_t        TotalCommands;       //!< Total Commands in the Data buffer
    struct
    {
        uint16_t    ID;              //!< Command ID, defined and order must be same as that in DMEM
        uint16_t    SizeOfData;      //!< data size in uint32_t
        uint32_t    data[40];
    } InputCOM[10];
};

//! \class    CodechalEncHevcState
//! \brief    HEVC dual-pipe encoder base class
//! \details  This class defines the base class for HEVC dual-pipe encoder, it includes
//!        common member fields, functions, interfaces etc shared by all GENs.
//!        Gen specific definitions, features should be put into their corresponding classes.
//!        To create a HEVC dual-pipe encoder instance, client needs to call CodechalEncHevcState::CreateHevcState()
//!
class CodechalEncHevcState : public CodechalEncodeHevcBase
{
public:
    //! QP type
    enum {
        QP_TYPE_CONSTANT = 0,
        QP_TYPE_FRAME,
        QP_TYPE_CU_LEVEL
    };

    enum {
        BRCINIT_USEHUCBRC                  = 0x0001,
        BRCINIT_ISCBR                      = 0x0010,
        BRCINIT_ISVBR                      = 0x0020,
        BRCINIT_ISAVBR                     = 0x0040,
        BRCINIT_ISQVBR                     = 0x0080,
        BRCINIT_FIELD_PIC                  = 0x0100,
        BRCINIT_ISICQ                      = 0x0200,
        BRCINIT_ISVCM                      = 0x0400,
        BRCINIT_PANIC_MODE_ISENABLED       = 0x1000,
        BRCINIT_IGNORE_PICTURE_HEADER_SIZE = 0x2000,
        BRCINIT_ISCQP                      = 0x4000,
        BRCINIT_DISABLE_MBBRC              = 0x8000
    };

    //!
    //! \brief    HEVC BRC buffers
    //!
    struct HevcEncBrcBuffers
    {
        MOS_RESOURCE            resBrcHistoryBuffer;                                                    // BRC history buffer
        MOS_RESOURCE            resBrcPakStatisticBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
        uint32_t                uiCurrBrcPakStasIdxForRead;
        uint32_t                uiCurrBrcPakStasIdxForWrite;
        MOS_RESOURCE            resBrcImageStatesReadBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];   // read only BRC image state buffer
        MOS_RESOURCE            resBrcImageStatesWriteBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< Write only HEVC BRC image state buffers
        uint32_t                dwBrcHcpPicStateSize;
        MOS_SURFACE             sBrcConstantDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
        MOS_RESOURCE            resMbBrcConstDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
        uint32_t                dwBrcConstantSurfaceWidth;
        uint32_t                dwBrcConstantSurfaceHeight;
        MOS_SURFACE             sBrcIntraDistortionBuffer;                      //!< BRC Intra distortion buffer
        MOS_SURFACE             sMeBrcDistortionBuffer;
        uint32_t                dwMeBrcDistortionBottomFieldOffset;
        MOS_SURFACE             sBrcMbQpBuffer;
        uint32_t                dwBrcMbQpBottomFieldOffset;
        MOS_RESOURCE            resBrcPicHeaderInputBuffer;
        MOS_RESOURCE            resBrcPicHeaderOutputBuffer;
        MOS_RESOURCE            resMbEncAdvancedDsh;
        MOS_RESOURCE            resMbEncBrcBuffer;
        MOS_SURFACE             sBrcRoiSurface;                 // BRC ROI surface
        PMOS_SURFACE            pMbStatisticsSurface;
        PCODECHAL_ENCODE_BUFFER pMvAndDistortionSumSurface;
        PMHW_KERNEL_STATE       pMbEncKernelStateInUse;
        CmSurface2D*            brcIntraDistortionSurface = nullptr;
        CmSurface2D*            meBrcDistortionSurface    = nullptr;
        CmBuffer*               mvAndDistortionSumSurface = nullptr;
    };

    //!
    //! \struct EncStatsBuffers
    //! \brief  MbEnc statistic buffers
    //!
    struct EncStatsBuffers
    {
        MOS_SURFACE             m_puStatsSurface;
        MOS_SURFACE             m_8x8PuHaarDist;
        CODECHAL_ENCODE_BUFFER  m_8x8PuFrameStats;
        MOS_SURFACE             m_mbEncStatsSurface;
        CODECHAL_ENCODE_BUFFER  m_mbEncFrameStats;
    };

    static const uint32_t                       m_8x8PuFrameStatsSize = 32;                     //!< The size of 8x8 PU frame statistic buffer
    static const uint32_t                       m_mbEncFrameStatsSize = 32;
    static constexpr uint32_t                   NUM_FORMAT_CONV_FRAMES = (CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC + 1);  //!< Number of format conversion frames

    uint32_t m_widthAlignedLcu32  = 0;  //!< Picture width aligned to LCU32
    uint32_t m_heightAlignedLcu32 = 0;  //!< Picture height aligned to LCU32

    uint32_t m_numRegionsInSlice = 1;  //!< Number of Regions

    // Resources for the render engine
    MOS_SURFACE m_formatConvertedSurface[NUM_FORMAT_CONV_FRAMES];  //!< // Handle of the format converted surface

    // ME
    CodechalKernelHme                           *m_hmeKernel = nullptr;                         //!< ME kernel object
    PMHW_KERNEL_STATE                            m_meKernelState        = nullptr;                         //!< ME kernel state
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC       m_meKernelBindingTable = nullptr;                         //!< ME kernel binding table

    // BRC
    PMHW_KERNEL_STATE                           m_brcKernelStates       = nullptr;              //!< Pointer to BRC kernel state
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC      m_brcKernelBindingTable = nullptr;              //!< BRC kernel binding table
    PMOS_SURFACE                                m_brcDistortion         = nullptr;              //!< Pointer to BRC distortion surface
    HevcEncBrcBuffers                           m_brcBuffers;                                   //!< BRC buffers
    uint32_t                                    m_numBrcKrnStates = 0;                              //!< Number of BRC kernel states
    uint8_t                                     m_slidingWindowSize = 0;                        //!< Sliding window size in number of frames
    bool                                        m_roiRegionSmoothEnabled = false;               //!< ROI region smooth transition enable flag
    HEVC_BRC_FRAME_TYPE                         m_currFrameBrcLevel = HEVC_BRC_FRAME_TYPE_I;    //!< frame brc level

    // MBENC
    PMHW_KERNEL_STATE                           m_mbEncKernelStates       = nullptr;  //!< Pointer to MbEnc kernel state
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC      m_mbEncKernelBindingTable = nullptr;  //!< MbEnc kernel binding table
    uint32_t                                    m_numMbEncEncKrnStates    = 0;        //!< Number of MbEnc kernel states
    EncStatsBuffers                             m_encStatsBuffers;
    uint8_t                                     m_mbCodeIdxForTempMVP     = 0xFF;     //!< buf index for current frame temporal mvp 
    uint8_t                                     m_roundingIntraInUse = 10;             //!< rounding intra actually used
    uint8_t                                     m_roundingInterInUse = 4;             //!< rounding inter actually used

    // ScalingAndConversion
    PMHW_KERNEL_STATE                      m_scalingAndConversionKernelState        = nullptr;  //!< Pointer to ScalingAndConversion kernel state
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC m_scalingAndConversionKernelBindingTable = nullptr;  //!< ScalingAndConversion kernel binding table

    bool m_pakOnlyTest                                       = false;  //!< PAK only test enable flag
    char m_pakOnlyDataFolder[MOS_USER_CONTROL_MAX_DATA_SIZE] = {0};    //!< Pak only test data folder name
    bool m_cqpEnabled                                        = false;  //!< CQP Rate Control
    bool m_sseSupported                                      = false;  //!< PAK SSE support flag

    // Below values will be set if qp control params are sent by app
    bool    m_minMaxQpControlEnabled = false;  //!< Flag to indicate if min/max QP feature is enabled or not.
    int16_t m_minQpForI              = 0;      //!< I frame Minimum QP.
    int16_t m_maxQpForI              = 0;      //!< I frame Maximum QP.
    int16_t m_minQpForP              = 0;      //!< P frame Minimum QP.
    int16_t m_maxQpForP              = 0;      //!< P frame Maximum QP.
    int16_t m_minQpForB              = 0;      //!< B frame Minimum QP.
    int16_t m_maxQpForB              = 0;      //!< B frame Maximum QP.
    bool    m_minMaxQpControlForP    = false;  //!< Indicates min/max QP values for P-frames are set separately or not.
    bool    m_minMaxQpControlForB    = false;  //!< Indicates min/max QP values for B-frames are set separately or not.

protected:
    //!
    //! \brief    Constructor
    //!
    CodechalEncHevcState(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

public:
    //!
    //! \brief    Copy constructor
    //!
    CodechalEncHevcState(const CodechalEncHevcState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncHevcState& operator=(const CodechalEncHevcState&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncHevcState();

    //!
    //! \brief    Help function to initialize surface parameters for 1D buffer
    //!
    //! \param    [in, out] params
    //!           Pointer to surface codec parameters
    //! \param    [in] buffer
    //!           Pointer to buffer resource
    //! \param    [in] size
    //!           Buffer size
    //! \param    [in] offset
    //!           Offset within the buffer
    //! \param    [in] cacheabilityControl
    //!           Buffer cache control setting
    //! \param    [in] bindingTableOffset
    //!           Binding table offset for the buffer
    //! \param    [in] isWritable
    //!           True if buffer is writable, false if it is read only
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitSurfaceCodecParams1D(
        CODECHAL_SURFACE_CODEC_PARAMS* params,
        PMOS_RESOURCE buffer,
        uint32_t size,
        uint32_t offset,
        uint32_t cacheabilityControl,
        uint32_t bindingTableOffset,
        bool isWritable);

    //!
    //! \brief    Help function to initialize surface parameters for 2D surface
    //!
    //! \param    [in, out] params
    //!           Pointer to surface codec parameters
    //! \param    [in] surface
    //!           Pointer to surface resource
    //! \param    [in] cacheabilityControl
    //!           Cache control setting for the surface
    //! \param    [in] bindingTableOffset
    //!           Binding table offset for the surface
    //! \param    [in] verticalLineStride
    //!           Vertical line stride for the surface
    //! \param    [in] isWritable
    //!           True if surface is writable, false if it is read only
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitSurfaceCodecParams2D(
        CODECHAL_SURFACE_CODEC_PARAMS* params,
        PMOS_SURFACE surface,
        uint32_t cacheabilityControl,
        uint32_t bindingTableOffset,
        uint32_t verticalLineStride,
        bool isWritable);

    //!
    //! \brief    Help function to initialize surface parameters for VME surface
    //!
    //! \param    [in, out] params
    //!           Pointer to surface codec parameters
    //! \param    [in] surface
    //!           Pointer to surface resource
    //! \param    [in] cacheabilityControl
    //!           Cache control setting for the surface
    //! \param    [in] bindingTableOffset
    //!           Binding table offset for the surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitSurfaceCodecParamsVME(
        CODECHAL_SURFACE_CODEC_PARAMS* params,
        PMOS_SURFACE surface,
        uint32_t cacheabilityControl,
        uint32_t bindingTableOffset);

    //!
    //! \brief    Help function to calcuate ROI Ratio need by BRC Kernel
    //!
    //! \return   ROI ratio
    //!
    uint8_t CalculateROIRatio();

    //!
    //! \brief    Help function to calcuate the temporal difference between current and reference picture
    //!
    //! \param    [in] refPic
    //!           Reference picture.
    //!
    //! \return   Temporal difference between current and reference picture
    //!
    int16_t ComputeTemporalDifference(const CODEC_PICTURE& refPic);

    //!
    //! \brief    Help function to get start code offset
    //! \details  Search the start code from address addr to (addr + size)
    //!
    //! \param    [in] addr
    //!           Pointer to memory location to start searching for start code
    //! \param    [in] size
    //!           End of the memory address to search for start code is [addr + size]
    //!
    //! \return   Offset of start code
    //!
    uint32_t GetStartCodeOffset(uint8_t* addr, uint32_t size);

    //!
    //! \brief    Help function to get picture header size
    //!
    //! \return   Size of picture header
    //!
    uint32_t GetPicHdrSize();

    //!
    //! \brief    Wait for PAK engine ready
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WaitForPak();

    //!
    //! \brief    Wait for reference frame ready
    //!
    //! \param    [in] mbCodeIdx
    //!           Mb code index for reference frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WaitForRefFrameReady(uint8_t mbCodeIdx);

    //!
    //! \brief    Add HCP_WEIGHT_OFFSET_STATE command to command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in] hevcSlcParams
    //!           Pointer to HEVC slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,      
        PMHW_BATCH_BUFFER               batchBuffer,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams);

    //!
    //! \brief    Put slice level commands in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] params
    //!           Pointer to slice state parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendHwSliceEncodeCommand(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE     params);

    //!
    //! \brief    Allocate encoder states resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateEncStatsResources();

    //!
    //! \brief    Free encoder states resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeEncStatsResources();

    //!
    //! \brief    Get Current Frame BRC Level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetFrameBrcLevel();

    //! Inherited virtual functions
    virtual bool CheckSupportedFormat(PMOS_SURFACE surface);
    virtual MOS_STATUS Initialize(CodechalSetting * settings);
    virtual MOS_STATUS AllocateBrcResources();
    virtual MOS_STATUS FreeBrcResources();
    virtual MOS_STATUS InitializePicture(const EncoderParams& params);
    virtual MOS_STATUS SetSequenceStructs();
    virtual MOS_STATUS SetPictureStructs();
    virtual MOS_STATUS SetSliceStructs();
    virtual MOS_STATUS ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer);
    virtual MOS_STATUS UserFeatureKeyReport();
    virtual MOS_STATUS ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);
    virtual MOS_STATUS ExecutePictureLevel();
    virtual MOS_STATUS ExecuteSliceLevel();

    //!
    //! \brief    Read stats for BRC from PAK
    //!
    //! \param    [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadBrcPakStats(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup ME curbe params
    //!
    //! \param    [in, out] curbeParams
    //!           ME curbe params to be initialized in this function
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMeCurbeParams(
        CodechalKernelHme::CurbeParam &curbeParams);

    //!
    //! \brief    Setup ME surface params
    //!
    //! \param    [in] surfaceParams
    //!           ME curbe params
    //! \param    [in, out] surfaceParams
    //!           ME surface params to be initialized in this function
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMeSurfaceParams(
        CodechalKernelHme::SurfaceParams     &surfaceParams);

    //!
    //! \brief    Top level function for ME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeMeKernel();

    //!
    //! \brief    Allocate ENC resources when LCU size is 64
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateEncResourcesLCU64()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get max supported number of reference frames
    //!
    //! \param    [out] maxNumRef0
    //!           Max suppoted number of referenace frame 0
    //! \param    [out] maxNumRef1
    //!           Max suppoted number of referenace frame 1
    //!
    //! \return   void
    //!
    virtual void GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1) = 0;

    //!
    //! \brief    Prepare and add Hcp Pipe Mode Select Cmd
    //!
    //! \param    [out] cmdBuffer
    //!           CmdBuffer to add the cmd
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER* cmdBuffer);

    //!
    //! \brief    Prepare and add all Hcp Surface State Cmds
    //!
    //! \param    [out] cmdBuffer
    //!           CmdBuffer to add the cmd
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpSurfaceStateCmds(MOS_COMMAND_BUFFER* cmdBuffer);

    //!
    //! \brief    Prepare and add Hcp Picture State Cmd
    //!
    //! \param    [out] cmdBuffer
    //!           CmdBuffer to add the cmd
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPictureStateCmd(MOS_COMMAND_BUFFER* cmdBuffer);

    //!
    //! \brief    Create ROI surfaces for BRC LCU Update kernel
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupROISurface();
    //!
    //! \brief    Generate codechal dumps for HME kernel
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpHMESurfaces();
    //!
    //! \brief    Get rounding inter/intra for current frame to use
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetRoundingIntraInterToUse();
};
#endif  // __CODECHAL_ENCODE_HEVC_H__
