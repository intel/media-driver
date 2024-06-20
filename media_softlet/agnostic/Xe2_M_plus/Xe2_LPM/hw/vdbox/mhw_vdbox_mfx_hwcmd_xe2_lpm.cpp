
/*===================== begin_copyright_notice ==================================

# Copyright (c) 2024, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_vdbox_mfx_hwcmd_xe2_lpm.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_mfx_hwcmd_xe2_lpm.h"
#include "mos_utilities.h"
#include <string.h>

using namespace mhw::vdbox::mfx::xe2_lpm_base::xe2_lpm;

Cmd::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value = 0x00000000;
    //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.Tilemode                                     = TILEMODE_LINEAR;
}

Cmd::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;
}

Cmd::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;
}

Cmd::MFX_QM_STATE_CMD::MFX_QM_STATE_CMD()
{
    DW0.Value = 0x70070010;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED7;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.Obj0.Mpeg2                                   = MPEG2_MPEGINTRAQUANTIZERMATRIX;
    //DW1.Obj1.Jpeg                                    = JPEG_JPEGLUMAYQUANTIZERMATRIX_ORR;
    //DW1.Obj2.Avc                                     = AVC_AVC4X4INTRAMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS;

    memset(&ForwardQuantizerMatrix, 0, sizeof(ForwardQuantizerMatrix));
}

Cmd::MFX_FQM_STATE_CMD::MFX_FQM_STATE_CMD()
{
    DW0.Value = 0x70080020;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.Obj0.Mpeg2                                   = MPEG2_MPEGINTRAQUANTIZERMATRIX;
    //DW1.Obj1.Jpeg                                    = JPEG_JPEGLUMAYQUANTIZERMATRIX_ORR;
    //DW1.Obj2.Avc                                     = AVC_AVC4X4INTRAMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS;

    memset(&ForwardQuantizerMatrix, 0, sizeof(ForwardQuantizerMatrix));
}

Cmd::MFX_PIPE_MODE_SELECT_CMD::MFX_PIPE_MODE_SELECT_CMD()
{
    DW0.Value = 0x70000003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopb                                       = SUBOPB_MFXPIPEMODESELECT;
    //DW0.Subopa                                       = SUBOPA_UNNAMED0;
    //DW0.Opcode                                       = OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.StandardSelect                               = STANDARD_SELECT_MPEG2;
    //DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    //DW1.StitchMode                                   = STITCH_MODE_NOTINSTITCHMODE;
    //DW1.FrameStatisticsStreamoutEnable               = FRAME_STATISTICS_STREAMOUT_ENABLE_DISABLE;
    //DW1.ScaledSurfaceEnable                          = SCALED_SURFACE_ENABLE_DISABLE;
    //DW1.PreDeblockingOutputEnablePredeblockoutenable = PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE_DISABLE;
    //DW1.PostDeblockingOutputEnablePostdeblockoutenable = POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE_DISABLE;
    //DW1.StreamOutEnable                              = STREAM_OUT_ENABLE_DISABLE;
    //DW1.PicErrorStatusReportEnable                   = PIC_ERRORSTATUS_REPORT_ENABLE_DISABLE;
    //DW1.DeblockerStreamOutEnable                     = DEBLOCKER_STREAM_OUT_ENABLE_DISABLE;
    //DW1.VdencMode                                    = VDENC_MODE_MBENCMODE;
    //DW1.StandaloneVdencModeEnable                    = STANDALONE_VDENC_MODE_ENABLE_VDENCPAK;
    //DW1.DecoderModeSelect                            = DECODER_MODE_SELECT_VLDMODE;
    //DW1.DecoderShortFormatMode                       = DECODER_SHORT_FORMAT_MODE_SHORTFORMATDRIVERINTERFACE;

    DW2.Value = 0x00000000;
    //DW2.PerformanceCounterEnableVmxVmcVadVdsBsp      = PERFORMANCE_COUNTER_ENABLE_VMX_VMC_VAD_VDS_BSP_DISABLE;
    //DW2.VmxRowStoreCounterOutputSelect               = VMX_ROW_STORE_COUNTER_OUTPUT_SELECT_UNNAMED0;
    //DW2.VdsIldbCalculation                           = VDS_ILDB_CALCULATION_DISABLE;
    //DW2.ClockGateEnableAtSliceLevel                  = CLOCK_GATE_ENABLE_AT_SLICE_LEVEL_DISABLE;
    //DW2.VmxDummyFetchControl                         = VMX_DUMMY_FETCH_CONTROL_ENABLE;
    //DW2.MpcPref08X8DisableFlagDefault0               = MPC_PREF08X8_DISABLE_FLAG_DEFAULT_0_DISABLE;
    //DW2.VinClockGatingCmclkDisable                   = VIN_CLOCK_GATING_CMCLK_DISABLE_DISABLE;
    //DW2.VlfMbaffOsStorageForSvcDisable               = VLF_MBAFF_OS_STORAGE_FOR_SVC_DISABLE_ENABLE;
    //DW2.Vlf720IOddHeightInVc1Mode                    = VLF_720I_ODD_HEIGHT_IN_VC1_MODE_DISABLE;
    //DW2.VinClockGatingCuclkDisable                   = VIN_CLOCK_GATING_CUCLK_DISABLE_DISABLE;
    //DW2.VinSliceStartGenerationForItMode             = VIN_SLICE_START_GENERATION_FOR_IT_MODE_FRAME;
    //DW2.VhrReferenceListGenerationAdjustmentUsingCurrentPocForBFrame = VHR_REFERENCE_LIST_GENERATION_ADJUSTMENT_USING_CURRENT_POC_FOR_B_FRAME_UNNAMED0;
    //DW2.VamErrorHandlingForReferenceIndexInSvcAvcDisable = VAM_ERROR_HANDLING_FOR_REFERENCE_INDEX_IN_SVCAVC_DISABLE_ENABLE;
    //DW2.VadSvcDecodeErrorHandlingDisable             = VAD_SVC_DECODE_ERROR_HANDLING_DISABLE_ENABLE;
    //DW2.VmbSvcTlbDummyFetchDisableForPerformance     = VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE_ENABLE;
    //DW2.VmbSvcMvReplicationFor8X8EnableErrorHandling = VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING_DISABLE;
    //DW2.VdsSvcSvcBlktypeEquationAdjustment           = VDS_SVC_SVC_BLKTYPE_EQUATION_ADJUSTMENT_ENABLE;

    DW3.Value = 0x00000000;
    //DW3.PicStatusErrorReportId                       = PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED;

    DW4.Value = 0x00000000;
}

Cmd::MFX_SURFACE_STATE_CMD::MFX_SURFACE_STATE_CMD()
{
    DW0.Value = 0x70010004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopb                                       = SUBOPB_UNNAMED1;
    //DW0.Subopa                                       = SUBOPA_UNNAMED0;
    //DW0.Opcode                                       = OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.SurfaceId                                    = SURFACE_ID_DECODEDPICTUREANDREFERENCEPICTURES_SVCUPSAMPLINGSTREAMOUTRECONSTRUCTEDPIXELSCOEFFPRED_UPPERLAYERSIZE;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.Tilemode                                     = TILEMODE_LINEAR;
    //DW3.CompressionFormat                            = COMPRESSION_FORMAT_CMFR8;
    //DW3.InterleaveChroma                             = INTERLEAVE_CHROMA_DISABLE;
    //DW3.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;
}

Cmd::MFX_IND_OBJ_BASE_ADDR_STATE_CMD::MFX_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value = 0x70030018;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_MFXINDOBJBASEADDRSTATE;
    //DW0.SubOpcodea                                   = SUB_OPCODEA_MFXINDOBJBASEADDRSTATE;
    //DW0.CommonOpcode                                 = COMMON_OPCODE_MFXINDOBJBASEADDRSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXINDOBJBASEADDRSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
}

Cmd::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::MFX_BSP_BUF_BASE_ADDR_STATE_CMD()
{
    DW0.Value = 0x70040008;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED4;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_PIPELINE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.BsdMpcRowStoreScratchBufferArbitrationPriorityControl = BSDMPC_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW3.BsdMpcRowStoreScratchBufferCacheSelect       = BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW3.BsdMpcRowStoreScratchBufferTiledResourceMode = BSDMPC_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODENONE;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;
    //DW6.MprRowStoreScratchBufferArbitrationPriorityControl = MPR_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW6.MprRowStoreScratchBufferCacheSelect          = MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW6.MprRowStoreScratchBufferTiledResourceMode    = MPR_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODENONE;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;
    //DW9.BitplaneReadBufferArbitrationPriorityControl = BITPLANE_READ_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW9.BitplaneReadBufferTiledResourceMode          = BITPLANE_READ_BUFFER_TILED_RESOURCE_MODE_TRMODENONE;
}

Cmd::MFD_AVC_PICID_STATE_CMD::MFD_AVC_PICID_STATE_CMD()
{
    DW0.Value = 0x71250008;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_MEDIA;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_DEC;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFDAVCDPBSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.PictureidRemappingDisable                    = PICTUREID_REMAPPING_DISABLE_AVCDECODERWILLUSE16BITSPICTUREIDTOHANDLEDMVANDIDENTIFYTHEREFERENCEPICTURE;

    memset(&Pictureidlist1616Bits, 0, sizeof(Pictureidlist1616Bits));
}

Cmd::MFX_AVC_IMG_STATE_CMD::MFX_AVC_IMG_STATE_CMD()
{
    DW0.Value = 0x71000013;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED0;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCCOMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXAVCIMGSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.ImgstructImageStructureImgStructure10        = IMGSTRUCT_IMAGE_STRUCTURE_IMG_STRUCTURE10_FRAMEPICTURE;
    //DW3.WeightedBipredIdc                            = WEIGHTED_BIPRED_IDC_DEFAULT;
    //DW3.WeightedPredFlag                             = WEIGHTED_PRED_FLAG_DISABLE;
    //DW3.RhodomainRateControlEnable                   = RHODOMAIN_RATE_CONTROL_ENABLE_DISABLE;

    DW4.Value = 0x00000000;
    //DW4.Fieldpicflag                                 = FIELDPICFLAG_FRAME;
    //DW4.Mbaffflameflag                               = MBAFFFLAMEFLAG_FALSE;
    //DW4.Framembonlyflag                              = FRAMEMBONLYFLAG_FALSE;
    //DW4.Transform8X8Flag                             = TRANSFORM8X8FLAG_4X4;
    //DW4.Direct8X8Infflag                             = DIRECT8X8INFFLAG_SUBBLOCK;
    //DW4.Constrainedipredflag                         = CONSTRAINEDIPREDFLAG_INTRAANDINTER;
    //DW4.Imgdisposableflag                            = IMGDISPOSABLEFLAG_REFERENCE;
    //DW4.Entropycodingflag                            = ENTROPYCODINGFLAG_CAVLCBIT_SERIALENCODINGMODE;
    //DW4.Mbmvformatflag                               = MBMVFORMATFLAG_IGNORE;
    //DW4.Chromaformatidc                              = CHROMAFORMATIDC_MONOCHROMEPICTURE;
    //DW4.Mvunpackedflag                               = MVUNPACKEDFLAG_PACKED;
    //DW4.Inserttestflag                               = INSERTTESTFLAG_UNNAMED0;
    //DW4.Loadslicepointerflag                         = LOADSLICEPOINTERFLAG_DISABLE;
    //DW4.Mbstatenabled                                = MBSTATENABLED_DISABLE;
    //DW4.Minframewsize                                = MINFRAMEWSIZE_UNNAMED0;

    DW5.Value = 0x30000000;
    //DW5.IntrambmaxbitflagIntrambmaxsizereportmask    = INTRAMBMAXBITFLAG_INTRAMBMAXSIZEREPORTMASK_DISABLE;
    //DW5.IntermbmaxbitflagIntermbmaxsizereportmask    = INTERMBMAXBITFLAG_INTERMBMAXSIZEREPORTMASK_DISABLE;
    //DW5.FrameszoverflagFramebitratemaxreportmask     = FRAMESZOVERFLAG_FRAMEBITRATEMAXREPORTMASK_DISABLE;
    //DW5.FrameszunderflagFramebitrateminreportmask    = FRAMESZUNDERFLAG_FRAMEBITRATEMINREPORTMASK_DISABLE;
    //DW5.IntraIntermbipcmflagForceipcmcontrolmask     = INTRAINTERMBIPCMFLAG_FORCEIPCMCONTROLMASK_DISABLE;
    //DW5.MbratectrlflagMbLevelRateControlEnablingFlag = MBRATECTRLFLAG_MB_LEVEL_RATE_CONTROL_ENABLING_FLAG_DISABLE;
    //DW5.Minframewsizeunits                           = MINFRAMEWSIZEUNITS_COMPATIBILITYMODE;
    //DW5.Nonfirstpassflag                             = NONFIRSTPASSFLAG_DISABLE;
    //DW5.TrellisQuantizationChromaDisableTqchromadisable = TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE_UNNAMED0;
    //DW5.TrellisQuantizationRoundingTqr               = TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED3;
    //DW5.TrellisQuantizationEnabledTqenb              = TRELLIS_QUANTIZATION_ENABLED_TQENB_DISABLE;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;
    //DW7.VslTopMbTrans8X8Flag                         = VSL_TOP_MB_TRANS8X8FLAG_DISABLE;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;
    //DW10.Framebitrateminunitmode                     = FRAMEBITRATEMINUNITMODE_COMPATIBILITYMODE;
    //DW10.Framebitrateminunit                         = FRAMEBITRATEMINUNIT_BYTE;
    //DW10.Framebitratemaxunitmode                     = FRAMEBITRATEMAXUNITMODE_COMPATIBILITYMODE;
    //DW10.Framebitratemaxunit                         = FRAMEBITRATEMAXUNIT_BYTE;

    DW11.Value = 0x00000000;
    //DW11.Framebitratemaxdelta                        = FRAMEBITRATEMAXDELTA_UNNAMED0;

    DW12.Value = 0x00000000;
    //DW12.VadErrorLogic                               = VAD_ERROR_LOGIC_ENABLE;
    //DW12.VmdErrorLogic                               = VMD_ERROR_LOGIC_DISABLE;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

    DW15.Value = 0x00000000;

    DW16.Value = 0x00000000;
    //DW16.InterViewOrderDisable                       = INTER_VIEW_ORDER_DISABLE_DEFAULT;

    DW17.Value = 0x00000000;

    DW18.Value = 0x00000000;

    DW19.Value = 0x00000000;

    DW20.Value = 0x00000000;
}

Cmd::MFX_AVC_REF_IDX_STATE_CMD::MFX_AVC_REF_IDX_STATE_CMD()
{
    DW0.Value = 0x71040008;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_MFXAVCREFIDXSTATE;
    //DW0.Subopcodea                                   = SUBOPCODEA_MFXAVCREFIDXSTATE;
    //DW0.CommandOpcode                                = COMMAND_OPCODE_AVC;
    //DW0.Pipeline                                     = PIPELINE_MFXAVCREFIDXSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.RefpiclistSelect                             = REFPICLIST_SELECT_REFPICLIST0;

    memset(&ReferenceListEntry, 0, sizeof(ReferenceListEntry));
}

Cmd::MFX_AVC_WEIGHTOFFSET_STATE_CMD::MFX_AVC_WEIGHTOFFSET_STATE_CMD()
{
    DW0.Value = 0x71050060;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED5;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCCOMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXAVCWEIGHTOFFSETSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.WeightAndOffsetSelect                        = WEIGHT_AND_OFFSET_SELECT_WEIGHTANDOFFSETL0TABLE;

    memset(&Weightoffset, 0, sizeof(Weightoffset));
}

Cmd::MFX_AVC_SLICE_STATE_CMD::MFX_AVC_SLICE_STATE_CMD()
{
    DW0.Value = 0x71030009;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.CommandSubopcodeb                            = COMMAND_SUBOPCODEB_MFXAVCSLICESTATE;
    //DW0.Subopcodea                                   = SUBOPCODEA_MFXAVCSLICESTATE;
    //DW0.CommandOpcode                                = COMMAND_OPCODE_AVC;
    //DW0.Pipeline                                     = PIPELINE_MFXAVCSLICESTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.SliceType                                    = SLICE_TYPE_PSLICE;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.DisableDeblockingFilterIndicator             = DISABLE_DEBLOCKING_FILTER_INDICATOR_UNNAMED0;
    //DW3.DirectPredictionType                         = DIRECT_PREDICTION_TYPE_TEMPORAL;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;
    //DW6.Cabaczerowordinsertionenable                 = CABACZEROWORDINSERTIONENABLE_UNNAMED0;
    //DW6.Emulationbytesliceinsertenable               = EMULATIONBYTESLICEINSERTENABLE_UNNAMED0;
    //DW6.TailInsertionPresentInBitstream              = TAIL_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0;
    //DW6.SlicedataInsertionPresentInBitstream         = SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0;
    //DW6.HeaderInsertionPresentInBitstream            = HEADER_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0;
    //DW6.CompressedBitstreamOutputDisableFlag         = COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENABLETHEWRITINGOFTHEOUTPUTCOMPRESSEDBITSTREAM;
    //DW6.IsLastSlice                                  = IS_LAST_SLICE_UNNAMED0;
    //DW6.MbTypeSkipConversionDisable                  = MB_TYPE_SKIP_CONVERSION_DISABLE_ENABLESKIPTYPECONVERSION;
    //DW6.MbTypeDirectConversionDisable                = MB_TYPE_DIRECT_CONVERSION_DISABLE_ENABLEDIRECTMODECONVERSION;
    //DW6.RcPanicType                                  = RC_PANIC_TYPE_QPPANIC;
    //DW6.RcPanicEnable                                = RC_PANIC_ENABLE_DISABLE;
    //DW6.RcTriggleMode                                = RC_TRIGGLE_MODE_ALWAYSRATECONTROL;
    //DW6.Resetratecontrolcounter                      = RESETRATECONTROLCOUNTER_NOTRESET;
    //DW6.RateControlCounterEnable                     = RATE_CONTROL_COUNTER_ENABLE_DISABLE;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;
    //DW9.Roundintra                                   = ROUNDINTRA_116;
    //DW9.Roundinter                                   = ROUNDINTER_116;

    DW10.Value = 0x00000000;
}

Cmd::MFD_AVC_DPB_STATE_CMD::MFD_AVC_DPB_STATE_CMD()
{
    DW0.Value = 0x71260019;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED6;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCDEC;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.NonExistingframeFlag161Bit                   = NON_EXISTINGFRAME_FLAG161_BIT_VALID;
    //DW1.LongtermframeFlag161Bit                      = LONGTERMFRAME_FLAG161_BIT_THEPICTUREISASHORTTERMREFERENCEPICTURE;

    DW2.Value = 0x00000000;
    //DW2.UsedforreferenceFlag162Bits                  = USEDFORREFERENCE_FLAG162_BITS_NOTREFERENCE;

    memset(&Ltstframenumlist1616Bits, 0, sizeof(Ltstframenumlist1616Bits));
    //DW3.Ltstframenumlist1616Bits                     = LTSTFRAMENUMLIST1616_BITS_SHORTTERMFRAMEFLAGI;

    memset(&Viewidlist1616Bits, 0, sizeof(Viewidlist1616Bits));

    memset(&Vieworderlistl0168Bits, 0, sizeof(Vieworderlistl0168Bits));

    memset(&Vieworderlistl1168Bits, 0, sizeof(Vieworderlistl1168Bits));
}

Cmd::MFD_AVC_SLICEADDR_CMD::MFD_AVC_SLICEADDR_CMD()
{
    DW0.Value = 0x71270002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED7;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCDEC;
    //DW0.Pipeline                                     = PIPELINE_MFDAVCSLICEADDR;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.IndirectBsdDataStartAddress                  = INDIRECT_BSD_DATA_START_ADDRESS_UNNAMED0;

    DW3.Value = 0x00000000;
    //DW3.AvcNalTypeFirstByteOverrideBit               = AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT_USEBITSTREAMDECODEDNALTYPE;
}

Cmd::MFD_AVC_BSD_OBJECT_CMD::MFD_AVC_BSD_OBJECT_CMD()
{
    DW0.Value = 0x71280005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCDEC;
    //DW0.Pipeline                                     = PIPELINE_MFDAVCBSDOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.IndirectBsdDataStartAddress                  = INDIRECT_BSD_DATA_START_ADDRESS_UNNAMED0;

    DW3.Value = 0x00000000;
    //DW3.MbErrorConcealmentPSliceWeightPredictionDisableFlag = MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentPSliceMotionVectorsOverrideDisableFlag = MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentBSpatialWeightPredictionDisableFlag = MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentBSpatialMotionVectorsOverrideDisableFlag = MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentBSpatialPredictionMode     = MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_UNNAMED0;
    //DW3.MbHeaderErrorHandling                        = MB_HEADER_ERROR_HANDLING_UNNAMED0;
    //DW3.EntropyErrorHandling                         = ENTROPY_ERROR_HANDLING_UNNAMED0;
    //DW3.MprErrorMvOutOfRangeHandling                 = MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_UNNAMED0;
    //DW3.BsdPrematureCompleteErrorHandling            = BSD_PREMATURE_COMPLETE_ERROR_HANDLING_UNNAMED0;
    //DW3.MbErrorConcealmentBTemporalWeightPredictionDisableFlag = MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentBTemporalMotionVectorsOverrideEnableFlag = MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG_UNNAMED0;
    //DW3.MbErrorConcealmentBTemporalPredictionMode    = MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE_UNNAMED0;
    //DW3.IntraPredmode4X48X8LumaErrorControlBit       = INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT_UNNAMED0;
    //DW3.ConcealmentMethod                            = CONCEALMENT_METHOD_UNNAMED0;

    DW4.Value = 0x00000000;
    //DW4.LastsliceFlag                                = LASTSLICE_FLAG_UNNAMED0;
    //DW4.EmulationPreventionBytePresent               = EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED0;

    DW5.Value = 0x00000000;
    //DW5.IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma = INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA_UNNAMED0;
    //DW5.Intra8X84X4PredictionErrorConcealmentControlBit = INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT_UNNAMED0;
    //DW5.BSliceTemporalInterConcealmentMode           = B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED0;
    //DW5.BSliceSpatialInterConcealmentMode            = B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE_UNNAMED0;
    //DW5.BSliceInterDirectTypeConcealmentMode         = B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE_UNNAMED0;
    //DW5.BSliceConcealmentMode                        = B_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT;
    //DW5.PSliceInterConcealmentMode                   = P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED0;
    //DW5.PSliceConcealmentMode                        = P_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT;
    //DW5.ISliceConcealmentMode                        = I_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT;

    DW6.Value = 0x00000000;
}

Cmd::MFX_PAK_INSERT_OBJECT_CMD::MFX_PAK_INSERT_OBJECT_CMD()
{
    DW0.Value = 0x70480000;
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED2;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFXCOMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXPAKINSERTOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.BitstreamstartresetResetbitstreamstartingpos = BITSTREAMSTARTRESET_RESETBITSTREAMSTARTINGPOS_INSERT;
    //DW1.EmulationflagEmulationbytebitsinsertenable   = EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE_NONE;
    //DW1.SliceHeaderIndicator                         = SLICE_HEADER_INDICATOR_LEGACY;
    //DW1.Headerlengthexcludefrmsize                   = HEADERLENGTHEXCLUDEFRMSIZE_ACCUMULATE;
}

Cmd::MFX_MPEG2_PIC_STATE_CMD::MFX_MPEG2_PIC_STATE_CMD()
{
    DW0.Value = 0x7300000b;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED0;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MPEG2COMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXMPEG2PICSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.ScanOrder                                    = SCAN_ORDER_UNNAMED0;
    //DW1.QuantizerScaleType                           = QUANTIZER_SCALE_TYPE_UNNAMED0;

    DW2.Value = 0x00000000;
    //DW2.Mismatchcontroldisabled                      = MISMATCHCONTROLDISABLED_UNNAMED0;
    //DW2.PictureCodingType                            = 0;
    //DW2.LoadslicepointerflagLoadbitstreampointerperslice = LOADSLICEPOINTERFLAG_LOADBITSTREAMPOINTERPERSLICE_UNNAMED0;
    //DW2.PBSlicePredictedMotionVectorOverrideFinalMvValueOverride = PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE_PREDICTED;
    //DW2.PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride = PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE_BI_DIRECTION_MV_TYPE_OVERRIDE_BID;
    //DW2.PBSliceConcealmentMode                       = PB_SLICE_CONCEALMENT_MODE_INTER;
    //DW2.ISliceConcealmentMode                        = I_SLICE_CONCEALMENT_MODE_INTRACONCEALMENT;

    DW3.Value = 0x00000000;
    //DW3.SliceConcealmentDisableBit                   = SLICE_CONCEALMENT_DISABLE_BIT_ENABLE;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;
    //DW5.Intrambmaxsizereportmask                     = INTRAMBMAXSIZEREPORTMASK_UNNAMED0;
    //DW5.Intermbmaxsizereportmask                     = INTERMBMAXSIZEREPORTMASK_UNNAMED0;
    //DW5.Framebitratemaxreportmask                    = FRAMEBITRATEMAXREPORTMASK_DISABLE;
    //DW5.Framebitrateminreportmask                    = FRAMEBITRATEMINREPORTMASK_DISABLE;
    //DW5.Mbratecontrolmask                            = MBRATECONTROLMASK_UNNAMED0;
    //DW5.Minframewsizeunits                           = MINFRAMEWSIZEUNITS_COMPATIBILITYMODE;
    //DW5.Intermbforcecbpzerocontrolmask               = INTERMBFORCECBPZEROCONTROLMASK_UNNAMED0;
    //DW5.Framesizecontrolmask                         = FRAMESIZECONTROLMASK_UNNAMED0;

    DW6.Value = 0x0fff0fff;
    //DW6.Intrambmaxsize                               = INTRAMBMAXSIZE_UNNAMED4095;
    //DW6.Intermbmaxsize                               = INTERMBMAXSIZE_UNNAMED4095;

    DW7.Value = 0x00000000;
    //DW7.VslTopMbTrans8X8Flag                         = VSL_TOP_MB_TRANS8X8FLAG_DISABLE;

    DW8.Value = 0x00000000;
    //DW8.Slicedeltaqpmax3                             = SLICEDELTAQPMAX3_DISABLE;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;
    //DW10.Framebitrateminunitmode                     = FRAMEBITRATEMINUNITMODE_COMPATIBILITYMODE;
    //DW10.Framebitrateminunit                         = FRAMEBITRATEMINUNIT_BYTE;
    //DW10.Framebitratemaxunitmode                     = FRAMEBITRATEMAXUNITMODE_COMPATIBILITYMODE;
    //DW10.Framebitratemaxunit                         = FRAMEBITRATEMAXUNIT_BYTE;

    DW11.Value = 0x00000000;
    //DW11.Framebitratemaxdelta                        = FRAMEBITRATEMAXDELTA_UNNAMED0;

    DW12.Value = 0x00000000;
    //DW12.VadErrorLogic                               = VAD_ERROR_LOGIC_ENABLE;
    //DW12.VmdErrorLogic                               = VMD_ERROR_LOGIC_DISABLE;
}

Cmd::MFD_MPEG2_BSD_OBJECT_CMD::MFD_MPEG2_BSD_OBJECT_CMD()
{
    DW0.Value = 0x73280003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MPEG2DEC;
    //DW0.Pipeline                                     = PIPELINE_MFDMPEG2BSDOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.IsLastMb                                     = IS_LAST_MB_UNNAMED0;
    //DW3.MbRowLastSlice                               = MB_ROW_LAST_SLICE_UNNAMED0;
    //DW3.LastPicSlice                                 = LAST_PIC_SLICE_UNNAMED0;
    //DW3.SliceConcealmentTypeBit                      = SLICE_CONCEALMENT_TYPE_BIT_UNNAMED0;
    //DW3.SliceConcealmentOverrideBit                  = SLICE_CONCEALMENT_OVERRIDE_BIT_UNNAMED0;

    DW4.Value = 0x00000000;
}

Cmd::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD()
{
    DW0.Value = 0x00000000;
    //DW0.MacroblockIntraType                          = MACROBLOCK_INTRA_TYPE_NON_INTRAMACROBLOCK;
    //DW0.MacroblockMotionForward                      = MACROBLOCK_MOTION_FORWARD_NOFORWARDMOTIONVECTOR;
    //DW0.MacroblockMotionBackward                     = MACROBLOCK_MOTION_BACKWARD_NOBACKWARDMOTIONVECTOR;
    //DW0.DctType                                      = DCT_TYPE_MCFRAMEDCT;
    //DW0.MotionVerticalFieldSelect                    = MOTION_VERTICAL_FIELD_SELECT_TOPFIELD;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;
}

Cmd::MFD_IT_OBJECT_CMD::MFD_IT_OBJECT_CMD()
{
    DW0.Value = 0x70290010;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED9;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MFXCOMMONDEC;
    //DW0.Pipeline                                     = PIPELINE_MFDITOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.IndirectItMvDataStartAddressOffset           = INDIRECT_IT_MV_DATA_START_ADDRESS_OFFSET_UNNAMED0;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
    //DW4.IndirectItCoeffDataStartAddressOffset        = INDIRECT_IT_COEFF_DATA_START_ADDRESS_OFFSET_UNNAMED0;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;
    //DW6.IndirectItDblkControlDataStartAddressOffset  = INDIRECT_IT_DBLK_CONTROL_DATA_START_ADDRESS_OFFSET_UNNAMED0;
}

Cmd::MFC_MPEG2_SLICEGROUP_STATE_CMD::MFC_MPEG2_SLICEGROUP_STATE_CMD()
{
    DW0.Value = 0x73430006;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_MEDIA;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_MEDIA;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MPEG2;
    //DW0.Pipeline                                     = PIPELINE_MFXMPEG2SLICEGROUPSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.TailpresentflagTailInsertionPresentInBitstreamEncoderOnly = TAILPRESENTFLAG_TAIL_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_UNNAMED0;
    //DW1.SlicedataPresentflagSlicedataInsertionPresentInBitstreamEncoderOnly = SLICEDATA_PRESENTFLAG_SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_DISABLE;
    //DW1.HeaderpresentflagHeaderInsertionPresentInBitstreamEncoderOnly = HEADERPRESENTFLAG_HEADER_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_DISABLE;
    //DW1.BitstreamoutputflagCompressedBitstreamOutputDisableFlagEncoderOnly = BITSTREAMOUTPUTFLAG_COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENCODER_ONLY_ENABLE;
    //DW1.SkipconvdisabledMbTypeSkipConversionDisableEncoderOnly = SKIPCONVDISABLED_MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY_ENABLE;
    //DW1.RatectrlpanictypeRcPanicTypeEncoderOnly      = RATECTRLPANICTYPE_RC_PANIC_TYPE_ENCODER_ONLY_UNNAMED0;
    //DW1.RatectrlpanicflagRcPanicEnableEncoderOnly    = RATECTRLPANICFLAG_RC_PANIC_ENABLE_ENCODER_ONLY_DISABLE;
    //DW1.MbratectrlmodeRcTriggleModeEncoderOnly       = MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY_UNNAMED0;
    //DW1.MbratectrlresetResetratecontrolcounterEncoderOnly = MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY_DISABLE;
    //DW1.MbratectrlflagRatecontrolcounterenableEncoderOnly = MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY_DISABLE;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
    //DW4.BitstreamoffsetIndirectPakBseDataStartAddressWrite = BITSTREAMOFFSET_INDIRECT_PAK_BSE_DATA_START_ADDRESS_WRITE_UNNAMED0;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;
}

Cmd::MFX_VC1_PRED_PIPE_STATE_CMD::MFX_VC1_PRED_PIPE_STATE_CMD()
{
    DW0.Value = 0x72010004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED1;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VC1COMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXVC1PREDPIPESTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;
}

Cmd::MFD_VC1_LONG_PIC_STATE_CMD::MFD_VC1_LONG_PIC_STATE_CMD()
{
    DW0.Value = 0x72210004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED1;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VC1DEC;
    //DW0.Pipeline                                     = PIPELINE_MFDVC1LONGPICSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.Vc1Profile                                   = VC1_PROFILE_DISABLE;
    //DW2.OverlapSmoothingEnableFlag                   = OVERLAP_SMOOTHING_ENABLE_FLAG_DISABLE;
    //DW2.LoopfilterEnableFlag                         = LOOPFILTER_ENABLE_FLAG_DISABLE;
    //DW2.RangereductionEnable                         = RANGEREDUCTION_ENABLE_DISABLE;
    //DW2.Rangereductionscale                          = RANGEREDUCTIONSCALE_UNNAMED0;
    //DW2.MotionVectorMode                             = MOTION_VECTOR_MODE_UNNAMED0;
    //DW2.Syncmarker                                   = SYNCMARKER_NOTPRESENT;

    DW3.Value = 0x00000000;
    //DW3.FcmFrameCodingMode                           = FCM_FRAME_CODING_MODE_DISABLE;
    //DW3.Condover                                     = CONDOVER_UNNAMED0;

    DW4.Value = 0x00000000;
    //DW4.Pquantuniform                                = PQUANTUNIFORM_UNNAMED0;
    //DW4.AltpquantconfigAlternativePictureQuantizationConfiguration = ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION_UNNAMED0;
    //DW4.ExtendedmvrangeExtendedMotionVectorRangeFlag = EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG_UNNAMED0;
    //DW4.ExtendeddmvrangeExtendedDifferentialMotionVectorRangeFlag = EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG_UNNAMED0;
    //DW4.NumrefNumberOfReferences                     = NUMREF_NUMBER_OF_REFERENCES_UNNAMED0;
    //DW4.ReffieldpicpolarityReferenceFieldPicturePolarity = REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY_UNNAMED0;
    //DW4.FastuvmcflagFastUvMotionCompensationFlag     = FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED0;
    //DW4.FourmvswitchFourMotionVectorSwitch           = FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH_DISABLE;
    //DW4.UnifiedmvmodeUnifiedMotionVectorMode         = UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE_UNNAMED0;

    DW5.Value = 0x00000000;
    //DW5.TransdctabIntraTransformDcTable              = TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE_UNNAMED0;
    //DW5.TranstypembflagMacroblockTransformTypeFlag   = TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG_UNNAMED0;
    //DW5.Fieldtxraw                                   = FIELDTXRAW_DISABLE;
    //DW5.Acpredraw                                    = ACPREDRAW_DISABLE;
    //DW5.Overflagsraw                                 = OVERFLAGSRAW_UNNAMED0;
    //DW5.Directmbraw                                  = DIRECTMBRAW_UNNAMED0;
    //DW5.Skipmbraw                                    = SKIPMBRAW_DISABLE;
    //DW5.Mvtypembraw                                  = MVTYPEMBRAW_UNNAMED0;
    //DW5.Forwardmbraw                                 = FORWARDMBRAW_UNNAMED0;
    //DW5.BitplanepresentflagBitplaneBufferPresentFlag = BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG_UNNAMED0;
}

Cmd::MFD_VC1_SHORT_PIC_STATE_CMD::MFD_VC1_SHORT_PIC_STATE_CMD()
{
    DW0.Value = 0x72200003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED0;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VC1DEC;
    //DW0.Pipeline                                     = PIPELINE_MFDVC1SHORTPICSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.PictureStructure                             = PICTURE_STRUCTURE_UNNAMED0;
    //DW2.IntraPictureFlag                             = INTRA_PICTURE_FLAG_UNNAMED0;
    //DW2.Vc1Profile                                   = VC1_PROFILE_UNNAMED0;

    DW3.Value = 0x00000000;
    //DW3.VstransformFlag                              = VSTRANSFORM_FLAG_DISABLE;
    //DW3.Dquant                                       = DQUANT_UNNAMED0;
    //DW3.ExtendedMvPresentFlag                        = EXTENDED_MV_PRESENT_FLAG_UNNAMED0;
    //DW3.FastuvmcflagFastUvMotionCompensationFlag     = FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED0;
    //DW3.LoopfilterEnableFlag                         = LOOPFILTER_ENABLE_FLAG_UNNAMED0;
    //DW3.PanscanPresentFlag                           = PANSCAN_PRESENT_FLAG_UNNAMED0;
    //DW3.RangeredPresentFlagForSimpleMainProfileOnly  = RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0;
    //DW3.SyncmarkerPresentFlagForSimpleMainProfileOnly = SYNCMARKER_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0;
    //DW3.MultiresPresentFlagForSimpleMainProfileOnly  = MULTIRES_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0;
    //DW3.Quantizer                                    = QUANTIZER_UNNAMED0;
    //DW3.PPicRefDistance                              = P_PIC_REF_DISTANCE_UNNAMED0;
    //DW3.ProgressivePicType                           = PROGRESSIVE_PIC_TYPE_UNNAMED0;
    //DW3.RangeReductionEnable                         = RANGE_REDUCTION_ENABLE_DISABLE;
    //DW3.RangeReductionScale                          = RANGE_REDUCTION_SCALE_DISABLE;
    //DW3.OverlapSmoothingEnableFlag                   = OVERLAP_SMOOTHING_ENABLE_FLAG_DISABLE;

    DW4.Value = 0x00000000;
    //DW4.ExtendedDmvPresentFlag                       = EXTENDED_DMV_PRESENT_FLAG_UNNAMED0;
    //DW4.RefpicFlag                                   = REFPIC_FLAG_UNNAMED0;
}

Cmd::MFX_VC1_DIRECTMODE_STATE_CMD::MFX_VC1_DIRECTMODE_STATE_CMD()
{
    DW0.Value = 0x72020005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED2;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VC1COMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXVC1DIRECTMODESTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
}

Cmd::MFD_VC1_BSD_OBJECT_CMD::MFD_VC1_BSD_OBJECT_CMD()
{
    DW0.Value = 0x72280003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VC1DEC;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.IndirectDataStartAddress                     = INDIRECT_DATA_START_ADDRESS_UNNAMED0;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
    //DW4.EmulationPreventionBytePresent               = EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED0;
}

Cmd::MFX_JPEG_PIC_STATE_CMD::MFX_JPEG_PIC_STATE_CMD()
{
    DW0.Value = 0x77000001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_MEDIA;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_JPEG;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.Obj0.OutputMcuStructure                      = OUTPUT_MCU_STRUCTURE_YUV400;
    //DW1.Obj0.InputSurfaceFormatYuv                   = INPUT_SURFACE_FORMAT_YUV_UNNAMED0;
    //DW1.Obj1.InputFormatYuv                          = INPUT_FORMAT_YUV_UNNAMED0;
    //DW1.Obj1.Rotation                                = ROTATION_UNNAMED0;
    //DW1.Obj1.OutputFormatYuv                         = OUTPUT_FORMAT_YUV_UNNAMED0;
    //DW1.Obj1.AverageDownSampling                     = AVERAGE_DOWN_SAMPLING_UNNAMED0;
    //DW1.Obj1.VerticalDownSamplingEnable              = VERTICAL_DOWN_SAMPLING_ENABLE_UNNAMED0;
    //DW1.Obj1.HorizontalDownSamplingEnable            = HORIZONTAL_DOWN_SAMPLING_ENABLE_UNNAMED0;
    //DW1.Obj1.VerticalUpSamplingEnable                = VERTICAL_UP_SAMPLING_ENABLE_UNNAMED0;

    DW2.Value = 0x00000000;
    //DW2.Obj0.OutputPixelNormalize                    = OUTPUT_PIXEL_NORMALIZE_UNNAMED0;
    //DW2.Obj1.Roundingquant                           = ROUNDINGQUANT_UNNAMED0;
}

Cmd::MFC_JPEG_HUFF_TABLE_STATE_CMD::MFC_JPEG_HUFF_TABLE_STATE_CMD()
{
    DW0.Value = 0x774300ae;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_MEDIA;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_JPEG;
    //DW0.Pipeline                                     = PIPELINE_MFCJPEGHUFFTABLESTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.HuffTableId                                  = HUFF_TABLE_ID_UNNAMED0;

    memset(&DcTable, 0, sizeof(DcTable));

    memset(&AcTable, 0, sizeof(AcTable));
}

Cmd::MFD_JPEG_BSD_OBJECT_CMD::MFD_JPEG_BSD_OBJECT_CMD()
{
    DW0.Value = 0x77280004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED8;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_JPEGDEC;
    //DW0.Pipeline                                     = PIPELINE_MFDJPEGBSDOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
    //DW4.Interleaved                                  = INTERLEAVED_NON_INTERLEAVED;

    DW5.Value = 0x00000000;
}

Cmd::MFC_JPEG_SCAN_OBJECT_CMD::MFC_JPEG_SCAN_OBJECT_CMD()
{
    DW0.Value = 0x77490001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED9;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED2;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_JPEGENC;
    //DW0.Pipeline                                     = PIPELINE_MFCJPEGSCANOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.IsLastScan                                   = IS_LAST_SCAN_UNNAMED0;
    //DW2.HeadPresentFlag                              = HEAD_PRESENT_FLAG_UNNAMED0;
    //DW2.HuffmanDcTable                               = HUFFMAN_DC_TABLE_BIT20_V0;
    //DW2.HuffmanAcTable                               = HUFFMAN_AC_TABLE_BIT24_V0;
}

Cmd::MFX_VP8_Encoder_CFG_CMD::MFX_VP8_Encoder_CFG_CMD()
{
    DW0.Value = 0x7441001c;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubOpcodeB                                   = SUB_OPCODE_B_MFXVP8ENCODERCFG;
    //DW0.SubOpcodeA                                   = SUB_OPCODE_A_VP8COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VP8;
    //DW0.Pipeline                                     = PIPELINE_VIDEOCODEC;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.RateControlInitialPass                       = RATE_CONTROL_INITIAL_PASS_SUBSEQUENCEPASS_ES;

    DW2.Value = 0x00000000;
    //DW2.MaxFrameBitCountRateControlEnableMask        = MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED0;
    //DW2.MinFrameBitCountRateControlEnableMask        = MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED0;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
    //DW4.FrameBitRateMaxUnit                          = FRAME_BIT_RATE_MAX_UNIT_32_B;
    //DW4.FrameBitrateMaxUnitMode                      = FRAME_BITRATE_MAX_UNIT_MODE_COMPATIBILITYMODE;
    //DW4.FrameBitRateMinUnit                          = FRAME_BIT_RATE_MIN_UNIT_32_B;
    //DW4.FrameBitrateMinUnitMode                      = FRAME_BITRATE_MIN_UNIT_MODE_COMPATIBILITYMODE;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

    DW15.Value = 0x00000000;

    DW16.Value = 0x00000000;

    DW17.Value = 0x00000000;

    DW18.Value = 0x00000000;

    DW19.Value = 0x00000000;

    DW20.Value = 0x00000000;

    DW21.Value = 0x00000000;

    DW22.Value = 0x00000000;
    //DW22.MinFrameWsizeUnit                           = MIN_FRAME_WSIZE_UNIT_COMPATIBILITYMODE;

    DW23.Value = 0x00000000;

    DW24.Value = 0x00000000;

    DW25.Value = 0x00000000;

    DW26.Value = 0x00000000;

    DW27.Value = 0x00000000;

    DW28.Value = 0x00000000;

    DW29.Value = 0x00000000;
}

Cmd::MFX_VP8_BSP_BUF_BASE_ADDR_STATE_CMD::MFX_VP8_BSP_BUF_BASE_ADDR_STATE_CMD()
{
    DW0.Value = 0x7443001e;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubOpcodeB                                   = SUB_OPCODE_B_MFXVP8BSPBUFBASEADDRSTATE;
    //DW0.SubOpcodeA                                   = SUB_OPCODE_A_VP8COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VP8;
    //DW0.Pipeline                                     = PIPELINE_VIDEOCODEC;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    memset(&IntermediateBufferPartitionOffset, 0, sizeof(IntermediateBufferPartitionOffset));

    DW15.Value = 0x00000000;

    DW19.Value = 0x00000000;
}

Cmd::MFD_VP8_BSD_OBJECT_CMD::MFD_VP8_BSD_OBJECT_CMD()
{
    DW0.Value = 0x74280014;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_UNNAMED8;
    //DW0.Subopcodea                                   = SUBOPCODEA_UNNAMED1;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VP8DEC;
    //DW0.Pipeline                                     = PIPELINE_MFDVP8BSDOBJECT;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

    DW15.Value = 0x00000000;

    DW16.Value = 0x00000000;

    DW17.Value = 0x00000000;

    DW18.Value = 0x00000000;

    DW19.Value = 0x00000000;

    DW20.Value = 0x00000000;

    DW21.Value = 0x00000000;
    //DW21.MbHeaderErrorHandling                       = MB_HEADER_ERROR_HANDLING_IGNORETHEERRORANDCONTINUE_MASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING;
    //DW21.EntropyErrorHandling                        = ENTROPY_ERROR_HANDLING_IGNORETHEERRORANDCONTINUE_MASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING;
    //DW21.MprErrorMvOutOfRangeHandling                = MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_IGNORETHEERRORANDCONTINUE_MASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING;
    //DW21.BsdprematurecompleteErrorHandling           = BSDPREMATURECOMPLETE_ERROR_HANDLING_IGNORETHEERRORANDCONTINUE_MASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING;
    //DW21.ConcealmentMethod                           = CONCEALMENT_METHOD_INTRA16X16PREDICTION;
}

Cmd::MFX_VP8_PIC_STATE_CMD::MFX_VP8_PIC_STATE_CMD()
{
    DW0.Value = 0x74000024;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubOpcodeB                                   = SUB_OPCODE_B_MFXVP8PICSTATE;
    //DW0.SubOpcodeA                                   = SUB_OPCODE_A_VP8COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VP8;
    //DW0.Pipeline                                     = PIPELINE_VIDEOCODEC;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.McFilterSelect                               = MC_FILTER_SELECT_UNNAMED0;
    //DW2.ChromaFullPixelMcFilterMode                  = CHROMA_FULL_PIXEL_MC_FILTER_MODE_UNNAMED0;
    //DW2.Dblkfiltertype                               = DBLKFILTERTYPE_UNNAMED0;
    //DW2.Skeyframeflag                                = SKEYFRAMEFLAG_NON_KEYFRAME_P_FRAME;
    //DW2.SegmentationIdStreamoutEnable                = SEGMENTATION_ID_STREAMOUT_ENABLE_STREAMOUTDISABLED;
    //DW2.SegmentationIdStreaminEnable                 = SEGMENTATION_ID_STREAMIN_ENABLE_STREAMINDISABLED;
    //DW2.SegmentEnableFlag                            = SEGMENT_ENABLE_FLAG_UNNAMED0;
    //DW2.UpdateMbsegmentMapFlag                       = UPDATE_MBSEGMENT_MAP_FLAG_UNNAMED0;
    //DW2.MbNocoeffSkipflag                            = MB_NOCOEFF_SKIPFLAG_UNNAMED0;
    //DW2.ModeReferenceLoopFilterDeltaEnabled          = MODE_REFERENCE_LOOP_FILTER_DELTA_ENABLED_UNNAMED0;
    //DW2.Log2NumOfPartition                           = LOG2_NUM_OF_PARTITION_1TOKENPARTITION;

    DW3.Value = 0x00000000;
    //DW3.DblkfilterlevelForSegment0                   = DBLKFILTERLEVEL_FOR_SEGMENT0_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION;
    //DW3.DblkfilterlevelForSegment1                   = DBLKFILTERLEVEL_FOR_SEGMENT1_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION;
    //DW3.DblkfilterlevelForSegment2                   = DBLKFILTERLEVEL_FOR_SEGMENT2_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION;
    //DW3.DblkfilterlevelForSegment3                   = DBLKFILTERLEVEL_FOR_SEGMENT3_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

    DW15.Value = 0x00000000;

    DW18.Value = 0x00000000;
    //DW18.CoeffprobabilityStreaminArbitrationPriorityControl = COEFFPROBABILITY_STREAMIN_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW18.CoeffprobabilityStreaminMemoryCompressionMode = COEFFPROBABILITY_STREAMIN_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    //DW18.CoeffprobabilityStreaminTiledResourceMode   = COEFFPROBABILITY_STREAMIN_TILED_RESOURCE_MODE_TRMODENONE;

    DW19.Value = 0x00000000;

    DW20.Value = 0x00000000;

    DW21.Value = 0x00000000;

    DW22.Value = 0x00000000;

    DW23.Value = 0x00000000;

    DW24.Value = 0x00000000;

    DW25.Value = 0x00000000;

    DW26.Value = 0x00000000;

    DW27.Value = 0x00000000;

    DW28.Value = 0x00000000;

    DW29.Value = 0x00000000;

    DW30.Value = 0x00000000;

    DW31.Value = 0x00000000;

    DW32.Value = 0x00000000;

    DW33.Value = 0x00000000;

    DW34.Value = 0x00000000;

    DW37.Value = 0x00000000;
    //DW37.SegmentationIdStreamArbitrationPriorityControl = SEGMENTATION_ID_STREAM_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW37.SegmentationIdStreamMemoryCompressionMode   = SEGMENTATION_ID_STREAM_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    //DW37.SegmentationIdStreamTiledResourceMode       = SEGMENTATION_ID_STREAM_TILED_RESOURCE_MODE_TRMODENONE;
}

Cmd::MFX_JPEG_HUFF_TABLE_STATE_CMD::MFX_JPEG_HUFF_TABLE_STATE_CMD()
{
    DW0.Value = 0x77020033;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED2;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_JPEGCOMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXMULTIDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.Hufftableid1Bit                              = HUFFTABLEID_1_BIT_Y;

    memset(&DcBits128BitArray, 0, sizeof(DcBits128BitArray));

    memset(&DcHuffval128BitArray, 0, sizeof(DcHuffval128BitArray));

    memset(&AcBits168BitArray, 0, sizeof(AcBits168BitArray));

    memset(&AcHuffval1608BitArray, 0, sizeof(AcHuffval1608BitArray));

    DW52.Value = 0x00000000;
}

Cmd::MFX_PIPE_BUF_ADDR_STATE_CMD::MFX_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value = 0x70020042;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED2;
    //DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    //DW0.CommonOpcode                                 = COMMON_OPCODE_MFXCOMMONSTATE;
    //DW0.Pipeline                                     = PIPELINE_MFXPIPEBUFADDRSTATE;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;
    //DW3.PreDeblockingArbitrationPriorityControl      = PRE_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;
    //DW6.PostDeblockingArbitrationPriorityControl     = POST_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;
    //DW9.OriginalUncompressedPictureSourceArbitrationPriorityControl = ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;
    //DW12.StreamoutDataDestinationArbitrationPriorityControl = STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

    DW15.Value = 0x00000000;
    //DW15.IntraRowStoreScratchBufferArbitrationPriorityControl = INTRA_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW15.IntraRowStoreScratchBufferCacheSelect       = INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;

    DW16.Value = 0x00000000;

    DW17.Value = 0x00000000;

    DW18.Value = 0x00000000;
    //DW18.DeblockingFilterRowStoreScratchArbitrationPriorityControl = DEBLOCKING_FILTER_ROW_STORE_SCRATCH_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW18.DeblockingFilterRowStoreScratchBufferCacheSelect = DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;

    MOS_ZeroMemory(Refpicbaseaddr, sizeof(Refpicbaseaddr));

    DW51.Value = 0x00000000;
    //DW51.ReferencePictureArbitrationPriorityControl  = REFERENCE_PICTURE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW52.Value = 0x00000000;

    DW53.Value = 0x00000000;

    DW54.Value = 0x00000000;
    //DW54.MacroblockStatusBufferArbitrationPriorityControl = MACROBLOCK_STATUS_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW55.Value = 0x00000000;

    DW56.Value = 0x00000000;

    DW57.Value = 0x00000000;
    //DW57.MacroblockIldbStreamoutBufferArbitrationPriorityControl = MACROBLOCK_ILDB_STREAMOUT_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW58.Value = 0x00000000;

    DW59.Value = 0x00000000;

    DW60.Value = 0x00000000;

    DW61.Value                                       = 0x00000000;

    DW62.Value = 0x00000000;

    DW63.Value = 0x00000000;

    DW64.Value = 0x00000000;
    //DW64.ScaleReferenceSurfaceArbitrationPriorityControl = SCALE_REFERENCE_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    //DW64.ScaledReferenceSurfaceRenderCompressionEnable = SCALED_REFERENCE_SURFACE_RENDER_COMPRESSION_ENABLE_DISABLE;

    DW65.Value = 0x00000000;

    DW66.Value = 0x00000000;

    DW67.Value = 0x00000000;
    //DW67.SlicesizeStreamoutDataDestinationArbitrationPriorityControl = SLICESIZE_STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
}

Cmd::MFX_AVC_DIRECTMODE_STATE_CMD::MFX_AVC_DIRECTMODE_STATE_CMD()
{
    DW0.Value = 0x71020045;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_UNNAMED2;
    //DW0.Subopcodea                                   = SUBOPCODEA_UNNAMED0;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_AVCCOMMON;
    //DW0.Pipeline                                     = PIPELINE_MFXSINGLEDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    memset(&PocList, 0, sizeof(PocList));
}
