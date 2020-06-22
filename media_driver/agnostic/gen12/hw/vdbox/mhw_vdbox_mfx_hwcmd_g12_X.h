/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     mhw_vdbox_mfx_hwcmd_g12_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g11_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_VDBOX_MFX_HWCMD_G12_X_H__
#define __MHW_VDBOX_MFX_HWCMD_G12_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_mfx_g12_X
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH( x ) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief MFX_QM_STATE
    //! \details
    //!     This is a common state command for AVC encoder modes. For encoder, it
    //!     represents both the forward QM matrices as well as the decoding QM
    //!     matrices.This is a Frame-level state. Only Scaling Lists specified by an
    //!     application are being sent to the hardware. The driver is responsible
    //!     for determining the final set of scaling lists to be used for decoding
    //!     the current slice, based on the AVC Spec Table 7-2 (Fall-Back Rules A
    //!     and B).In MFX AVC PAK mode, PAK needs both forward Q scaling lists and
    //!     IQ scaling lists. The IQ scaling lists are sent as in MFD in raster scan
    //!     order. But the Forward Q scaling lists are sent in column-wise raster
    //!     order (column-by-column) to simplify the H/W. Driver will perform all
    //!     the scan order conversion for both ForwardQ and IQ.
    //!     
    struct MFX_QM_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Avc                                              : __CODEGEN_BITFIELD( 0,  1)    ; //!< AVC, AVC- Decoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved, AVC- Decoder Only
            } Obj0;
            struct
            {
                uint32_t                 Mpeg2                                            : __CODEGEN_BITFIELD( 0,  1)    ; //!< MPEG2, MPEG2- Decoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved, MPEG2- Decoder Only
            } Obj1;
            struct
            {
                uint32_t                 Jpeg                                             : __CODEGEN_BITFIELD( 0,  1)    ; //!< JPEG, JPEG- Encoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            }Obj2;
            uint32_t                     Value;
        } DW1;

        uint32_t                         ForwardQuantizerMatrix[16];                                                      //!< Forward Quantizer Matrix


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED7                                             = 7, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE                              = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief AVC
        //! \details
        //!     <b>For AVC QM Type</b>: This field specifies which Quantizer Matrix is
        //!     loaded.
        enum AVC
        {
            AVC_AVC_4X_4INTRAMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS     = 0, //!< No additional details
            AVC_AVC_4X_4INTERMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS     = 1, //!< No additional details
            AVC_AVC8X8INTRAMATRIX                                            = 2, //!< No additional details
            AVC_AVC8X8INTERMATRIX                                            = 3, //!< No additional details
        };

        //! \brief MPEG2
        //! \details
        //!     <b>For MPEG2 QM Type</b>: This field specifies which Quantizer Matrix is
        //!     loaded.
        enum MPEG2
        {
            MPEG2_MPEGINTRAQUANTIZERMATRIX                                   = 0, //!< No additional details
            MPEG2_MPEGNONINTRAQUANTIZERMATRIX                                = 1, //!< No additional details
        };

        //! \brief JPEG
        //! \details
        //!     <b> For JPEG QM Type</b>:This field specifies which Quantizer Matrix is
        //!     loaded.
        enum JPEG
        {
            JPEG_JPEGLUMAYQUANTIZERMATRIXORR                                 = 0, //!< No additional details
            JPEG_JPEGCHROMACBQUANTIZERMATRIXORG                              = 1, //!< No additional details
            JPEG_JPEGCHROMACRQUANTIZERMATRIXORB                              = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_QM_STATE_CMD();

        static const size_t dwSize = 18;
        static const size_t byteSize = 72;
    };

    //!
    //! \brief MFX_FQM_STATE
    //! \details
    //!     This is a common state command for AVC encoder modes. For encoder, it
    //!     represents both the forward QM matrices as well as the decoding QM
    //!     matrices.This is a Frame-level state. Only Scaling Lists specified by an
    //!     application are being sent to the hardware. The driver is responsible
    //!     for determining the final set of scaling lists to be used for decoding
    //!     the current slice, based on the AVC Spec Table 7-2 (Fall-Back Rules A
    //!     and B).In MFX AVC PAK mode, PAK needs both forward Q scaling lists and
    //!     IQ scaling lists. The IQ scaling lists are sent as in MFD in raster scan
    //!     order. But the Forward Q scaling lists are sent in column-wise raster
    //!     order (column-by-column) to simplify the H/W. Driver will perform all
    //!     the scan order conversion for both ForwardQ and IQ.
    //!     
    struct MFX_FQM_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Avc                                              : __CODEGEN_BITFIELD( 0,  1)    ; //!< AVC, AVC- Decoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved, AVC- Decoder Only
            } Obj0;
            struct
            {
                uint32_t                 Mpeg2                                            : __CODEGEN_BITFIELD( 0,  1)    ; //!< MPEG2, MPEG2- Decoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved, MPEG2- Decoder Only
            } Obj1;
            struct
            {
                uint32_t                 Jpeg                                             : __CODEGEN_BITFIELD( 0,  1)    ; //!< JPEG, JPEG- Encoder Only
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            }Obj2;
            uint32_t                     Value;
        } DW1;

        uint32_t                         ForwardQuantizerMatrix[32];                                                      //!< Forward Quantizer Matrix


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE                              = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief AVC
        //! \details
        //!     <b>For AVC QM Type</b>: This field specifies which Quantizer Matrix is
        //!     loaded.
        enum AVC
        {
            AVC_AVC_4X_4INTRAMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS     = 0, //!< No additional details
            AVC_AVC_4X_4INTERMATRIX_Y_4DWS_CB_4DWS_CR_4DWS_RESERVED_4DWS     = 1, //!< No additional details
            AVC_AVC8X8INTRAMATRIX                                            = 2, //!< No additional details
            AVC_AVC8X8INTERMATRIX                                            = 3, //!< No additional details
        };

        //! \brief MPEG2
        //! \details
        //!     <b>For MPEG2 QM Type</b>: This field specifies which Quantizer Matrix is
        //!     loaded.
        enum MPEG2
        {
            MPEG2_MPEGINTRAQUANTIZERMATRIX                                   = 0, //!< No additional details
            MPEG2_MPEGNONINTRAQUANTIZERMATRIX                                = 1, //!< No additional details
        };

        //! \brief JPEG
        //! \details
        //!     <b> For JPEG QM Type</b>:This field specifies which Quantizer Matrix is
        //!     loaded.
        enum JPEG
        {
            JPEG_JPEGLUMAYQUANTIZERMATRIXORR                                 = 0, //!< No additional details
            JPEG_JPEGCHROMACBQUANTIZERMATRIXORG                              = 1, //!< No additional details
            JPEG_JPEGCHROMACRQUANTIZERMATRIXORB                              = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_FQM_STATE_CMD();

        static const size_t dwSize = 34;
        static const size_t byteSize = 136;
    };

    //!
    //! \brief MFX_PIPE_MODE_SELECT
    //! \details
    //!     Specifies which codec and hardware module is being used to encode/decode
    //!     the video data, on a per-frame basis. The MFX_PIPE_MODE_SELECT command
    //!     specifies which codec and hardware module is being used to encode/decode
    //!     the video data, on a per-frame basis. It also configures the hardware
    //!     pipeline according to the active encoder/decoder operating mode for
    //!     encoding/decoding the current picture. Commands issued specifically for
    //!     AVC and MPEG2 are ignored when VC1 is the active codec.
    //!     
    struct MFX_PIPE_MODE_SELECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(24, 26)    ; //!< OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 StandardSelect                                   : __CODEGEN_BITFIELD( 0,  3)    ; //!< STANDARD_SELECT
                uint32_t                 CodecSelect                                      : __CODEGEN_BITFIELD( 4,  4)    ; //!< CODEC_SELECT
                uint32_t                 StitchMode                                       : __CODEGEN_BITFIELD( 5,  5)    ; //!< STITCH_MODE
                uint32_t                 FrameStatisticsStreamoutEnable                   : __CODEGEN_BITFIELD( 6,  6)    ; //!< FRAME_STATISTICS_STREAMOUT_ENABLE
                uint32_t                 ScaledSurfaceEnable                              : __CODEGEN_BITFIELD( 7,  7)    ; //!< SCALED_SURFACE_ENABLE
                uint32_t                 PreDeblockingOutputEnablePredeblockoutenable     : __CODEGEN_BITFIELD( 8,  8)    ; //!< PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE
                uint32_t                 PostDeblockingOutputEnablePostdeblockoutenable   : __CODEGEN_BITFIELD( 9,  9)    ; //!< POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE
                uint32_t                 StreamOutEnable                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< STREAM_OUT_ENABLE
                uint32_t                 PicErrorStatusReportEnable                       : __CODEGEN_BITFIELD(11, 11)    ; //!< PIC_ERRORSTATUS_REPORT_ENABLE
                uint32_t                 DeblockerStreamOutEnable                         : __CODEGEN_BITFIELD(12, 12)    ; //!< DEBLOCKER_STREAM_OUT_ENABLE
                uint32_t                 VdencMode                                        : __CODEGEN_BITFIELD(13, 13)    ; //!< VDENC_MODE
                uint32_t                 StandaloneVdencModeEnable                        : __CODEGEN_BITFIELD(14, 14)    ; //!< STANDALONE_VDENC_MODE_ENABLE
                uint32_t                 DecoderModeSelect                                : __CODEGEN_BITFIELD(15, 16)    ; //!< DECODER_MODE_SELECT
                uint32_t                 DecoderShortFormatMode                           : __CODEGEN_BITFIELD(17, 17)    ; //!< DECODER_SHORT_FORMAT_MODE
                uint32_t                 ExtendedStreamOutEnable                          : __CODEGEN_BITFIELD(18, 18)    ; //!< Extended stream out enable
                uint32_t                 Reserved51                                       : __CODEGEN_BITFIELD(19, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved
                uint32_t                 VdsIldbCalculation                               : __CODEGEN_BITFIELD( 3,  3)    ; //!< VDS_ILDB_CALCULATION
                uint32_t                 Reserved68                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 ClockGateEnableForSignals                        : __CODEGEN_BITFIELD( 5,  5)    ; //!< Clock gate Enable for signals
                uint32_t                 ClockGateEnableAtSliceLevel                      : __CODEGEN_BITFIELD( 6,  6)    ; //!< CLOCK_GATE_ENABLE_AT_SLICE_LEVEL
                uint32_t                 Reserved71                                       : __CODEGEN_BITFIELD( 7,  9)    ; //!< Reserved
                uint32_t                 MpcPref08X8DisableFlagDefault0                   : __CODEGEN_BITFIELD(10, 10)    ; //!< MPC_PREF08X8_DISABLE_FLAG_DEFAULT_0
                uint32_t                 Reserved75                                       : __CODEGEN_BITFIELD(11, 13)    ; //!< Reserved
                uint32_t                 Vlf720IOddHeightInVc1Mode                        : __CODEGEN_BITFIELD(14, 14)    ; //!< VLF_720I_ODD_HEIGHT_IN_VC1_MODE_
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 26)    ; //!< Reserved
                uint32_t                 VmbSvcTlbDummyFetchDisableForPerformance         : __CODEGEN_BITFIELD(27, 27)    ; //!< VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE
                uint32_t                 VmbSvcMvReplicationFor8X8EnableErrorHandling     : __CODEGEN_BITFIELD(28, 28)    ; //!< VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 PicStatusErrorReportId                                                           ; //!< PIC_STATUSERROR_REPORT_ID
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 MediaSoftResetCounterPer1000Clocks                                               ; //!< MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_MFXPIPEMODESELECT                                         = 0, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_MFXCOMMONSTATE                                            = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum STANDARD_SELECT
        {
            STANDARD_SELECT_MPEG2                                            = 0, //!< No additional details
            STANDARD_SELECT_VC1                                              = 1, //!< No additional details
            STANDARD_SELECT_AVC                                              = 2, //!< Covers both AVC and MVC
            STANDARD_SELECT_JPEG                                             = 3, //!< No additional details
            STANDARD_SELECT_SVC                                              = 4, //!< No additional details
            STANDARD_SELECT_VP8                                              = 5, //!< Decoder starting from BDW, Encoder starting from CHV
            STANDARD_SELECT_UVLD                                             = 15, //!< SW decoder w/ embedded micro-controller and co-processor
        };

        enum CODEC_SELECT
        {
            CODEC_SELECT_DECODE                                              = 0, //!< No additional details
            CODEC_SELECT_ENCODE                                              = 1, //!< Valid only if StandardSel is AVC, MPEG2 and SVC)
        };

        enum STITCH_MODE
        {
            STITCH_MODE_NOTINSTITCHMODE                                      = 0, //!< No additional details
            STITCH_MODE_INTHESPECIALSTITCHMODE                               = 1, //!< This mode can be used for any Codec as long as bitfield conditions are met.
        };

        //! \brief FRAME_STATISTICS_STREAMOUT_ENABLE
        //! \details
        //!     This field controls the frame level statistics streamout from the PAK. 
        //!                         <p><b>Note</b>: This field needs to be always "Enabled" in VD_Enc
        //!     mode. In case of non-VDEnc mode,
        //!                             this can be used to control the frame statistics output from the
        //!     PAK.</p>
        enum FRAME_STATISTICS_STREAMOUT_ENABLE
        {
            FRAME_STATISTICS_STREAMOUT_ENABLE_DISABLE                        = 0, //!< No additional details
            FRAME_STATISTICS_STREAMOUT_ENABLE_ENABLE                         = 1, //!< No additional details
        };

        //! \brief SCALED_SURFACE_ENABLE
        //! \details
        //!     This field indicates if the scaled surface is enabled. This field
        //!                         enables the 4x HME downscalar of the reconstructed image. Only
        //!     supported for AVC and VP8 formats.
        enum SCALED_SURFACE_ENABLE
        {
            SCALED_SURFACE_ENABLE_DISABLE                                    = 0, //!< No additional details
            SCALED_SURFACE_ENABLE_ENABLE                                     = 1, //!< No additional details
        };

        //! \brief PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE
        //! \details
        //!     This field controls the output write for the reconstructed pixels BEFORE
        //!     the deblocking filter.
        enum PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE
        {
            PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE_DISABLE         = 0, //!< No additional details
            PRE_DEBLOCKING_OUTPUT_ENABLE_PREDEBLOCKOUTENABLE_ENABLE          = 1, //!< No additional details
        };

        //! \brief POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE
        //! \details
        //!     This field controls the output write for the reconstructed pixels AFTER
        //!     the deblocking filter.In MPEG2 decoding mode, if this is enabled, VC1
        //!     deblocking filter is used.
        enum POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE
        {
            POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE_DISABLE       = 0, //!< No additional details
            POST_DEBLOCKING_OUTPUT_ENABLE_POSTDEBLOCKOUTENABLE_ENABLE        = 1, //!< No additional details
        };

        //! \brief STREAM_OUT_ENABLE
        //! \details
        //!     This field controls whether the macroblock parameter stream-out is
        //!     enabled during VLD decoding for transcoding purpose.
        enum STREAM_OUT_ENABLE
        {
            STREAM_OUT_ENABLE_DISABLE                                        = 0, //!< No additional details
            STREAM_OUT_ENABLE_ENABLE                                         = 1, //!< No additional details
        };

        //! \brief PIC_ERRORSTATUS_REPORT_ENABLE
        //! \details
        //!     <p>This field control whether the error/status reporting is enable or
        //!     not.0: Disable1: EnableIn decoder modes: Error reporting is written out
        //!     once per frame. The Error Report frame ID listed in DW3 along with the
        //!     VLD/IT error status bits are packed into one cache and written to the
        //!     "Decoded Picture Error/Status Buffer address" listed in the
        //!     MFX_PIPE_BUF_ADDR_STATE Command. Note: driver shall program different
        //!     error buffer addresses between pictrues; otherwise, hardware might
        //!     overwrite previous written data if driver does not read it fast
        //!     enough.In encoder modes: Not used</p>
        //!     <p>Please refer to "Media VDBOX -&gt; Video Codec -&gt; Other Codec
        //!     Functions -&gt; MFX Error Handling -&gt; Decoder" session for the output
        //!     format.</p>
        enum PIC_ERRORSTATUS_REPORT_ENABLE
        {
            PIC_ERRORSTATUS_REPORT_ENABLE_DISABLE                            = 0, //!< No additional details
            PIC_ERRORSTATUS_REPORT_ENABLE_ENABLE                             = 1, //!< No additional details
        };

        //! \brief DEBLOCKER_STREAM_OUT_ENABLE
        //! \details
        //!     This field indicates if Deblocker information is going to be streamout
        //!     during VLD decoding.
        //!                         For AVC, it is needed to enable the deblocker streamout as the AVC
        //!     Disable_DLKFilterIdc is a slice level parameters.  Driver needs to
        //!     determine ahead of time if at least one slice of the current frame/ has
        //!     deblocker ON.  
        //!                         For SVC, there are two deblocking control streamout buffers
        //!     (specified in MFX_BUF_ADDR State Command).  This field is still
        //!     associated with the slice level SVC Disable.DLK_Filter_Idc.
        enum DEBLOCKER_STREAM_OUT_ENABLE
        {
            DEBLOCKER_STREAM_OUT_ENABLE_DISABLE                              = 0, //!< Disable streamout of deblocking control information for standalone deblocker operation.It needs other fields to determine one or two SVC deblocking surface streamout (Post Deblocking Output Enable, Pre Deblocking Output Enable, interlayer idc and regular deblock idc).
            DEBLOCKER_STREAM_OUT_ENABLE_ENABLE                               = 1, //!< No additional details
        };

        //! \brief VDENC_MODE
        //! \details
        //!     This field indicates if PAK is working in legacy MBEnc mode or the VDEnc
        //!     mode.
        enum VDENC_MODE
        {
            VDENC_MODE_MBENCMODE                                             = 0, //!< PAK is working in legacy mode
            VDENC_MODE_VDENCMODE                                             = 1, //!< PAK is working in VDEnc mode
        };

        //! \brief STANDALONE_VDENC_MODE_ENABLE
        //! \details
        //!     This field indicates to PAK if this is standalone VDEnc mode. This is
        //!     primarily a  validation mode.
        enum STANDALONE_VDENC_MODE_ENABLE
        {
            STANDALONE_VDENC_MODE_ENABLE_VDENCPAK                            = 0, //!< No additional details
            STANDALONE_VDENC_MODE_ENABLE_PAKONLY                             = 1, //!< No additional details
        };

        //! \brief DECODER_MODE_SELECT
        //! \details
        //!     Each coding standard supports two entry points: VLD entry point and IT
        //!     (IDCT) entry point. This field selects which one is in use.This field is
        //!     only valid if Codec Select is 0 (decoder).
        enum DECODER_MODE_SELECT
        {
            DECODER_MODE_SELECT_VLDMODE                                      = 0, //!< All codec minimum must support this mode Configure the MFD Engine for VLD ModeNote: All codec minimum must support this mode
            DECODER_MODE_SELECT_ITMODE                                       = 1, //!< Configure the MFD Engine for IT ModeNote: Only VC1 and MPEG2 support this mode
            DECODER_MODE_SELECT_DEBLOCKERMODE                                = 2, //!< Configure the MFD Engine for Standalone Deblocker Mode. Require streamout AVC edge control information from preceeding decoding pass.Note: [HSW, EXCLUDE(HSW:GT3:A, HSW:GT3:B, HSW:GT2:B)] Only AVC, MPEG2 and SVC are supported.
            DECODER_MODE_SELECT_INTERLAYERMODE                               = 3, //!< Configure the MFX Engine for standalone SVC interlayer upsampling for motion info, residual and reconstructed pixel. Require information being streamout from the preceding encoding and decoding pass of a reference layer.>
        };

        //! \brief DECODER_SHORT_FORMAT_MODE
        //! \details
        //!     For IT mode, this bit must be 0.
        enum DECODER_SHORT_FORMAT_MODE
        {
            DECODER_SHORT_FORMAT_MODE_SHORTFORMATDRIVERINTERFACE             = 0, //!< AVC/VC1/MVC/SVC/VP8 Short Format Mode is in useNote: There is no Short Format for SVC and VP8 yet, so this field must be set to 1 for SVC and VP8.
            DECODER_SHORT_FORMAT_MODE_LONGFORMATDRIVERINTERFACE              = 1, //!< AVC/VC1/MVC/SVC/VP8 Long Format Mode is in use.
        };

        //! \brief VDS_ILDB_CALCULATION
        //! \details
        //!     This bit forces all MB into INTRA MBs before doing ILDB control
        //!     generation in VDS.
        enum VDS_ILDB_CALCULATION
        {
            VDS_ILDB_CALCULATION_DISABLE                                     = 0, //!< Use original definition for ILDB calculation.
            VDS_ILDB_CALCULATION_ENABLE                                      = 1, //!< Force neighbor Intra MB = 1 on ILDB BS calculation.
        };

        //! \brief CLOCK_GATE_ENABLE_AT_SLICE_LEVEL
        //! \details
        //!     BitFieldDesc:
        enum CLOCK_GATE_ENABLE_AT_SLICE_LEVEL
        {
            CLOCK_GATE_ENABLE_AT_SLICE_LEVEL_DISABLE                         = 0, //!< Disable Slice-level Clock gating, Unit-level Clock gating will apply
            CLOCK_GATE_ENABLE_AT_SLICE_LEVEL_ENABLE                          = 1, //!< Enable Slice-level Clock gating, overrides any Unit level Clock gating
        };

        enum MPC_PREF08X8_DISABLE_FLAG_DEFAULT_0
        {
            MPC_PREF08X8_DISABLE_FLAG_DEFAULT_0_DISABLE                      = 0, //!< No additional details
            MPC_PREF08X8_DISABLE_FLAG_DEFAULT_0_ENABLE                       = 1, //!< No additional details
        };

        //! \brief VLF_720I_ODD_HEIGHT_IN_VC1_MODE_
        //! \details
        //!     This bit indicates VLF write out VC1 picture with odd height (in MBs).
        enum VLF_720I_ODD_HEIGHT_IN_VC1_MODE_
        {
            VLF_720I_ODD_HEIGHT_IN_VC1_MODE_DISABLE                          = 0, //!< No additional details
            VLF_720I_ODD_HEIGHT_IN_VC1_MODE_ENABLE                           = 1, //!< 720i Enable
        };

        //! \brief VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE
        //! \details
        //!     This bit disables TLB dummy fetch in SVC mode in VMB.
        enum VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE
        {
            VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE_ENABLE           = 0, //!< Enable VMB TLB Dummy Fetch for Performance
            VMB_SVC_TLB_DUMMY_FETCH_DISABLE_FOR_PERFORMANCE_DISABLE          = 1, //!< Disable VMB TLB Dummy Fetch
        };

        //! \brief VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING
        //! \details
        //!     This bit enables Motion Vector replication on 8x8 level during SVC mode
        //!     for error handling.
        enum VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING
        {
            VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING_DISABLE     = 0, //!< Disable MV 8x8 replication in SVC mode
            VMB_SVC_MV_REPLICATION_FOR_8X8_ENABLE_ERROR_HANDLING_ENABLE      = 1, //!< Enable MV 8x8 Replication in SVC Mode
        };

        //! \brief PIC_STATUSERROR_REPORT_ID
        //! \details
        //!     In decoder modes: Error reporting is written out once per frame. This
        //!     field along with the VLD error status bits are packed into one cache and
        //!     written to the memory location specified by "Decoded Picture
        //!     Error/Status Buffer address" listed in the MFX_PIPE_BUF_ADDR_STATE
        //!     Command.
        enum PIC_STATUSERROR_REPORT_ID
        {
            PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED                         = 0, //!< Unique ID Number
        };

        //! \brief MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        //! \details
        //!     In decoder modes, this counter value specifies the number of clocks (per
        //!     1000) of GAC inactivity
        //!                         before a media soft-reset is applied to the HCP and HuC. If counter
        //!     value is set to 0, the media
        //!                         soft-reset feature is disabled and no reset will occur.
        //!                         <p>In encoder modes, this counter must be set to 0 to disable media
        //!     soft reset. This feature is not
        //!                             supported for the encoder.</p>
        enum MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        {
            MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE                 = 0,   //!< No additional details
            MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_ENABLE                  = 500, //!< number of clocks (per 1000) of GAC inactivity
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MFX_SURFACE_STATE
    //! \details
    //!     This command is common for all encoding/decoding modes, to specify the
    //!     uncompressed YUV picture (i.e. destination surface) or intermediate
    //!     streamout in/out surface (e.g. coefficient/residual) (field, frame or
    //!     interleaved frame) format for reading and writing:  Uncompressed,
    //!     original input picture to be encoded
    //!      Reconstructed non-filtered/filtered display picturec(becoming reference
    //!     pictures as well for subsequent temporal inter-prediction)
    //!      Residual in SVC
    //!      Reconstructed Intra pixel in SVC
    //!      CoeffPred in SVC
    //!       Since there is only one media surface state being active during the
    //!     entire encoding/decoding process, all the uncompressed/reconstructed
    //!     pictures are defined to have the same surface state. For each media
    //!     object call (decoding or encoding), multiple SVC surfaces can be active
    //!     concurrently,  to distinguish among them, a surfaceID is added to
    //!     specify for each type of surface.   The primary difference among picture
    //!     surface states is their individual programmed base addresses, which are
    //!     provided  by other state commands and not included in this command. MFX
    //!     engine is making the association of surface states and corresponding
    //!     buffer base addresses.   MFX engine currently supports only one media
    //!     surface type for video and that is the NV12 (Planar YUV420 with
    //!     interleaved U (Cb) and V (Cr). For optimizing memory efficiency based on
    //!     access patterns, only TileY is supported. For JPEG decoder, only IMC1
    //!     and IMC3 are supported. Pitch can be wider than the Picture Width in
    //!     pixels and garbage will be there at the end of each line. The following
    //!     describes all the different formats that are supported and not supported
    //!     in Gen7 MFX :  NV12 - 4:2:0 only; UV interleaved; Full Pitch, U and V
    //!     offset is set to 0 (the only format supported for video codec); vertical
    //!     UV offset is MB aligned; UV xoffsets = 0. JPEG does not support NV12
    //!     format because non-interleave JPEG has performance issue with partial
    //!     write (in interleaved UV format)
    //!      IMC 1 &amp; 3 - Full Pitch, U and V are separate plane; (JPEG only; U
    //!     plane + garbage first in full pitch followed by V plane + garbage in
    //!     full pitch). U and V vertical offsets are block aligned; U and V xoffset
    //!     = 0; there is no gap between Y, U and V planes. IMC1 and IMC3 are
    //!     different by a swap of U and V. This is the only format supported in
    //!     JPEG for all video subsampling types (4:4:4, 4:2:2 and 4:2:0)
    //!      We are not supporting IMC 2 &amp; 4 - Full Pitch, U and V are separate
    //!     plane (JPEG only; U plane first in full pitch followed by V plane in
    //!     full pitch - U and V plane are side-by-side). U and V vertical offsets
    //!     are 16-pixel aligned; V xoffset is half-pitch aligned; U xoffset is 0;
    //!     there is no gap between Y, U and V planes. IMC2 and IMC4 are different
    //!     by a swap of U and V.
    //!      We are not supporting YV12 - half pitch for each U and V plane, and
    //!     separate planes for Y, U and V (U plane first in half pitch followed by
    //!     V plane in half pitch). For YV12, U and V vertical offsets are block
    //!     aligned; U and V xoffset = 0; there is no gap between Y, U and V planes
    //!       Note that the following data structures are not specified through the
    //!     media surface state  1D buffers for row-store and other miscellaneous
    //!     information.
    //!      2D buffers for per-MB data-structures (e.g. DMV biffer, MB info record,
    //!     ILDB Control and Tcoeff/Stocoeff).
    //!       This surface state here is identical to the Surface State for
    //!     deinterlace and sample_8x8 messages described in the Shared Function
    //!     Volume and Sampler Chapter. For non pixel data, such as row stores,
    //!     indirect data (Compressed Slice Data, AVC MV record, Coeff record and
    //!     AVC ILDB record) and streamin/out and output compressed bitstream, a
    //!     linear buffer is employed. For row stores, the H/W is designed to
    //!     guarantee legal memory accesses (read and write). For the remaining
    //!     cases, indirect object base address, indirect object address upper
    //!     bound, object data start address (offset) and object data length are
    //!     used to fully specified their corresponding buffer. This mechanism is
    //!     chosen over the pixel surface type because of their variable record
    //!     sizes. All row store surfaces are linear surface. Their addresses are
    //!     programmed in Pipe_Buf_Base_State or Bsp_Buf_Base_Addr_State
    //!     
    //!     VC1 I picture scaling: Even though VC1 allows I reconstructed picture
    //!     scaling (via RESPIC), as such scaling is only allowed at I picture. All
    //!     subsequent P (and B) pictures must have the same picture dimensions with
    //!     the preceding I picture. Therefore, all reference pictures for P or B
    //!     picture can share the same surface state with the current P and B
    //!     picture. Note : H/W is not processing RESPIC. Application is no longer
    //!     expecting intel decoder pipelineand kernel to perform this function, it
    //!     is going to be done in the video post-processing scaler or display
    //!     controller scale as a separate step and controller.
    //!     
    //!     All video codec surfaces must be NV12 Compliant, except JPEG. U/V
    //!     vertical must be MB aligned for all video codec (further contrained for
    //!     field picture), but JPEG can be block aligned. All video codec and JPEG
    //!     uses Tiled - Y format only, for uncompressed pixel surfaces.
    //!     
    //!     Even for JPEG planar 420 surface, application may provide only 1
    //!     buffers, but there is still only one single surface state for all of
    //!     them. If IMC equal to 1, 2, 3 or 4, U and V have the pitch same as Y.
    //!     And U and V will have different offset, each offset is block aligned.
    //!     
    struct MFX_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(24, 26)    ; //!< OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SurfaceId                                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< SURFACE_ID
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 CrVCbUPixelOffsetVDirection                      : __CODEGEN_BITFIELD( 0,  1)    ; //!< Cr(V)/Cb(U) Pixel Offset V Direction 
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 4, 17)    ; //!< Width
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(18, 31)    ; //!< Height
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 TileWalk                                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< TILE_WALK
                uint32_t                 TiledSurface                                     : __CODEGEN_BITFIELD( 1,  1)    ; //!< TILED_SURFACE
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< Half Pitch for Chroma
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 3, 19)    ; //!< Surface Pitch
                uint32_t                 Reserved116                                      : __CODEGEN_BITFIELD( 20,26)    ; //!< Reserved
                uint32_t                 InterleaveChroma                                 : __CODEGEN_BITFIELD(27, 27)    ; //!< INTERLEAVE_CHROMA_
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(28, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 YOffsetForUCb                                    : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for U(Cb)
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 XOffsetForUCb                                    : __CODEGEN_BITFIELD(16, 30)    ; //!< X Offset for U(Cb) 
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 YOffsetForVCr                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Y Offset for V(Cr)
                uint32_t                 XOffsetForVCr                                    : __CODEGEN_BITFIELD(16, 28)    ; //!< X Offset for V(Cr)
                uint32_t                 Reserved189                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_UNNAMED1                                                  = 1, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_MFXCOMMONSTATE                                            = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum SURFACE_ID
        {
            SURFACE_ID_DECODEDPICTUREANDREFERENCEPICTURES_SVCUPSAMPLINGSTREAMOUTRECONSTRUCTEDPIXELSCOEFFPREDUPPERLAYERSIZE = 0, //!< 8-bit uncompressed data
            SURFACE_ID_SVCRESIDUALUPSAMPLINGSTREAMOUTSURFACEUPPERLAYERSIZE   = 1, //!< 16-bit uncompressed data
            SURFACE_ID_SVCRECONSTRUCTEDPIXELANDCOEFFPREDUPSAMPLINGSTREAMINSURFACELOWERLAYERSIZE = 2, //!< 8-bit uncompressed data.
            SURFACE_ID_SVCRESIDUALUPSAMPLINGSTREAMINSURFACELOWERLAYERSIZE    = 3, //!< 16-bit uncompressed data
            SURFACE_ID_SOURCEINPUTPICTUREENCODER                             = 4, //!< 8-bit uncompressed data
            SURFACE_ID_RECONSTRUCTEDSCALEDREFERENCEPICTURE                   = 5, //!< 8-bit data
        };

        //! \brief TILE_WALK
        //! \details
        //!     (This field must be set to 1: TILEWALK_YMAJOR)This field specifies the
        //!     type of memory tiling (XMajor or YMajor) employed to tile this surface.
        //!     See Memory Interface Functions for details on memory tiling and
        //!     restrictions.This field is ignored when the surface is linear.This field
        //!     is ignored by MFX. Internally H/W is always treated this set to 1 for
        //!     all video codec and for JPEG.
        enum TILE_WALK
        {
            TILE_WALK_XMAJOR                                                 = 0, //!< TILEWALK_XMAJOR
            TILE_WALK_YMAJOR                                                 = 1, //!< TILEWALK_YMAJOR
        };

        //! \brief TILED_SURFACE
        //! \details
        //!     (This field must be set to TRUE: Tiled)This field specifies whether the
        //!     surface is tiled.This field is ignored by MFX
        enum TILED_SURFACE
        {
            TILED_SURFACE_FALSE                                              = 0, //!< Linear
            TILED_SURFACE_TRUE                                               = 1, //!< Tiled
        };

        //! \brief INTERLEAVE_CHROMA_
        //! \details
        //!     This field indicates that the chroma fields are interleaved in a single
        //!     plane rather than stored as two separate planes. This field is only used
        //!     for PLANAR surface formats.For AVC/VC1/MPEG VLD and IT modes : set to
        //!     Enable to support interleave U/V only.For JPEG : set to Disable for all
        //!     formats (including 4:2:0) - because JPEG does not support NV12. (This
        //!     field is needed only if JPEG will support NV12; otherwise is ignored.)
        enum INTERLEAVE_CHROMA_
        {
            INTERLEAVE_CHROMA_DISABLE                                        = 0, //!< No additional details
            INTERLEAVE_CHROMA_ENABLE                                         = 1, //!< No additional details
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface.  All of the Y and G channels will
        //!     use table 0 and all of the Cr/Cb/R/B channels will use table 1.Usage:
        //!     For 420 planar YUV surface, use 4; for monochrome surfaces, use 12. For
        //!     monochrome surfaces, hardware ignores control fields for Chroma
        //!     planes.This field must be set to 4 - PLANAR_420_8, or 12 - Y8_UNORMNot
        //!     used for MFX, and is ignored.  But for JPEG decoding, this field should
        //!     be programmed to the same format as JPEG_PIC_STATE.  For video codec, it
        //!     should set to 4 always.
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_YCRCBNORMAL                                       = 0, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUVY                                      = 1, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUV                                       = 2, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPY                                        = 3, //!< No additional details
            SURFACE_FORMAT_PLANAR_4208                                       = 4, //!< (NV12, IMC1,2,3,4, YV12)
            SURFACE_FORMAT_PLANAR_4118                                       = 5, //!< Deinterlace Only
            SURFACE_FORMAT_PLANAR_4228                                       = 6, //!< Deinterlace Only
            SURFACE_FORMAT_STMMDNSTATISTICS                                  = 7, //!< Deinterlace Only
            SURFACE_FORMAT_R10G10B10A2UNORM                                  = 8, //!< Sample_8x8 Only
            SURFACE_FORMAT_R8G8B8A8UNORM                                     = 9, //!< Sample_8x8 Only
            SURFACE_FORMAT_R8B8UNORMCRCB                                     = 10, //!< Sample_8x8 Only
            SURFACE_FORMAT_R8UNORMCRCB                                       = 11, //!< Sample_8x8 Only
            SURFACE_FORMAT_Y8UNORM                                           = 12, //!< Sample_8x8 Only
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_SURFACE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFX_IND_OBJ_BASE_ADDR_STATE
    //! \details
    //!     This state command provides the memory base addresses for all row
    //!     stores, StreamOut buffer and reconstructed picture output buffers
    //!     required by the MFD or MFC Engine (that are in addition to the row
    //!     stores of the Bit Stream Decoding/Encoding Unit (BSD/BSE) and the
    //!     reference picture buffers). This is a picture level state command and is
    //!     common among all codec standards and for both encoder and decoder
    //!     operating modes. However, some fields may only applicable to a specific
    //!     codec standard. All Pixel Surfaces (original, reference frame and
    //!     reconstructed frame) in the Encoder are programmed with the same surface
    //!     state (NV12 and TileY format), except each has its own frame buffer base
    //!     address. In the tile format, there is no need to provide buffer offset
    //!     for each slice; since from each MB address, the hardware can calculated
    //!     the corresponding memory location within the frame buffer directly.
    //!     
    //!     The MFX_IND_OBJ_BASE_ADDR command sets the memory base address pointers
    //!     for the corresponding Indirect Object Data Start Addresses (Offsets)
    //!     specified in each OBJECT commands. The characteristic of these indirect
    //!     object data is their variable size (per MB or per Slice). Hence, each
    //!     OBJECT command must specify the indirect object data offset from the
    //!     base address to start fetching or writing object data.
    //!     
    //!     While the use of base address is unconditional, the indirection can be
    //!     effectively disabled by setting the base address to zero. For decoder,
    //!     there are:  1 read-only per-slice indirect object in the BSD_OBJECT
    //!     Command, and
    //!      2 read-only per-MB indirect objects in the IT_OBJECT Command.
    //!       For decoder: the Video Command Streamer (VCS) will perform the memory
    //!     access bound check automatically  using the corresponding MFC Indirect
    //!     Object Access Upper Bound specification. If any access is at or beyond
    //!     the upper bound, zero value is returned. The request to memory is still
    //!     being sent, but the corresponding  codec's BSD unit will detect this
    //!     condition and perform the zeroing return. If the Upper Bound is turned
    //!     off, the beyond bound request will return whatever on the bus (invalid
    //!     data). For encoder, there are:  1 read-only per-MB indirect object in
    //!     the PAK_OBJECT Command, and
    //!      1 write-only per-slice indirect object in the PAK Slice_State Command
    //!       For encoder: whenever an out of bound address accessing request is
    //!     generated, VMX will detect such requests and snap the address to the
    //!     corresponding [indirect object base address + indirect data start
    //!     address]. VMX will return all 0s as the data to the requestor.
    //!     NotationDefinitionPhysicalAddress[n:m] Corresponding bits of a physical
    //!     graphics memory byte address (not mapped by a GTT) GraphicsAddress[n:m]
    //!     Corresponding bits of an absolute, virtual graphics memory byte address
    //!     (mapped by a GTT).
    //!     
    struct MFX_IND_OBJ_BASE_ADDR_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 SubOpcodea                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUB_OPCODEA
                uint32_t                 CommonOpcode                                     : __CODEGEN_BITFIELD(24, 26)    ; //!< COMMON_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfxIndirectBitstreamObjectBaseAddressDecoderAndStitchModes : __CODEGEN_BITFIELD(12, 31)    ; //!< MFX Indirect Bitstream Object - Base Address (Decoder and Stitch Modes)
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 MfxIndirectBitstreamObjectDestinationAddressDecoderAndStitchModes4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFX Indirect Bitstream Object - Destination Address (Decoder and Stitch Modes)[47:32]
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MfxIndirectBitstreamObjectbaseArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MFX_INDIRECT_BITSTREAM_OBJECTBASE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 MfxIndirectBitstreamObjectTiledResourceMode      : __CODEGEN_BITFIELD(13, 14)    ; //!< MFX_INDIRECT_BITSTREAM_OBJECT__TILED_RESOURCE_MODE
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfxIndirectBitstreamObjectAccessUpperBoundDecoderAndStitchModes : __CODEGEN_BITFIELD(12, 31)    ; //!< MFX Indirect Bitstream Object - Access Upper Bound (Decoder and Stitch Modes)
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 MfxIndirectBitstreamObjectUpperboundDecoderAndStitchModes4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFX Indirect Bitstream Object UpperBound (Decoder and Stitch Modes)[47:32]
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfxIndirectMvObjectBaseAddress                   : __CODEGEN_BITFIELD(12, 31)    ; //!< MFX Indirect MV Object - Base Address
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 MfxIndirectMvObjectBaseAddress4732               : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFX Indirect MV Object Base Address [47:32]
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MfxIndirectMvObjectArbitrationPriorityControl    : __CODEGEN_BITFIELD( 7,  8)    ; //!< MFX_INDIRECT_MV_OBJECT__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved265                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 MfxIndirectMvObjectDestinationTiledResourceMode  : __CODEGEN_BITFIELD(13, 14)    ; //!< MFX_INDIRECT_MV_OBJECT_DESTINATION__TILED_RESOURCE_MODE
                uint32_t                 Reserved271                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Reserved288                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfxIndirectMvObjectAccessUpperBound              : __CODEGEN_BITFIELD(12, 31)    ; //!< MFX Indirect MV Object Access Upper Bound
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 MfxIndirectMvObjectUpperbound4732                : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFX Indirect MV Object UpperBound [47:32]
                uint32_t                 Reserved336                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Reserved352                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfdIndirectItCoeffObjectBaseAddressDecoderOnly   : __CODEGEN_BITFIELD(12, 31)    ; //!< MFD Indirect IT-COEFF Object - Base Address (Decoder Only)
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 MfdIndirectItCoeffObjectBaseAddress4732          : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFD Indirect IT-COEFF Object Base Address [47:32]
                uint32_t                 Reserved400                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MfdIndirectItCoeffObjectDesitnationArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved425                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 MfdIndirectItCoeffTiledResourceMode              : __CODEGEN_BITFIELD(13, 14)    ; //!< MFD_INDIRECT_IT_COEFF__TILED_RESOURCE_MODE
                uint32_t                 Reserved431                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 Reserved448                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfdIndirectItCoeffObjectAccessUpperBoundDecoderOnly : __CODEGEN_BITFIELD(12, 31)    ; //!< MFD Indirect IT-COEFF Object - Access Upper Bound (Decoder Only)
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 MfdIndirectItCoeffObjectUpperbound4732           : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFD Indirect IT-COEFF Object UpperBound [47:32]
                uint32_t                 Reserved496                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved512                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfdIndirectItDblkObjectBaseAddressDecoderOnly    : __CODEGEN_BITFIELD(12, 31)    ; //!< MFD Indirect IT-DBLK Object - Base Address (Decoder Only)
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 MfdIndirectItDblkObjectBaseAddress4732           : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFD Indirect IT-DBLK Object Base Address [47:32]
                uint32_t                 Reserved560                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MfdIndirectItDblkObjectArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MFD_INDIRECT_IT_DBLK_OBJECT__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved585                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 MfdIndirectItDblkObjectDestinationTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION__TILED_RESOURCE_MODE
                uint32_t                 Reserved591                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Reserved608                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfdIndirectItDblkObjectAccessUpperBoundDecoderOnly : __CODEGEN_BITFIELD(12, 31)    ; //!< MFD Indirect IT-DBLK Object - Access Upper Bound (Decoder Only)
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 MfdIndirectItDblkObjectUpperbound4732            : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFD Indirect IT-DBLK Object UpperBound [47:32]
                uint32_t                 Reserved656                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 Reserved672                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfcIndirectPakBseObjectBaseAddressEncoderOnly    : __CODEGEN_BITFIELD(12, 31)    ; //!< MFC Indirect PAK-BSE Object - Base Address (Encoder Only)
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 MfcIndirectPakBseObjectBaseAddress4732           : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFC Indirect PAK-BSE Object Base Address [47:32]
                uint32_t                 Reserved720                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MfcIndirectPakBseObjectDesitnationArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved745                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 MfcIndirectPakBseObjectDestinationTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION__TILED_RESOURCE_MODE
                uint32_t                 Reserved751                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 Reserved768                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 MfcIndirectPakBseObjectAccessUpperBoundEecoderOnly : __CODEGEN_BITFIELD(12, 31)    ; //!< MFC Indirect PAK-BSE Object - Access Upper Bound (Eecoder Only)
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 MfcIndirectPakBseObjectUpperbound4732            : __CODEGEN_BITFIELD( 0, 15)    ; //!< MFC Indirect PAK-BSE Object UpperBound [47:32]
                uint32_t                 Reserved816                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_MFXINDOBJBASEADDRSTATE                                = 3, //!< No additional details
        };

        enum SUB_OPCODEA
        {
            SUB_OPCODEA_MFXINDOBJBASEADDRSTATE                               = 0, //!< No additional details
        };

        enum COMMON_OPCODE
        {
            COMMON_OPCODE_MFXINDOBJBASEADDRSTATE                             = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXINDOBJBASEADDRSTATE                                  = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief MFX_INDIRECT_BITSTREAM_OBJECTBASE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MFX_INDIRECT_BITSTREAM_OBJECTBASE__ARBITRATION_PRIORITY_CONTROL
        {
            MFX_INDIRECT_BITSTREAM_OBJECTBASE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MFX_INDIRECT_BITSTREAM_OBJECTBASE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MFX_INDIRECT_BITSTREAM_OBJECTBASE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MFX_INDIRECT_BITSTREAM_OBJECTBASE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MFX_INDIRECT_BITSTREAM_OBJECT__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MFX_INDIRECT_BITSTREAM_OBJECT__TILED_RESOURCE_MODE
        {
            MFX_INDIRECT_BITSTREAM_OBJECT_TILED_RESOURCE_MODE_TRMODENONE     = 0, //!< No tiled resource
            MFX_INDIRECT_BITSTREAM_OBJECT_TILED_RESOURCE_MODE_TRMODETILEYF   = 1, //!< 4KB tiled resources
            MFX_INDIRECT_BITSTREAM_OBJECT_TILED_RESOURCE_MODE_TRMODETILEYS   = 2, //!< 64KB tiled resources
        };

        //! \brief MFX_INDIRECT_MV_OBJECT__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MFX_INDIRECT_MV_OBJECT__ARBITRATION_PRIORITY_CONTROL
        {
            MFX_INDIRECT_MV_OBJECT_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MFX_INDIRECT_MV_OBJECT_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MFX_INDIRECT_MV_OBJECT_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MFX_INDIRECT_MV_OBJECT_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MFX_INDIRECT_MV_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MFX_INDIRECT_MV_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        {
            MFX_INDIRECT_MV_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            MFX_INDIRECT_MV_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            MFX_INDIRECT_MV_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \brief MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
        {
            MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MFD_INDIRECT_IT_COEFF_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MFD_INDIRECT_IT_COEFF__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MFD_INDIRECT_IT_COEFF__TILED_RESOURCE_MODE
        {
            MFD_INDIRECT_IT_COEFF_TILED_RESOURCE_MODE_TRMODENONE             = 0, //!< No tiled resource
            MFD_INDIRECT_IT_COEFF_TILED_RESOURCE_MODE_TRMODETILEYF           = 1, //!< 4KB tiled resources
            MFD_INDIRECT_IT_COEFF_TILED_RESOURCE_MODE_TRMODETILEYS           = 2, //!< 64KB tiled resources
        };

        //! \brief MFD_INDIRECT_IT_DBLK_OBJECT__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MFD_INDIRECT_IT_DBLK_OBJECT__ARBITRATION_PRIORITY_CONTROL
        {
            MFD_INDIRECT_IT_DBLK_OBJECT_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MFD_INDIRECT_IT_DBLK_OBJECT_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MFD_INDIRECT_IT_DBLK_OBJECT_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MFD_INDIRECT_IT_DBLK_OBJECT_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        {
            MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            MFD_INDIRECT_IT_DBLK_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \brief MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION__ARBITRATION_PRIORITY_CONTROL
        {
            MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MFC_INDIRECT_PAK_BSE_OBJECT_DESITNATION_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION__TILED_RESOURCE_MODE
        {
            MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            MFC_INDIRECT_PAK_BSE_OBJECT_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_IND_OBJ_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 26;
        static const size_t byteSize = 104;
    };

    //!
    //! \brief MFX_BSP_BUF_BASE_ADDR_STATE
    //! \details
    //!     This frame-level state command is used to specify all the buffer base
    //!     addresses needed for the operation of the AVC Bit Stream Processing
    //!     Units (for decoder, it is BSD Unit; for encoder, it is BSE Unit) For
    //!     both encoder and decoder, currently it is assumed that all codec
    //!     standards can share the same BSP_BUF_BASE_STATE. The simplicity of this
    //!     command is the result of moving all the direct MV related processing
    //!     into the ENC Subsystem. Since all implicit weight calculations and
    //!     directMV calculations are done in ENC and all picture buffer management
    //!     are done in the Host, there is no need to provide POC (POC List -
    //!     FieldOrderCntList, CurrPic POC - CurrFieldOrderCnt) information to PAK.
    //!     For decoder, all the direct mode information are sent in a separate
    //!     slice-level command (AVC_DIRECTMODE_STATE command). In addition, in
    //!     Encoder, the row stores for CABAC encoding and MB Parameters
    //!     Construction (MPC) are combined into one single row store. The row
    //!     stores specified in this command do not combine with those specified in
    //!     the MFC_PIPE_BUF_ADDR_STATE command for hardware simplification reason.
    //!     
    struct MFX_BSP_BUF_BASE_ADDR_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 BsdMpcRowStoreScratchBufferBaseAddressReadWrite  : __CODEGEN_BITFIELD( 6, 31)    ; //!< BSD/MPC Row Store Scratch Buffer Base Address - Read/Write
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 BsdMpcRowStoreScratchBufferBaseAddressReadWrite4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< BSD/MPC  Row Store Scratch Buffer Base Address - Read/Write [47:32]
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 BsdMpcRowStoreScratchBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< BSDMPC_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 11)    ; //!< Reserved
                uint32_t                 BsdMpcRowStoreScratchBufferCacheSelect           : __CODEGEN_BITFIELD(12, 12)    ; //!< BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 BsdMpcRowStoreScratchBufferTiledResourceMode     : __CODEGEN_BITFIELD(13, 14)    ; //!< BSDMPC_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 MprRowStoreScratchBufferBaseAddressReadWriteDecoderOnly : __CODEGEN_BITFIELD( 6, 31)    ; //!< MPR Row Store Scratch Buffer Base Address - Read/Write (Decoder Only)
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 MprRowStoreScratchBufferBaseAddressReadWrite4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< MPR Row Store Scratch Buffer Base Address - Read/Write [47:32]
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MprRowStoreScratchBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MPR_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved201                                      : __CODEGEN_BITFIELD( 9, 11)    ; //!< Reserved
                uint32_t                 MprRowStoreScratchBufferCacheSelect              : __CODEGEN_BITFIELD(12, 12)    ; //!< MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 MprRowStoreScratchBufferTiledResourceMode        : __CODEGEN_BITFIELD(13, 14)    ; //!< MPR_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved207                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved224                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 BitplaneReadBufferBaseAddress                    : __CODEGEN_BITFIELD( 6, 31)    ; //!< Bitplane Read Buffer Base Address
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 BitplaneReadBufferBaseAddressReadWrite4732       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bitplane Read Buffer Base Address - Read/Write [47:32]
                uint32_t                 Reserved272                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 BitplaneReadBufferArbitrationPriorityControl     : __CODEGEN_BITFIELD( 7,  8)    ; //!< BITPLANE_READ_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved297                                      : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 BitplaneReadBufferTiledResourceMode              : __CODEGEN_BITFIELD(13, 14)    ; //!< BITPLANE_READ_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved303                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED_4                                            = 4, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFXCOMMONSTATE                              = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_PIPELINE                                                = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief BSDMPC_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum BSDMPC_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if Intra Row Store is going to store inside Media
        //!     Internal Storage or to LLC.
        enum BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0            = 0, //!< Buffer going to LLC
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1            = 1, //!< Buffer going to Internal Media Storage
        };

        //! \brief BSDMPC_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum BSDMPC_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        {
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODENONE   = 0, //!< No tiled resource
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            BSDMPC_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \brief MPR_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MPR_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            MPR_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MPR_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MPR_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MPR_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if Intra Row Store is going to store inside Media
        //!     Internal Storage or to LLC.
        enum MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0               = 0, //!< Buffer going to LLC
            MPR_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1               = 1, //!< Buffer going to Internal Media Storage
        };

        //! \brief MPR_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MPR_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        {
            MPR_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODENONE      = 0, //!< No tiled resource
            MPR_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF    = 1, //!< 4KB tiled resources
            MPR_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS    = 2, //!< 64KB tiled resources
        };

        //! \brief BITPLANE_READ_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum BITPLANE_READ_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            BITPLANE_READ_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            BITPLANE_READ_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            BITPLANE_READ_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            BITPLANE_READ_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief BITPLANE_READ_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum BITPLANE_READ_BUFFER__TILED_RESOURCE_MODE
        {
            BITPLANE_READ_BUFFER_TILED_RESOURCE_MODE_TRMODENONE              = 0, //!< No tiled resource
            BITPLANE_READ_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF            = 1, //!< 4KB tiled resources
            BITPLANE_READ_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS            = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_BSP_BUF_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

    //!
    //! \brief MFD_AVC_PICID_STATE
    //! \details
    //!     This is a frame level state command used for both AVC Long and Short
    //!     Format in VLD mode.PictureID[16] contains the pictureID of each
    //!     reference picture (16 maximum) so hardware can uniquely identify the
    //!     reference picture across frames (this will be used for DMV
    //!     operation).This command will be needed for both short and long format.
    //!     
    struct MFD_AVC_PICID_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 PictureidRemappingDisable                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< PICTUREID_REMAPPING_DISABLE
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        uint32_t                         Pictureidlist1616Bits[8];                                                        //!< PictureIDList[16][16 bits]


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_MEDIA                                                = 5, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_DEC                                                  = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFDAVCDPBSTATE                              = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief PICTUREID_REMAPPING_DISABLE
        //! \details
        //!     If Picture ID Remapping Disable is "1", PictureIDList will not be used.
        enum PICTUREID_REMAPPING_DISABLE
        {
            PICTUREID_REMAPPING_DISABLE_AVCDECODERWILLUSE16BITSPICTUREIDTOHANDLEDMVANDIDENTIFYTHEREFERENCEPICTURE = 0, //!< Desc
            PICTUREID_REMAPPING_DISABLE_AVCDECODERWILLUSE_4BITSFRAMESTOREIDINDEXTOREFFRAMELISTTOHANDLEDMVANDIDENTIFYTHEREFERENCEPICTURETHISCAUSESDMVLOGICTOFUNCTIONTHESAMEINPROJECTIVBANDBEFORE = 1, //!< Desc
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_AVC_PICID_STATE_CMD();

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

    //!
    //! \brief MFX_AVC_IMG_STATE
    //! \details
    //!     This must be the very first command to issue after the surface state,
    //!     the pipe select and base address setting commands. This command supports
    //!     both Long and Short VLD and IT AVC Decoding Interface.
    //!     
    struct MFX_AVC_IMG_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 FrameSize                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Frame Size
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FrameWidth                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Width
                uint32_t                 Reserved72                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 FrameHeight                                      : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame Height
                uint32_t                 Reserved88                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 ImgstructImageStructureImgStructure10            : __CODEGEN_BITFIELD( 8,  9)    ; //!< IMGSTRUCT__IMAGE_STRUCTURE_IMG_STRUCTURE10
                uint32_t                 WeightedBipredIdc                                : __CODEGEN_BITFIELD(10, 11)    ; //!< WEIGHTED_BIPRED_IDC
                uint32_t                 WeightedPredFlag                                 : __CODEGEN_BITFIELD(12, 12)    ; //!< WEIGHTED_PRED_FLAG
                uint32_t                 RhodomainRateControlEnable                       : __CODEGEN_BITFIELD(13, 13)    ; //!< RHODOMAIN_RATE_CONTROL_ENABLE
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 FirstChromaQpOffset                              : __CODEGEN_BITFIELD(16, 20)    ; //!< First Chroma QP Offset
                uint32_t                 Reserved117                                      : __CODEGEN_BITFIELD(21, 23)    ; //!< Reserved
                uint32_t                 SecondChromaQpOffset                             : __CODEGEN_BITFIELD(24, 28)    ; //!< Second Chroma QP Offset
                uint32_t                 Reserved125                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Fieldpicflag                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< FIELDPICFLAG
                uint32_t                 Mbaffflameflag                                   : __CODEGEN_BITFIELD( 1,  1)    ; //!< MBAFFFLAMEFLAG
                uint32_t                 Framembonlyflag                                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< FRAMEMBONLYFLAG
                uint32_t                 Transform8X8Flag                                 : __CODEGEN_BITFIELD( 3,  3)    ; //!< TRANSFORM8X8FLAG
                uint32_t                 Direct8X8Infflag                                 : __CODEGEN_BITFIELD( 4,  4)    ; //!< DIRECT8X8INFFLAG
                uint32_t                 Constrainedipredflag                             : __CODEGEN_BITFIELD( 5,  5)    ; //!< CONSTRAINEDIPREDFLAG
                uint32_t                 Imgdisposableflag                                : __CODEGEN_BITFIELD( 6,  6)    ; //!< IMGDISPOSABLEFLAG
                uint32_t                 Entropycodingflag                                : __CODEGEN_BITFIELD( 7,  7)    ; //!< ENTROPYCODINGFLAG
                uint32_t                 Mbmvformatflag                                   : __CODEGEN_BITFIELD( 8,  8)    ; //!< MBMVFORMATFLAG
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 Chromaformatidc                                  : __CODEGEN_BITFIELD(10, 11)    ; //!< CHROMAFORMATIDC
                uint32_t                 Mvunpackedflag                                   : __CODEGEN_BITFIELD(12, 12)    ; //!< MVUNPACKEDFLAG
                uint32_t                 Reserved141                                      : __CODEGEN_BITFIELD(13, 13)    ; //!< Reserved
                uint32_t                 Loadslicepointerflag                             : __CODEGEN_BITFIELD(14, 14)    ; //!< LOADSLICEPOINTERFLAG
                uint32_t                 Mbstatenabled                                    : __CODEGEN_BITFIELD(15, 15)    ; //!< MBSTATENABLED
                uint32_t                 Minframewsize                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< MINFRAMEWSIZE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IntrambmaxbitflagIntrambmaxsizereportmask        : __CODEGEN_BITFIELD( 0,  0)    ; //!< INTRAMBMAXBITFLAG__INTRAMBMAXSIZEREPORTMASK
                uint32_t                 IntermbmaxbitflagIntermbmaxsizereportmask        : __CODEGEN_BITFIELD( 1,  1)    ; //!< INTERMBMAXBITFLAG__INTERMBMAXSIZEREPORTMASK
                uint32_t                 FrameszoverflagFramebitratemaxreportmask         : __CODEGEN_BITFIELD( 2,  2)    ; //!< FRAMESZOVERFLAG__FRAMEBITRATEMAXREPORTMASK
                uint32_t                 FrameszunderflagFramebitrateminreportmask        : __CODEGEN_BITFIELD( 3,  3)    ; //!< FRAMESZUNDERFLAG__FRAMEBITRATEMINREPORTMASK
                uint32_t                 Reserved164                                      : __CODEGEN_BITFIELD( 4,  6)    ; //!< Reserved
                uint32_t                 IntraIntermbipcmflagForceipcmcontrolmask         : __CODEGEN_BITFIELD( 7,  7)    ; //!< INTRAINTERMBIPCMFLAG__FORCEIPCMCONTROLMASK
                uint32_t                 Reserved168                                      : __CODEGEN_BITFIELD( 8,  8)    ; //!< Reserved
                uint32_t                 MbratectrlflagMbLevelRateControlEnablingFlag     : __CODEGEN_BITFIELD( 9,  9)    ; //!< MBRATECTRLFLAG__MB_LEVEL_RATE_CONTROL_ENABLING_FLAG
                uint32_t                 Minframewsizeunits                               : __CODEGEN_BITFIELD(10, 11)    ; //!< MINFRAMEWSIZEUNITS
                uint32_t                 Reserved172                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Nonfirstpassflag                                 : __CODEGEN_BITFIELD(16, 16)    ; //!< NONFIRSTPASSFLAG
                uint32_t                 Reserved177                                      : __CODEGEN_BITFIELD(17, 26)    ; //!< Reserved
                uint32_t                 TrellisQuantizationChromaDisableTqchromadisable  : __CODEGEN_BITFIELD(27, 27)    ; //!< TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE
                uint32_t                 TrellisQuantizationRoundingTqr                   : __CODEGEN_BITFIELD(28, 30)    ; //!< TRELLIS_QUANTIZATION_ROUNDING_TQR
                uint32_t                 TrellisQuantizationEnabledTqenb                  : __CODEGEN_BITFIELD(31, 31)    ; //!< TRELLIS_QUANTIZATION_ENABLED_TQENB
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Intrambmaxsz                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< IntraMbMaxSz
                uint32_t                 Reserved204                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Intermbmaxsz                                     : __CODEGEN_BITFIELD(16, 27)    ; //!< InterMbMaxSz
                uint32_t                 Reserved220                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 VslTopMbTrans8X8Flag                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< VSL_TOP_MB_TRANS8X8FLAG
                uint32_t                 Reserved225                                      : __CODEGEN_BITFIELD( 1, 15)    ; //!< Reserved
                uint32_t                 BspEncoderEcoEnable                              : __CODEGEN_BITFIELD(16, 16)    ; //!< BSP_ENCODER_ECO_ENABLE
                uint32_t                 Reserved241                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Slicedeltaqppmax0                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< SliceDeltaQpPMax[0]
                uint32_t                 Slicedeltaqpmax1                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< SliceDeltaQpMax[1]
                uint32_t                 Slicedeltaqpmax2                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< SliceDeltaQpMax[2]
                uint32_t                 Slicedeltaqpmax3                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< SliceDeltaQpMax[3] 
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Slicedeltaqpmin0                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< SliceDeltaQpMin[0] 
                uint32_t                 Slicedeltaqpmin1                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< SliceDeltaQpMin[1]
                uint32_t                 Slicedeltaqpmin2                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< SliceDeltaQpMin[2]
                uint32_t                 Slicedeltaqpmin3                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< SliceDeltaQpMin[3] 
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Framebitratemin                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMin
                uint32_t                 Framebitrateminunitmode                          : __CODEGEN_BITFIELD(14, 14)    ; //!< FRAMEBITRATEMINUNITMODE
                uint32_t                 Framebitrateminunit                              : __CODEGEN_BITFIELD(15, 15)    ; //!< FRAMEBITRATEMINUNIT
                uint32_t                 Framebitratemax                                  : __CODEGEN_BITFIELD(16, 29)    ; //!< FrameBitRateMax
                uint32_t                 Framebitratemaxunitmode                          : __CODEGEN_BITFIELD(30, 30)    ; //!< FRAMEBITRATEMAXUNITMODE
                uint32_t                 Framebitratemaxunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMAXUNIT_
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Framebitratemindelta                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< FrameBitRateMinDelta
                uint32_t                 Reserved367                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Framebitratemaxdelta                             : __CODEGEN_BITFIELD(16, 30)    ; //!< FRAMEBITRATEMAXDELTA
                uint32_t                 SliceStatsStreamoutEnable                        : __CODEGEN_BITFIELD(31, 31)    ; //!< Slice Stats Streamout Enable
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 Reserved384                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 InitialQpValue                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< Initial QP Value
                uint32_t                 NumberOfActiveReferencePicturesFromL0            : __CODEGEN_BITFIELD( 8, 13)    ; //!< Number of Active Reference Pictures from L0
                uint32_t                 Reserved430                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 NumberOfActiveReferencePicturesFromL1            : __CODEGEN_BITFIELD(16, 21)    ; //!< Number of Active Reference Pictures from L1
                uint32_t                 Reserved438                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 NumberOfReferenceFrames                          : __CODEGEN_BITFIELD(24, 28)    ; //!< Number of Reference Frames
                uint32_t                 CurrentPictureHasPerformedMmco5                  : __CODEGEN_BITFIELD(29, 29)    ; //!< Current Picture Has Performed MMCO5
                uint32_t                 Reserved446                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 PicOrderPresentFlag                              : __CODEGEN_BITFIELD( 0,  0)    ; //!< Pic_order_present_flag
                uint32_t                 DeltaPicOrderAlwaysZeroFlag                      : __CODEGEN_BITFIELD( 1,  1)    ; //!< Delta_pic_order_always_zero_flag
                uint32_t                 PicOrderCntType                                  : __CODEGEN_BITFIELD( 2,  3)    ; //!< Pic_order_cnt_type
                uint32_t                 Reserved452                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 SliceGroupMapType                                : __CODEGEN_BITFIELD( 8, 10)    ; //!< slice_group_map_type
                uint32_t                 RedundantPicCntPresentFlag                       : __CODEGEN_BITFIELD(11, 11)    ; //!< redundant_pic_cnt_present_flag
                uint32_t                 NumSliceGroupsMinus1                             : __CODEGEN_BITFIELD(12, 14)    ; //!< num_slice_groups_minus1
                uint32_t                 DeblockingFilterControlPresentFlag               : __CODEGEN_BITFIELD(15, 15)    ; //!< deblocking_filter_control_present_flag
                uint32_t                 Log2MaxFrameNumMinus4                            : __CODEGEN_BITFIELD(16, 23)    ; //!< Log2_max_frame_num_minus4
                uint32_t                 Log2MaxPicOrderCntLsbMinus4                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Log2_max_pic_order_cnt_lsb_minus4
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 SliceGroupChangeRate                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Slice Group Change Rate
                uint32_t                 CurrPicFrameNum                                  : __CODEGEN_BITFIELD(16, 31)    ; //!< Curr Pic Frame Num
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 CurrentFrameViewId                               : __CODEGEN_BITFIELD( 0,  9)    ; //!< Current Frame View ID
                uint32_t                 Reserved522                                      : __CODEGEN_BITFIELD(10, 11)    ; //!< Reserved
                uint32_t                 MaxViewIdxl0                                     : __CODEGEN_BITFIELD(12, 15)    ; //!< Max View IDXL0
                uint32_t                 Reserved528                                      : __CODEGEN_BITFIELD(16, 17)    ; //!< Reserved
                uint32_t                 MaxViewIdxl1                                     : __CODEGEN_BITFIELD(18, 21)    ; //!< Max View IDXL1
                uint32_t                 Reserved534                                      : __CODEGEN_BITFIELD(22, 30)    ; //!< Reserved
                uint32_t                 InterViewOrderDisable                            : __CODEGEN_BITFIELD(31, 31)    ; //!< INTER_VIEW_ORDER_DISABLE
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 FractionalQpInput                                : __CODEGEN_BITFIELD( 0,  2)    ; //!< Fractional QP input
                uint32_t                 FractionalQpOffset                               : __CODEGEN_BITFIELD( 3,  5)    ; //!< Fractional QP offset
                uint32_t                 Reserved550                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 ExtendedRhodomainStatisticsEnable                : __CODEGEN_BITFIELD( 8,  8)    ; //!< Extended RhoDomain Statistics Enable
                uint32_t                 Reserved553                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 RhodomainAveragemacroblockqp                     : __CODEGEN_BITFIELD(16, 21)    ; //!< RhoDomain AverageMacroblockQP
                uint32_t                 Reserved566                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 Reserved576                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 ThresholdSizeInBytes                                                             ; //!< Threshold Size in Bytes
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 TargetSliceSizeInBytes                                                           ; //!< Target Slice Size in Bytes
            };
            uint32_t                     Value;
        } DW20;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED0                                             = 0, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCCOMMON                                   = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXAVCIMGSTATE                                          = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief IMGSTRUCT__IMAGE_STRUCTURE_IMG_STRUCTURE10
        //! \details
        //!     The current encoding picture structure can only takes on 3 possible
        //!     values
        enum IMGSTRUCT__IMAGE_STRUCTURE_IMG_STRUCTURE10
        {
            IMGSTRUCT_IMAGE_STRUCTURE_IMG_STRUCTURE10_FRAMEPICTURE           = 0, //!< No additional details
            IMGSTRUCT_IMAGE_STRUCTURE_IMG_STRUCTURE10_TOPFIELDPICTURE        = 1, //!< No additional details
            IMGSTRUCT_IMAGE_STRUCTURE_IMG_STRUCTURE10_INVALID_NOTALLOWED     = 2, //!< No additional details
            IMGSTRUCT_IMAGE_STRUCTURE_IMG_STRUCTURE10_BOTTOMFIELDPICTURE     = 3, //!< No additional details
        };

        //! \brief WEIGHTED_BIPRED_IDC
        //! \details
        //!     (This field is defined differently from DevSNB; DevIVB follows strictly
        //!     AVC interface.)
        enum WEIGHTED_BIPRED_IDC
        {
            WEIGHTED_BIPRED_IDC_DEFAULT                                      = 0, //!< Specifies that the default weighted prediction is used for B slices
            WEIGHTED_BIPRED_IDC_EXPLICIT                                     = 1, //!< Specifies that explicit weighted prediction is used for B slices
            WEIGHTED_BIPRED_IDC_IMPLICIT                                     = 2, //!< Specifies that implicit weighted prediction is used for B slices.
        };

        //! \brief WEIGHTED_PRED_FLAG
        //! \details
        //!     (This field is defined differently from Gen6, Gen7 follows strictly
        //!     AVC interface.)
        enum WEIGHTED_PRED_FLAG
        {
            WEIGHTED_PRED_FLAG_DISABLE                                       = 0, //!< specifies that weighted prediction is not used for P and SP slices
            WEIGHTED_PRED_FLAG_ENABLE                                        = 1, //!< specifies that weighted prediction is used for P and SP slices
        };

        //! \brief RHODOMAIN_RATE_CONTROL_ENABLE
        //! \details
        //!     This field indicates if RhoDomain related parameters are present in the
        //!     MFX_AVC_IMAGE_STATE. (AverageMacroblockQP). It enables the Rho Domain
        //!     statistics collection.
        enum RHODOMAIN_RATE_CONTROL_ENABLE
        {
            RHODOMAIN_RATE_CONTROL_ENABLE_DISABLE                            = 0, //!< RhoDomain rate control parameters are not present in MFX_AVC_IMAGE_STATE
            RHODOMAIN_RATE_CONTROL_ENABLE_ENABLE                             = 1, //!< RhoDomain rate control parameters are present in MFX_AVC_IMAGE_STATE.
        };

        //! \brief FIELDPICFLAG
        //! \details
        //!     Field picture flag, field_pic_flag, specifies the current slice is a
        //!     coded field or not.It is set to the same value as the syntax element in
        //!     the Slice Header. It must be consistent with the img_structure[1:0] and
        //!     the frame_mbs_only_flag settings.Although field_pic_flag is a Slice
        //!     Header parameter, its value is expected to be the same for all the
        //!     slices of a picture.
        enum FIELDPICFLAG
        {
            FIELDPICFLAG_FRAME                                               = 0, //!< a slice of a coded frame
            FIELDPICFLAG_FIELD                                               = 1, //!< a slice of a coded field
        };

        //! \brief MBAFFFLAMEFLAG
        //! \details
        //!     MBAFF mode is active, mbaff_frame_flag.It is derived from MbaffFrameFlag
        //!     = (mb_adaptive_frame_field_flag &amp;&amp; ! field_pic_flag ).
        //!     mb_adaptive_frame_field_flag is a syntax element in the current active
        //!     SPS and field_pic_flag is a syntax element in the current Slice Header.
        //!     They both are present only if frame_mbs_only_flag is 0. Although
        //!     mbaff_frame_flag is a Slice Header parameter, its value is expected to
        //!     be the same for all the slices of a picture.It must be consistent with
        //!     the mb_adaptive_frame_field_flag, the field_pic_flag and the
        //!     frame_mbs_only_flag settings.This bit is valid only when the
        //!     img_structure[1:0] indicates the current picture is a frame.
        enum MBAFFFLAMEFLAG
        {
            MBAFFFLAMEFLAG_FALSE                                             = 0, //!< not in MBAFF mode
            MBAFFFLAMEFLAG_TRUE                                              = 1, //!< in MBAFF mode
        };

        //! \brief FRAMEMBONLYFLAG
        //! \details
        //!     Frame MB only flag, frame_mbs_only_flagIt is set to the value of the
        //!     syntax element in the current active SPS.
        enum FRAMEMBONLYFLAG
        {
            FRAMEMBONLYFLAG_FALSE                                            = 0, //!< not true ; effectively enables the possibility of MBAFF mode.
            FRAMEMBONLYFLAG_TRUE                                             = 1, //!< true, only frame MBs can occur in this sequence, hence disallows the MBAFF mode and field picture.
        };

        //! \brief TRANSFORM8X8FLAG
        //! \details
        //!     8x8 IDCT Transform Mode Flag, trans8x8_mode_flagSpecifies 8x8 IDCT
        //!     transform may be used in this pictureIt is set to the value of the
        //!     syntax element in the current active PPS.
        enum TRANSFORM8X8FLAG
        {
            TRANSFORM8X8FLAG_4X_4                                            = 0, //!< no 8x8 IDCT Transform, only 4x4 IDCT transform blocks are present
            TRANSFORM8X8FLAG_8X8                                             = 1, //!< 8x8 Transform is allowed
        };

        //! \brief DIRECT8X8INFFLAG
        //! \details
        //!     Direct 8x8 Inference Flag, direct_8x8_inference_flagIt is set to the
        //!     value of the syntax element in the current active SPS.It specifies the
        //!     derivation process for luma motion vectors in the Direct MV coding modes
        //!     (B_Skip, B_Direct_16x16 and B_Direct_8x8). When frame_mbs_only_flag is
        //!     equal to 0, direct_8x8_inference_flag shall be equal to 1.It must be
        //!     consistent with the frame_mbs_only_flag and transform_8x8_mode_flag
        //!     settings.
        enum DIRECT8X8INFFLAG
        {
            DIRECT8X8INFFLAG_SUBBLOCK                                        = 0, //!< allows subpartitioning to go below 8x8 block size (i.e. 4x4, 8x4 or 4x8)
            DIRECT8X8INFFLAG_BLOCK                                           = 1, //!< allows processing only at 8x8 block size.  MB Info is stored for 8x8 block size.
        };

        //! \brief CONSTRAINEDIPREDFLAG
        //! \details
        //!     Constrained Intra Prediction Flag, constrained_ipred_flagIt is set to
        //!     the value of the syntax element in the current active PPS.
        enum CONSTRAINEDIPREDFLAG
        {
            CONSTRAINEDIPREDFLAG_INTRAANDINTER                               = 0, //!< allows both intra and inter neighboring MB to be used in the intra-prediction encoding of the current MB.
            CONSTRAINEDIPREDFLAG_INTRAONLY                                   = 1, //!< allows only to use neighboring Intra MBs in the intra-prediction encoding of the current MB.  If the neighbor is an inter MB, it is considered as not available.
        };

        //! \brief IMGDISPOSABLEFLAG
        //! \details
        //!     Current Img Disposable Flag or Non-Reference Picture Flag
        enum IMGDISPOSABLEFLAG
        {
            IMGDISPOSABLEFLAG_REFERENCE                                      = 0, //!< the current decoding picture may be used as a reference picture for others
            IMGDISPOSABLEFLAG_DISPOSABLE                                     = 1, //!< the current decoding picture is not used as a reference picture (e.g. a B-picture cannot be a reference picture for any subsequent decoding)
        };

        //! \brief ENTROPYCODINGFLAG
        //! \details
        //!     Entropy Coding Flag, entropy_coding_flag
        enum ENTROPYCODINGFLAG
        {
            ENTROPYCODINGFLAG_CAVLCBIT_SERIALENCODINGMODE                    = 0, //!< Desc
            ENTROPYCODINGFLAG_CABACBIT_SERIALENCODINGMODE                    = 1, //!< Desc
        };

        //! \brief MBMVFORMATFLAG
        //! \details
        //!     Use MB level MvFormat flag (Encoder Only)(This bit must be set to zero
        //!     in IVB:GT2:A0)
        enum MBMVFORMATFLAG
        {
            MBMVFORMATFLAG_IGNORE                                            = 0, //!< HW PAK ignore MvFormat in the MB data.  When bit 12 == 0, all MBs use packed MV formatWhen bit 12 == 1, each MB data must use unpacked MV format, 8MV when there is no minor MV involved, and 32MV if there are some minor MVs.
            MBMVFORMATFLAG_FOLLOW                                            = 1, //!< HW PAK will follow MvFormat value set within each MB data.
        };

        //! \brief CHROMAFORMATIDC
        //! \details
        //!     Chroma Format IDC, ChromaFormatIdc[1:0]It specifies the sampling of
        //!     chroma component (Cb, Cr) in the current picture as follows :
        enum CHROMAFORMATIDC
        {
            CHROMAFORMATIDC_MONOCHROMEPICTURE                                = 0, //!< Desc
            CHROMAFORMATIDC_420PICTURE                                       = 1, //!< Desc
            CHROMAFORMATIDC_422PICTURENOTSUPPORTED                           = 2, //!< No additional details
            CHROMAFORMATIDC_4_4_4PICTURENOTSUPPORTED                         = 3, //!< No additional details
        };

        //! \brief MVUNPACKEDFLAG
        //! \details
        //!     MVUnPackedEnable (Encoder Only)This field is reserved in Decode mode.
        enum MVUNPACKEDFLAG
        {
            MVUNPACKEDFLAG_PACKED                                            = 0, //!< use packed MV format (compliant to )
            MVUNPACKEDFLAG_UNPACKED                                          = 1, //!< use unpacked 8MV/32MV format only
        };

        //! \brief LOADSLICEPOINTERFLAG
        //! \details
        //!     LoadBitStreamPointerPerSlice (Encoder-only)To support multiple slice
        //!     picture and additional header/data insertion before and after an encoded
        //!     slice.When this field is set to 0, bitstream pointer is only loaded once
        //!     for the first slice of a frame. For subsequent slices in the frame,
        //!     bitstream data are stitched together to form a single output data
        //!     stream.When this field is set to 1, bitstream pointer is loaded for each
        //!     slice of a frame. Basically bitstream data for different slices of a
        //!     frame will be written to different memory locations.
        enum LOADSLICEPOINTERFLAG
        {
            LOADSLICEPOINTERFLAG_DISABLE                                     = 0, //!< Load BitStream Pointer only once for the first slice of a frame
            LOADSLICEPOINTERFLAG_ENABLE                                      = 1, //!< Load/reload BitStream Pointer only once for the each slice, reload the start location of the bitstream buffer from the Indirect PAK-BSE Object Data Start Address field
        };

        //! \brief MBSTATENABLED
        //! \details
        //!     <p>Enable reading in MB status buffer (a.k.a. encoding stream-out
        //!     buffer) Note: For multi-pass encoder, all passes except the first one
        //!     need to set this value to 1. By setting the first pass to 0, it does
        //!     save some memory bandwidth.</p>
        //!     <p><span style="color: rgb(0, 0, 0); font-family: Arial, sans-serif;
        //!     line-height: normal;">In VDenc mode this must be set to zero as no MB
        //!     level rate control is used. </span></p>
        enum MBSTATENABLED
        {
            MBSTATENABLED_DISABLE                                            = 0, //!< Disable Reading of Macroblock Status Buffer
            MBSTATENABLED_ENABLE                                             = 1, //!< Enable Reading of Macroblock Status Buffer
        };

        //! \brief MINFRAMEWSIZE
        //! \details
        //!     <p><b>Minimum Frame Size [15:0] (in Word, 16-bit)(Encoder Only)</b>
        //!                         Mininum Frame Size is specified to compensate for intel Rate
        //!                         Control Currently zero fill (no need to perform emulation byte
        //!     insertion) is done
        //!                         only to the end of the CABAC_ZERO_WORD insertion (if any) at the
        //!     last slice of a
        //!                         picture. Intel encoder parameter, not part of . The caller
        //!     should always make
        //!                         sure that the value, represented by  Mininum Frame Size, is always
        //!     less than maximum
        //!                         frame size <b>FrameBitRateMax (DWORD 10 bits</b> 29:16).This field
        //!     is reserved in
        //!                         Decode mode.</p>                    
        //!                         <p>The programmable range 0…2^18-1</p>
        //!                         <p>When MinFrameWSizeUnits is 00.</p>
        //!                         <p>Programmable range is 0…2^20-1 when MinFrameWSizeUnits is
        //!     01.</p>
        //!                         <p>Programmable range is 0…2^26-1 when MinFrameWSizeUnits is
        //!     10.</p>
        //!                         <p>Programmable range is 0…2^32-1 when MinFrameWSizeUnits is
        //!     11.</p>
        enum MINFRAMEWSIZE
        {
            MINFRAMEWSIZE_UNNAMED0                                           = 0, //!< No additional details
        };

        //! \brief INTRAMBMAXBITFLAG__INTRAMBMAXSIZEREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of any intra MB in the
        //!     frame exceeds IntraMBMaxSize.
        enum INTRAMBMAXBITFLAG__INTRAMBMAXSIZEREPORTMASK
        {
            INTRAMBMAXBITFLAG_INTRAMBMAXSIZEREPORTMASK_DISABLE               = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            INTRAMBMAXBITFLAG_INTRAMBMAXSIZEREPORTMASK_ENABLE                = 1, //!< set bit0 of MFC_IMAGE_STATUS control register if the total bit counter for the current MB is greater than the Intra MB Conformance Max size limit.
        };

        //! \brief INTERMBMAXBITFLAG__INTERMBMAXSIZEREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of any inter MB in the
        //!     frame exceeds InterMBMaxSize.
        enum INTERMBMAXBITFLAG__INTERMBMAXSIZEREPORTMASK
        {
            INTERMBMAXBITFLAG_INTERMBMAXSIZEREPORTMASK_DISABLE               = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            INTERMBMAXBITFLAG_INTERMBMAXSIZEREPORTMASK_ENABLE                = 1, //!< Set bit0 of MFC_IMAGE_STATUS control register if the total bit counter for the current MB is greater than the Inter MB Conformance Max size limit.
        };

        //! \brief FRAMESZOVERFLAG__FRAMEBITRATEMAXREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     exceeds FrameBitRateMax.
        enum FRAMESZOVERFLAG__FRAMEBITRATEMAXREPORTMASK
        {
            FRAMESZOVERFLAG_FRAMEBITRATEMAXREPORTMASK_DISABLE                = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            FRAMESZOVERFLAG_FRAMEBITRATEMAXREPORTMASK_ENABLE                 = 1, //!< Set bit0 and bit 1 of MFC_IMAGE_STATUS control register if the total frame level bit counter is greater than or equal to Frame Bit rate Maximum limit.
        };

        //! \brief FRAMESZUNDERFLAG__FRAMEBITRATEMINREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     is less than FrameBitRateMin
        enum FRAMESZUNDERFLAG__FRAMEBITRATEMINREPORTMASK
        {
            FRAMESZUNDERFLAG_FRAMEBITRATEMINREPORTMASK_DISABLE               = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            FRAMESZUNDERFLAG_FRAMEBITRATEMINREPORTMASK_ENABLE                = 1, //!< set bit0 and bit 1of MFC_IMAGE_STATUS control register if the total frame level bit counter is less than or equal to Frame Bit rate Minimum limit.
        };

        //! \brief INTRAINTERMBIPCMFLAG__FORCEIPCMCONTROLMASK
        //! \details
        //!     This field is to Force <b>IPCM</b> for Intra or Inter Macroblock size
        //!     conformance mask.
        enum INTRAINTERMBIPCMFLAG__FORCEIPCMCONTROLMASK
        {
            INTRAINTERMBIPCMFLAG_FORCEIPCMCONTROLMASK_DISABLE                = 0, //!< Do not change intra or Inter macroblocks even
            INTRAINTERMBIPCMFLAG_FORCEIPCMCONTROLMASK_ENABLE                 = 1, //!< Change intra or Inter macroblocks MB_type to IPCM
        };

        //! \brief MBRATECTRLFLAG__MB_LEVEL_RATE_CONTROL_ENABLING_FLAG
        //! \details
        //!     <p>MB Rate Control conformance mask</p>
        //!     <p>In VDenc mode, this field must be zero as frame level rate control is
        //!     used.</p>
        enum MBRATECTRLFLAG__MB_LEVEL_RATE_CONTROL_ENABLING_FLAG
        {
            MBRATECTRLFLAG_MB_LEVEL_RATE_CONTROL_ENABLING_FLAG_DISABLE       = 0, //!< Apply accumulative delta QP for consecutive passes on top of the macroblock QP values in inline data
            MBRATECTRLFLAG_MB_LEVEL_RATE_CONTROL_ENABLING_FLAG_ENABLE        = 1, //!< Apply RC QP delta to suggested QP values in Macroblock Status Buffer except the first pass.
        };

        //! \brief MINFRAMEWSIZEUNITS
        //! \details
        //!     This field is the Minimum Frame Size Units
        enum MINFRAMEWSIZEUNITS
        {
            MINFRAMEWSIZEUNITS_COMPATIBILITYMODE                             = 0, //!< Minimum Frame Size is in old mode (words, 2bytes)
            MINFRAMEWSIZEUNITS_16BYTE                                        = 1, //!< Minimum Frame Size is in 16bytes
            MINFRAMEWSIZEUNITS_4KB                                           = 2, //!< Minimum Frame Size is in 4Kbytes
            MINFRAMEWSIZEUNITS_16KB                                          = 3, //!< Minimum Frame Size is in 16Kbytes
        };

        //! \brief NONFIRSTPASSFLAG
        //! \details
        //!     This signals the current pass is not the first pass. It will imply
        //!     designate HW behavior: e.g
        enum NONFIRSTPASSFLAG
        {
            NONFIRSTPASSFLAG_DISABLE                                         = 0, //!< Always use the MbQpY from initial PAK inline object for all passes of PAK
            NONFIRSTPASSFLAG_ENABLE                                          = 1, //!< Use MbQpY from stream-out buffer if MbRateCtrlFlag is set to 1
        };

        //! \brief TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE
        //! \details
        //!     This signal is used to disable chroma TQ. To enable TQ for both luma and
        //!     chroma, TQEnb=1, TQChromaDisable=0. To enable TQ only for luma, TQEnb=1,
        //!     TQChromaDisable=1.
        enum TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE
        {
            TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE_UNNAMED0     = 0, //!< Enable Trellis Quantization chroma
            TRELLIS_QUANTIZATION_CHROMA_DISABLE_TQCHROMADISABLE_DEFAULT      = 1, //!< Disable Trellis Quantization chroma
        };

        //! \brief TRELLIS_QUANTIZATION_ROUNDING_TQR
        //! \details
        //!     This rounding scheme is only applied to the quantized coefficients
        //!     ranging from 0 to 1 when TQEnb is set to 1 in AVC CABAC mode. One of the
        //!     following values is added to quantized coefficients before truncating
        //!     fractional part.
        enum TRELLIS_QUANTIZATION_ROUNDING_TQR
        {
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED0                       = 0, //!< Add 1/8
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED1                       = 1, //!< Add 2/8
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED2                       = 2, //!< Add 3/8
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED3                       = 3, //!< Add 4/8 (rounding 0.5)
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED_4                      = 4, //!< Add 5/8
            TRELLIS_QUANTIZATION_ROUNDING_TQR_UNNAMED5                       = 5, //!< Add 6/8
            TRELLIS_QUANTIZATION_ROUNDING_TQR_DEFAULT                        = 6, //!< Add 7/8 (Default rounding 0.875)
        };

        //! \brief TRELLIS_QUANTIZATION_ENABLED_TQENB
        //! \details
        //!     The TQ improves output video quality of AVC CABAC encoder by selecting
        //!     quantized values for each non-zero coefficient so as to minimize the
        //!     total R-D cost.This flag is only valid AVC CABAC mode. Otherwise, this
        //!     flag should be disabled.
        enum TRELLIS_QUANTIZATION_ENABLED_TQENB
        {
            TRELLIS_QUANTIZATION_ENABLED_TQENB_DISABLE                       = 0, //!< Use Normal
            TRELLIS_QUANTIZATION_ENABLED_TQENB_ENABLE                        = 1, //!< Use Trellis quantization
        };

        enum VSL_TOP_MB_TRANS8X8FLAG
        {
            VSL_TOP_MB_TRANS8X8FLAG_DISABLE                                  = 0, //!< VSL  will only fetch the current MB data.
            VSL_TOP_MB_TRANS8X8FLAG_ENABLE                                   = 1, //!< When this bit is set VSL will make extra fetch to memory to fetch the MB data for top MB.
        };

        //! \brief BSP_ENCODER_ECO_ENABLE
        //! \details
        //!     <p>Enable AVC Encoder BSP Bit Outstanding ECO</p>
        //!     <p>This bit must be same as bit9 of Dword2 of MFX_PIPE_MODE_SELECT</p>
        enum BSP_ENCODER_ECO_ENABLE
        {
            BSP_ENCODER_ECO_ENABLE_DISABLE                                   = 0, //!< No additional details
            BSP_ENCODER_ECO_ENABLE_ENABLE                                    = 1, //!< No additional details
        };

        //! \brief FRAMEBITRATEMINUNITMODE
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit Units.
        enum FRAMEBITRATEMINUNITMODE
        {
            FRAMEBITRATEMINUNITMODE_COMPATIBILITYMODE                        = 0, //!< FrameBitRateMaxUnit is in old mode (128b/16Kb)
            FRAMEBITRATEMINUNITMODE_NEWMODE                                  = 1, //!< FrameBitRateMaxUnit is in new mode (32byte/4Kb)
        };

        //! \brief FRAMEBITRATEMINUNIT
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit Units.
        enum FRAMEBITRATEMINUNIT
        {
            FRAMEBITRATEMINUNIT_BYTE                                         = 0, //!< FrameBitRateMax is in units of 32 Bytes when FrameBitrateMinUnitMode is 1 and in units of 128 Bytes if FrameBitrateMinUnitMode is 0
            FRAMEBITRATEMINUNIT_KILOBYTE                                     = 1, //!< FrameBitRateMax is in units of 4KBytes Bytes when FrameBitrateMaxUnitMode is 1 and in units of 16KBytes if FrameBitrateMaxUnitMode is 0
        };

        //! \brief FRAMEBITRATEMAXUNITMODE
        //! \details
        //!     This field is the Frame Bitrate Maximum Limit Units.
        enum FRAMEBITRATEMAXUNITMODE
        {
            FRAMEBITRATEMAXUNITMODE_COMPATIBILITYMODE                        = 0, //!< FrameBitRateMaxUnit is in old mode (128b/16Kb)
            FRAMEBITRATEMAXUNITMODE_NEWMODE                                  = 1, //!< FrameBitRateMaxUnit is in new mode (32byte/4Kb)
        };

        //! \brief FRAMEBITRATEMAXUNIT_
        //! \details
        //!     This field is the Frame Bitrate Maximum Limit Units.
        enum FRAMEBITRATEMAXUNIT_
        {
            FRAMEBITRATEMAXUNIT_BYTE                                         = 0, //!< FrameBitRateMax is in units of 32 Bytes when FrameBitrateMaxUnitMode is 1 and in units of 128 Bytes if FrameBitrateMaxUnitMode is 0
            FRAMEBITRATEMAXUNIT_KILOBYTE                                     = 1, //!< FrameBitRateMax is in units of 4KBytes Bytes when FrameBitrateMaxUnitMode is 1 and in units of 16KBytes if FrameBitrateMaxUnitMode is 0
        };

        //! \brief FRAMEBITRATEMAXDELTA
        //! \details
        //!     This field is used to select the slice delta QP when FrameBitRateMax Is
        //!     exceeded. It shares the same FrameBitrateMaxUnit. When
        //!     FrameBitrateMaxUnitMode is 0(compatibility mode) bits 16:27 should be
        //!     used, bits 28, 29 and 30 should be 0.
        enum FRAMEBITRATEMAXDELTA
        {
            FRAMEBITRATEMAXDELTA_UNNAMED0                                    = 0, //!< No additional details
        };

        enum VAD_ERROR_LOGIC
        {
            VAD_ERROR_LOGIC_ENABLE                                           = 0, //!< Error reporting ON in case of premature Slice done
            VAD_ERROR_LOGIC_DISABLE                                          = 1, //!< CABAC Engine will auto decode the bitstream in case of premature slice done.
        };

        //! \brief INTER_VIEW_ORDER_DISABLE
        //! \details
        //!     It indicates how to append inter-view picture into initial sorted
        //!     reference list. (due to ambiguity in the MVC Spec)
        enum INTER_VIEW_ORDER_DISABLE
        {
            INTER_VIEW_ORDER_DISABLE_DEFAULT                                 = 0, //!< View Order Ascending
            INTER_VIEW_ORDER_DISABLE_DISABLE                                 = 1, //!< View ID Ascending
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_AVC_IMG_STATE_CMD();

        static const size_t dwSize = 21;
        static const size_t byteSize = 84;
    };

    //!
    //! \brief MFX_AVC_REF_IDX_STATE
    //! \details
    //!     This is a slice level command and can be issued multiple times within a
    //!     picture that is comprised of multiple slices.  The same command is used
    //!     for AVC encoder (PAK mode) and decoder (VLD mode); it is not need in
    //!     decoder IT mode.   The inline data of this command is interpreted
    //!     differently for encoder as for decoder.  For decoder, it is interpreted
    //!     as RefIdx List L0/L1 as in AVC spec., and it matches with the AVC
    //!     API data structure for decoder in VLD mode : RefPicList[2][32] (L0:L1,
    //!     0:31 RefPic).  But for encoder, it is interpreted as a Reference Index
    //!     Mapping Table for L0 and L1 reference pictures.  For packing the bits at
    //!     the output of PAK, the syntax elements must follow the definition of
    //!     RefIdxL0/L1 list according to the AVC spec.  However, the decoder
    //!     pipeline was designed to use a variation of that standard definition, as
    //!     such a conversion (mapping) is needed to support the hardware design.
    //!     The Reference lists are needed in processing both P and B slice in AVC
    //!     codec. For P-MB, only L0 list is used; for B-MB both L0 and L1 lists are
    //!     needed.  For a B-MB that is coded in L1-only Prediction, only L1 list is
    //!     used.
    //!     
    //!     specifies that an application will create the RefPicList L0 and L1
    //!     and pass onto the driver.  The content of each entry of RefPicList
    //!     L0/L1[ ] is a 7-bit picture index.  This picture index is the same as
    //!     that of RefFrameList[ ] content.  This picture index, however, is not
    //!     defined the same as the frame store ID (0 to 16, 5-bits) we have
    //!     implemented in H/W.  Hence, driver is required to manage a table to
    //!     convert between picture index and intel frame store ID.   As such,
    //!     the final RefPicList L0/L1[ ] that the driver passes onto the H/W is not
    //!     the same as that defined.
    //!     
    struct MFX_AVC_REF_IDX_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODEA
                uint32_t                 CommandOpcode                                    : __CODEGEN_BITFIELD(24, 26)    ; //!< COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 RefpiclistSelect                                 : __CODEGEN_BITFIELD( 0,  0)    ; //!< REFPICLIST_SELECT
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        uint32_t                         ReferenceListEntry[8];                                                           //!< Reference List Entry


        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_MFXAVCREFIDXSTATE                                     = 4, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_MFXAVCREFIDXSTATE                                     = 0, //!< No additional details
        };

        enum COMMAND_OPCODE
        {
            COMMAND_OPCODE_AVC                                               = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXAVCREFIDXSTATE                                       = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief REFPICLIST_SELECT
        //! \details
        //!     <p>Num_ref_idx_l1_active is resulted from the specifications in both PPS
        //!     and Slice Header for the current slice.  However, since the full
        //!     reference list L0 and/or L1 are always sent, only present flags are
        //!     specified instead.</p>
        //!                         <p>This parameter is specified for Intel interface only, not
        //!     present in the .</p>
        enum REFPICLIST_SELECT
        {
            REFPICLIST_SELECT_REFPICLIST0                                    = 0, //!< The list that followed represents RefList L0 (Decoder VLD mode) or Ref Idx Mapping Table L0 (Encoder PAK mode)
            REFPICLIST_SELECT_REFPICLIST1                                    = 1, //!< The list that followed represents RefList L1 (Decoder VLD mode) or Ref Idx Mapping Table L1 (Encoder PAK mode)
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_AVC_REF_IDX_STATE_CMD();

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

    //!
    //! \brief MFX_AVC_WEIGHTOFFSET_STATE
    //! \details
    //!     This is a slice level command and can be issued multiple times within a
    //!     picture that is comprised of multiple slices. The same command is used
    //!     for AVC encoder (PAK mode) and decoder (VLD and IT modes). However,
    //!     since for AVC decoder VLD and IT modes, and AVC encoder mode, the
    //!     implicit weights are computed in hardware, this command is not issued.
    //!     For encoder, regardless of the type of weight calculation is active for
    //!     the current slice (default, implicit or explicit), they are all sent to
    //!     the PAK as if they were all in explicit mode. However, for implicit
    //!     weight and offset, each entry contains only a 16-bit weight and no
    //!     offset (offset = 0 always in implicit mode and can be hard-coded inside
    //!     the hardware).The weights (and offsets) are needed in processing both P
    //!     and B slice in AVC codec. For P-MB, at most only L0 list is used; for
    //!     B-MB both L0 and L1 lists may be needed. For a B-MB that is coded in
    //!     L1-only Prediction, only L1 list is sent.The content of this command
    //!     matches with the AVC API data structure for explicit prediction
    //!     mode only : Weights[2][32][3][2] (L0:L1, 0:31 RefPic, Y:Cb:Cr, W:0)
    //!     
    struct MFX_AVC_WEIGHTOFFSET_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 WeightAndOffsetSelect                            : __CODEGEN_BITFIELD( 0,  0)    ; //!< WEIGHT_AND_OFFSET_SELECT
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        uint32_t                         Weightoffset[96];                                                                //!< WeightOffset


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED5                                             = 5, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCCOMMON                                   = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXAVCWEIGHTOFFSETSTATE                                 = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief WEIGHT_AND_OFFSET_SELECT
        //! \details
        //!     It must be set in consistent with the WeightedPredFlag and
        //!     WeightedBiPredIdc in the Img_State command.
        //!                         This parameter is specified for Intel interface only, not present
        //!     in the .
        //!                         For implicit even though only one entry may be used, still loading
        //!     the whole 32-entry table.
        enum WEIGHT_AND_OFFSET_SELECT
        {
            WEIGHT_AND_OFFSET_SELECT_WEIGHTANDOFFSETL0TABLE                  = 0, //!< The list that followed is associated with the weight and offset for RefPicList L0
            WEIGHT_AND_OFFSET_SELECT_WEIGHTANDOFFSETL1TABLE                  = 1, //!< The list that followed is associated with the weight and offset for RefPicList L1
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_AVC_WEIGHTOFFSET_STATE_CMD();

        static const size_t dwSize = 98;
        static const size_t byteSize = 392;
    };

    //!
    //! \brief MFX_AVC_SLICE_STATE
    //! \details
    //!     This is a slice level command and can be issued multiple times within a
    //!     picture that is comprised of multiple slices.  The same command is used
    //!     for AVC encoder (PAK mode) and decoder (VLD and IT modes).
    //!     
    //!     In VDEnc mode, this command is programmed for every super-slice. However
    //!     not all parameters are allowed to change across super-slices.
    //!     
    //!     MFX_AVC_SLICE_STATE command is not issued for AVC Short Format
    //!     Bitstream decode, instead MFD_AVC_SLICEADDR command is executed to
    //!     retrieve the next slice MB Start Address X and Y by H/W itself.
    //!     
    struct MFX_AVC_SLICE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 CommandSubopcodeb                                : __CODEGEN_BITFIELD(16, 20)    ; //!< COMMAND_SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODEA
                uint32_t                 CommandOpcode                                    : __CODEGEN_BITFIELD(24, 26)    ; //!< COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SliceType                                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< SLICE_TYPE
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Log2WeightDenomLuma                              : __CODEGEN_BITFIELD( 0,  2)    ; //!< Log 2 Weight Denom Luma
                uint32_t                 Reserved67                                       : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 Log2WeightDenomChroma                            : __CODEGEN_BITFIELD( 8, 10)    ; //!< Log 2 Weight Denom Chroma
                uint32_t                 Reserved75                                       : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 NumberOfReferencePicturesInInterPredictionList0  : __CODEGEN_BITFIELD(16, 21)    ; //!< Number of Reference Pictures in Inter-prediction List 0
                uint32_t                 Reserved86                                       : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 NumberOfReferencePicturesInInterPredictionList1  : __CODEGEN_BITFIELD(24, 29)    ; //!< Number of Reference Pictures in Inter-prediction List 1
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 SliceAlphaC0OffsetDiv2                           : __CODEGEN_BITFIELD( 0,  3)    ; //!< Slice Alpha C0 Offset Div2
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 SliceBetaOffsetDiv2                              : __CODEGEN_BITFIELD( 8, 11)    ; //!< Slice Beta Offset Div2
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SliceQuantizationParameter                       : __CODEGEN_BITFIELD(16, 21)    ; //!< Slice Quantization Parameter
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 CabacInitIdc10                                   : __CODEGEN_BITFIELD(24, 25)    ; //!< Cabac Init Idc[1:0]
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 26)    ; //!< Reserved
                uint32_t                 DisableDeblockingFilterIndicator                 : __CODEGEN_BITFIELD(27, 28)    ; //!< DISABLE_DEBLOCKING_FILTER_INDICATOR
                uint32_t                 DirectPredictionType                             : __CODEGEN_BITFIELD(29, 29)    ; //!< DIRECT_PREDICTION_TYPE
                uint32_t                 WeightedPredictionIndicator                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Weighted Prediction Indicator
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 SliceStartMbNum                                  : __CODEGEN_BITFIELD( 0, 14)    ; //!< Slice Start Mb Num
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 SliceHorizontalPosition                          : __CODEGEN_BITFIELD(16, 23)    ; //!<  Slice Horizontal Position
                uint32_t                 SliceVerticalPosition                            : __CODEGEN_BITFIELD(24, 31)    ; //!< Slice Vertical Position
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 NextSliceHorizontalPosition                      : __CODEGEN_BITFIELD( 0,  8)    ; //!< Next Slice Horizontal Position
                uint32_t                 Reserved168                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 NextSliceVerticalPosition                        : __CODEGEN_BITFIELD(16, 24)    ; //!< Next Slice Vertical Position
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 StreamId10                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Stream ID [1:0]
                uint32_t                 Reserved194                                      : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved
                uint32_t                 SliceId30                                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< Slice ID [3:0]
                uint32_t                 Reserved200                                      : __CODEGEN_BITFIELD( 8, 11)    ; //!< Reserved
                uint32_t                 Cabaczerowordinsertionenable                     : __CODEGEN_BITFIELD(12, 12)    ; //!< CABACZEROWORDINSERTIONENABLE
                uint32_t                 Emulationbytesliceinsertenable                   : __CODEGEN_BITFIELD(13, 13)    ; //!< EMULATIONBYTESLICEINSERTENABLE
                uint32_t                 Reserved206                                      : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved
                uint32_t                 TailInsertionPresentInBitstream                  : __CODEGEN_BITFIELD(15, 15)    ; //!< TAIL_INSERTION_PRESENT_IN_BITSTREAM
                uint32_t                 SlicedataInsertionPresentInBitstream             : __CODEGEN_BITFIELD(16, 16)    ; //!< SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM
                uint32_t                 HeaderInsertionPresentInBitstream                : __CODEGEN_BITFIELD(17, 17)    ; //!< HEADER_INSERTION_PRESENT_IN_BITSTREAM
                uint32_t                 Reserved210                                      : __CODEGEN_BITFIELD(18, 18)    ; //!< Reserved
                uint32_t                 IsLastSlice                                      : __CODEGEN_BITFIELD(19, 19)    ; //!< IS_LAST_SLICE
                uint32_t                 MbTypeSkipConversionDisable                      : __CODEGEN_BITFIELD(20, 20)    ; //!< MB_TYPE_SKIP_CONVERSION_DISABLE
                uint32_t                 MbTypeDirectConversionDisable                    : __CODEGEN_BITFIELD(21, 21)    ; //!< MB_TYPE_DIRECT_CONVERSION_DISABLE
                uint32_t                 RcPanicType                                      : __CODEGEN_BITFIELD(22, 22)    ; //!< RC_PANIC_TYPE
                uint32_t                 RcPanicEnable                                    : __CODEGEN_BITFIELD(23, 23)    ; //!< RC_PANIC_ENABLE
                uint32_t                 RcStableTolerance                                : __CODEGEN_BITFIELD(24, 27)    ; //!< RC Stable Tolerance
                uint32_t                 RcTriggleMode                                    : __CODEGEN_BITFIELD(28, 29)    ; //!< RC_TRIGGLE_MODE
                uint32_t                 Resetratecontrolcounter                          : __CODEGEN_BITFIELD(30, 30)    ; //!< RESETRATECONTROLCOUNTER
                uint32_t                 RateControlCounterEnable                         : __CODEGEN_BITFIELD(31, 31)    ; //!< RATE_CONTROL_COUNTER_ENABLE
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 IndirectPakBseDataStartAddressWrite              : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect PAK-BSE Data Start Address (Write)
                uint32_t                 Reserved253                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 GrowParamGrowInit                                : __CODEGEN_BITFIELD( 0,  3)    ; //!< Grow Param - Grow Init
                uint32_t                 GrowParamGrowResistance                          : __CODEGEN_BITFIELD( 4,  7)    ; //!< Grow Param - Grow Resistance
                uint32_t                 ShrinkParamShrinkInit                            : __CODEGEN_BITFIELD( 8, 11)    ; //!< Shrink Param - Shrink Init
                uint32_t                 ShrinkParamShrinkResistance                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Shrink Param - Shrink Resistance
                uint32_t                 MagnitudeOfQpMaxPositiveModifier                 : __CODEGEN_BITFIELD(16, 23)    ; //!< Magnitude of QP Max Positive Modifier
                uint32_t                 MagnitudeOfQpMaxNegativeModifier                 : __CODEGEN_BITFIELD(24, 31)    ; //!< Magnitude of QP Max Negative Modifier
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Correct1                                         : __CODEGEN_BITFIELD( 0,  3)    ; //!< Correct 1
                uint32_t                 Correct2                                         : __CODEGEN_BITFIELD( 4,  7)    ; //!< Correct 2
                uint32_t                 Correct3                                         : __CODEGEN_BITFIELD( 8, 11)    ; //!< Correct 3
                uint32_t                 Correct4                                         : __CODEGEN_BITFIELD(12, 15)    ; //!< Correct 4
                uint32_t                 Correct5                                         : __CODEGEN_BITFIELD(16, 19)    ; //!< Correct 5
                uint32_t                 Correct6                                         : __CODEGEN_BITFIELD(20, 23)    ; //!< Correct 6
                uint32_t                 Roundintra                                       : __CODEGEN_BITFIELD(24, 26)    ; //!< ROUNDINTRA
                uint32_t                 Roundintraenable                                 : __CODEGEN_BITFIELD(27, 27)    ; //!< RoundIntraEnable
                uint32_t                 Roundinter                                       : __CODEGEN_BITFIELD(28, 30)    ; //!< ROUNDINTER
                uint32_t                 Roundinterenable                                 : __CODEGEN_BITFIELD(31, 31)    ; //!< RoundInterEnable
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Cv0ClampValue0                                   : __CODEGEN_BITFIELD( 0,  3)    ; //!< CV0 - Clamp Value 0
                uint32_t                 Cv1                                              : __CODEGEN_BITFIELD( 4,  7)    ; //!< CV1
                uint32_t                 Cv2                                              : __CODEGEN_BITFIELD( 8, 11)    ; //!< CV2
                uint32_t                 Cv3                                              : __CODEGEN_BITFIELD(12, 15)    ; //!< CV3
                uint32_t                 Cv4                                              : __CODEGEN_BITFIELD(16, 19)    ; //!< CV4
                uint32_t                 Cv5                                              : __CODEGEN_BITFIELD(20, 23)    ; //!< CV5
                uint32_t                 Cv6                                              : __CODEGEN_BITFIELD(24, 27)    ; //!< CV6
                uint32_t                 ClampvaluesCv7                                   : __CODEGEN_BITFIELD(28, 31)    ; //!< ClampValues - CV7
            };
            uint32_t                     Value;
        } DW10;

        //! \name Local enumerations

        enum COMMAND_SUBOPCODEB
        {
            COMMAND_SUBOPCODEB_MFXAVCSLICESTATE                              = 3, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_MFXAVCSLICESTATE                                      = 0, //!< No additional details
        };

        enum COMMAND_OPCODE
        {
            COMMAND_OPCODE_AVC                                               = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXAVCSLICESTATE                                        = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SLICE_TYPE
        //! \details
        //!     It is set to the value of the syntax element read from the Slice Header.
        enum SLICE_TYPE
        {
            SLICE_TYPE_PSLICE                                                = 0, //!< No additional details
            SLICE_TYPE_BSLICE                                                = 1, //!< No additional details
            SLICE_TYPE_ISLICE                                                = 2, //!< No additional details
        };

        enum DISABLE_DEBLOCKING_FILTER_INDICATOR
        {
            DISABLE_DEBLOCKING_FILTER_INDICATOR_UNNAMED0                     = 0, //!< FilterInternalEdgesFlag is set equal to 1
            DISABLE_DEBLOCKING_FILTER_INDICATOR_UNNAMED1                     = 1, //!< Disable all deblocking operation, no deblocking parameter syntax element is read; filterInternalEdgesFlag is set equal to 0
            DISABLE_DEBLOCKING_FILTER_INDICATOR_UNNAMED2                     = 2, //!< Macroblocks in different slices are considered not available; filterInternalEdgesFlag is set equal to 1
        };

        //! \brief DIRECT_PREDICTION_TYPE
        //! \details
        //!     Type of direct prediction used for B Slices.  This field is valid only
        //!     for Slice_Type = B Slice; otherwise, it must be set to 0.
        enum DIRECT_PREDICTION_TYPE
        {
            DIRECT_PREDICTION_TYPE_TEMPORAL                                  = 0, //!< No additional details
            DIRECT_PREDICTION_TYPE_SPATIAL                                   = 1, //!< No additional details
        };

        //! \brief CABACZEROWORDINSERTIONENABLE
        //! \details
        //!     To pad the end of a SliceLayer RBSP to meet the encoded size
        //!     requirement.
        enum CABACZEROWORDINSERTIONENABLE
        {
            CABACZEROWORDINSERTIONENABLE_UNNAMED0                            = 0, //!< No Cabac_Zero_Word Insertion
            CABACZEROWORDINSERTIONENABLE_UNNAMED1                            = 1, //!< Allow internal Cabac_Zero_Word generation and append to the end of RBSP(effectively can be used as an indicator for last slice of a picture, if the assumption is only the last slice of a picture needs to insert CABAC_ZERO_WORDs.
        };

        //! \brief EMULATIONBYTESLICEINSERTENABLE
        //! \details
        //!     To have PAK outputting SODB or EBSP to the output bitstream buffer
        enum EMULATIONBYTESLICEINSERTENABLE
        {
            EMULATIONBYTESLICEINSERTENABLE_UNNAMED0                          = 0, //!< outputting RBSP
            EMULATIONBYTESLICEINSERTENABLE_UNNAMED1                          = 1, //!< outputting EBSP
        };

        //! \brief TAIL_INSERTION_PRESENT_IN_BITSTREAM
        //! \details
        //!     <p>This bit should only be set for the last super slice.</p>
        //!     <p><span style="color: rgb(0, 0, 0); font-family: Arial, sans-serif;
        //!     line-height: normal;">SKL Restriction: In VDENC mode, SW should insert
        //!     1000 </span><b style="color: rgb(0, 0, 0); font-family: Arial,
        //!     sans-serif; line-height: normal;">VD_PIPELINE_FLUSH commands with
        //!     VDENC_pipeline_Done set to 1 before inserting tail command. This is for
        //!     delaying the tail insertion in HW. The HW recommendation is to insert
        //!     tail only at the end of sequence to avoid performance loss since this
        //!     restriction potentially cause performance degradation.</b></p>
        enum TAIL_INSERTION_PRESENT_IN_BITSTREAM
        {
            TAIL_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0                     = 0, //!< No tail insertion into the output bitstream buffer, after the current slice encoded bits
            TAIL_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED1                     = 1, //!< Tail insertion into the output bitstream buffer is present, and is after the current slice encoded bits.
        };

        //! \brief SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM
        //! \details
        //!     <p>This bit should be set for all super-slices.</p>
        enum SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM
        {
            SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0                = 0, //!< No Slice Data insertion into the output bitstream buffer
            SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED1                = 1, //!< Slice Data insertion into the output bitstream buffer is present.
        };

        //! \brief HEADER_INSERTION_PRESENT_IN_BITSTREAM
        //! \details
        //!     Note: In VDEnc mode, the slice header PAK object maximum size is 25 DWs.
        enum HEADER_INSERTION_PRESENT_IN_BITSTREAM
        {
            HEADER_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED0                   = 0, //!< No header insertion into the output bitstream buffer, in front of the current slice encoded bits.
            HEADER_INSERTION_PRESENT_IN_BITSTREAM_UNNAMED1                   = 1, //!< Header insertion into the output bitstream buffer is present, and is in front of the current slice encoded bits.
        };

        //! \brief IS_LAST_SLICE
        //! \details
        //!     It is used by the zero filling in the Minimum Frame Size test.
        enum IS_LAST_SLICE
        {
            IS_LAST_SLICE_UNNAMED0                                           = 0, //!< Current slice is NOT the last slice of a picture
            IS_LAST_SLICE_UNNAMED1                                           = 1, //!< Current slice is the last slice of a picture
        };

        //! \brief MB_TYPE_SKIP_CONVERSION_DISABLE
        //! \details
        //!     For all Macroblock type conversions in different slices, refer to
        //!     Section "Macroblock Type Conversion Rules" in the same volume.
        enum MB_TYPE_SKIP_CONVERSION_DISABLE
        {
            MB_TYPE_SKIP_CONVERSION_DISABLE_ENABLESKIPTYPECONVERSION         = 0, //!< No additional details
            MB_TYPE_SKIP_CONVERSION_DISABLE_DISABLESKIPTYPECONVERSION        = 1, //!< No additional details
        };

        //! \brief MB_TYPE_DIRECT_CONVERSION_DISABLE
        //! \details
        //!     For all Macroblock type conversions in different slices, refer to
        //!     Section "Macroblock Type Conversion Rules" in the same volume.
        enum MB_TYPE_DIRECT_CONVERSION_DISABLE
        {
            MB_TYPE_DIRECT_CONVERSION_DISABLE_ENABLEDIRECTMODECONVERSION     = 0, //!< No additional details
            MB_TYPE_DIRECT_CONVERSION_DISABLE_DISABLEDIRECTMODECONVERSION    = 1, //!< No additional details
        };

        //! \brief RC_PANIC_TYPE
        //! \details
        //!     This field selects between two RC Panic methods
        enum RC_PANIC_TYPE
        {
            RC_PANIC_TYPE_QPPANIC                                            = 0, //!< No additional details
            RC_PANIC_TYPE_CBPPANIC                                           = 1, //!< No additional details
        };

        //! \brief RC_PANIC_ENABLE
        //! \details
        //!     If this field is set to 1, RC enters panic mode when sum_act &gt;
        //!     sum_max. RC Panic Type field controls what type of panic behavior is
        //!     invoked.
        enum RC_PANIC_ENABLE
        {
            RC_PANIC_ENABLE_DISABLE                                          = 0, //!< No additional details
            RC_PANIC_ENABLE_ENABLE                                           = 1, //!< No additional details
        };

        enum RC_TRIGGLE_MODE
        {
            RC_TRIGGLE_MODE_ALWAYSRATECONTROL                                = 0, //!< Whereas RC becomes active if sum_act > sum_target or sum_act < sum_target
            RC_TRIGGLE_MODE_GENTLERATECONTROL                                = 1, //!< whereas RC becomes active if sum_act > upper_midpt or sum_act < lower_midpt
            RC_TRIGGLE_MODE_LOOSERATECONTROL                                 = 2, //!< whereas RC becomes active if sum_act > sum_max or sum_act < sum_min
        };

        //! \brief RESETRATECONTROLCOUNTER
        //! \details
        //!     To reset the bit allocation accumulation counter to 0 to restart the
        //!     rate control.
        enum RESETRATECONTROLCOUNTER
        {
            RESETRATECONTROLCOUNTER_NOTRESET                                 = 0, //!< No additional details
            RESETRATECONTROLCOUNTER_RESET                                    = 1, //!< No additional details
        };

        //! \brief RATE_CONTROL_COUNTER_ENABLE
        //! \details
        //!     To enable the accumulation of bit allocation for rate control
        //!                             This field enables hardware Rate Control logic. The rest of the RC
        //!     control fields are only valid when this field is set to 1. Otherwise,
        //!     hardware ignores these fields.
        enum RATE_CONTROL_COUNTER_ENABLE
        {
            RATE_CONTROL_COUNTER_ENABLE_DISABLE                              = 0, //!< No additional details
            RATE_CONTROL_COUNTER_ENABLE_ENABLE                               = 1, //!< No additional details
        };

        //! \brief ROUNDINTRA
        //! \details
        //!     Rounding precision for Intra quantized coefficients
        enum ROUNDINTRA
        {
            ROUNDINTRA_116                                                   = 0, //!< No additional details
            ROUNDINTRA_216                                                   = 1, //!< No additional details
            ROUNDINTRA_316                                                   = 2, //!< No additional details
            ROUNDINTRA_416                                                   = 3, //!< No additional details
            ROUNDINTRA_516                                                   = 4, //!< No additional details
            ROUNDINTRA_616                                                   = 5, //!< No additional details
            ROUNDINTRA_716                                                   = 6, //!< No additional details
            ROUNDINTRA_816                                                   = 7, //!< No additional details
        };

        //! \brief ROUNDINTER
        //! \details
        //!     Rounding precision for Inter quantized coefficients
        enum ROUNDINTER
        {
            ROUNDINTER_116                                                   = 0, //!< No additional details
            ROUNDINTER_216                                                   = 1, //!< No additional details
            ROUNDINTER_316                                                   = 2, //!< No additional details
            ROUNDINTER_416                                                   = 3, //!< No additional details
            ROUNDINTER_516                                                   = 4, //!< No additional details
            ROUNDINTER_616                                                   = 5, //!< No additional details
            ROUNDINTER_716                                                   = 6, //!< No additional details
            ROUNDINTER_816                                                   = 7, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_AVC_SLICE_STATE_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief MFD_AVC_DPB_STATE
    //! \details
    //!     This is a frame level state command used only in AVC Short Slice
    //!     Bitstream Format VLD mode. RefFrameList[16] of interface is
    //!     replaced with intel Reference Picture Addresses[16] of
    //!     MFX_PIPE_BUF_ADDR_STATE command. The LongTerm Picture flag indicator of
    //!     all reference pictures are collected into LongTermPic_Flag[16].
    //!     FieldOrderCntList[16][2] and CurrFieldOrderCnt[2] of interface are
    //!     replaced with intel POCList[34] of MFX_AVC_DIRECTMODE_STATE command.
    //!     
    struct MFD_AVC_DPB_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 NonExistingframeFlag161Bit                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< NON_EXISTINGFRAME_FLAG161_BIT
                uint32_t                 LongtermframeFlag161Bit                          : __CODEGEN_BITFIELD(16, 31)    ; //!< LONGTERMFRAME_FLAG161_BIT
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 UsedforreferenceFlag162Bits                                                      ; //!< USEDFORREFERENCE_FLAG162_BITS
            };
            uint32_t                     Value;
        } DW2;

        uint32_t                         Ltstframenumlist1616Bits[8];                                                     //!< LTSTFRAMENUMLIST1616_BITS


        uint32_t                         Viewidlist1616Bits[8];                                                           //!< ViewIDList[16][16 bits]


        uint32_t                         Vieworderlistl0168Bits[4];                                                       //!< ViewOrderListL0[16][8 bits]


        uint32_t                         Vieworderlistl1168Bits[4];                                                       //!< ViewOrderListL1[16][8 bits]


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED6                                             = 6, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCDEC                                      = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief NON_EXISTINGFRAME_FLAG161_BIT
        //! \details
        //!     One-to-one correspondence with the entries of the Intel
        //!     RefFrameList[16]. 1 bit per reference frame.
        enum NON_EXISTINGFRAME_FLAG161_BIT
        {
            NON_EXISTINGFRAME_FLAG161_BIT_VALID                              = 0, //!< the reference picture in that entry of RefFrameList[] is a valid reference
            NON_EXISTINGFRAME_FLAG161_BIT_INVALID                            = 1, //!< the reference picture in that entry of RefFrameList[] does not exist anymore.
        };

        //! \brief LONGTERMFRAME_FLAG161_BIT
        //! \details
        //!     One-to-one correspondence with the entries of the Intel
        //!     RefFrameList[16]. 1 bit per reference frame.
        enum LONGTERMFRAME_FLAG161_BIT
        {
            LONGTERMFRAME_FLAG161_BIT_THEPICTUREISASHORTTERMREFERENCEPICTURE = 0, //!< No additional details
            LONGTERMFRAME_FLAG161_BIT_THEPICTUREISALONGTERMREFERENCEPICTURE  = 1, //!< No additional details
        };

        //! \brief USEDFORREFERENCE_FLAG162_BITS
        //! \details
        //!     One-to-one correspondence with the entries of the Intel
        //!     RefFrameList[16]. 2 bits per reference frame.
        enum USEDFORREFERENCE_FLAG162_BITS
        {
            USEDFORREFERENCE_FLAG162_BITS_NOTREFERENCE                       = 0, //!< indicates a frame is "not used for reference".
            USEDFORREFERENCE_FLAG162_BITS_TOPFIELD                           = 1, //!< bit[0] indicates that the top field of a frame is marked as "used for reference".
            USEDFORREFERENCE_FLAG162_BITS_BOTTOMFIELD                        = 2, //!< bit[1] indicates that the bottom field of a frame is marked as "used for reference".
            USEDFORREFERENCE_FLAG162_BITS_FRAME                              = 3, //!< bit[1:0] indicates that a frame (or field pair) is marked as "used for reference".
        };

        //! \brief LTSTFRAMENUMLIST1616_BITS
        //! \details
        //!     One-to-one correspondence with the entries of the Intel
        //!     RefFrameList[16]. 16 bits per reference frame.Depending on the
        //!     corresponding LongTermFrame_Flag[], the content of this field is
        //!     interpreted differently.
        enum LTSTFRAMENUMLIST1616_BITS
        {
            LTSTFRAMENUMLIST1616_BITS_SHORTTERMFRAMEFLAGI                    = 0, //!< LTSTFrameNumList[i]represent Short Term Picture FrameNum.
            LTSTFRAMENUMLIST1616_BITS_LONGTERMFRAMEFLAGI                     = 1, //!< LTSTFrameNumList[i] represent LongTermFrameIdx.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_AVC_DPB_STATE_CMD();

        static const size_t dwSize = 27;
        static const size_t byteSize = 108;
    };

    //!
    //! \brief MFD_AVC_SLICEADDR
    //! \details
    //!     This is a Slice level command used only for AVC Short Slice
    //!     Bitstream Format VLD mode.When decoding a slice, H/W needs to know the
    //!     last MB of the slice has reached in order to start decoding the next
    //!     slice. It also needs to know if a slice is terminated but the last MB
    //!     has not reached, error conealment should be invoked to generate those
    //!     missing MBs. For AVC Short Format, the only way to know the last
    //!     MB position of the current slice, H/W needs to snoop into the next
    //!     slice's start MB address (a linear address encoded in the Slice Header).
    //!     Since each BSD Object command can have only one indirect bitstream
    //!     buffer address, this command is added to help H/W to snoop into the next
    //!     slice's slice header and retrieve its Start MB Address. This command
    //!     will take the next slice's bitstream buffer address as input (exactly
    //!     the same way as a BSD Object command), and parse only the
    //!     first_mb_in_slice syntax element. The result will stored inside the H/W,
    //!     and will be used to decode the current slice specified in the BSD Object
    //!     command.Only the very first few bytes (max 5 bytes for a max 4K picture)
    //!     of the Slice Header will be decoded, the rest of the bitstream are don't
    //!     care. This is because the first_mb_in_slice is encoded in Exponential
    //!     Golomb, and will take 33 bits to represent the max 256 x 256 = 64K-1
    //!     value. The indirect data of MFD_AVC_SLICEADDR is a valid BSD object and
    //!     is decoded as in BSD OBJECT command.The next Slice Start MB Address is
    //!     also exposed to the MMIO interface.The Slice Start MB Address
    //!     (first_mb_in_slice) is a linear MB address count; but it is translated
    //!     into the corresponding 2D MB X and Y raster position, and are stored
    //!     internally as NextSliceMbY and NextSliceMbX.
    //!     
    struct MFD_AVC_SLICEADDR_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectBsdDataLength                                                            ; //!< Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectBsdDataStartAddress                      : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect BSD Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 DriverProvidedNalTypeValue                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Driver Provided NAL Type Value
                uint32_t                 AvcNalTypeFirstByteOverrideBit                   : __CODEGEN_BITFIELD( 8,  8)    ; //!< AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED7                                             = 7, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCDEC                                      = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDAVCSLICEADDR                                         = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
        //! \details
        //!     <p>This bit indicates hardware should use the NAL Type (provided below)
        //!     programmed by driver instead of using the one from bitstream.  The NAL
        //!     byte from bitstream will not be correct.</p>
        enum AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
        {
            AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT_USEBITSTREAMDECODEDNALTYPE  = 0, //!< NAL Type should come from first byte of decoded bitstream.
            AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT_USEDRIVERPROGRAMMEDNALTYPE  = 1, //!< NAL Type should come from "Driver Provided NAL Type Values" programmed by driver.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_AVC_SLICEADDR_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MFD_AVC_BSD_OBJECT
    //! \details
    //!     The MFD_AVC_BSD_OBJECT command is the only primitive command for the AVC
    //!     Decoding Pipeline. The same command is used for both CABAC and CAVLD
    //!     modes. The Slice Data portion of the bitstream is loaded as indirect
    //!     data object.Before issuing a MFD_AVC_BSD_OBJECT command, all AVC states
    //!     of the MFD Engine need to be valid. Therefore the commands used to set
    //!     these states need to have been issued prior to the issue of a
    //!     MFD_AVC_BSD_OBJECT command.
    //!     
    //!     Context switch interrupt is not supported by this command.
    //!     
    struct MFD_AVC_BSD_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectBsdDataLength                                                            ; //!< Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectBsdDataStartAddress                      : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect BSD Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MbErrorConcealmentPSliceWeightPredictionDisableFlag : __CODEGEN_BITFIELD( 0,  0)    ; //!< MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG
                uint32_t                 MbErrorConcealmentPSliceMotionVectorsOverrideDisableFlag : __CODEGEN_BITFIELD( 1,  1)    ; //!< MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
                uint32_t                 Reserved98                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 MbErrorConcealmentBSpatialWeightPredictionDisableFlag : __CODEGEN_BITFIELD( 3,  3)    ; //!< MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG
                uint32_t                 MbErrorConcealmentBSpatialMotionVectorsOverrideDisableFlag : __CODEGEN_BITFIELD( 4,  4)    ; //!< MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
                uint32_t                 Reserved101                                      : __CODEGEN_BITFIELD( 5,  5)    ; //!< Reserved
                uint32_t                 MbErrorConcealmentBSpatialPredictionMode         : __CODEGEN_BITFIELD( 6,  7)    ; //!< MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_
                uint32_t                 MbHeaderErrorHandling                            : __CODEGEN_BITFIELD( 8,  8)    ; //!< MB_HEADER_ERROR_HANDLING_
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 EntropyErrorHandling                             : __CODEGEN_BITFIELD(10, 10)    ; //!< ENTROPY_ERROR_HANDLING
                uint32_t                 Reserved107                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 MprErrorMvOutOfRangeHandling                     : __CODEGEN_BITFIELD(12, 12)    ; //!< MPR_ERROR_MV_OUT_OF_RANGE_HANDLING
                uint32_t                 Reserved109                                      : __CODEGEN_BITFIELD(13, 13)    ; //!< Reserved
                uint32_t                 BsdPrematureCompleteErrorHandling                : __CODEGEN_BITFIELD(14, 14)    ; //!< BSD_PREMATURE_COMPLETE_ERROR_HANDLING
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 ConcealmentPictureId                             : __CODEGEN_BITFIELD(16, 21)    ; //!< Concealment Picture ID
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 MbErrorConcealmentBTemporalWeightPredictionDisableFlag : __CODEGEN_BITFIELD(24, 24)    ; //!< MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG
                uint32_t                 MbErrorConcealmentBTemporalMotionVectorsOverrideEnableFlag : __CODEGEN_BITFIELD(25, 25)    ; //!< MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 26)    ; //!< Reserved
                uint32_t                 MbErrorConcealmentBTemporalPredictionMode        : __CODEGEN_BITFIELD(27, 28)    ; //!< MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE
                uint32_t                 IntraPredmode4X48X8LumaErrorControlBit           : __CODEGEN_BITFIELD(29, 29)    ; //!< INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT
                uint32_t                 InitCurrentMbNumber                              : __CODEGEN_BITFIELD(30, 30)    ; //!< Init Current MB Number
                uint32_t                 ConcealmentMethod                                : __CODEGEN_BITFIELD(31, 31)    ; //!< CONCEALMENT_METHOD
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FirstMacroblockMbBitOffset                       : __CODEGEN_BITFIELD( 0,  2)    ; //!< First Macroblock (MB)Bit Offset
                uint32_t                 LastsliceFlag                                    : __CODEGEN_BITFIELD( 3,  3)    ; //!< LASTSLICE_FLAG
                uint32_t                 EmulationPreventionBytePresent                   : __CODEGEN_BITFIELD( 4,  4)    ; //!< EMULATION_PREVENTION_BYTE_PRESENT
                uint32_t                 Reserved133                                      : __CODEGEN_BITFIELD( 5,  6)    ; //!< Reserved
                uint32_t                 FixPrevMbSkipped                                 : __CODEGEN_BITFIELD( 7,  7)    ; //!< Fix Prev Mb Skipped
                uint32_t                 Reserved136                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 FirstMbByteOffsetOfSliceDataOrSliceHeader        : __CODEGEN_BITFIELD(16, 31)    ; //!< First MB Byte Offset of Slice Data or Slice Header
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma : __CODEGEN_BITFIELD( 0,  0)    ; //!< INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA
                uint32_t                 Intra8X84X4PredictionErrorConcealmentControlBit  : __CODEGEN_BITFIELD( 1,  1)    ; //!< INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT
                uint32_t                 Reserved162                                      : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved
                uint32_t                 BSliceTemporalInterConcealmentMode               : __CODEGEN_BITFIELD( 4,  6)    ; //!< B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE
                uint32_t                 Reserved167                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 BSliceSpatialInterConcealmentMode                : __CODEGEN_BITFIELD( 8, 10)    ; //!< B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE
                uint32_t                 Reserved171                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 BSliceInterDirectTypeConcealmentMode             : __CODEGEN_BITFIELD(12, 13)    ; //!< B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE
                uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved
                uint32_t                 BSliceConcealmentMode                            : __CODEGEN_BITFIELD(15, 15)    ; //!< B_SLICE_CONCEALMENT_MODE
                uint32_t                 PSliceInterConcealmentMode                       : __CODEGEN_BITFIELD(16, 18)    ; //!< P_SLICE_INTER_CONCEALMENT_MODE
                uint32_t                 Reserved179                                      : __CODEGEN_BITFIELD(19, 22)    ; //!< Reserved
                uint32_t                 PSliceConcealmentMode                            : __CODEGEN_BITFIELD(23, 23)    ; //!< P_SLICE_CONCEALMENT_MODE
                uint32_t                 ConcealmentReferencePictureFieldBit              : __CODEGEN_BITFIELD(24, 29)    ; //!< Concealment Reference Picture + Field Bit 
                uint32_t                 Reserved190                                      : __CODEGEN_BITFIELD(30, 30)    ; //!< Reserved
                uint32_t                 ISliceConcealmentMode                            : __CODEGEN_BITFIELD(31, 31)    ; //!< I_SLICE_CONCEALMENT_MODE
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 DriverProvidedNalTypeValue                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Driver Provided Nal Type Value
                uint32_t                 AvcNalTypeFirstByteOverrideBit                   : __CODEGEN_BITFIELD( 8,  8)    ; //!< AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
                uint32_t                 Reserved201                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCDEC                                      = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDAVCBSDOBJECT                                         = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG
        //! \details
        //!     During MB Error Concealment on P slice, weight prediction is disabled to
        //!     improve image quality.
        //!                         This bit can be set to preserve the original weight prediction.  
        //!                         This bit does not affect normal decoded MB.
        enum MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0 = 0, //!< Weight Prediction is Disabled during MB Concealment.
            MB_ERROR_CONCEALMENT_P_SLICE_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED1 = 1, //!< Weight Prediction will not be overridden during MB Concealment.
        };

        //! \brief MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
        //! \details
        //!     During MB Error Concealment on P slice, motion vectors are forced to 0
        //!     to improve image quality. 
        //!                         This bit can be set to use the predicted motion vectors instead.  
        //!                         This bit does not affect normal decoded MB.
        enum MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED0 = 0, //!< Motion Vectors are Overridden to 0 during MB Concealment
            MB_ERROR_CONCEALMENT_P_SLICE_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED1 = 1, //!< Predicted Motion Vectors are used during MB Concealment
        };

        //! \brief MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG
        //! \details
        //!     During MB Error Concealment on B slice with Spatial Direct Prediction,
        //!     weight prediction is disabled to improve image quality.
        //!                         This bit can be set to preserve the original weight prediction.  
        //!                         This bit does not affect normal decoded MB.
        enum MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0 = 0, //!< Weight Prediction is Disabled during MB Concealment.
            MB_ERROR_CONCEALMENT_B_SPATIAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED1 = 1, //!< Weight Prediction will not be overridden during MB Concealment.
        };

        //! \brief MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
        //! \details
        //!     During MB Error Concealment on B slice with Spatial Direct Prediction,
        //!     motion vectors are forced to 0 to improve image quality. 
        //!                         This bit can be set to use the predicted motion vectors instead.  
        //!                         This bit does not affect normal decoded MB.
        enum MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED0 = 0, //!< Motion Vectors are Overridden to 0 during MB Concealment
            MB_ERROR_CONCEALMENT_B_SPATIAL_MOTION_VECTORS_OVERRIDE_DISABLE_FLAG_UNNAMED1 = 1, //!< Predicted Motion Vectors are used during MB Concealment
        };

        //! \brief MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_
        //! \details
        //!     These two bits control how the reference L0/L1 are overridden in B
        //!     spatial slice.
        enum MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_
        {
            MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_UNNAMED0          = 0, //!< Both Reference Indexes L0/L1 are forced to 0 during Concealment
            MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_UNNAMED1          = 1, //!< Only Reference Index L1 is forced to 0;  Reference Index L0 is forced to -1
            MB_ERROR_CONCEALMENT_B_SPATIAL_PREDICTION_MODE_UNNAMED2          = 2, //!< Only Reference Index L0 is forced to 0; Reference Index L1 is forced to -1
        };

        //! \brief MB_HEADER_ERROR_HANDLING_
        //! \details
        //!     Software must follow the action for each Value as follow:
        enum MB_HEADER_ERROR_HANDLING_
        {
            MB_HEADER_ERROR_HANDLING_UNNAMED0                                = 0, //!< Ignore the error and continue (masked the interrupt), assume the hardware automatically perform the error concealment.
            MB_HEADER_ERROR_HANDLING_UNNAMED1                                = 1, //!< Set the interrupt to the driver (provide MMIO registers for MB address R/W).
        };

        //! \brief ENTROPY_ERROR_HANDLING
        //! \details
        //!     Software must follow the action for each Value as follow:
        enum ENTROPY_ERROR_HANDLING
        {
            ENTROPY_ERROR_HANDLING_UNNAMED0                                  = 0, //!< Ignore the error and continue (masked the interrupt), assume the hardware automatically perform the error handling.
            ENTROPY_ERROR_HANDLING_UNNAMED1                                  = 1, //!< Set the interrupt to the driver (provide MMIO registers for MB address R/W).
        };

        //! \brief MPR_ERROR_MV_OUT_OF_RANGE_HANDLING
        //! \details
        //!     Software must follow the action for each Value as follow:
        enum MPR_ERROR_MV_OUT_OF_RANGE_HANDLING
        {
            MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_UNNAMED0                      = 0, //!< Ignore the error and continue (masked the interrupt), assume the hardware automatically performs the error handling
            MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_UNNAMED1                      = 1, //!< Set the interrupt to the driver (provide MMIO registers for MB address R/W)
        };

        //! \brief BSD_PREMATURE_COMPLETE_ERROR_HANDLING
        //! \details
        //!     BSD Premature Complete Error occurs in situation where the Slice decode
        //!     is completed but there are still data in the bitstream.
        enum BSD_PREMATURE_COMPLETE_ERROR_HANDLING
        {
            BSD_PREMATURE_COMPLETE_ERROR_HANDLING_UNNAMED0                   = 0, //!< Ignore the error and continue (masked the interrupt), assume the hardware automatically performs the error handling
            BSD_PREMATURE_COMPLETE_ERROR_HANDLING_UNNAMED1                   = 1, //!< Set the interrupt to the driver (provide MMIO registers for MB address R/W)
        };

        //! \brief MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG
        //! \details
        //!     During MB Error Concealment on B slice with Temporal Direct Prediction,
        //!     weight prediction is disabled to improve image quality.
        //!                         This bit can be set to preserve the original weight prediction.
        enum MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED0 = 0, //!< Weight Prediction is Disabled during MB Concealment
            MB_ERROR_CONCEALMENT_B_TEMPORAL_WEIGHT_PREDICTION_DISABLE_FLAG_UNNAMED1 = 1, //!< Weight Prediction will not be overridden during MB Concealment
        };

        //! \brief MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG
        //! \details
        //!     During MB Error Concealment on B slice with Temporal Direct Prediction,
        //!     motion vectors are forced to 0 to improve image quality.
        //!                         This bit can be set to preserve the original weight prediction.
        enum MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG
        {
            MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG_UNNAMED0 = 0, //!< Predicted Motion Vectors are used during MB Concealment
            MB_ERROR_CONCEALMENT_B_TEMPORAL_MOTION_VECTORS_OVERRIDE_ENABLE_FLAG_UNNAMED1 = 1, //!< Motion Vectors are Overridden to 0 during MB Concealment
        };

        //! \brief MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE
        //! \details
        //!     These two bits control how the reference L0/L1 are overridden in B
        //!     temporal slice.
        enum MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE
        {
            MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE_UNNAMED0         = 0, //!< Both Reference Indexes L0/L1 are forced to 0 during Concealment
            MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE_UNNAMED1         = 1, //!< Only Reference Index L1 is forced to 0;  Reference Index L0 is forced to -1
            MB_ERROR_CONCEALMENT_B_TEMPORAL_PREDICTION_MODE_UNNAMED2         = 2, //!< Only Reference Index L0 is forced to 0; Reference Index L1 is forced to -1
        };

        //! \brief INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT
        //! \details
        //!     This field controls if AVC decoder will fix Intra Prediction Mode if the
        //!     decoded value is incorrect according to MB position
        enum INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT
        {
            INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT_UNNAMED0            = 0, //!< AVC decoder will detect and fix IntraPredMode (4x4/8x8 Luma) Errors.
            INTRA_PREDMODE_4X48X8_LUMA_ERROR_CONTROL_BIT_UNNAMED1            = 1, //!< AVC decoder will NOT detect IntraPredMode (4x4/8x8 Luma) Errors.  The wrong IntraPredMode value will be retaind.
        };

        //! \brief CONCEALMENT_METHOD
        //! \details
        //!     This field specifies the method used for concealment when error is
        //!     detected. If set, a copy from collocated macroblock location is
        //!     performed from the concealment reference indicated by the ConCeal_Pic_Id
        //!     field. If it is not set, a copy from the current picture is performed
        //!     using Intra 16x16 Prediction method.
        enum CONCEALMENT_METHOD
        {
            CONCEALMENT_METHOD_UNNAMED0                                      = 0, //!< Intra 16x16 Prediction
            CONCEALMENT_METHOD_UNNAMED1                                      = 1, //!< Inter P Copy
        };

        //! \brief LASTSLICE_FLAG
        //! \details
        //!     It is needed for both error concealment at the end of a picture (so, no
        //!     more phantom slice as in DevSNB).  It is also needed to know to set the
        //!     last MB in a picture correctly.
        enum LASTSLICE_FLAG
        {
            LASTSLICE_FLAG_UNNAMED0                                          = 0, //!< If the current Slice to be decoded is any slice other than the very last slice of the current picture
            LASTSLICE_FLAG_UNNAMED1                                          = 1, //!< If the current Slice to be decoded is the very last slice of the current picture.
        };

        enum EMULATION_PREVENTION_BYTE_PRESENT
        {
            EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED0                       = 0, //!< H/W needs to perform Emulation Byte Removal
            EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED1                       = 1, //!< H/W does not need to perform Emulation Byte Removal
        };

        //! \brief INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA
        //! \details
        //!     This field controls if AVC decoder will fix Intra Prediction Mode if the
        //!     decoded value is incorrect according to MB position.
        enum INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA
        {
            INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA_UNNAMED0 = 0, //!< AVC decoder will detect and fix Intra Prediction Mode Errors.
            INTRA_PREDICTION_ERROR_CONTROL_BIT_APPLIED_TO_INTRA16X16INTRA8X8INTRA4X4_LUMA_AND_CHROMA_UNNAMED1 = 1, //!< AVC decoder will retain the Intra Prediction value decoded from bitstream.
        };

        //! \brief INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT
        //! \details
        //!     This field controls if AVC goes into MB concealment mode (next MB) when
        //!     an error is detected on Intra8x8/4x4 Prediction Mode (these 2 modes have
        //!     fixed coding so it may not affect the bitstream.
        enum INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT
        {
            INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT_UNNAMED0   = 0, //!< AVC decoder will NOT go into MB concealment when Intra8x8/4x4 Prediction mode is incorrect.
            INTRA_8X84X4_PREDICTION_ERROR_CONCEALMENT_CONTROL_BIT_UNNAMED1   = 1, //!< AVC decoder will go into MB concealment when Intra8x8/4x4 Prediction mode is incorrect.
        };

        //! \brief B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder select reference picture for
        //!     Temporal Inter Concealment in B Slice
        enum B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE
        {
            B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED0                 = 0, //!< Top of Reference List L0/L1 (Use top entry of Reference List L0/L1)
            B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED1                 = 1, //!< Driver Specified Concealment Reference
            B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED2                 = 2, //!< Predicted Reference (Use reference picture predicted using B-Skip Algorithm)
            B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED3                 = 3, //!< " Temporal Closest (Using POC to select the closest forward picture)[For L0:  Closest POC smaller than current POC][For L1:  Closest POC larger than current POC]
            B_SLICE_TEMPORAL_INTER_CONCEALMENT_MODE_UNNAMED_4                = 4, //!< First Long Term Picture in Reference List L0/L1(If no long term picture available, use Temporal Closest Picture)
        };

        //! \brief B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder select reference picture for Spatial
        //!     Inter Concealment in B Slice.
        enum B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE
        {
            B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE_UNNAMED0                  = 0, //!< Top of Reference List L0/L1 (Use top entry of Reference List L0/L1).
            B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE_UNNAMED1                  = 1, //!< Driver Specified Concealment Reference
            B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE_UNNAMED3                  = 3, //!< Temporal Closest (Using POC to select the closest forward picture)[For L0:  Closest POC smaller than current POC][For L1:  Closest POC larger than current POC]
            B_SLICE_SPATIAL_INTER_CONCEALMENT_MODE_UNNAMED_4                 = 4, //!< " First Long Term Picture in Reference List L0/L1 (If no long term picture available, use Temporal Closest Picture)
        };

        //! \brief B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE
        //! \details
        //!     AVC decoder can use Spatial or Temporal Direct for B Skip/Direct.  
        //!                         This field determine can override the mode on how AVC decoder
        //!     handles MB concealment in B slice.
        enum B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE
        {
            B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE_UNNAMED0              = 0, //!< Use Default Direct Type (slice programmed direct type)
            B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE_UNNAMED1              = 1, //!< Forced to Spatial Direct Only
            B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE_UNNAMED2              = 2, //!< Forced to Temporal Direct Only
            B_SLICE_INTER_DIRECT_TYPE_CONCEALMENT_MODE_UNNAMED3              = 3, //!< Spatial Direct without Temporal Componenet (MovingBlock information)
        };

        //! \brief B_SLICE_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder handle MB concealment in B Slice
        enum B_SLICE_CONCEALMENT_MODE
        {
            B_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT                        = 0, //!< No additional details
            B_SLICE_CONCEALMENT_MODE_INTRACONCEALMENT                        = 1, //!< No additional details
        };

        //! \brief P_SLICE_INTER_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder select reference picture for
        //!     Concealment in P Slice.
        enum P_SLICE_INTER_CONCEALMENT_MODE
        {
            P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED0                          = 0, //!< Top of Reference List L0 (Use top entry of Reference List L0)
            P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED1                          = 1, //!< Driver Specified Concealment Reference
            P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED2                          = 2, //!< Predicted Reference (Use reference picture predicted using P-Skip Algorithm)
            P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED3                          = 3, //!< Temporal Closest (Using POC to select the closest forward picture)[For L0:  Closest POC smaller than current POC]
            P_SLICE_INTER_CONCEALMENT_MODE_UNNAMED_4                         = 4, //!< First Long Term Picture in Reference List L0 (If no long term picture available, use Temporal Closest Picture)
        };

        //! \brief P_SLICE_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder handle MB concealment in P Slice
        enum P_SLICE_CONCEALMENT_MODE
        {
            P_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT                        = 0, //!< No additional details
            P_SLICE_CONCEALMENT_MODE_INTRACONCEALMENT                        = 1, //!< No additional details
        };

        //! \brief I_SLICE_CONCEALMENT_MODE
        //! \details
        //!     This field controls how AVC decoder handle MB concealment in I Slice
        enum I_SLICE_CONCEALMENT_MODE
        {
            I_SLICE_CONCEALMENT_MODE_INTRACONCEALMENT                        = 0, //!< No additional details
            I_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT                        = 1, //!< No additional details
        };

        //! \brief AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
        //! \details
        //!     <p>This bit indicates hardware should use the NAL Type (provided below)
        //!     programmed by driver instead of using the one from bitstream.  The NAL
        //!     byte from bitstream will not be correct.</p>
        enum AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT
        {
            AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT_USEBITSTREAMDECODEDNALTYPE  = 0, //!< NAL Type should come from the first byte of Decoded Bitstream
            AVC_NAL_TYPE_FIRST_BYTE_OVERRIDE_BIT_USEDRIVERPROGRAMMEDNALTYPE  = 1, //!< NAL Type should come from "Driver Provided Nal Type" programmed by driver.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_AVC_BSD_OBJECT_CMD();

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief MFX_PAK_INSERT_OBJECT
    //! \details
    //!     The MFX_PAK_INSERT_OBJECT command is the first primitive command for the
    //!     AVC, MPEG2, JPEG, SVC and VP8 Encoding Pipeline.
    //!     
    //!     This command is issued to setup the control and parameters of inserting
    //!     a chunk of compressed/encoded bits into the current bitstream output
    //!     buffer starting at the specified bit locationto perform the actual
    //!     insertion by transferring the command inline data to the output buffer
    //!     max, 32 bits at a time. It is a variable length command as the data to
    //!     be inserted are presented as inline data of this command. It is a
    //!     multiple of 32-bit (1 DW), as the data bus to the bitstream buffer is
    //!     32-bit wide. Multiple insertion commands can be issued back to back in a
    //!     series. It is host software's responsibility to make sure their
    //!     corresponding data will properly stitch together to form a valid H.264
    //!     bitstream. Internally, MFX hardware will keep track of the very last two
    //!     bytes' (the very last byte can be a partial byte) values of the previous
    //!     insertion. It is required that the next Insertion Object Command or the
    //!     next PAK Object Command to perform the start code emulation sequence
    //!     check and prevention 0x03 byte insertion with this end condition of the
    //!     previous insertion. Hardware will keep track of an output bitstream
    //!     buffer current byte position and the associated next bit insertion
    //!     position index. Data to be inserted can be a valid H.264 NAL units or a
    //!     partial NAL unit. Certain NAL unit has a minimum byte size requirement.
    //!     As such the hardware will optionally (enabled by STATE Command)
    //!     determines the number of CABAC_ZERO_WORD to be inserted to the end of
    //!     the current NAL, based on the minimum byte size of a NAL and the actual
    //!     bin count of the encoded Slice. Since prior to the CABAC_ZERO_WORD
    //!     insertion, the RBSP or EBSP is already byte-aligned, so each
    //!     CABAC_ZERO_WORD insertion is actually a 3-byte sequence 0x00 00 03. The
    //!     inline data may have already been processed for start code emulation
    //!     byte insertion, except the possibility of the last 2 bytes plus the very
    //!     last partial byte (if any). Hence, when hardware performing the
    //!     concatenation of multiple consecutive insertion commands, or
    //!     concatenation of an insertion command and a PAK object command, it must
    //!     check and perform the necessary start code emulation byte insert at the
    //!     junction.The inline data is required to be byte aligned on the left
    //!     (first transmitted bit order) and may or may not be byte aligned on the
    //!     right (last transmitted bits).  The command will specify the bit offset
    //!     of the last valid DW.Each insertion state command defines a chunk of
    //!     bits (compressed data) to be inserted at a specific location of the
    //!     output compressed bitstream in the output buffer.Depend on CABAC or
    //!     CAVLC encoding mode (from Slice State), PAK Object Command is always
    //!     ended in byte aligned output bitstream except for CABAC header insertion
    //!     which is bit aligned. In the aligned cases, PAK will perform 0 filling
    //!     in CAVLC mode, and 1 filling in CABAC mode. Insertion data can
    //!     include:any encoded syntax elements bit data before the encoded Slice
    //!     Data (PAK Object Command) of the current SliceSPS NALPPS NALSEI NALOther
    //!     Non-Slice NALLeading_Zero_8_bits (as many bytes as there is)Start Code
    //!     PrefixNAL Header ByteSlice HeaderAny encoded syntax elements bit data
    //!     after the encoded Slice Data (PAK Object Command) of the current Slice
    //!     and prior to the next encoded Slice Data of the next Slice or prior to
    //!     the end of the bistream, whichever comes firstCabac_Zero_Word or
    //!     Trailing_Zero_8bits (as many bytes as there is). Anything listed above
    //!     before a Slice DataContext switch interrupt is not supported by this
    //!     command.
    //!     
    struct MFX_PAK_INSERT_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 BitstreamstartresetResetbitstreamstartingpos     : __CODEGEN_BITFIELD( 0,  0)    ; //!< BITSTREAMSTARTRESET__RESETBITSTREAMSTARTINGPOS
                uint32_t                 EndofsliceflagLastdstdatainsertcommandflag       : __CODEGEN_BITFIELD( 1,  1)    ; //!< EndOfSliceFlag - LastDstDataInsertCommandFlag
                uint32_t                 LastheaderflagLastsrcheaderdatainsertcommandflag : __CODEGEN_BITFIELD( 2,  2)    ; //!< LastHeaderFlag - LastSrcHeaderDataInsertCommandFlag
                uint32_t                 EmulationflagEmulationbytebitsinsertenable       : __CODEGEN_BITFIELD( 3,  3)    ; //!< EMULATIONFLAG__EMULATIONBYTEBITSINSERTENABLE
                uint32_t                 SkipemulbytecntSkipEmulationByteCount            : __CODEGEN_BITFIELD( 4,  7)    ; //!< SkipEmulByteCnt - Skip Emulation Byte Count
                uint32_t                 DatabitsinlastdwSrcdataendingbitinclusion50      : __CODEGEN_BITFIELD( 8, 13)    ; //!< DataBitsInLastDW - SrCDataEndingBitInclusion[5:0]
                uint32_t                 SliceHeaderIndicator                             : __CODEGEN_BITFIELD(14, 14)    ; //!< SLICE_HEADER_INDICATOR
                uint32_t                 Headerlengthexcludefrmsize                       : __CODEGEN_BITFIELD(15, 15)    ; //!< HEADERLENGTHEXCLUDEFRMSIZE_
                uint32_t                 DatabyteoffsetSrcdatastartingbyteoffset10        : __CODEGEN_BITFIELD(16, 17)    ; //!< DataByteOffset - SrcDataStartingByteOffset[1:0]
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(18, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED2                                             = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFXCOMMON                                   = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXPAKINSERTOBJECT                                      = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief BITSTREAMSTARTRESET__RESETBITSTREAMSTARTINGPOS
        //! \details
        //!     Must be set to 1 for JPEG encoder
        enum BITSTREAMSTARTRESET__RESETBITSTREAMSTARTINGPOS
        {
            BITSTREAMSTARTRESET_RESETBITSTREAMSTARTINGPOS_INSERT             = 0, //!< Insert the current command inline data starting at the current bitstream buffer insertion position
            BITSTREAMSTARTRESET_RESETBITSTREAMSTARTINGPOS_RESET              = 1, //!< Reset the bitstream buffer insertion position to the bitstream buffer starting position.
        };

        //! \brief EMULATIONFLAG__EMULATIONBYTEBITSINSERTENABLE
        //! \details
        //!     Must be set to 0 for JPEG encoder
        enum EMULATIONFLAG__EMULATIONBYTEBITSINSERTENABLE
        {
            EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE_NONE                 = 0, //!< No emulation
            EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE_EMULATE              = 1, //!< Instruct the hardware to perform Start Code Prefix (0x 00 00 01/02/03/00) Search and Prevention Byte (0x 03) insertion on the insertion data of this command. It is required that hardware will handle a start code prefix crossing the boundary between insertion commands, or an insertion command followed by a PAK Object command.
        };

        //! \brief SLICE_HEADER_INDICATOR
        //! \details
        //!     This bit indicates if the insert object is a slice header. In the VDEnc
        //!     mode, PAK only gets this command at the beginning of the frame for slice
        //!     position X=0, Y=0. It internally generates the header that needs to be
        //!     inserted per slice. For VDEnc mode, this bit should always be set.
        enum SLICE_HEADER_INDICATOR
        {
            SLICE_HEADER_INDICATOR_LEGACY                                    = 0, //!< Legacy Insertion Object command. The PAK Insertion Object command is not stored in HW.
            SLICE_HEADER_INDICATOR_SLICEHEADER                               = 1, //!< Insertion Object is a Slice Header. The command is stored internally by HW and is used for inserting slice headers.
        };

        //! \brief HEADERLENGTHEXCLUDEFRMSIZE_
        //! \details
        //!     In case this flag is on, bits are NOT accumulated during current access
        //!     unit coding neither for Cabac Zero Word insertion bits counting or  for
        //!     output in MMIO register MFC_BITSTREAM_BYTECOUNT_FRAME_NO_HEADER. 
        //!                         When using HeaderLenghtExcludeFrmSize for header insertion, the
        //!     software needs to make sure that data comes already with inserted start
        //!     code emulation bytes. SW shouldn't set EmulationFlag bit ( Bit 3 of
        //!     DWORD1 of MFX_PAK_INSERT_OBJECT).
        enum HEADERLENGTHEXCLUDEFRMSIZE_
        {
            HEADERLENGTHEXCLUDEFRMSIZE_ACCUMULATE                            = 0, //!< All bits accumulated
            HEADERLENGTHEXCLUDEFRMSIZE_NOACCUMULATION                        = 1, //!< Bits during current call are not accumulated
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_PAK_INSERT_OBJECT_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief MFX_MPEG2_PIC_STATE
    //! \details
    //!     This must be the very first command to issue after the surface state,
    //!     the pipe select and base address setting commands. For MPEG-2 the
    //!     encoder is called per slice-group, however the picture state is called
    //!     per picture.Notice that a slice-group is a group of consecutive slices
    //!     that no non-trivial slice headers are inserted in between.
    //!     
    struct MFX_MPEG2_PIC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 ScanOrder                                        : __CODEGEN_BITFIELD( 6,  6)    ; //!< SCAN_ORDER
                uint32_t                 IntraVlcFormat                                   : __CODEGEN_BITFIELD( 7,  7)    ; //!< Intra VLC Format
                uint32_t                 QuantizerScaleType                               : __CODEGEN_BITFIELD( 8,  8)    ; //!< QUANTIZER_SCALE_TYPE
                uint32_t                 ConcealmentMotionVectorFlag                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Concealment Motion Vector Flag
                uint32_t                 FramePredictionFrameDct                          : __CODEGEN_BITFIELD(10, 10)    ; //!< Frame Prediction Frame DCT
                uint32_t                 TffTopFieldFirst                                 : __CODEGEN_BITFIELD(11, 11)    ; //!< TFF (Top Field First)
                uint32_t                 PictureStructure                                 : __CODEGEN_BITFIELD(12, 13)    ; //!< Picture Structure
                uint32_t                 IntraDcPrecision                                 : __CODEGEN_BITFIELD(14, 15)    ; //!< Intra DC Precision
                uint32_t                 FCode00                                          : __CODEGEN_BITFIELD(16, 19)    ; //!< f_code[0][0]
                uint32_t                 FCode01                                          : __CODEGEN_BITFIELD(20, 23)    ; //!< f_code[0][1]
                uint32_t                 FCode10                                          : __CODEGEN_BITFIELD(24, 27)    ; //!< f_code[1][0].
                uint32_t                 FCode11                                          : __CODEGEN_BITFIELD(28, 31)    ; //!< f_code[1][1].
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 DisableMismatch                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< Disable Mismatch
                uint32_t                 Mismatchcontroldisabled                          : __CODEGEN_BITFIELD( 1,  1)    ; //!< MISMATCHCONTROLDISABLED
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 2,  8)    ; //!< Reserved
                uint32_t                 PictureCodingType                                : __CODEGEN_BITFIELD( 9, 10)    ; //!< PICTURE_CODING_TYPE
                uint32_t                 Reserved75                                       : __CODEGEN_BITFIELD(11, 13)    ; //!< Reserved
                uint32_t                 LoadslicepointerflagLoadbitstreampointerperslice : __CODEGEN_BITFIELD(14, 14)    ; //!< LOADSLICEPOINTERFLAG__LOADBITSTREAMPOINTERPERSLICE
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 23)    ; //!< Reserved
                uint32_t                 PBSlicePredictedMotionVectorOverrideFinalMvValueOverride : __CODEGEN_BITFIELD(24, 24)    ; //!< PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE
                uint32_t                 PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride : __CODEGEN_BITFIELD(25, 26)    ; //!< PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE__BI_DIRECTION_MV_TYPE_OVERRIDE
                uint32_t                 Reserved91                                       : __CODEGEN_BITFIELD(27, 27)    ; //!< Reserved
                uint32_t                 PBSliceConcealmentMode                           : __CODEGEN_BITFIELD(28, 29)    ; //!< PB_SLICE_CONCEALMENT_MODE_
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 30)    ; //!< Reserved
                uint32_t                 ISliceConcealmentMode                            : __CODEGEN_BITFIELD(31, 31)    ; //!< I_SLICE_CONCEALMENT_MODE_
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Framewidthinmbsminus170PictureWidthInMacroblocks : __CODEGEN_BITFIELD( 0,  7)    ; //!< FrameWidthInMBsMinus1[7:0] (Picture Width in Macroblocks) 
                uint32_t                 Reserved104                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Frameheightinmbsminus170PictureHeightInMacroblocks : __CODEGEN_BITFIELD(16, 23)    ; //!< FrameHeightInMBsMinus1[7:0] (Picture Height in Macroblocks) 
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 30)    ; //!< Reserved
                uint32_t                 SliceConcealmentDisableBit                       : __CODEGEN_BITFIELD(31, 31)    ; //!< SLICE_CONCEALMENT_DISABLE_BIT
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 Roundintradc                                     : __CODEGEN_BITFIELD( 1,  2)    ; //!< RoundIntraDC
                uint32_t                 Reserved131                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 Roundinterdc                                     : __CODEGEN_BITFIELD( 4,  6)    ; //!< RoundInterDC
                uint32_t                 Reserved135                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 Roundintraac                                     : __CODEGEN_BITFIELD( 8, 10)    ; //!< RoundIntraAC
                uint32_t                 Reserved139                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 Roundinterac                                     : __CODEGEN_BITFIELD(12, 14)    ; //!< RoundInterAC,
                uint32_t                 Mbstatenabled                                    : __CODEGEN_BITFIELD(15, 15)    ; //!< MBSTATENABLED
                uint32_t                 Minframewsize                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< MINFRAMEWSIZE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Intrambmaxsizereportmask                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< INTRAMBMAXSIZEREPORTMASK
                uint32_t                 Intermbmaxsizereportmask                         : __CODEGEN_BITFIELD( 1,  1)    ; //!< INTERMBMAXSIZEREPORTMASK
                uint32_t                 Framebitratemaxreportmask                        : __CODEGEN_BITFIELD( 2,  2)    ; //!< FRAMEBITRATEMAXREPORTMASK_
                uint32_t                 Framebitrateminreportmask                        : __CODEGEN_BITFIELD( 3,  3)    ; //!< FRAMEBITRATEMINREPORTMASK
                uint32_t                 Reserved164                                      : __CODEGEN_BITFIELD( 4,  8)    ; //!< Reserved
                uint32_t                 Mbratecontrolmask                                : __CODEGEN_BITFIELD( 9,  9)    ; //!< MBRATECONTROLMASK
                uint32_t                 Minframewsizeunits                               : __CODEGEN_BITFIELD(10, 11)    ; //!< MINFRAMEWSIZEUNITS
                uint32_t                 Intermbforcecbpzerocontrolmask                   : __CODEGEN_BITFIELD(12, 12)    ; //!< INTERMBFORCECBPZEROCONTROLMASK
                uint32_t                 Reserved173                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 Framesizecontrolmask                             : __CODEGEN_BITFIELD(16, 16)    ; //!< FRAMESIZECONTROLMASK
                uint32_t                 Reserved177                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Intrambmaxsize                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< INTRAMBMAXSIZE
                uint32_t                 Reserved204                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Intermbmaxsize                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< INTERMBMAXSIZE
                uint32_t                 Reserved220                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 VslTopMbTrans8X8Flag                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< VSL_TOP_MB_TRANS8X8FLAG
                uint32_t                 Reserved225                                      : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Slicedeltaqpmax0                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< SliceDeltaQPMax[0] 
                uint32_t                 Slicedeltaqpmax1                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< SliceDeltaQPMax[1]
                uint32_t                 Slicedeltaqpmax2                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< SliceDeltaQPMax[2] 
                uint32_t                 Slicedeltaqpmax3                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< SLICEDELTAQPMAX3
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Slicedeltaqpmin0                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< SliceDeltaQPMin[0] 
                uint32_t                 Slicedeltaqpmin1                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< SliceDeltaQPMin[1] 
                uint32_t                 Slicedeltaqpmin2                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< SliceDeltaQPMin[2] 
                uint32_t                 Slicedeltaqpmin3                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< SliceDeltaQPMin[3] 
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Framebitratemin                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMin 
                uint32_t                 Framebitrateminunitmode                          : __CODEGEN_BITFIELD(14, 14)    ; //!< FRAMEBITRATEMINUNITMODE
                uint32_t                 Framebitrateminunit                              : __CODEGEN_BITFIELD(15, 15)    ; //!< FRAMEBITRATEMINUNIT
                uint32_t                 Framebitratemax                                  : __CODEGEN_BITFIELD(16, 29)    ; //!< FrameBitRateMax
                uint32_t                 Framebitratemaxunitmode                          : __CODEGEN_BITFIELD(30, 30)    ; //!< FRAMEBITRATEMAXUNITMODE
                uint32_t                 Framebitratemaxunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMAXUNIT_
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Framebitratemindelta                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< FrameBitRateMinDelta
                uint32_t                 Reserved367                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Framebitratemaxdelta                             : __CODEGEN_BITFIELD(16, 30)    ; //!< FRAMEBITRATEMAXDELTA
                uint32_t                 Reserved383                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 Reserved384                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED0                                             = 0, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MPEG2COMMON                                 = 3, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMPEG2PICSTATE                                        = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SCAN_ORDER
        //! \details
        //!     This field specifies the Inverse Scan method for the DCT-domain
        //!     coefficients in the blocks of the current picture.
        enum SCAN_ORDER
        {
            SCAN_ORDER_UNNAMED0                                              = 0, //!< MPEG_ZIGZAG_SCAN
            SCAN_ORDER_UNNAMED1                                              = 1, //!< MPEG_ALTERNATE_VERTICAL_SCAN
        };

        //! \brief QUANTIZER_SCALE_TYPE
        //! \details
        //!     This field specifies the quantizer scaling type.
        enum QUANTIZER_SCALE_TYPE
        {
            QUANTIZER_SCALE_TYPE_UNNAMED0                                    = 0, //!< MPEG_QSCALE_LINEAR
            QUANTIZER_SCALE_TYPE_UNNAMED1                                    = 1, //!< D MPEG_QSCALE_NONLINEAR esc
        };

        //! \brief MISMATCHCONTROLDISABLED
        //! \details
        //!     These 2 bits flag disables mismatch control of the inverse
        //!     transformation for some specific cases during reference reconstruction.
        enum MISMATCHCONTROLDISABLED
        {
            MISMATCHCONTROLDISABLED_UNNAMED0                                 = 0, //!< Mismatch control applies to all MBs
            MISMATCHCONTROLDISABLED_UNNAMED1                                 = 1, //!< Disable mismatch control to all intra MBs whose all AC-coefficients are zero.
            MISMATCHCONTROLDISABLED_UNNAMED2                                 = 2, //!< Disable mismatch control to all MBs whose all AC-coefficients are zero.
            MISMATCHCONTROLDISABLED_UNNAMED3                                 = 3, //!< Disable mismatch control to all MBs.
        };

        //! \brief PICTURE_CODING_TYPE
        //! \details
        //!     This field identifies whether the picture is an intra-coded picture (I),
        //!     predictive-coded picture (P) or bi-directionally predictive-coded
        //!     picture (B). See ISO/IEC 13818-2 6.3.9 for details.
        enum PICTURE_CODING_TYPE
        {
            PICTURE_CODING_TYPE_MPEGIPICTURE                                 = 1, //!< No additional details
            PICTURE_CODING_TYPE_10MPEGPPICTURE                               = 2, //!< No additional details
            PICTURE_CODING_TYPE_MPEGBPICTURE                                 = 3, //!< No additional details
        };

        //! \brief LOADSLICEPOINTERFLAG__LOADBITSTREAMPOINTERPERSLICE
        //! \details
        //!     To support multiple slice picture and additional header/data insertion
        //!     before and after an encoded slice.When this field is set to 0, bitstream
        //!     pointer is only loaded once for the first slice of a frame. For
        //!     subsequent slices in the frame, bitstream data are stitched together to
        //!     form a single output data stream.When this field is set to 1, bitstream
        //!     pointer is loaded for each slice of a frame. Basically bitstream data
        //!     for different slices of a frame will be written to different memory
        //!     locations.
        enum LOADSLICEPOINTERFLAG__LOADBITSTREAMPOINTERPERSLICE
        {
            LOADSLICEPOINTERFLAG_LOADBITSTREAMPOINTERPERSLICE_UNNAMED0       = 0, //!< Load BitStream Pointer only once for the first slice of a frame
            LOADSLICEPOINTERFLAG_LOADBITSTREAMPOINTERPERSLICE_UNNAMED1       = 1, //!< Load/reload BitStream Pointer only once for the each slice, reload the start location of the bitstream buffer from the Indirect PAK-BSE Object Data Start Address field
        };

        //! \brief PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE
        //! \details
        //!     This field is only applicable if the Concealment Motion Vectors are
        //!     non-zero.
        //!                         It is only possible  if "P/B Slice Concealment Mode" is set to "00"
        //!     or "01" and left MB has non-zero motion vectors).
        enum PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE
        {
            PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE_PREDICTED = 0, //!< Motion Vectors use predicted values
            PB_SLICE_PREDICTED_MOTION_VECTOR_OVERRIDE_FINAL_MV_VALUE_OVERRIDE_ZERO = 1, //!< Motion Vectors force to 0
        };

        //! \brief PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE__BI_DIRECTION_MV_TYPE_OVERRIDE
        //! \details
        //!     This field is only applicable if the Concealment Motion Type is
        //!     predicted to be Bi-directional. 
        //!                         (It is only possible if "P/B Slice Concealment Mode" is set to "00"
        //!     or "01" and left MB is a bi-directional MB).
        enum PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE__BI_DIRECTION_MV_TYPE_OVERRIDE
        {
            PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE_BI_DIRECTION_MV_TYPE_OVERRIDE_BID = 0, //!< Keep Bi-direction Prediction
            PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE_BI_DIRECTION_MV_TYPE_OVERRIDE_FWD = 2, //!< Only use Forward Prediction (Backward MV is forced to invalid
            PB_SLICE_PREDICTED_BIDIR_MOTION_TYPE_OVERRIDE_BI_DIRECTION_MV_TYPE_OVERRIDE_BWD = 3, //!< Only use Backward Prediction (Forward MV is forced to invalid)
        };

        //! \brief PB_SLICE_CONCEALMENT_MODE_
        //! \details
        //!     This field controls how MPEG decoder handles MB concealment in P/B
        //!     Slice.
        enum PB_SLICE_CONCEALMENT_MODE_
        {
            PB_SLICE_CONCEALMENT_MODE_INTER                                  = 0, //!< If left MB is NOT Intra MB type (including skipMB), use left MB inter prediction mode [frame/field or forward/backward/bi] and MV final values as concealment.Otherwise (left MB is Intra MB), use forward reference (same polarity for field pic) with MV final values set to 0.
            PB_SLICE_CONCEALMENT_MODE_LEFT                                   = 1, //!< If left MB is NOT Intra MB type (including skipMB), use left MB inter prediction mode [frame/field or forward/backward/bi] and MV final values as concealment.Otherwise (left MB is Intra MB), use left MB dct_dc_pred[cc] values for concealment (Macroblock is concealed as INTRA MB and dct_dc_pred[cc] are DC predictor for Luma, Cr, Cb data)
            PB_SLICE_CONCEALMENT_MODE_ZERO                                   = 2, //!< Always use forward reference (same polarity for field pic) with MV final values set to 0 (Macroblock is concealed as INTER coded)
            PB_SLICE_CONCEALMENT_MODE_INTRA                                  = 3, //!< Use left MB dct_dc_pred[cc] values for concealment (Macroblock is concealed as INTRA MB and dct_dc_pred[cc] are DC predictor for Luma, Cr, Cb data
        };

        //! \brief I_SLICE_CONCEALMENT_MODE_
        //! \details
        //!     This field controls how MPEG decoder handles MB concealment in I Slice
        enum I_SLICE_CONCEALMENT_MODE_
        {
            I_SLICE_CONCEALMENT_MODE_INTRACONCEALMENT                        = 0, //!< Using Coefficient values to handle MB concealment
            I_SLICE_CONCEALMENT_MODE_INTERCONCEALMENT                        = 1, //!< Using Motion Vectors to handle MB concealment
        };

        //! \brief SLICE_CONCEALMENT_DISABLE_BIT
        //! \details
        //!     If VINunit detects the next slice starting position is either
        //!     out-of-bound or smaller than or equal to the current slice starting
        //!     position, VIN will set the current slice to be 1 MB and force VMDunit to
        //!     do slice concealment on the next slice.
        //!                         This bit will disable this feature and the MB data from the next
        //!     slice will be decoded from bitstream.
        enum SLICE_CONCEALMENT_DISABLE_BIT
        {
            SLICE_CONCEALMENT_DISABLE_BIT_ENABLE                             = 0, //!< VIN will force next slice to be concealment if detects slice boundary error
            SLICE_CONCEALMENT_DISABLE_BIT_DISABLE                            = 1, //!< VIN will not force next slice to be in concealment
        };

        //! \brief MINFRAMEWSIZE
        //! \details
        //!     - Minimum Frame Size [15:0] (16-bit) (Encoder Only)Mininum Frame Size is
        //!     specified to compensate for intel Rate ControlCurrently zero fill (no
        //!     need to perform emulation byte insertion) is done only to the end of the
        //!     CABAC_ZERO_WORD insertion (if any) at the last slice of a picture. Intel
        //!     encoder parameter, not part of . The caller should always make sure
        //!     that the value, represented by  Mininum Frame Size, is always less than
        //!     maximum frame size FrameBitRateMax (DWORD 10 bits 29:16). This field is
        //!     reserved in Decode mode.
        enum MINFRAMEWSIZE
        {
            MINFRAMEWSIZE_UNNAMED0                                           = 0, //!< No additional details
        };

        //! \brief INTRAMBMAXSIZEREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of any intra MB in the
        //!     frame exceeds IntraMBMaxSize.
        enum INTRAMBMAXSIZEREPORTMASK
        {
            INTRAMBMAXSIZEREPORTMASK_UNNAMED0                                = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            INTRAMBMAXSIZEREPORTMASK_UNNAMED1                                = 1, //!< set bit0 of MFC_IMAGE_STATUS control register if the total bit counter for the current MB is greater than the Intra MB Conformance Max size limit.
        };

        //! \brief INTERMBMAXSIZEREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of any inter MB in the
        //!     frame exceeds InterMBMaxSize.
        enum INTERMBMAXSIZEREPORTMASK
        {
            INTERMBMAXSIZEREPORTMASK_UNNAMED0                                = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            INTERMBMAXSIZEREPORTMASK_UNNAMED1                                = 1, //!< set bit0 of MFC_IMAGE_STATUS control register if the total bit counter for the current MB is greater than the Inter MB Conformance Max size limit.
        };

        //! \brief FRAMEBITRATEMAXREPORTMASK_
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     exceeds FrameBitRateMax.
        enum FRAMEBITRATEMAXREPORTMASK_
        {
            FRAMEBITRATEMAXREPORTMASK_DISABLE                                = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            FRAMEBITRATEMAXREPORTMASK_ENABLE                                 = 1, //!< set bit0 and bit 1 of MFC_IMAGE_STATUS control register if the total frame level bit counter is greater than or equal to Frame Bit rate Maximum limit.
        };

        //! \brief FRAMEBITRATEMINREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     is less than FrameBitRateMin.
        enum FRAMEBITRATEMINREPORTMASK
        {
            FRAMEBITRATEMINREPORTMASK_DISABLE                                = 0, //!< Do not update bit0 of MFC_IMAGE_STATUS control register.
            FRAMEBITRATEMINREPORTMASK_ENABLE                                 = 1, //!< set bit0 and bit 1of MFC_IMAGE_STATUS control register if the total frame level bit counter is less than or equal to Frame Bit rate Minimum limit.
        };

        //! \brief MBRATECONTROLMASK
        //! \details
        //!     MB Rate Control conformance maskThis field is ignored when
        //!     MacroblockStatEnable is disabled or MB level Rate control flag for the
        //!     current MB is disable in Macroblock Status Buffer.
        enum MBRATECONTROLMASK
        {
            MBRATECONTROLMASK_UNNAMED0                                       = 0, //!< Do not change QP values of inter macroblock with suggested QP values in Macroblock Status Buffer
            MBRATECONTROLMASK_UNNAMED1                                       = 1, //!< Apply RC QP delta for all macroblock
        };

        //! \brief MINFRAMEWSIZEUNITS
        //! \details
        //!     This field is the Minimum Frame Size Units
        enum MINFRAMEWSIZEUNITS
        {
            MINFRAMEWSIZEUNITS_COMPATIBILITYMODE                             = 0, //!< Minimum Frame Size is in old mode (words, 2bytes)
            MINFRAMEWSIZEUNITS_16BYTE                                        = 1, //!< Minimum Frame Size is in 16bytes
            MINFRAMEWSIZEUNITS_4KB                                           = 2, //!< Minimum Frame Size is in 4Kbytes
            MINFRAMEWSIZEUNITS_16KB                                          = 3, //!< Minimum Frame Size is in 16Kbytes
        };

        //! \brief INTERMBFORCECBPZEROCONTROLMASK
        //! \details
        //!     Inter MB Force CBP ZERO mask.
        enum INTERMBFORCECBPZEROCONTROLMASK
        {
            INTERMBFORCECBPZEROCONTROLMASK_UNNAMED0                          = 0, //!< No effect
            INTERMBFORCECBPZEROCONTROLMASK_UNNAMED1                          = 1, //!< Zero out all A/C coefficients for the inter MB violating Inter Confirmance
        };

        //! \brief FRAMESIZECONTROLMASK
        //! \details
        //!     Frame size conformance maskThis field is used when MacroblockStatEnable
        //!     is set to 1.
        enum FRAMESIZECONTROLMASK
        {
            FRAMESIZECONTROLMASK_UNNAMED0                                    = 0, //!< Do not change Slice Quantization Parameter values in MFC_MPEG2_SLICEGROUP_STATE with suggested slice QP value for frame level Rate control
            FRAMESIZECONTROLMASK_UNNAMED1                                    = 1, //!< Replace Slice Quantization Parameter values in MFC_MPEG2_SLICEGROUP_STATE with suggested slice QP value for frame level Rate control values in MFC_IMAGE_STATUS control register.
        };

        //! \brief INTRAMBMAXSIZE
        //! \details
        //!     This field, Intra MB Conformance Max size limit,indicates the allowed
        //!     max bit count size for Intra MB
        enum INTRAMBMAXSIZE
        {
            INTRAMBMAXSIZE_UNNAMED_4095                                      = 4095, //!< No additional details
        };

        //! \brief INTERMBMAXSIZE
        //! \details
        //!     This field, Inter MB Conformance Max size limit,indicates the allowed
        //!     max bit count size for Inter MB
        enum INTERMBMAXSIZE
        {
            INTERMBMAXSIZE_UNNAMED_4095                                      = 4095, //!< No additional details
        };

        enum VSL_TOP_MB_TRANS8X8FLAG
        {
            VSL_TOP_MB_TRANS8X8FLAG_DISABLE                                  = 0, //!< VSL  will only fetch the current MB data.
            VSL_TOP_MB_TRANS8X8FLAG_ENABLE                                   = 1, //!< When this bit is set VSL will make extra fetch to memory to fetch the MB data for top MB.
        };

        //! \brief SLICEDELTAQPMAX3
        //! \details
        //!     This field is the Slice level delta QP for total
        //!                         bit-count above FrameBitRateMax - first 1/8 regionThis field is
        //!     used to
        //!                         calculate the suggested slice QP into the MFC_IMAGE_STATUS control
        //!     register when
        //!                         total bit count for the entire frame exceeds FrameBitRateMax but is
        //!     within 1/8
        //!                         of FrameBitRateMaxDelta above FrameBitRateMax, i.e., in the range
        //!     of
        //!                         (FrameBitRateMax, (FrameBitRateMax+
        //!                         FrameBitRateMaxDelta&gt;&gt;3).
        enum SLICEDELTAQPMAX3
        {
            SLICEDELTAQPMAX3_DISABLE                                         = 0, //!< No additional details
            SLICEDELTAQPMAX3_ENABLE                                          = 1, //!< No additional details
        };

        //! \brief FRAMEBITRATEMINUNITMODE
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit
        //!     Units.ValueNameDescriptionProject
        enum FRAMEBITRATEMINUNITMODE
        {
            FRAMEBITRATEMINUNITMODE_COMPATIBILITYMODE                        = 0, //!< FrameBitRateMaxUnit is in old mode (128b/16Kb)
            FRAMEBITRATEMINUNITMODE_NEWMODE                                  = 1, //!< FrameBitRateMaxUnit is in new mode (32byte/4Kb)
        };

        //! \brief FRAMEBITRATEMINUNIT
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit Units.
        enum FRAMEBITRATEMINUNIT
        {
            FRAMEBITRATEMINUNIT_BYTE                                         = 0, //!< FrameBitRateMax is in units of 32 Bytes when FrameBitrateMinUnitMode is 1 and in units of 128 Bytes if FrameBitrateMinUnitMode is 0
            FRAMEBITRATEMINUNIT_KILOBYTE                                     = 1, //!< FrameBitRateMax is in units of 4KBytes Bytes when FrameBitrateMaxUnitMode is 1 and in units of 16KBytes if FrameBitrateMaxUnitMode is 0
        };

        //! \brief FRAMEBITRATEMAXUNITMODE
        //! \details
        //!     BitFiel This field is the Frame Bitrate Maximum Limit Units.dDesc
        enum FRAMEBITRATEMAXUNITMODE
        {
            FRAMEBITRATEMAXUNITMODE_COMPATIBILITYMODE                        = 0, //!< FrameBitRateMaxUnit is in old mode (128b/16Kb)
            FRAMEBITRATEMAXUNITMODE_NEWMODE                                  = 1, //!< FrameBitRateMaxUnit is in new mode (32byte/4Kb)
        };

        //! \brief FRAMEBITRATEMAXUNIT_
        //! \details
        //!     This field is the Frame Bitrate Maximum Limit Units.
        enum FRAMEBITRATEMAXUNIT_
        {
            FRAMEBITRATEMAXUNIT_BYTE                                         = 0, //!< FrameBitRateMax is in units of 32 Bytes when FrameBitrateMaxUnitMode is 1 and in units of 128 Bytes if FrameBitrateMaxUnitMode is 0
            FRAMEBITRATEMAXUNIT_KILOBYTE                                     = 1, //!< FrameBitRateMax is in units of 4KBytes Bytes when FrameBitrateMaxUnitMode is 1 and in units of 16KBytes if FrameBitrateMaxUnitMode is 0
        };

        //! \brief FRAMEBITRATEMAXDELTA
        //! \details
        //!     This field is used to select the slice delta QP when FrameBitRateMax Is
        //!     exceeded. It shares the same FrameBitrateMaxUnit.
        //!                     The programmable range is either 0- 512KB or 4MBB in
        //!     FrameBitrateMaxUnit of 128 Bytes or 16KB respectively.
        enum FRAMEBITRATEMAXDELTA
        {
            FRAMEBITRATEMAXDELTA_UNNAMED0                                    = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_MPEG2_PIC_STATE_CMD();

        static const size_t dwSize = 13;
        static const size_t byteSize = 52;
    };

    //!
    //! \brief MFD_MPEG2_BSD_OBJECT
    //! \details
    //!     Different from AVC and VC1, MFD_MPEG2_BSD_OBJECT command is pipelinable.
    //!     This is for performance purpose as in MPEG2 a slice is defined as a
    //!     group of MBs of any size that must be within a macroblock row.Slice
    //!     header parameters are passed in as inline data and the bitstream data
    //!     for the slice is passed in as indirect data. Of the inline data,
    //!     slice_horizontal_position and slice_vertical_position determines the
    //!     location within the destination picture of the first macroblock in the
    //!     slice. The content in this command is identical to that in the
    //!     MEDIA_OBJECT command in VLD mode described in the Media Chapter.
    //!     
    struct MFD_MPEG2_BSD_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectBsdDataLength                                                            ; //!< Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 FirstMacroblockBitOffset                         : __CODEGEN_BITFIELD( 0,  2)    ; //!< First Macroblock Bit Offset
                uint32_t                 IsLastMb                                         : __CODEGEN_BITFIELD( 3,  3)    ; //!< IS_LAST_MB
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 LastPicSlice                                     : __CODEGEN_BITFIELD( 5,  5)    ; //!< LAST_PIC_SLICE
                uint32_t                 SliceConcealmentTypeBit                          : __CODEGEN_BITFIELD( 6,  6)    ; //!< SLICE_CONCEALMENT_TYPE_BIT
                uint32_t                 SliceConcealmentOverrideBit                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< SLICE_CONCEALMENT_OVERRIDE_BIT
                uint32_t                 MacroblockCount                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< Macroblock Count
                uint32_t                 SliceVerticalPosition                            : __CODEGEN_BITFIELD(16, 23)    ; //!< Slice Vertical Position
                uint32_t                 SliceHorizontalPosition                          : __CODEGEN_BITFIELD(24, 31)    ; //!< Slice Horizontal Position
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 NextSliceHorizontalPosition                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Next Slice Horizontal Position
                uint32_t                 NextSliceVerticalPosition                        : __CODEGEN_BITFIELD( 8, 16)    ; //!< Next Slice Vertical Position
                uint32_t                 Reserved145                                      : __CODEGEN_BITFIELD(17, 23)    ; //!< Reserved
                uint32_t                 QuantizerScaleCode                               : __CODEGEN_BITFIELD(24, 28)    ; //!< Quantizer Scale Code
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MPEG2DEC                                    = 3, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDMPEG2BSDOBJECT                                       = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum IS_LAST_MB
        {
            IS_LAST_MB_UNNAMED0                                              = 0, //!< The current MB is not the last MB in the current Slice
            IS_LAST_MB_UNNAMED1                                              = 1, //!< The current MB is the last MB in the current Slice
        };

        //! \brief LAST_PIC_SLICE
        //! \details
        //!     This bit is added to support error concealment at the end of a picture.
        enum LAST_PIC_SLICE
        {
            LAST_PIC_SLICE_UNNAMED0                                          = 0, //!< The current Slice is not the last Slice of current picture
            LAST_PIC_SLICE_UNNAMED1                                          = 1, //!< The current Slice is the last Slice of the entire picture
        };

        //! \brief SLICE_CONCEALMENT_TYPE_BIT
        //! \details
        //!     This bit can be forced by driver ("Slice Concealment Override Bit") or
        //!     set by VINunit depending on slice boundary errors.
        enum SLICE_CONCEALMENT_TYPE_BIT
        {
            SLICE_CONCEALMENT_TYPE_BIT_UNNAMED0                              = 0, //!< VMD will decode MBs from the bitstream until the bitstream is run-out.  Then VMD will conceal the remaining MBs.
            SLICE_CONCEALMENT_TYPE_BIT_UNNAMED1                              = 1, //!< VMD will conceal all MBs of the slice regardless of bitstream. (If driver does not force the value of this bit, VIN will set this bit depending on slice boundary error.  If the next slice position of the current slice is out-of-bound or the same or earlier than the current slice start position, VIN will set this bit for the next slice)
        };

        //! \brief SLICE_CONCEALMENT_OVERRIDE_BIT
        //! \details
        //!     This bit forces hardware to handle the current slice in Conceal or
        //!     Deocde Mode.  If this bit is set to one, VIN will force the current
        //!     slice to do concealment or to decode from bitstream regardless if the
        //!     slice boundary has errors or not.
        enum SLICE_CONCEALMENT_OVERRIDE_BIT
        {
            SLICE_CONCEALMENT_OVERRIDE_BIT_UNNAMED0                          = 0, //!< Driver must program "Slice Concealment Type" to '0'. VIN will set "Slice Concealment Type" depending if the slice boundary has error or not
            SLICE_CONCEALMENT_OVERRIDE_BIT_UNNAMED1                          = 1, //!< VIN will use driver-provided "Slice Concealment Type" regardless of valid slice boundary
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_MPEG2_BSD_OBJECT_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MFD_IT_OBJECT_MPEG2_INLINE_DATA
    //! \details
    //!     The content in this command is similar to that in the MEDIA_OBJECT
    //!     command in IS mode described in the Media Chapter. Each MFD_IT_OBJECT
    //!     command corresponds to the processing of one macroblock. Macroblock
    //!     parameters are passed in as inline data and the non-zero DCT coefficient
    //!     data for the macroblock is passed in as indirect data.  Inline data
    //!     starts at dword 7 of MFD_IT_OBJECT command. There are 7 dwords total.
    //!     
    struct MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0 : __CODEGEN_BITFIELD(0, 2); //!< Reserved
                uint32_t                 Lastmbinrow : __CODEGEN_BITFIELD(3, 3); //!< LastMBInRow
                uint32_t                 Reserved4 : __CODEGEN_BITFIELD(4, 5); //!< Reserved
                uint32_t                 CodedBlockPattern : __CODEGEN_BITFIELD(6, 11); //!< Coded Block Pattern
                uint32_t                 Reserved12 : __CODEGEN_BITFIELD(12, 15); //!< Reserved
                uint32_t                 MacroblockIntraType : __CODEGEN_BITFIELD(16, 16); //!< MACROBLOCK_INTRA_TYPE
                uint32_t                 MacroblockMotionForward : __CODEGEN_BITFIELD(17, 17); //!< MACROBLOCK_MOTION_FORWARD
                uint32_t                 MacroblockMotionBackward : __CODEGEN_BITFIELD(18, 18); //!< MACROBLOCK_MOTION_BACKWARD
                uint32_t                 Reserved19 : __CODEGEN_BITFIELD(19, 20); //!< Reserved
                uint32_t                 DctType : __CODEGEN_BITFIELD(21, 21); //!< DCT_TYPE
                uint32_t                 Reserved22 : __CODEGEN_BITFIELD(22, 23); //!< Reserved
                uint32_t                 MotionType : __CODEGEN_BITFIELD(24, 25); //!< Motion Type
                uint32_t                 Reserved26 : __CODEGEN_BITFIELD(26, 27); //!< Reserved
                uint32_t                 MotionVerticalFieldSelect : __CODEGEN_BITFIELD(28, 31); //!< MOTION_VERTICAL_FIELD_SELECT
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Horzorigin : __CODEGEN_BITFIELD(0, 7); //!< HorzOrigin
                uint32_t                 Vertorigin : __CODEGEN_BITFIELD(8, 15); //!< VertOrigin
                uint32_t                 Reserved48 : __CODEGEN_BITFIELD(16, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 MotionVectorsField0ForwardHorizontalComponent : __CODEGEN_BITFIELD(0, 15); //!< Motion Vectors - Field 0, Forward, Horizontal Component
                uint32_t                 MotionVectorsField0ForwardVerticalComponent : __CODEGEN_BITFIELD(16, 31); //!< Motion Vectors - Field 0, Forward, Vertical Component
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MotionVectorsField0BackwardHorizontalComponent : __CODEGEN_BITFIELD(0, 15); //!< Motion Vectors - Field 0, Backward, Horizontal Component
                uint32_t                 MotionVectorsField0BackwardVerticalComponent : __CODEGEN_BITFIELD(16, 31); //!< Motion Vectors - Field 0, Backward, Vertical Component
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 MotionVectorsField1ForwardHorizontalComponent : __CODEGEN_BITFIELD(0, 15); //!< Motion Vectors - Field 1, Forward, Horizontal Component
                uint32_t                 MotionVectorsField1ForwardVerticalComponent : __CODEGEN_BITFIELD(16, 31); //!< Motion Vectors - Field 1, Forward, Vertical Component
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 MotionVectorsField1BackwardHorizontalComponent : __CODEGEN_BITFIELD(0, 15); //!< Motion Vectors - Field 1, Backward, Horizontal Component
                uint32_t                 MotionVectorsField1BackwardVerticalComponent : __CODEGEN_BITFIELD(16, 31); //!< Motion Vectors - Field 1, Backward, Vertical Component
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        //! \brief MACROBLOCK_INTRA_TYPE
        //! \details
        //!     This field specifies if the current macroblock is intra-coded. When set,
        //!     Coded Block Pattern is ignored and no prediction is performed (i.e., no
        //!     motion vectors are used). See ISO/IEC 13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_INTRA_TYPE
        {
            MACROBLOCK_INTRA_TYPE_NON_INTRAMACROBLOCK = 0, //!< No additional details
            MACROBLOCK_INTRA_TYPE_INTRAMACROBLOCK = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_MOTION_FORWARD
        //! \details
        //!     This field specifies if the forward motion vector is active. See ISO/IEC
        //!     13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_MOTION_FORWARD
        {
            MACROBLOCK_MOTION_FORWARD_NOFORWARDMOTIONVECTOR = 0, //!< No additional details
            MACROBLOCK_MOTION_FORWARD_USEFORWARDMOTIONVECTORS = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_MOTION_BACKWARD
        //! \details
        //!     This field specifies if the backward motion vector is active. See
        //!     ISO/IEC 13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_MOTION_BACKWARD
        {
            MACROBLOCK_MOTION_BACKWARD_NOBACKWARDMOTIONVECTOR = 0, //!< No additional details
            MACROBLOCK_MOTION_BACKWARD_USEBACKWARDMOTIONVECTORS = 1, //!< No additional details
        };

        //! \brief DCT_TYPE
        //! \details
        //!     This field specifies the DCT type of the current macroblock. The kernel
        //!     should ignore this field when processing Cb/Cr data. See ISO/IEC 13818-2
        //!     #167;6.3.17.1.  This field is zero if Coded Block Pattern is also zero
        //!     (no coded blocks present).
        enum DCT_TYPE
        {
            DCT_TYPE_MCFRAMEDCT = 0, //!< Macroblock is frame DCT coded
            DCT_TYPE_MCFIELDDCT = 1, //!< Macroblock is field DCT coded
        };

        //! \brief MOTION_VERTICAL_FIELD_SELECT
        //! \details
        //!     A bit-wise representation of a long [2][2] array as defined in
        //!     #167;6.3.17.2 of the ISO/IEC 13818-2 (see also #167;7.6.4).
        //!                             <table>
        //!                                 <thead>
        //!     
        //!                                     <tr><td>Bit</td><td>MVector[r]</td><td>MVector[s]</td><td>MotionVerticalFieldSelect
        //!     Index</td></tr>
        //!                                 </thead>
        //!                                 <tbody>
        //!                                     <tr><td>28</td><td>0</td><td>0</td><td>0</td></tr>
        //!                                     <tr><td>29</td><td>0</td><td>1</td><td>1</td></tr>
        //!                                     <tr><td>30</td><td>1</td><td>0</td><td>2</td></tr>
        //!                                     <tr><td>31</td><td>1</td><td>1</td><td>3</td></tr>
        //!                                 </tbody>
        //!                             </table>
        enum MOTION_VERTICAL_FIELD_SELECT
        {
            MOTION_VERTICAL_FIELD_SELECT_TOPFIELD = 0, //!< The prediction is taken from the top reference field.
            MOTION_VERTICAL_FIELD_SELECT_BOTTOMFIELD = 1, //!< The prediction is taken from the bottom reference field.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFD_IT_OBJECT_VC1_INLINE_DATA
    //! \details
    //!     The content in this command is similar to that in the MEDIA_OBJECT
    //!     command in IS mode described in the Media Chapter. Each MFD_IT_OBJECT
    //!     command corresponds to the processing of one macroblock. Macroblock
    //!     parameters are passed in as inline data and the non-zero DCT coefficient
    //!     data for the macroblock is passed in as indirect data.  Inline data
    //!     starts at dword 7 of MFD_IT_OBJECT command. There are 7 dwords total.
    //!     
    struct MFD_IT_OBJECT_VC1_INLINE_DATA_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0 : __CODEGEN_BITFIELD(0, 2); //!< Reserved
                uint32_t                 Lastmbinrow : __CODEGEN_BITFIELD(3, 3); //!< LastMBInRow
                uint32_t                 LastRowFlag : __CODEGEN_BITFIELD(4, 4); //!<  Last Row Flag
                uint32_t                 ChromaIntraFlag : __CODEGEN_BITFIELD(5, 5); //!< Chroma Intra Flag
                uint32_t                 CodedBlockPattern : __CODEGEN_BITFIELD(6, 11); //!< Coded Block Pattern
                uint32_t                 LumaIntra8X8Flag : __CODEGEN_BITFIELD(12, 15); //!<  Luma Intra 8x8 Flag
                uint32_t                 MacroblockIntraType : __CODEGEN_BITFIELD(16, 16); //!< MACROBLOCK_INTRA_TYPE
                uint32_t                 MacroblockMotionForward : __CODEGEN_BITFIELD(17, 17); //!< MACROBLOCK_MOTION_FORWARD
                uint32_t                 MacroblockMotionBackward : __CODEGEN_BITFIELD(18, 18); //!< MACROBLOCK_MOTION_BACKWARD
                uint32_t                 Motion4Mv : __CODEGEN_BITFIELD(19, 19); //!< MOTION4MV
                uint32_t                 Overlaptransform : __CODEGEN_BITFIELD(20, 20); //!< OVERLAPTRANSFORM
                uint32_t                 DctType : __CODEGEN_BITFIELD(21, 21); //!< DCT_TYPE
                uint32_t                 Mvswitch : __CODEGEN_BITFIELD(22, 22); //!< MvSwitch
                uint32_t                 Reserved23 : __CODEGEN_BITFIELD(23, 23); //!< Reserved
                uint32_t                 MotionType : __CODEGEN_BITFIELD(24, 25); //!< Motion Type
                uint32_t                 Mvfieldselectchroma : __CODEGEN_BITFIELD(26, 26); //!< MvFieldSelectChroma
                uint32_t                 Reserved27 : __CODEGEN_BITFIELD(27, 27); //!< Reserved
                uint32_t                 MotionVerticalFieldSelect : __CODEGEN_BITFIELD(28, 31); //!< MOTION_VERTICAL_FIELD_SELECT
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Horzorigin : __CODEGEN_BITFIELD(0, 7); //!< HorzOrigin
                uint32_t                 Vertorigin : __CODEGEN_BITFIELD(8, 15); //!< VertOrigin
                uint32_t                 Osedgemaskluma : __CODEGEN_BITFIELD(16, 23); //!< OSEdgeMaskLuma
                uint32_t                 Osedgemaskchroma : __CODEGEN_BITFIELD(24, 25); //!< OSEdgeMaskChroma
                uint32_t                 Reserved58 : __CODEGEN_BITFIELD(26, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 MotionVectorsField0ForwardHorizontalComponent : __CODEGEN_BITFIELD(0, 15); //!< Motion Vectors - Field 0, Forward, Horizontal Component
                uint32_t                 MotionVectorsField0ForwardVerticalComponent : __CODEGEN_BITFIELD(16, 31); //!< Motion Vectors - Field 0, Forward, Vertical Component
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Motionvector1; //!< MotionVector1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Motionvector2; //!< MotionVector2
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Motionvector3; //!< MotionVector3
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Motionvectorchroma; //!< MotionVectorChroma
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 SubblockCodeForY0 : __CODEGEN_BITFIELD(0, 7); //!< Subblock Code for Y0
                uint32_t                 SubblockCodeForY1 : __CODEGEN_BITFIELD(8, 15); //!< Subblock Code for Y1
                uint32_t                 SubblockCodeForY2 : __CODEGEN_BITFIELD(16, 23); //!< Subblock Code for Y2
                uint32_t                 SubblockCodeForY3 : __CODEGEN_BITFIELD(24, 31); //!< Subblock Code for Y3
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 SubblockCodeForCb : __CODEGEN_BITFIELD(0, 7); //!< Subblock Code for Cb
                uint32_t                 SubblockCodeForCr : __CODEGEN_BITFIELD(8, 15); //!< Subblock Code for Cr
                uint32_t                 Reserved272 : __CODEGEN_BITFIELD(16, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 IldbControlDataForBlockY0 : __CODEGEN_BITFIELD(0, 7); //!< ILDB control data for block Y0
                uint32_t                 IldbControlDataForBlockY1 : __CODEGEN_BITFIELD(8, 15); //!< ILDB control data for block Y1
                uint32_t                 IldbControlDataForBlockY2 : __CODEGEN_BITFIELD(16, 23); //!< ILDB control data for block Y2
                uint32_t                 IldbControlDataForBlockY3 : __CODEGEN_BITFIELD(24, 31); //!< ILDB control data for block Y3
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 IldbControlDataForCbBlock : __CODEGEN_BITFIELD(0, 7); //!< ILDB control data for Cb block
                uint32_t                 IldbControlDataForCrBlock : __CODEGEN_BITFIELD(8, 15); //!< ILDB control data for Cr block
                uint32_t                 Reserved336 : __CODEGEN_BITFIELD(16, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW10;

        //! \name Local enumerations

        //! \brief MACROBLOCK_INTRA_TYPE
        //! \details
        //!     This field specifies if the current macroblock is intra-coded. When set,
        //!     Coded Block Pattern is ignored and no prediction is performed (i.e., no
        //!     motion vectors are used). See ISO/IEC 13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_INTRA_TYPE
        {
            MACROBLOCK_INTRA_TYPE_NON_INTRAMACROBLOCK = 0, //!< No additional details
            MACROBLOCK_INTRA_TYPE_INTRAMACROBLOCK = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_MOTION_FORWARD
        //! \details
        //!     This field specifies if the forward motion vector is active. See ISO/IEC
        //!     13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_MOTION_FORWARD
        {
            MACROBLOCK_MOTION_FORWARD_NOFORWARDMOTIONVECTOR = 0, //!< No additional details
            MACROBLOCK_MOTION_FORWARD_USEFORWARDMOTIONVECTORS = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_MOTION_BACKWARD
        //! \details
        //!     This field specifies if the backward motion vector is active. See
        //!     ISO/IEC 13818-2 Tables B-2 through B-4.
        enum MACROBLOCK_MOTION_BACKWARD
        {
            MACROBLOCK_MOTION_BACKWARD_NOBACKWARDMOTIONVECTOR = 0, //!< No additional details
            MACROBLOCK_MOTION_BACKWARD_USEBACKWARDMOTIONVECTORS = 1, //!< No additional details
        };

        enum MOTION4MV
        {
            MOTION4MV_1MV_MODE = 0, //!< No additional details
            MOTION4MV_4MV_MODE = 1, //!< No additional details
        };

        //! \brief OVERLAPTRANSFORM
        //! \details
        //!     Was Overlap Transform - H261 Loop Filter
        enum OVERLAPTRANSFORM
        {
            OVERLAPTRANSFORM_NOOVERLAPSMOOTHINGFILTER = 0, //!< This field indicates whether overlap smoothing filter should be performed on I-block boundaries.
            OVERLAPTRANSFORM_OVERLAPSMOOTHINGFILTERPERFORMED = 1, //!< Macroblock is field DCT coded
        };

        //! \brief DCT_TYPE
        //! \details
        //!     This field specifies the DCT type of the current macroblock. The kernel
        //!     should ignore this field when processing Cb/Cr data. See ISO/IEC 13818-2
        //!     #167;6.3.17.1.  This field is zero if Coded Block Pattern is also zero
        //!     (no coded blocks present).
        enum DCT_TYPE
        {
            DCT_TYPE_MCFRAMEDCT = 0, //!< Macroblock is frame DCT coded
            DCT_TYPE_MCFIELDDCT = 1, //!< Macroblock is field DCT coded
        };

        //! \brief MOTION_VERTICAL_FIELD_SELECT
        //! \details
        //!     A bit-wise representation of a long [2][2] array as defined in
        //!     #167;6.3.17.2 of the ISO/IEC 13818-2 (see also #167;7.6.4).
        //!                             <table>
        //!                                 <thead>
        //!     
        //!                                     <tr><td>Bit</td><td>MVector[r]</td><td>MVector[s]</td><td>MotionVerticalFieldSelect
        //!     Index</td></tr>
        //!                                 </thead>
        //!                                 <tbody>
        //!                                     <tr><td>28</td><td>0</td><td>0</td><td>0</td></tr>
        //!                                     <tr><td>29</td><td>0</td><td>1</td><td>1</td></tr>
        //!                                     <tr><td>30</td><td>1</td><td>0</td><td>2</td></tr>
        //!                                     <tr><td>31</td><td>1</td><td>1</td><td>3</td></tr>
        //!                                 </tbody>
        //!                             </table>
        enum MOTION_VERTICAL_FIELD_SELECT
        {
            MOTION_VERTICAL_FIELD_SELECT_TOPFIELD = 0, //!< The prediction is taken from the top reference field.
            MOTION_VERTICAL_FIELD_SELECT_BOTTOMFIELD = 1, //!< The prediction is taken from the bottom reference field.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_IT_OBJECT_VC1_INLINE_DATA_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief MFD_IT_OBJECT
    //! \details
    //!     All weight mode (default and implicit) are mapped to explicit mode. But
    //!     the weights come in either as explicit or implicit.
    //!     
    struct MFD_IT_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectItMvDataLength                           : __CODEGEN_BITFIELD( 0,  9)    ; //!< Indirect IT-MV Data Length
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectItMvDataStartAddressOffset               : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect IT-MV Data Start Address Offset
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 IndirectItCoeffDataLength                        : __CODEGEN_BITFIELD( 0, 11)    ; //!< Indirect IT-COEFF Data Length
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 IndirectItCoeffDataStartAddressOffset            : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect IT-COEFF Data Start Address Offset
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IndirectItDblkControlDataLength                  : __CODEGEN_BITFIELD( 0,  5)    ; //!< Indirect IT-DBLK Control Data Length
                uint32_t                 Reserved166                                      : __CODEGEN_BITFIELD( 6, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 IndirectItDblkControlDataStartAddressOffset      : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect IT-DBLK Control Data Start Address Offset
                uint32_t                 Reserved221                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED9                                             = 9, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MFXCOMMONDEC                                = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDITOBJECT                                             = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_IT_OBJECT_CMD();

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief MFC_MPEG2_SLICEGROUP_STATE
    //! \details
    //!     This is a slice group level command and can be issued multiple times
    //!     within a picture that is comprised of multiple slice groups. The same
    //!     command is used for AVC encoder (PAK mode) and decoder (VLD and IT
    //!     modes).
    //!     
    struct MFC_MPEG2_SLICEGROUP_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Streamid10EncoderOnly                            : __CODEGEN_BITFIELD( 0,  1)    ; //!< StreamID[1:0] (Encoder-only)
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved
                uint32_t                 Sliceid30EncoderOnly                             : __CODEGEN_BITFIELD( 4,  7)    ; //!< SliceID[3:0] (Encoder-only)
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 11)    ; //!< Reserved
                uint32_t                 Intrasliceflag                                   : __CODEGEN_BITFIELD(12, 12)    ; //!< IntraSliceFlag
                uint32_t                 Intraslice                                       : __CODEGEN_BITFIELD(13, 13)    ; //!< IntraSlice
                uint32_t                 Firstslicehdrdisabled                            : __CODEGEN_BITFIELD(14, 14)    ; //!< FirstSliceHdrDisabled
                uint32_t                 TailpresentflagTailInsertionPresentInBitstreamEncoderOnly : __CODEGEN_BITFIELD(15, 15)    ; //!< TAILPRESENTFLAG__TAIL_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
                uint32_t                 SlicedataPresentflagSlicedataInsertionPresentInBitstreamEncoderOnly : __CODEGEN_BITFIELD(16, 16)    ; //!< SLICEDATA_PRESENTFLAG__SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
                uint32_t                 HeaderpresentflagHeaderInsertionPresentInBitstreamEncoderOnly : __CODEGEN_BITFIELD(17, 17)    ; //!< HEADERPRESENTFLAG__HEADER_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
                uint32_t                 BitstreamoutputflagCompressedBitstreamOutputDisableFlagEncoderOnly : __CODEGEN_BITFIELD(18, 18)    ; //!< BITSTREAMOUTPUTFLAG__COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENCODER_ONLY
                uint32_t                 Islastslicegrp                                   : __CODEGEN_BITFIELD(19, 19)    ; //!< IsLastSliceGrp
                uint32_t                 SkipconvdisabledMbTypeSkipConversionDisableEncoderOnly : __CODEGEN_BITFIELD(20, 20)    ; //!< SKIPCONVDISABLED__MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY
                uint32_t                 Reserved53                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                uint32_t                 RatectrlpanictypeRcPanicTypeEncoderOnly          : __CODEGEN_BITFIELD(22, 22)    ; //!< RATECTRLPANICTYPE__RC_PANIC_TYPE_ENCODER_ONLY
                uint32_t                 RatectrlpanicflagRcPanicEnableEncoderOnly        : __CODEGEN_BITFIELD(23, 23)    ; //!< RATECTRLPANICFLAG__RC_PANIC_ENABLE_ENCODER_ONLY
                uint32_t                 MbratectrlparamRcStableToleranceEncoderOnly      : __CODEGEN_BITFIELD(24, 27)    ; //!< MbRateCtrlParam- RC Stable Tolerance (Encoder-only)
                uint32_t                 MbratectrlmodeRcTriggleModeEncoderOnly           : __CODEGEN_BITFIELD(28, 29)    ; //!< MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY
                uint32_t                 MbratectrlresetResetratecontrolcounterEncoderOnly : __CODEGEN_BITFIELD(30, 30)    ; //!< MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY
                uint32_t                 MbratectrlflagRatecontrolcounterenableEncoderOnly : __CODEGEN_BITFIELD(31, 31)    ; //!< MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FirstmbxcntAlsoCurrstarthorzpos                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< FirstMbXcnt - also CurrStartHorzPos
                uint32_t                 FirstmbycntAlsoCurrstartvertpos                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< FirstMbYcnt - also CurrStartVertPos
                uint32_t                 NextsgmbxcntAlsoNextstarthorzpos                 : __CODEGEN_BITFIELD(16, 23)    ; //!< NextSgMbXcnt - also NextStartHorzPos
                uint32_t                 NextsgmbycntAlsoNextstartvertpos                 : __CODEGEN_BITFIELD(24, 31)    ; //!< NextSgMbYcnt - also NextStartVertPos
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Slicegroupqp                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< SliceGroupQp
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Slicegroupskip                                   : __CODEGEN_BITFIELD( 8,  8)    ; //!< SliceGroupSkip
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 BitstreamoffsetIndirectPakBseDataStartAddressWrite : __CODEGEN_BITFIELD( 0, 28)    ; //!< BitstreamOffset - Indirect PAK-BSE Data Start Address (Write)
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 GrowparamGrowInitEncoderOnly                     : __CODEGEN_BITFIELD( 0,  3)    ; //!< GrowParam - Grow Init (Encoder-only)
                uint32_t                 GrowparamGrowResistanceEncoderOnly               : __CODEGEN_BITFIELD( 4,  7)    ; //!< GrowParam - Grow Resistance (Encoder-only)
                uint32_t                 ShrinkaramShrinkInitEncoderOnly                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< Shrinkaram - Shrink Init (Encoder-only)
                uint32_t                 ShrinkparamShrinkResistanceEncoderOnly           : __CODEGEN_BITFIELD(12, 15)    ; //!< ShrinkParam - Shrink Resistance (Encoder-only)
                uint32_t                 MaxqpposmodifierMagnitudeOfQpMaxPositiveModifierEncoderOnly : __CODEGEN_BITFIELD(16, 23)    ; //!< MaxQpPosModifier - Magnitude of QP Max Positive Modifier (Encoder-only)
                uint32_t                 MaxqpnegmodifierMagnitudeOfQpMaxNegativeModifierEncoderOnly : __CODEGEN_BITFIELD(24, 31)    ; //!< MaxQpNegModifier - Magnitude of QP Max Negative Modifier (Encoder-only)
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 CorrectpointsCorrect1EncoderOnly                 : __CODEGEN_BITFIELD( 0,  3)    ; //!< CorrectPoints - Correct 1 (Encoder-only)
                uint32_t                 CorrectpointsCorrect2EncoderOnly                 : __CODEGEN_BITFIELD( 4,  7)    ; //!< CorrectPoints - Correct 2 (Encoder-only)
                uint32_t                 CorrectpointsCorrect3EncoderOnly                 : __CODEGEN_BITFIELD( 8, 11)    ; //!< CorrectPoints - Correct 3 (Encoder-only)
                uint32_t                 CorrectpointsCorrect4EncoderOnly                 : __CODEGEN_BITFIELD(12, 15)    ; //!< CorrectPoints - Correct 4 (Encoder-only)
                uint32_t                 CorrectpointsCorrect5EncoderOnly                 : __CODEGEN_BITFIELD(16, 19)    ; //!< CorrectPoints - Correct 5 (Encoder-only)
                uint32_t                 CorrectpointsCorrect6EncoderOnly                 : __CODEGEN_BITFIELD(20, 23)    ; //!< CorrectPoints - Correct 6 (Encoder-only)
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Cv0ClampValue0EncoderOnly                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< CV0 - Clamp Value 0 (Encoder-only)
                uint32_t                 Cv1ClampValue1EncoderOnly                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< CV1 - Clamp Value 1 (Encoder-only)
                uint32_t                 Cv2ClampValue2EncoderOnly                        : __CODEGEN_BITFIELD( 8, 11)    ; //!< CV2 - Clamp Value 2 (Encoder-only)
                uint32_t                 Cv3ClampValue3EncoderOnly                        : __CODEGEN_BITFIELD(12, 15)    ; //!< CV3 - Clamp Value 3 (Encoder-only)
                uint32_t                 Cv4ClampValue4EncoderOnly                        : __CODEGEN_BITFIELD(16, 19)    ; //!< CV4 - Clamp Value 4 (Encoder-only)
                uint32_t                 Cv5ClampValue5EncoderOnly                        : __CODEGEN_BITFIELD(20, 23)    ; //!< CV5 - Clamp Value 5 (Encoder-only)
                uint32_t                 Cv6ClampValue6EncoderOnly                        : __CODEGEN_BITFIELD(24, 27)    ; //!< CV6 - Clamp Value 6 (Encoder-only)
                uint32_t                 Cv7ClampValue7EncoderOnly                        : __CODEGEN_BITFIELD(28, 31)    ; //!< CV7 - Clamp Value 7 (Encoder-only)
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_MEDIA                                                = 3, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_MEDIA                                                = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MPEG2                                       = 3, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMPEG2SLICEGROUPSTATE                                 = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum TAILPRESENTFLAG__TAIL_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
        {
            TAILPRESENTFLAG_TAIL_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_UNNAMED0 = 0, //!< no tail insertion into the output bitstream buffer, after the current slice encoded bits
            TAILPRESENTFLAG_TAIL_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_UNNAMED1 = 1, //!< tail insertion into the output bitstream buffer is present, and is after the current slice encoded bits.
        };

        enum SLICEDATA_PRESENTFLAG__SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
        {
            SLICEDATA_PRESENTFLAG_SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_DISABLE = 0, //!< no Slice Data insertion into the output bitstream buffer
            SLICEDATA_PRESENTFLAG_SLICEDATA_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_ENABLE = 1, //!< Slice Data insertion into the output bitstream buffer is present.
        };

        enum HEADERPRESENTFLAG__HEADER_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY
        {
            HEADERPRESENTFLAG_HEADER_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_DISABLE = 0, //!< no header insertion into the output bitstream buffer, in front of the current slice encoded bits
            HEADERPRESENTFLAG_HEADER_INSERTION_PRESENT_IN_BITSTREAM_ENCODER_ONLY_ENABLE = 1, //!< header insertion into the output bitstream buffer is present, and is in front of the current slice encoded bits.
        };

        enum BITSTREAMOUTPUTFLAG__COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENCODER_ONLY
        {
            BITSTREAMOUTPUTFLAG_COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENCODER_ONLY_ENABLE = 0, //!< enable the writing of the output compressed bitstream
            BITSTREAMOUTPUTFLAG_COMPRESSED_BITSTREAM_OUTPUT_DISABLE_FLAG_ENCODER_ONLY_DISABLE = 1, //!< disable the writing of the output compressed bitstream
        };

        //! \brief SKIPCONVDISABLED__MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY
        //! \details
        //!     This field is only valid for a P or B slice. It must be zero for other
        //!     slice types. Rules are provided in Section 2.3.3.1.6
        enum SKIPCONVDISABLED__MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY
        {
            SKIPCONVDISABLED_MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY_ENABLE = 0, //!< Enable skip type conversion
            SKIPCONVDISABLED_MB_TYPE_SKIP_CONVERSION_DISABLE_ENCODER_ONLY_DISABLE = 1, //!< Disable skip type conversion
        };

        //! \brief RATECTRLPANICTYPE__RC_PANIC_TYPE_ENCODER_ONLY
        //! \details
        //!     This field selects between two RC Panic methods. If it is set to 0, in
        //!     panic mode, the macroblock QP is maxed out, setting to requested QP +
        //!     QP_max_pos_mod. If it is set to 1, for an intra macroblock, AC CBPs are
        //!     set to zero (note that DC CBPs are not modified). For inter macroblocks,
        //!     AC and DC CBPs are forced to zero.
        enum RATECTRLPANICTYPE__RC_PANIC_TYPE_ENCODER_ONLY
        {
            RATECTRLPANICTYPE_RC_PANIC_TYPE_ENCODER_ONLY_UNNAMED0            = 0, //!< QP Panic
            RATECTRLPANICTYPE_RC_PANIC_TYPE_ENCODER_ONLY_UNNAMED1            = 1, //!< CBP Panic
        };

        //! \brief RATECTRLPANICFLAG__RC_PANIC_ENABLE_ENCODER_ONLY
        //! \details
        //!     If this field is set to 1, RC enters panic mode
        //!                         when sum_act &gt; sum_max. RC Panic Type field controls what type
        //!     of panic
        //!                         behavior is invoked.
        enum RATECTRLPANICFLAG__RC_PANIC_ENABLE_ENCODER_ONLY
        {
            RATECTRLPANICFLAG_RC_PANIC_ENABLE_ENCODER_ONLY_DISABLE           = 0, //!< No additional details
            RATECTRLPANICFLAG_RC_PANIC_ENABLE_ENCODER_ONLY_ENABLE            = 1, //!< No additional details
        };

        enum MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY
        {
            MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY_UNNAMED0             = 0, //!< Always Rate Control, whereas RC becomes activeif sum_act > sum_target or sum_act < sum_target
            MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY_UNNAMED1             = 1, //!< Gentle Rate Control, whereas RC becomes activeif sum_act > upper_midpt or sum_act < lower_midpt
            MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY_UNNAMED2             = 2, //!< Loose Rate Control, whereas RC becomes activeif sum_act > sum_max or sum_act < sum_min
            MBRATECTRLMODE_RC_TRIGGLE_MODE_ENCODER_ONLY_UNNAMED3             = 3, //!< Reserved
        };

        //! \brief MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY
        //! \details
        //!     To reset the bit allocation accumulation counter to 0 to restart the
        //!     rate control.
        enum MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY
        {
            MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY_DISABLE     = 0, //!< Not reset
            MBRATECTRLRESET_RESETRATECONTROLCOUNTER_ENCODER_ONLY_ENABLE      = 1, //!< reset
        };

        //! \brief MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY
        //! \details
        //!     To enable the accumulation of bit allocation for rate controlThis field
        //!     enables hardware Rate Control logic. The rest of the RC control fields
        //!     are only valid when this field is set to 1. Otherwise, hardware ignores
        //!     these fields.Note: To reset MB level rate control (QRC), we need to set
        //!     both bits MbRateCtrlFlag and MbRateCtrlReset to 1 in the new slice
        enum MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY
        {
            MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY_DISABLE     = 0, //!< No additional details
            MBRATECTRLFLAG_RATECONTROLCOUNTERENABLE_ENCODER_ONLY_ENABLE      = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFC_MPEG2_SLICEGROUP_STATE_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief MFX_VC1_PRED_PIPE_STATE
    //! \details
    //!     This command is used to set the operating states of the MFD Engine
    //!     beyond the BSD unit. It is used with both VC1 Long and Short
    //!     format.Driver is responsible to take the intensity compensation enable
    //!     signal, the LumScale and the LumShift provided from the VC1
    //!     interface, and maintain a history of these values for reference
    //!     pictures. Together with these three parameters specified for the current
    //!     picture being decoded, driver will derive and supply the above sets of
    //!     LumScaleX, LumShiftX and intensity compensation enable (single or
    //!     double, forward or backward) signals. H/W is responsible to take these
    //!     state values, and use them to build the lookup table (including the
    //!     derivation of iScale and iShift) for remapping the reference frame
    //!     pixels, as well as perfoming the actual pixel remapping
    //!     calculations/process.
    //!     
    struct MFX_VC1_PRED_PIPE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint32_t                 ReferenceFrameBoundaryReplicationMode            : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reference Frame Boundary Replication Mode
                uint32_t                 VinIntensitycompSingleBwden                      : __CODEGEN_BITFIELD( 8,  9)    ; //!< vin_intensitycomp_Single_BWDen
                uint32_t                 VinIntensitycompSingleFwden                      : __CODEGEN_BITFIELD(10, 11)    ; //!< vin_intensitycomp_Single_FWDen
                uint32_t                 VinIntensitycompDoubleBwden                      : __CODEGEN_BITFIELD(12, 13)    ; //!< vin_intensitycomp_Double_BWDen
                uint32_t                 VinIntensitycompDoubleFwden                      : __CODEGEN_BITFIELD(14, 15)    ; //!< vin_intensitycomp_Double_FWDen
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Lumscale1SingleFwd                               : __CODEGEN_BITFIELD( 0,  5)    ; //!< LumScale1 - Single - FWD
                uint32_t                 Reserved70                                       : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Lumscale2SingleFwd                               : __CODEGEN_BITFIELD( 8, 13)    ; //!< LumScale2 - single - FWD
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Lumshift1SingleFwd                               : __CODEGEN_BITFIELD(16, 21)    ; //!< LumShift1 - single - FWD
                uint32_t                 Reserved86                                       : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Lumshift2SingleFwd                               : __CODEGEN_BITFIELD(24, 29)    ; //!< LumShift2- single - FWD
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Lumscale1DoubleFwd                               : __CODEGEN_BITFIELD( 0,  5)    ; //!< LumScale1 - double - FWD
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Lumscale2DoubleFwd                               : __CODEGEN_BITFIELD( 8, 13)    ; //!< LumScale2 - double - FWD
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Lumshift1DoubleFwd                               : __CODEGEN_BITFIELD(16, 21)    ; //!< LumShift1 - double -FWD
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Lumshift2DoubleFwd                               : __CODEGEN_BITFIELD(24, 29)    ; //!< LumShift2- double - FWD
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Lumscale1SingleBwd                               : __CODEGEN_BITFIELD( 0,  5)    ; //!< LumScale1 - Single - BWD
                uint32_t                 Reserved134                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Lumscale2SingleBwd                               : __CODEGEN_BITFIELD( 8, 13)    ; //!< LumScale2 - single - BWD
                uint32_t                 Reserved142                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Lumshift1SingleBwd                               : __CODEGEN_BITFIELD(16, 21)    ; //!< LumShift1 - single - BWD
                uint32_t                 Reserved150                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Lumshift2SingleBwd                               : __CODEGEN_BITFIELD(24, 29)    ; //!< LumShift2- single - BWD
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Lumscale1DoubleBwd                               : __CODEGEN_BITFIELD( 0,  5)    ; //!< LumScale1 - double - BWD
                uint32_t                 Reserved166                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Lumscale2DoubleBwd                               : __CODEGEN_BITFIELD( 8, 13)    ; //!< LumScale2 - double - BWD
                uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Lumshift1DoubleBwd                               : __CODEGEN_BITFIELD(16, 21)    ; //!< LumShift1 - double -BWD
                uint32_t                 Reserved182                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Lumshift2DoubleBwd                               : __CODEGEN_BITFIELD(24, 29)    ; //!< LumShift2- double - BWD
                uint32_t                 Reserved190                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED1                                             = 1, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VC1COMMON                                   = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXVC1PREDPIPESTATE                                     = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_VC1_PRED_PIPE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFD_VC1_LONG_PIC_STATE
    //! \details
    //!     MFX_VC1_LONG PIC_STATE command encapsulates the decoding parameters that
    //!     are read or derived from bitstream syntax elements above (inclusive)
    //!     picture header layer. These parameters are static for a picture and when
    //!     slice structure is present, these parameters are not changed from slice
    //!     to slice of the same picture. Hence, this command is only issued at the
    //!     beginning of processing a new picture and prior to the VC1_*_OBJECT
    //!     command. The values set for these state variables are retained
    //!     internally across slices.Only the parameters needed by hardware (BSD
    //!     unit) to decode bit sequence for the macroblocks in a picture layer or a
    //!     slice layer are presented in this command. Other parameters such as the
    //!     ones used for inverse transform or motion compensation are provided in
    //!     MFX_VC1_PRED_PIPE_STATE command.This Long interface format is intel
    //!     proprietary interface. Driver will need to perform addition operations
    //!     to generate all the fields in this command.
    //!     
    struct MFD_VC1_LONG_PIC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Picturewidthinmbsminus1PictureWidthMinus1InMacroblocks : __CODEGEN_BITFIELD( 0,  7)    ; //!< PICTUREWIDTHINMBSMINUS1_PICTURE_WIDTH_MINUS_1_IN_MACROBLOCKS
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Pictureheightinmbsminus1PictureHeightMinus1InMacroblocks : __CODEGEN_BITFIELD(16, 23)    ; //!< PICTUREHEIGHTINMBSMINUS1_PICTURE_HEIGHT_MINUS_1_IN_MACROBLOCKS_
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Vc1Profile                                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< VC1_PROFILE
                uint32_t                 Reserved65                                       : __CODEGEN_BITFIELD( 1,  2)    ; //!< Reserved
                uint32_t                 Secondfield                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Secondfield
                uint32_t                 OverlapSmoothingEnableFlag                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< OVERLAP_SMOOTHING_ENABLE_FLAG
                uint32_t                 LoopfilterEnableFlag                             : __CODEGEN_BITFIELD( 5,  5)    ; //!< LOOPFILTER_ENABLE_FLAG
                uint32_t                 RangereductionEnable                             : __CODEGEN_BITFIELD( 6,  6)    ; //!< RANGEREDUCTION_ENABLE
                uint32_t                 Rangereductionscale                              : __CODEGEN_BITFIELD( 7,  7)    ; //!< RANGEREDUCTIONSCALE
                uint32_t                 MotionVectorMode                                 : __CODEGEN_BITFIELD( 8, 11)    ; //!< MOTION_VECTOR_MODE
                uint32_t                 Syncmarker                                       : __CODEGEN_BITFIELD(12, 12)    ; //!< SYNCMARKER
                uint32_t                 InterpolationRounderContro                       : __CODEGEN_BITFIELD(13, 13)    ; //!< Interpolation Rounder Contro
                uint32_t                 Implicitquantizer                                : __CODEGEN_BITFIELD(14, 14)    ; //!< ImplicitQuantizer
                uint32_t                 Dmvsurfacevalid                                  : __CODEGEN_BITFIELD(15, 15)    ; //!< DmvSurfaceValid
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Reserved
                uint32_t                 BitplaneBufferPitchMinus1                        : __CODEGEN_BITFIELD(24, 31)    ; //!< Bitplane Buffer Pitch Minus 1
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Bscalefactor                                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< BScaleFactor
                uint32_t                 PquantPictureQuantizationValue                   : __CODEGEN_BITFIELD( 8, 12)    ; //!< PQuant (Picture Quantization Value)
                uint32_t                 Reserved109                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 AltpquantAlternativePictureQuantizationValue     : __CODEGEN_BITFIELD(16, 20)    ; //!< AltPQuant (Alternative Picture Quantization Value)
                uint32_t                 Reserved117                                      : __CODEGEN_BITFIELD(21, 23)    ; //!< Reserved
                uint32_t                 FcmFrameCodingMode                               : __CODEGEN_BITFIELD(24, 25)    ; //!< FCM_FRAME_CODING_MODE
                uint32_t                 PictypePictureType                               : __CODEGEN_BITFIELD(26, 28)    ; //!< PicType (Picture Type)
                uint32_t                 Condover                                         : __CODEGEN_BITFIELD(29, 30)    ; //!< CONDOVER
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Pquantuniform                                    : __CODEGEN_BITFIELD( 0,  0)    ; //!< PQUANTUNIFORM
                uint32_t                 Halfqp                                           : __CODEGEN_BITFIELD( 1,  1)    ; //!< HalfQP
                uint32_t                 AltpquantconfigAlternativePictureQuantizationConfiguration : __CODEGEN_BITFIELD( 2,  3)    ; //!< ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION
                uint32_t                 AltpquantedgemaskAlternativePictureQuantizationEdgeMask : __CODEGEN_BITFIELD( 4,  7)    ; //!< AltPQuantEdgeMask (Alternative Picture Quantization Edge Mask)
                uint32_t                 ExtendedmvrangeExtendedMotionVectorRangeFlag     : __CODEGEN_BITFIELD( 8,  9)    ; //!< EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG
                uint32_t                 ExtendeddmvrangeExtendedDifferentialMotionVectorRangeFlag : __CODEGEN_BITFIELD(10, 11)    ; //!< EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG
                uint32_t                 Reserved140                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 FwdrefdistReferenceDistance                      : __CODEGEN_BITFIELD(16, 19)    ; //!< FwdRefDist (Reference Distance)
                uint32_t                 BwdrefdistReferenceDistance                      : __CODEGEN_BITFIELD(20, 23)    ; //!< BwdRefDist (Reference Distance)
                uint32_t                 NumrefNumberOfReferences                         : __CODEGEN_BITFIELD(24, 24)    ; //!< NUMREF_NUMBER_OF_REFERENCES
                uint32_t                 ReffieldpicpolarityReferenceFieldPicturePolarity : __CODEGEN_BITFIELD(25, 25)    ; //!< REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY
                uint32_t                 FastuvmcflagFastUvMotionCompensationFlag         : __CODEGEN_BITFIELD(26, 26)    ; //!< FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
                uint32_t                 FourmvswitchFourMotionVectorSwitch               : __CODEGEN_BITFIELD(27, 27)    ; //!< FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH
                uint32_t                 UnifiedmvmodeUnifiedMotionVectorMode             : __CODEGEN_BITFIELD(28, 29)    ; //!< UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 CbptabCodedBlockPatternTable                     : __CODEGEN_BITFIELD( 0,  2)    ; //!< CbpTab (Coded Block Pattern Table)
                uint32_t                 TransdctabIntraTransformDcTable                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE
                uint32_t                 TransacuvPictureLevelTransformChromaAcCodingSetIndexTransactable : __CODEGEN_BITFIELD( 4,  5)    ; //!< TransAcUV (Picture-level Transform Chroma AC Coding Set Index, TRANSACTABLE)
                uint32_t                 TransacyPictureLevelTransformLumaAcCodingSetIndexTransactable2 : __CODEGEN_BITFIELD( 6,  7)    ; //!< TransAcY (Picture-level Transform Luma AC Coding Set Index, TRANSACTABLE2
                uint32_t                 MbmodetabMacroblockModeTable                     : __CODEGEN_BITFIELD( 8, 10)    ; //!< MbModeTab (Macroblock Mode Table)
                uint32_t                 TranstypembflagMacroblockTransformTypeFlag       : __CODEGEN_BITFIELD(11, 11)    ; //!< TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG
                uint32_t                 TranstypePictureLevelTransformType               : __CODEGEN_BITFIELD(12, 13)    ; //!< TransType (Picture-level Transform Type)
                uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Twomvbptab2MvBlockPatternTable                   : __CODEGEN_BITFIELD(16, 17)    ; //!< TwoMvBpTab (2MV Block Pattern Table)
                uint32_t                 Fourmvbptab4MvBlockPatternTable                  : __CODEGEN_BITFIELD(18, 19)    ; //!< FourMvBpTab (4-MV Block Pattern Table)
                uint32_t                 MvtabMotionVectorTable                           : __CODEGEN_BITFIELD(20, 22)    ; //!< MvTab (Motion Vector Table)
                uint32_t                 Reserved183                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 Fieldtxraw                                       : __CODEGEN_BITFIELD(24, 24)    ; //!< FIELDTXRAW
                uint32_t                 Acpredraw                                        : __CODEGEN_BITFIELD(25, 25)    ; //!< ACPREDRAW
                uint32_t                 Overflagsraw                                     : __CODEGEN_BITFIELD(26, 26)    ; //!< OVERFLAGSRAW
                uint32_t                 Directmbraw                                      : __CODEGEN_BITFIELD(27, 27)    ; //!< DIRECTMBRAW
                uint32_t                 Skipmbraw                                        : __CODEGEN_BITFIELD(28, 28)    ; //!< SKIPMBRAW
                uint32_t                 Mvtypembraw                                      : __CODEGEN_BITFIELD(29, 29)    ; //!< MVTYPEMBRAW
                uint32_t                 Forwardmbraw                                     : __CODEGEN_BITFIELD(30, 30)    ; //!< FORWARDMBRAW
                uint32_t                 BitplanepresentflagBitplaneBufferPresentFlag     : __CODEGEN_BITFIELD(31, 31)    ; //!< BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED1                                             = 1, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VC1DEC                                      = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDVC1LONGPICSTATE                                      = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief PICTUREWIDTHINMBSMINUS1_PICTURE_WIDTH_MINUS_1_IN_MACROBLOCKS
        //! \details
        //!     This field indicates the width of the picture in unit of macroblocks.
        //!     For example, for a 1920x1080 frame picture, PictureWidthInMBs equals 120
        //!     (1920 divided by 16).This field is used in VLD and IT modes
        enum PICTUREWIDTHINMBSMINUS1_PICTURE_WIDTH_MINUS_1_IN_MACROBLOCKS
        {
            PICTUREWIDTHINMBSMINUS1_PICTURE_WIDTH_MINUS_1_IN_MACROBLOCKS_VALUE255 = 255, //!< No additional details
        };

        //! \brief PICTUREHEIGHTINMBSMINUS1_PICTURE_HEIGHT_MINUS_1_IN_MACROBLOCKS_
        //! \details
        //!     This field indicates the height of the picture in unit of macroblocks.
        //!     For example, for a 1920x1080 frame picture, PictureHeightInMBs equals 68
        //!     (1080 divided by 16, and rounded up, i.e. effectively specified as 1088
        //!     instead).This field is used in VLD and IT modes.
        enum PICTUREHEIGHTINMBSMINUS1_PICTURE_HEIGHT_MINUS_1_IN_MACROBLOCKS_
        {
            PICTUREHEIGHTINMBSMINUS1_PICTURE_HEIGHT_MINUS_1_IN_MACROBLOCKS_VALUE255 = 255, //!< No additional details
        };

        //! \brief VC1_PROFILE
        //! \details
        //!     specifies the bitstream profile.This field is used in both VLD and IT
        //!     modes.
        enum VC1_PROFILE
        {
            VC1_PROFILE_DISABLE                                              = 0, //!< current picture is in Simple or Main Profile (No need to distinguish Simple and Main Profile)
            VC1_PROFILE_ENABLE                                               = 1, //!< current picture is in Advanced Profile
        };

        //! \brief OVERLAP_SMOOTHING_ENABLE_FLAG
        //! \details
        //!     This field is the decoded syntax element OVERLAP in bitstreamIndicates
        //!     if Overlap smoothing is ON at the picture levelThis field is used in
        //!     both VLD and IT modes.
        enum OVERLAP_SMOOTHING_ENABLE_FLAG
        {
            OVERLAP_SMOOTHING_ENABLE_FLAG_DISABLE                            = 0, //!< to disable overlap smoothing filter
            OVERLAP_SMOOTHING_ENABLE_FLAG_ENABLE                             = 1, //!< to enable overlap smoothing filter
        };

        //! \brief LOOPFILTER_ENABLE_FLAG
        //! \details
        //!     This filed is the decoded syntax element LOOPFILTER in bitstream. It
        //!     indicates if In-loop Deblocking is ON according to picture level
        //!     bitstream syntax control. This bit affects BSD unit and also the loop
        //!     filter unit.When this bit is set to 1, PostDeblockOutEnable field in
        //!     MFX_PIPE_MODE_SELECT command must also be set to 1. In this case,
        //!     in-loop deblocking operation follows the VC1 standard - deblocking
        //!     doesn't cross slice boundary.When this bit is set to 0, but
        //!     PostDeblockOutEnable field in MFX_PIPE_MODE_SELECT command is set to 1.
        //!     It indicates the loop filter unit is used for out-of-loop deblocking. In
        //!     this case, deblocking operation does cross slice boundary.This field is
        //!     used in VLD mode only, not in IT mode.
        enum LOOPFILTER_ENABLE_FLAG
        {
            LOOPFILTER_ENABLE_FLAG_DISABLE                                   = 0, //!< Disables loop filter
            LOOPFILTER_ENABLE_FLAG_ENABLE                                    = 1, //!< Enables loop filter
        };

        //! \brief RANGEREDUCTION_ENABLE
        //! \details
        //!     This field specifies whether on-the-fly pixel value range reduction
        //!     should be performed for the preceding (or forward) reference picture.
        //!     Along with RangeReductionScale to specify whether scale up or down
        //!     should be performed. It is not the same value as RANGEREDFRM Syntax
        //!     Element (_PictureParameters bPicDeblocked bit 5) in the Picture
        //!     Header.
        enum RANGEREDUCTION_ENABLE
        {
            RANGEREDUCTION_ENABLE_DISABLE                                    = 0, //!< Range reduction is not performed
            RANGEREDUCTION_ENABLE_ENABLE                                     = 1, //!< Range reduction is performed
        };

        //! \brief RANGEREDUCTIONSCALE
        //! \details
        //!     This field specifies whether the reference picture pixel values should
        //!     be scaled up or scaled down on-the-fly, if RangeReduction is Enabled.
        enum RANGEREDUCTIONSCALE
        {
            RANGEREDUCTIONSCALE_UNNAMED0                                     = 0, //!< Scale down reference picture by factor of 2
            RANGEREDUCTIONSCALE_UNNAMED1                                     = 1, //!< Scale up reference picture by factor of 2
        };

        //! \brief MOTION_VECTOR_MODE
        //! \details
        //!     This field indicates one of the following motion compensation
        //!     interpolation modes for P and B pictures. The MC interpolation modes
        //!     apply to prediction values of luminance blocks and are always in
        //!     quarter-sample. For chrominance blocks, it always performs bilinear
        //!     interpolation with either half-pel or quarter-pel precision.Before the
        //!     polarity of Chroma Half-pel or Q-pel is reversed from Spec, now I
        //!     have fixed it to match with VC1 Spec.
        enum MOTION_VECTOR_MODE
        {
            MOTION_VECTOR_MODE_UNNAMED0                                      = 0, //!< Chroma Quarter -pel + Luma bicubic. (can only be 1MV)
            MOTION_VECTOR_MODE_UNNAMED1                                      = 1, //!< Chroma Half-pel + Luma bicubic. (can be 1MV or 4MV)
            MOTION_VECTOR_MODE_UNNAMED8                                      = 8, //!< Chroma Quarter -pel + Luma bilinear. (can only be 1MV)
            MOTION_VECTOR_MODE_UNNAMED9                                      = 9, //!< Chroma Half-pel + Luma bilinear
        };

        //! \brief SYNCMARKER
        //! \details
        //!     Indicates whether sync markers are enabled/disabled. If enable, sync
        //!     markers "may be" present in the current video sequence being decoded. It
        //!     is a sequence level syntax element and is valid only for Simple and Main
        //!     Profiles.
        enum SYNCMARKER
        {
            SYNCMARKER_NOTPRESENT                                            = 0, //!< Sync Marker is not present in the bitstream
            SYNCMARKER_MAYBEPRESENT                                          = 1, //!< Sync Marker maybe present in the bitstream
        };

        //! \brief FCM_FRAME_CODING_MODE
        //! \details
        //!     This is the same as the variable FCM defined in VC1.This field must be
        //!     set to 0 for Simple and Main ProfilesThis field is unique to intel VC1
        //!     VLD Long format, and is used in IT mode as well. For VC1 IT mode,
        //!     driver needs to convert the interface to intel HW VLD Long Format
        //!     interface.
        enum FCM_FRAME_CODING_MODE
        {
            FCM_FRAME_CODING_MODE_DISABLE                                    = 0, //!< Progressive Frame Picture
            FCM_FRAME_CODING_MODE_ENABLE                                     = 1, //!< Interlaced Frame Picture
            FCM_FRAME_CODING_MODE_UNNAMED2                                   = 2, //!< Field Picture with Top Field First
            FCM_FRAME_CODING_MODE_UNNAMED3                                   = 3, //!< Field Picture with Bottom Field First
        };

        //! \brief CONDOVER
        //! \details
        //!     This field is the decoded syntax element CONDOVER in a bitstream of
        //!     advanced profile. It controls the overlap smoothing filter operation for
        //!     an I frame or an BI frame when the picture level qualization step size
        //!     PQUANT is 8 or lower.This field is used in intel VC1 VLD mode only, not
        //!     in VC1 and IT modes.
        enum CONDOVER
        {
            CONDOVER_UNNAMED0                                                = 0, //!< No overlap smoothing
            CONDOVER_UNNAMED1                                                = 1, //!< Reserved
            CONDOVER_UNNAMED2                                                = 2, //!< Always perform overlap smoothing filter
            CONDOVER_UNNAMED3                                                = 3, //!< Overlap smoothing on a per macroblock basis based on OVERFLAGS
        };

        //! \brief PQUANTUNIFORM
        //! \details
        //!     Indicating if uniform quantization applies to the
        //!                         picture. It is used for inverse quantization of the AC
        //!     coefficients.QUANTIZER
        //!                         001123PQUANTIZER -
        //!                         -01--PQINDEX&gt;=9&lt;=8----PQuantUniform010201ImplicitQuantizer =
        //!     0, and
        //!                         PQuantUniform = 0 is used to represent 2 cases : 1) QUANTIZER=01
        //!     and
        //!                         PQUANTIZER=0; and 2) QUANTIZER = 10b.ImplicitQuantizer = 0, and
        //!     PQuantUniform =
        //!                         1 is used to represent 2 cases : 1) QUANTIZER=01 and PQUANTIZER=1;
        //!     and 2)
        //!                         QUANTIZER = 11bThis field is unique to intel VC1 VLD Long format
        //!     mode, and is
        //!                         not used in IT and VC1 modes.
        enum PQUANTUNIFORM
        {
            PQUANTUNIFORM_UNNAMED0                                           = 0, //!< Non-uniform
            PQUANTUNIFORM_UNNAMED1                                           = 1, //!< Uniform
        };

        //! \brief ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION
        //! \details
        //!     This field specifies the way AltPQuant is used in the picture. It
        //!     determines how to compute the macroblock quantizer step size, MQUANT. It
        //!     is derived based on the following variables DQUANT, DQUANTFRM,
        //!     DQPROFILE, DQSBEDGE, DQDBEDGE, and DQBILEVEL defined in the VC1
        //!     standard, as shown in Error! Reference source not found..This field is
        //!     unique to intel VC1 VLD Long format mode, and is not used in IT and
        //!     VC1 modes.
        enum ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION
        {
            ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION_UNNAMED0 = 0, //!< AltPQuant not used
            ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION_UNNAMED1 = 1, //!< AltPQuant is used and applied to edge macroblocks only
            ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION_UNNAMED2 = 2, //!< MQUANT is encoded in macroblock layer
            ALTPQUANTCONFIG_ALTERNATIVE_PICTURE_QUANTIZATION_CONFIGURATION_UNNAMED3 = 3, //!< AltPQuant and PQuant are selected on macroblock basis
        };

        //! \brief EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG
        //! \details
        //!     This field specifies the motion vector range in quarter-pel or half-pel
        //!     modes. It is equivalent to the variable MVRANGE in the VC1 standard.
        //!     This field is unique to intel VC1 VLD Long format mode, and is not used
        //!     in IT and VC1 modes
        enum EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG
        {
            EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG_UNNAMED0       = 0, //!< [-256, 255] x [-128, 127]
            EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG_UNNAMED1       = 1, //!< 512, 511] x [-256, 255]
            EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG_UNNAMED2       = 2, //!< [-2048, 2047] x [-1024, 1023]
            EXTENDEDMVRANGE_EXTENDED_MOTION_VECTOR_RANGE_FLAG_UNNAMED3       = 3, //!< [-4096, 4095] x [-2048, 2047]
        };

        //! \brief EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG
        //! \details
        //!     This field specifies the differential motion vector range in interlaced
        //!     pictures. It is equivalent to the variable DMVRANGE in the VC1 standard.
        //!     This field is unique to intel VC1 VLD Long format mode, and is not used
        //!     in IT and VC1 modes.
        enum EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG
        {
            EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG_UNNAMED0 = 0, //!< No extended range
            EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG_UNNAMED1 = 1, //!< Extended horizontally
            EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG_UNNAMED2 = 2, //!< Extended vertically
            EXTENDEDDMVRANGE_EXTENDED_DIFFERENTIAL_MOTION_VECTOR_RANGE_FLAG_UNNAMED3 = 3, //!< Extended in both directions
        };

        //! \brief NUMREF_NUMBER_OF_REFERENCES
        //! \details
        //!     This field indicates how many reference fields are referenced by the
        //!     current (field) picture. It is identical to the variable NUMREF in the
        //!     VC1 standard. This field is only valid for field P picture (FCM = 10 |
        //!     11).This field is unique to intel VC1 VLD Long format mode, and is not
        //!     used in IT and VC1 modes.
        enum NUMREF_NUMBER_OF_REFERENCES
        {
            NUMREF_NUMBER_OF_REFERENCES_UNNAMED0                             = 0, //!< One field referenced
            NUMREF_NUMBER_OF_REFERENCES_UNNAMED1                             = 1, //!< Two fields referenced
        };

        //! \brief REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY
        //! \details
        //!     This field specifies the polarity of the one reference field picture
        //!     used for a field P picture. It is derived from the variable REFFIELD
        //!     defined in VC1 standard and is only valid when one field is referenced
        //!     (NUMREF = 0) for a field P picture.When NUMREF = 0 and REFFIELD = 0,
        //!     this field is the polarity of the reference I/P field that is temporally
        //!     closest; When NUMREF = 0 and REFFIELD = 1, this field is the polarity of
        //!     the reference I/P field that is the second most temporally closest. The
        //!     distance is measured based on display order but ignoring the repeated
        //!     field if present (due to RFF = 1).This field is unique to intel VC1 VLD
        //!     Long format mode, and is not used in IT and VC1 modes.
        enum REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY
        {
            REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY_UNNAMED0    = 0, //!< Top (even) field
            REFFIELDPICPOLARITY_REFERENCE_FIELD_PICTURE_POLARITY_UNNAMED1    = 1, //!< Bottom (odd) field
        };

        //! \brief FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
        //! \details
        //!     This field specifies whether the motion vectors for
        //!                         UV is rounded to half or full pel position. It is identical to the
        //!     variable
        //!                         FASTUVMC in VC1 standard.This field is used in both VLD and IT
        //!     modes.It is
        //!                         derived from FASTUVMC = (bPicSpatialResid8 &gt;&gt; 4) &amp; 1 in
        //!     both VLD and
        //!                         IT modes, and should have the same value as Motion Vector Mode
        //!                         LSBit.
        enum FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
        {
            FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED0           = 0, //!< no rounding
            FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED1           = 1, //!< quarter-pel offsets to half/full pel positions
        };

        //! \brief FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH
        //! \details
        //!     This field indicates if 4-MV is present for an interlaced frame P
        //!     picture. It is identical to the variable 4MVSWITCH (4 Motion Vector
        //!     Switch) in VC1 standard.This field is used in intel VC1 VLD Long Format
        //!     mode only, it is not used in VC1 VLD and IT modes.
        enum FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH
        {
            FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH_DISABLE                   = 0, //!< only 1-MV
            FOURMVSWITCH_FOUR_MOTION_VECTOR_SWITCH_ENABLE                    = 1, //!< 1, 2, or 4 MVs
        };

        //! \brief UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE
        //! \details
        //!     This field is a combination of the variables MVMODE and MVMODE2 in the
        //!     VC1 standard, for parsing Luma MVD from the bitstream. This field is
        //!     used to signal 1MV vs 4MVallowed (Mixed Mode). This field is also used
        //!     to signal Q-pel or Half-pel MVD read from the bitstream. The bicubic or
        //!     bilinear Luma MC interpolation mode is duplicate information from Motion
        //!     Vector Mode field, and is ignored here.This field is used in intel VC1
        //!     VLD Long Format mode only, it is not used in VC1 VLD and IT modes.
        enum UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE
        {
            UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE_UNNAMED0                = 0, //!< Mixed MV, Q-pel bicubic
            UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE_UNNAMED1                = 1, //!< 1-MV, Q-pel bicubic
            UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE_UNNAMED2                = 2, //!< 1-MV half-pel bicubic
            UNIFIEDMVMODE_UNIFIED_MOTION_VECTOR_MODE_UNNAMED3                = 3, //!< 1-MV half-pel bilinear
        };

        //! \brief TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE
        //! \details
        //!     This field specifies whether the low motion tables or the high motion
        //!     tables are used to decode the Transform DC coefficients in intra-coded
        //!     blocks. This field is identical to the variable TRANSDCTAB in the VC1
        //!     standard, section 8.1.1.2.This field is valid for all picture types.This
        //!     field is unique to intel VC1 VLD Long format mode, and is not used in IT
        //!     and VC1 modes.
        enum TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE
        {
            TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE_UNNAMED0                     = 0, //!< The high motion tables
            TRANSDCTAB_INTRA_TRANSFORM_DC_TABLE_UNNAMED1                     = 1, //!< The low motion tables
        };

        //! \brief TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG
        //! \details
        //!     This field indicates whether Transform Type is fixed at picture level or
        //!     variable at macroblock level. It is identical to the variable TTMBF in
        //!     the VC1 standard, section 7.1.1.40.This field is set to 1 when
        //!     VSTRANSFORM is 0 in the entry point layer.This field is unique to intel
        //!     VC1 VLD Long format mode, and is not used in IT and VC1 modes.
        enum TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG
        {
            TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG_UNNAMED0          = 0, //!< variable transform type in macroblock layer
            TRANSTYPEMBFLAG_MACROBLOCK_TRANSFORM_TYPE_FLAG_UNNAMED1          = 1, //!< use picture level transform type TransType
        };

        //! \brief FIELDTXRAW
        //! \details
        //!     This field indicates whether the FIELDTX field is coded in raw or
        //!     non-raw mode.This field is only valid when PictureType is I or BI.This
        //!     field is unique to intel VC1 VLD Long format mode, and is not used in IT
        //!     and VC1 modes.
        enum FIELDTXRAW
        {
            FIELDTXRAW_DISABLE                                               = 0, //!< Non-Raw Mode
            FIELDTXRAW_ENABLE                                                = 1, //!< Raw Mode
        };

        //! \brief ACPREDRAW
        //! \details
        //!     This field indicates whether the ACPRED field is coded in raw or non-raw
        //!     mode.This field is only valid when PictureType is I or BI.This field is
        //!     unique to intel VC1 VLD Long format mode, and is not used in IT and
        //!     VC1 modes.
        enum ACPREDRAW
        {
            ACPREDRAW_DISABLE                                                = 0, //!< Non-Raw Mode
            ACPREDRAW_ENABLE                                                 = 1, //!< Raw Mode
        };

        //! \brief OVERFLAGSRAW
        //! \details
        //!     This field indicates whether the OVERFLAGS field is coded in raw or
        //!     non-raw mode.This field is only valid when PictureType is I or BI.This
        //!     field is unique to intel VC1 VLD Long format mode, and is not used in IT
        //!     and VC1 modes.
        enum OVERFLAGSRAW
        {
            OVERFLAGSRAW_UNNAMED0                                            = 0, //!< Non-Raw Mode
            OVERFLAGSRAW_UNNAMED1                                            = 1, //!< Raw Mode
        };

        //! \brief DIRECTMBRAW
        //! \details
        //!     This field indicates whether the DIRECTMB field is coded in raw or
        //!     non-raw mode.This field is only valid when PictureType is P or B.This
        //!     field is unique to intel VC1 VLD Long format mode, and is not used in IT
        //!     and VC1 modes.
        enum DIRECTMBRAW
        {
            DIRECTMBRAW_UNNAMED0                                             = 0, //!< Non-Raw Mode
            DIRECTMBRAW_UNNAMED1                                             = 1, //!< Raw Mode
        };

        //! \brief SKIPMBRAW
        //! \details
        //!     This field indicates whether the SKIPMB field is coded in raw or non-raw
        //!     mode.This field is only valid when PictureType is P or B.0 = non-raw
        //!     mode1 = raw modeThis field is unique to intel VC1 VLD Long format mode,
        //!     and is not used in IT and VC1 modes.
        enum SKIPMBRAW
        {
            SKIPMBRAW_DISABLE                                                = 0, //!< Non-Raw Mode
            SKIPMBRAW_ENABLE                                                 = 1, //!< Raw Mode
        };

        //! \brief MVTYPEMBRAW
        //! \details
        //!     This field indicates whether the MVTYPREMB field is coded in raw or
        //!     non-raw mode.This field is only valid when PictureType is P.This field
        //!     is unique to intel VC1 VLD Long format mode, and is not used in IT and
        //!     VC1 modes.
        enum MVTYPEMBRAW
        {
            MVTYPEMBRAW_UNNAMED0                                             = 0, //!< Non-Raw Mode
            MVTYPEMBRAW_UNNAMED1                                             = 1, //!< Raw Mode
        };

        //! \brief FORWARDMBRAW
        //! \details
        //!     This field indicates whether the FORWARDMB field is coded in raw or
        //!     non-raw mode.This field is only valid when PictureType is B.This field
        //!     is unique to intel VC1 VLD Long format mode, and is not used in IT and
        //!     VC1 modes.
        enum FORWARDMBRAW
        {
            FORWARDMBRAW_UNNAMED0                                            = 0, //!< non-raw mode
            FORWARDMBRAW_UNNAMED1                                            = 1, //!< raw mode
        };

        //! \brief BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG
        //! \details
        //!     This field indicates whether the bitplane buffer is present for the
        //!     picture. If set, at least one of the fields listed in bits 22:16 is
        //!     coded in non-raw mode, and Bitplane Buffer Base Address field in the
        //!     VC1_BSD_BUF_BASE_STATE command points to the bitplane buffer. Otherwise,
        //!     all the fields that are applicable for the current picture in bits 22:16
        //!     must be coded in raw mode.This field is unique to intel VC1 VLD Long
        //!     format mode, and is not used in IT and VC1 modes.
        enum BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG
        {
            BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG_UNNAMED0        = 0, //!< bitplane buffer is not present
            BITPLANEPRESENTFLAG_BITPLANE_BUFFER_PRESENT_FLAG_UNNAMED1        = 1, //!< bitplane buffer is present
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_VC1_LONG_PIC_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFD_VC1_SHORT_PIC_STATE
    //! \details
    //!     
    //!     
    struct MFD_VC1_SHORT_PIC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 PictureWidth                                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< Picture Width
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 PictureHeight                                    : __CODEGEN_BITFIELD(16, 23)    ; //!< Picture Height
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 PictureStructure                                 : __CODEGEN_BITFIELD( 0,  1)    ; //!< PICTURE_STRUCTURE
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 Secondfield                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< SecondField
                uint32_t                 IntraPictureFlag                                 : __CODEGEN_BITFIELD( 4,  4)    ; //!< INTRA_PICTURE_FLAG
                uint32_t                 BackwardPredictionPresentFlag                    : __CODEGEN_BITFIELD( 5,  5)    ; //!< Backward Prediction Present Flag
                uint32_t                 Reserved70                                       : __CODEGEN_BITFIELD( 6, 10)    ; //!< Reserved
                uint32_t                 Vc1Profile                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< VC1_PROFILE
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 14)    ; //!< Reserved
                uint32_t                 Dmvsurfacevalid                                  : __CODEGEN_BITFIELD(15, 15)    ; //!< DmvSurfaceValid
                uint32_t                 MotionVectorMode                                 : __CODEGEN_BITFIELD(16, 19)    ; //!< Motion Vector Mode
                uint32_t                 Reserved84                                       : __CODEGEN_BITFIELD(20, 22)    ; //!< Reserved
                uint32_t                 InterpolationRounderControl                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Interpolation Rounder Control
                uint32_t                 BitplaneBufferPitchMinus1                        : __CODEGEN_BITFIELD(24, 31)    ; //!< Bitplane Buffer Pitch Minus 1
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 VstransformFlag                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< VSTRANSFORM_FLAG
                uint32_t                 Dquant                                           : __CODEGEN_BITFIELD( 1,  2)    ; //!< DQUANT
                uint32_t                 ExtendedMvPresentFlag                            : __CODEGEN_BITFIELD( 3,  3)    ; //!< EXTENDED_MV_PRESENT_FLAG
                uint32_t                 FastuvmcflagFastUvMotionCompensationFlag         : __CODEGEN_BITFIELD( 4,  4)    ; //!< FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
                uint32_t                 LoopfilterEnableFlag                             : __CODEGEN_BITFIELD( 5,  5)    ; //!< LOOPFILTER_ENABLE_FLAG
                uint32_t                 RefdistFlag                                      : __CODEGEN_BITFIELD( 6,  6)    ; //!< REFDIST_FLAG
                uint32_t                 PanscanPresentFlag                               : __CODEGEN_BITFIELD( 7,  7)    ; //!< PANSCAN_PRESENT_FLAG
                uint32_t                 Maxbframes                                       : __CODEGEN_BITFIELD( 8, 10)    ; //!< MAXBFRAMES
                uint32_t                 RangeredPresentFlagForSimpleMainProfileOnly      : __CODEGEN_BITFIELD(11, 11)    ; //!< RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
                uint32_t                 SyncmarkerPresentFlagForSimpleMainProfileOnly    : __CODEGEN_BITFIELD(12, 12)    ; //!< SYNCMARKER_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
                uint32_t                 MultiresPresentFlagForSimpleMainProfileOnly      : __CODEGEN_BITFIELD(13, 13)    ; //!< MULTIRES_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
                uint32_t                 Quantizer                                        : __CODEGEN_BITFIELD(14, 15)    ; //!< QUANTIZER
                uint32_t                 PPicRefDistance                                  : __CODEGEN_BITFIELD(16, 20)    ; //!< P_PIC_REF_DISTANCE
                uint32_t                 Reserved117                                      : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                uint32_t                 ProgressivePicType                               : __CODEGEN_BITFIELD(22, 23)    ; //!< PROGRESSIVE_PIC_TYPE
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 27)    ; //!< Reserved
                uint32_t                 RangeReductionEnable                             : __CODEGEN_BITFIELD(28, 28)    ; //!< RANGE_REDUCTION_ENABLE
                uint32_t                 RangeReductionScale                              : __CODEGEN_BITFIELD(29, 29)    ; //!< RANGE_REDUCTION_SCALE
                uint32_t                 OverlapSmoothingEnableFlag                       : __CODEGEN_BITFIELD(30, 30)    ; //!< OVERLAP_SMOOTHING_ENABLE_FLAG
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 ExtendedDmvPresentFlag                           : __CODEGEN_BITFIELD( 0,  0)    ; //!< EXTENDED_DMV_PRESENT_FLAG
                uint32_t                 Psf                                              : __CODEGEN_BITFIELD( 1,  1)    ; //!< PSF
                uint32_t                 RefpicFlag                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< REFPIC_FLAG
                uint32_t                 Finterflag                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< FINTERFLAG
                uint32_t                 Tfcntrflag                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< TFCNTRFLAG
                uint32_t                 Interlace                                        : __CODEGEN_BITFIELD( 5,  5)    ; //!< INTERLACE
                uint32_t                 Pulldown                                         : __CODEGEN_BITFIELD( 6,  6)    ; //!< PULLDOWN
                uint32_t                 PostprocFlag                                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< POSTPROC Flag
                uint32_t                 _4MvAllowedFlag                                  : __CODEGEN_BITFIELD( 8,  8)    ; //!< _4MV Allowed Flag
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9, 23)    ; //!< Reserved
                uint32_t                 BfractionEnumeration                             : __CODEGEN_BITFIELD(24, 28)    ; //!< BFraction Enumeration
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED0                                             = 0, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VC1DEC                                      = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDVC1SHORTPICSTATE                                     = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief PICTURE_STRUCTURE
        //! \details
        //!     This field is used in both VC1 VLD mode and IT mode. It is the
        //!     same parameter as bPicStructure in VC1 spec.
        //!                         The Picture Structure and Progressive Pic Type are used to derive
        //!     the picture structure as specified in FCM, in VC1 VLD and IT mode.
        enum PICTURE_STRUCTURE
        {
            PICTURE_STRUCTURE_UNNAMED0                                       = 0, //!< illegal
            PICTURE_STRUCTURE_UNNAMED1                                       = 1, //!< top field (bit 0)
            PICTURE_STRUCTURE_UNNAMED2                                       = 2, //!< bottom field (bit 1)
            PICTURE_STRUCTURE_UNNAMED3                                       = 3, //!< frame (both fields are present)
        };

        //! \brief INTRA_PICTURE_FLAG
        //! \details
        //!     This field is used in both VC1 VLD mode and IT mode. It is the
        //!     same parameter as bPicIntra in VC1 spec.
        //!                         The Intra Picture Flag, Backward Prediction Present Flag and
        //!     RefPicFlag are used to derive the picture type, as specified in PTYPE
        //!     for a frame, and in FPTYPE for a field, in VC1 VLD and IT mode.
        enum INTRA_PICTURE_FLAG
        {
            INTRA_PICTURE_FLAG_UNNAMED0                                      = 0, //!< entire picture can have a mixture of intra and inter MB type or just inter MB type.
            INTRA_PICTURE_FLAG_UNNAMED1                                      = 1, //!< entire picture is coded in intra MB type
        };

        //! \brief VC1_PROFILE
        //! \details
        //!     specifies the bitstream profile.
        //!                         Note: This is required because 128 is added for intra blocks post
        //!     inverse transform in advanced profile and also to find out if Motion
        //!     vectors are adjusted or not.
        //!                         This field is used in both VLD and IT modes.
        enum VC1_PROFILE
        {
            VC1_PROFILE_UNNAMED0                                             = 0, //!< current picture is in Simple or Main Profile (No need to distinguish Simple and Main Profile)
            VC1_PROFILE_UNNAMED1                                             = 1, //!< current picture is in Advanced Profile
        };

        enum VSTRANSFORM_FLAG
        {
            VSTRANSFORM_FLAG_DISABLE                                         = 0, //!< variable-sized transform coding is not enabled
            VSTRANSFORM_FLAG_ENABLE                                          = 1, //!< variable-sized transform coding is enabled
        };

        //! \brief DQUANT
        //! \details
        //!     Use for Picture Header Parsing of VOPDUANT elements
        enum DQUANT
        {
            DQUANT_UNNAMED0                                                  = 0, //!< no VOPDQUANT elements; Quantizer cannot vary in frame, same quantization step size PQUANT is used for all MBs in the frame
            DQUANT_UNNAMED1                                                  = 1, //!< refer to VC1 Spec. for all the MB position dependent quantizer selection
            DQUANT_UNNAMED2                                                  = 2, //!< The macroblocks located on the picture edge boundary shall be quantized with ALTPQUANT while the rest of the macroblocks shall be quantized with PQUANT.
        };

        //! \brief EXTENDED_MV_PRESENT_FLAG
        //! \details
        //!     BitFieldDesc
        enum EXTENDED_MV_PRESENT_FLAG
        {
            EXTENDED_MV_PRESENT_FLAG_UNNAMED0                                = 0, //!< Extended_MV is not present in the picture header
            EXTENDED_MV_PRESENT_FLAG_UNNAMED1                                = 1, //!< Extended_MV is present in the picture header
        };

        //! \brief FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
        //! \details
        //!     This field specifies whether the motion vectors for
        //!                         UV is rounded to half or full pel position. It is identical to the
        //!     variable
        //!                         FASTUVMC in VC1 standard.This field is used in both VLD and IT
        //!     modes.It is
        //!                         derived from FASTUVMC = (bPicSpatialResid8 &gt;&gt; 4) &amp; 1 in
        //!     both VLD and
        //!                         IT modes, and should have the same value as Motion Vector Mode
        //!                         LSBit.
        enum FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG
        {
            FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED0           = 0, //!< no rounding
            FASTUVMCFLAG_FAST_UV_MOTION_COMPENSATION_FLAG_UNNAMED1           = 1, //!< quarter-pel offsets to half/full pel positions
        };

        //! \brief LOOPFILTER_ENABLE_FLAG
        //! \details
        //!     This filed is the decoded syntax element LOOPFILTER in bitstream. It
        //!     indicates if In-loop Deblocking is ON according to picture level
        //!     bitstream syntax control. This bit affects BSD unit and also the loop
        //!     filter unit.When this bit is set to 1, PostDeblockOutEnable field in
        //!     MFX_PIPE_MODE_SELECT command must also be set to 1. In this case,
        //!     in-loop deblocking operation follows the VC1 standard - deblocking
        //!     doesn't cross slice boundary.When this bit is set to 0, but
        //!     PostDeblockOutEnable field in MFX_PIPE_MODE_SELECT command is set to 1.
        //!     It indicates the loop filter unit is used for out-of-loop deblocking. In
        //!     this case, deblocking operation does cross slice boundary.This field is
        //!     used in VLD mode only, not in IT mode.
        enum LOOPFILTER_ENABLE_FLAG
        {
            LOOPFILTER_ENABLE_FLAG_UNNAMED0                                  = 0, //!< In-Loop-Deblocking-Filter is disabled
            LOOPFILTER_ENABLE_FLAG_UNNAMED1                                  = 1, //!< In-Loop-Deblocking-Filter is enabled
        };

        enum PANSCAN_PRESENT_FLAG
        {
            PANSCAN_PRESENT_FLAG_UNNAMED0                                    = 0, //!< Pan Scan Parameters are not present in the picture header
            PANSCAN_PRESENT_FLAG_UNNAMED1                                    = 1, //!< Pan Scan Parameters are present in the picture header
        };

        //! \brief RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
        //! \details
        //!     It is needed for Picture Header Parsing.Driver is responsible to keep
        //!     RangeReductionScale, RangeReduction Enable and RANGERED Present Flag of
        //!     current picture coherent.
        enum RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
        {
            RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0       = 0, //!< Range Reduction Parameter (RANGEREDFRM) is not present in the picture header
            RANGERED_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED1       = 1, //!< Range Reduction Parameter (RANGEREDFRM) is present in the picture header.
        };

        enum SYNCMARKER_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
        {
            SYNCMARKER_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0     = 0, //!< Bitstream for Simple and Main Profile has no sync marker
            SYNCMARKER_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED1     = 1, //!< Bitstream for Simple and Main Profile may have sync marker(s)
        };

        enum MULTIRES_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY
        {
            MULTIRES_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED0       = 0, //!< RESPIC Parameter is present in the picture header
            MULTIRES_PRESENT_FLAG_FOR_SIMPLEMAIN_PROFILE_ONLY_UNNAMED1       = 1, //!< RESPIC Parameter is present in the picture header
        };

        enum QUANTIZER
        {
            QUANTIZER_UNNAMED0                                               = 0, //!< implicit quantizer at frame leve
            QUANTIZER_UNNAMED1                                               = 1, //!< explicit quantizer at frame level, and use PQUANTIZER SE to specify uniform or non-uniform
            QUANTIZER_UNNAMED2                                               = 2, //!< explicit quantizer, and non-uniform quantizer for all frames
            QUANTIZER_UNNAMED3                                               = 3, //!< explicit quantizer, and uniform quantizer for all frames
        };

        //! \brief P_PIC_REF_DISTANCE
        //! \details
        //!     This element defines the number of frames between the current frame and
        //!     the reference frame. It is the same as the REFDIST SE in VC1 interlaced
        //!     field picture header. It is present if the entry-level flag REFDIST_FLAG
        //!     == 1, and if the picture type is not one of the following types: B/B,
        //!     B/BI, BI/B, BI/BI. If the entry level flag REFDIST_FLAG == 0, REFDIST
        //!     shall be set to the default value of 0.This field is used in VC1
        //!     VLD mode only, not used in IT and intel VC1 VLD Long Format modes.
        enum P_PIC_REF_DISTANCE
        {
            P_PIC_REF_DISTANCE_UNNAMED0                                      = 0, //!< No additional details
        };

        //! \brief PROGRESSIVE_PIC_TYPE
        //! \details
        //!     This field is used in both VC1 VLD mode and IT mode. It is the
        //!     same parameter as bPicExtrapolation in VC1 spec.The Picture
        //!     Structure and Progressive Pic Type are used to derive the picture
        //!     structure as specified in FCM, in VC1 VLD and IT mode.
        enum PROGRESSIVE_PIC_TYPE
        {
            PROGRESSIVE_PIC_TYPE_UNNAMED0                                    = 0, //!< progressive only picture
            PROGRESSIVE_PIC_TYPE_UNNAMED1                                    = 1, //!< progressive only picture
            PROGRESSIVE_PIC_TYPE_UNNAMED2                                    = 2, //!< interlace picture (frame-interlace or field-interlace)
            PROGRESSIVE_PIC_TYPE_UNNAMED3                                    = 3, //!< illegal
        };

        //! \brief RANGE_REDUCTION_ENABLE
        //! \details
        //!     This field specifies whether on-the-fly pixel value
        //!                         range reduction should be performed for the preceding (or forward)
        //!     reference
        //!                         picture. Along with RangeReductionScale to specify whether scale up
        //!     or down
        //!                         should be performed. It is not the same value as RANGEREDFRM Syntax
        //!     Element
        //!                         (_PictureParameters bPicDeblocked bit 5) in the Picture
        //!     Header.This field is
        //!                         for Main Profile only. Simple Profile is always disable, and not
        //!     applicable to
        //!                         Advanced Profile. This field is used in both VLD and IT modes.This
        //!     is derived by
        //!                         driver from the history of RANGERED and RANGEREDFRM syntax elements
        //!     (i.e. of
        //!                         forward/preceding reference picture) and those of the current
        //!     picture.RANGERED
        //!                         is the same as (bPicOverflowBlocks &gt;&gt; 3) &amp; 1. RANGEREDFRM
        //!     is the same
        //!                         as (bPicDeblocked &gt;&gt; 5) &amp; 1.For the current picture is a
        //!     B picture,
        //!                         this field represents the state of the forward/preceding reference
        //!     picture
        //!                         onlyDriver is responsible to keep RangeReductionScale,
        //!     RangeReduction Enable and
        //!                         RANGERED Present Flag of current picture coherent.
        enum RANGE_REDUCTION_ENABLE
        {
            RANGE_REDUCTION_ENABLE_DISABLE                                   = 0, //!< Range reduction is not performed
            RANGE_REDUCTION_ENABLE_ENABLE                                    = 1, //!< Range reduction is performed
        };

        //! \brief RANGE_REDUCTION_SCALE
        //! \details
        //!     This field specifies whether the reference picture
        //!                         pixel values should be scaled up or scaled down on-the-fly, if
        //!     RangeReduction is
        //!                         Enabled.NOTE: This bit is derived by driver for Main Profile only.
        //!     Ignored in
        //!                         Simple and Advanced Profiles. This field is used in both VLD and IT
        //!     modes.This
        //!                         is derived by driver from the history of RANGERED and RANGEREDFRM
        //!     syntax
        //!                         elements (i.e. of forward/preceding reference picture) and those of
        //!     the current
        //!                         picture. RANGERED is the same as (bPicOverflowBlocks &gt;&gt; 3)
        //!     &amp; 1.
        //!                         RANGEREDFRM is the same as (bPicDeblocked &gt;&gt; 5) &amp; 1. For
        //!     the current
        //!                         picture is a B picture, this field represents the state of the
        //!     forward/preceding
        //!                         reference picture onlyDriver is responsible to keep
        //!     RangeReductionScale,
        //!                         RangeReduction Enable and RANGERED Present Flag of current picture
        //!                         coherent.
        enum RANGE_REDUCTION_SCALE
        {
            RANGE_REDUCTION_SCALE_DISABLE                                    = 0, //!< Scale down reference picture by factor of 2
            RANGE_REDUCTION_SCALE_ENABLE                                     = 1, //!< Scale up reference picture by factor of 2
        };

        //! \brief OVERLAP_SMOOTHING_ENABLE_FLAG
        //! \details
        //!     This field is the decoded syntax element OVERLAP in bitstreamIndicates
        //!     if Overlap smoothing is ON at the picture levelThis field is used in
        //!     both VLD and IT modes
        enum OVERLAP_SMOOTHING_ENABLE_FLAG
        {
            OVERLAP_SMOOTHING_ENABLE_FLAG_DISABLE                            = 0, //!< to disable overlap smoothing filter
            OVERLAP_SMOOTHING_ENABLE_FLAG_ENABLE                             = 1, //!< to enable overlap smoothing filter
        };

        enum EXTENDED_DMV_PRESENT_FLAG
        {
            EXTENDED_DMV_PRESENT_FLAG_UNNAMED0                               = 0, //!< Extended_DMV is not present in the picture header
            EXTENDED_DMV_PRESENT_FLAG_UNNAMED1                               = 1, //!< Extended_DMV is present in the picture header
        };

        //! \brief REFPIC_FLAG
        //! \details
        //!     For a BI picture, REFPIC flag must set to 0For I and P picture, REFPIC
        //!     flag must set to 0.For a B picture, REFPIC flag must set to 0, except
        //!     for a B-field in interlaced field mode which can be 0 or 1 (e.g. the top
        //!     B field can be used as a reference for decoding its corresponding bottom
        //!     B-field in a field pair).In VLD mode, this flag cannot be used as an
        //!     optimization signaling for an I or P picture that is not used as a
        //!     reference picture.This field is used in both VC1 VLD mode and IT
        //!     mode. It is the same parameter as bPicDeblockConfined[bit2] in VC1
        //!     spec.The Intra Picture Flag, Backward Prediction Present Flag and
        //!     RefPicFlag are used to derive the picture type, as specified in PTYPE
        //!     for a frame, and in FPTYPE for a field, in VC1 VLD and IT mode.
        enum REFPIC_FLAG
        {
            REFPIC_FLAG_UNNAMED0                                             = 0, //!< the current picture after decoded, will never used as a reference picture
            REFPIC_FLAG_UNNAMED1                                             = 1, //!< the current picture after decoded, will be used as a reference picture later
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_VC1_SHORT_PIC_STATE_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MFX_VC1_DIRECTMODE_STATE
    //! \details
    //!     This is a picture level command and should be issued only once, even for
    //!     a multi-slices picture.   There is only one DMV buffer for read (when
    //!     processing a B-picture) and one for write (when processing a P-Picture).
    //!      Each DMV record is 64 bits per MB, to store the top and bottom field
    //!     MVs (32-bit MVx,y each).
    //!     
    struct MFX_VC1_DIRECTMODE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 DirectMvWriteBufferBaseAddressForTheCurrentPicture : __CODEGEN_BITFIELD( 6, 31)    ; //!< Direct MV Write Buffer Base Address for the Current Picture
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 DirectMvWriteBufferBaseAddressForTheCurrentPicture4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Direct MV Write Buffer Base Address for the Current Picture [47:32]
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 DirectMvWriteBufferBaseAddressForTheCurrentPictureArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 DirectMvWriteBufferMemoryCompressionEnable       : __CODEGEN_BITFIELD( 9,  9)    ; //!< Direct MV Write Buffer - Memory Compression Enable
                uint32_t                 DirectMvWriteBufferMemoryCompressionMode         : __CODEGEN_BITFIELD(10, 10)    ; //!< DIRECT_MV_WRITE_BUFFER__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved107                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 DirectMvWriteBufferTiledResourceMode             : __CODEGEN_BITFIELD(13, 14)    ; //!< DIRECT_MV_WRITE_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 DirectMvReadBufferBaseAddressForTheReferencePicture : __CODEGEN_BITFIELD( 6, 31)    ; //!< Direct MV Read Buffer Base Address for the Reference Picture
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 DirectMvReadBufferBaseAddressForTheCurrentPicture4732 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Direct MV Read Buffer Base Address for the Current Picture [47:32]
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 DirectMvReadBufferBaseAddressForTheCurrentPictureArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 DirectMvReadBufferMemoryCompressionEnable        : __CODEGEN_BITFIELD( 9,  9)    ; //!< Direct MV Read Buffer - Memory Compression Enable
                uint32_t                 DirectMvReadBufferMemoryCompressionMode          : __CODEGEN_BITFIELD(10, 10)    ; //!< DIRECT_MV_READ_BUFFER__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved203                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 DirectMvReadBufferTiledResourceMode              : __CODEGEN_BITFIELD(13, 14)    ; //!< DIRECT_MV_READ_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved207                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED2                                             = 2, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VC1COMMON                                   = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXVC1DIRECTMODESTATE                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
        {
            DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            DIRECT_MV_WRITE_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_WRITE_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a
        //!                         <b>Memory Data Formats</b> chapter, <b>Media Memory Compression</b>
        //!     section for more details.
        enum DIRECT_MV_WRITE_BUFFER__MEMORY_COMPRESSION_MODE
        {
            DIRECT_MV_WRITE_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            DIRECT_MV_WRITE_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief DIRECT_MV_WRITE_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum DIRECT_MV_WRITE_BUFFER__TILED_RESOURCE_MODE
        {
            DIRECT_MV_WRITE_BUFFER_TILED_RESOURCE_MODE_TRMODENONE            = 0, //!< No tiled resource
            DIRECT_MV_WRITE_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF          = 1, //!< 4KB tiled resources
            DIRECT_MV_WRITE_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS          = 2, //!< 64KB tiled resources
        };

        //! \brief DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE__ARBITRATION_PRIORITY_CONTROL
        {
            DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            DIRECT_MV_READ_BUFFER_BASE_ADDRESS_FOR_THE_CURRENT_PICTURE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_READ_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a
        //!                         <b>Memory Data Formats</b> chapter, <b>Media Memory Compression</b>
        //!     section for more details.
        enum DIRECT_MV_READ_BUFFER__MEMORY_COMPRESSION_MODE
        {
            DIRECT_MV_READ_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            DIRECT_MV_READ_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief DIRECT_MV_READ_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum DIRECT_MV_READ_BUFFER__TILED_RESOURCE_MODE
        {
            DIRECT_MV_READ_BUFFER_TILED_RESOURCE_MODE_TRMODENONE             = 0, //!< No tiled resource
            DIRECT_MV_READ_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF           = 1, //!< 4KB tiled resources
            DIRECT_MV_READ_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS           = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_VC1_DIRECTMODE_STATE_CMD();

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief MFD_VC1_BSD_OBJECT
    //! \details
    //!     The MFD_VC1_BSD_OBJECT command is the only primitive command for the VC1
    //!     Decoding Pipeline. The macroblock data portion of the bitstream is
    //!     loaded as indirect data object.Before issuing a MFD_VC1_BSD_OBJECT
    //!     command, all VC1 states of the MFD Engine need to be valid. Therefore
    //!     the commands used to set these states need to have been issued prior to
    //!     the issue of a MFD_VC1_BSD_OBJECT command.VC1 deblock filter kernel
    //!     cross the slice boundary if in the last MB row of a slice, so need to
    //!     know the last MB row of a slice to disable the edge mask. There is why
    //!     VC1 BSD hardware need to know the end of MB address for the current
    //!     slice. As such no more phantom slice is needed for VC1, as long as the
    //!     driver will program both start MB address in the current slice and the
    //!     start MB address of the next slice. As a result, we can also support
    //!     multiple picture state commands in between slices.
    //!     
    struct MFD_VC1_BSD_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectBsdDataLength                            : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect BSD Data Length
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 NextSliceVerticalPosition                        : __CODEGEN_BITFIELD( 0,  8)    ; //!< Next Slice Vertical Position
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 SliceStartVerticalPosition                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Slice Start Vertical Position
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FirstmbbitoffsetFirstMacroblockBitOffset         : __CODEGEN_BITFIELD( 0,  2)    ; //!< FirstMbBitOffset (First Macroblock Bit Offset )
                uint32_t                 Reserved131                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 EmulationPreventionBytePresent                   : __CODEGEN_BITFIELD( 4,  4)    ; //!< EMULATION_PREVENTION_BYTE_PRESENT
                uint32_t                 Reserved133                                      : __CODEGEN_BITFIELD( 5, 15)    ; //!< Reserved
                uint32_t                 FirstMbByteOffsetOfSliceDataOrSliceHeader        : __CODEGEN_BITFIELD(16, 31)    ; //!< First_MB_Byte_Offset of Slice Data or Slice Header
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VC1DEC                                      = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum EMULATION_PREVENTION_BYTE_PRESENT
        {
            EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED0                       = 0, //!< H/W needs to perform Emulation Byte Removal
            EMULATION_PREVENTION_BYTE_PRESENT_UNNAMED1                       = 1, //!< H/W does not need to perform Emulation Byte Removal
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_VC1_BSD_OBJECT_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief MFX_JPEG_PIC_STATE
    //! \details
    //!     
    //!     
    struct MFX_JPEG_PIC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 OutputMcuStructure                               : __CODEGEN_BITFIELD( 0,  2)    ; //!< OUTPUT_MCU_STRUCTURE, Encoder Only
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved, Encoder Only
                uint32_t                 InputSurfaceFormatYuv                            : __CODEGEN_BITFIELD( 8, 11)    ; //!< INPUT_SURFACE_FORMAT_YUV, Encoder Only
                uint32_t                 Reserved44                                       : __CODEGEN_BITFIELD(12, 20)    ; //!< Reserved, Encoder Only
                uint32_t                 PixelsInVerticalLastMcu                          : __CODEGEN_BITFIELD(21, 25)    ; //!< Pixels In Vertical Last MCU, Encoder Only
                uint32_t                 PixelsInHorizontalLastMcu                        : __CODEGEN_BITFIELD(26, 30)    ; //!< Pixels In Horizontal Last MCU, Encoder Only
                uint32_t                 Reserved63                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved, Encoder Only
            } Obj0;
            struct
            {
                uint32_t                 InputFormatYuv                                   : __CODEGEN_BITFIELD( 0,  2)    ; //!< INPUT_FORMAT_YUV, Decoder Only
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved, Decoder Only
                uint32_t                 Rotation                                         : __CODEGEN_BITFIELD( 4,  5)    ; //!< ROTATION, Decoder Only
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved, Decoder Only
                uint32_t                 OutputFormatYuv                                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< OUTPUT_FORMAT_YUV, Decoder Only
                uint32_t                 Reserved44                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 AverageDownSampling                              : __CODEGEN_BITFIELD(16, 16)    ; //!< AVERAGE_DOWN_SAMPLING, Decoder Only
                uint32_t                 VerticalDownSamplingEnable                       : __CODEGEN_BITFIELD(17, 17)    ; //!< VERTICAL_DOWN_SAMPLING_ENABLE, Decoder Only
                uint32_t                 HorizontalDownSamplingEnable                     : __CODEGEN_BITFIELD(18, 18)    ; //!< HORIZONTAL_DOWN_SAMPLING_ENABLE, Decoder Only
                uint32_t                 Reserved51                                       : __CODEGEN_BITFIELD(19, 19)    ; //!< Reserved, Decoder Only
                uint32_t                 VerticalUpSamplingEnable                         : __CODEGEN_BITFIELD(20, 20)    ; //!< VERTICAL_UP_SAMPLING_ENABLE, Decoder Only
                uint32_t                 Reserved53                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved, Decoder Only
            } Obj1;
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FrameWidthInBlocksMinus1                         : __CODEGEN_BITFIELD( 0, 12)    ; //!< Frame Width In Blocks Minus 1, Decoder Only
                uint32_t                 Reserved77                                       : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 FrameHeightInBlocksMinus1                        : __CODEGEN_BITFIELD(16, 28)    ; //!< Frame Height In Blocks Minus 1, Decoder Only
                uint32_t                 OutputPixelNormalize                             : __CODEGEN_BITFIELD(29, 29)    ; //!< OUTPUT_PIXEL_NORMALIZE, Decoder Only
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved, Decoder Only
            } Obj0;
            struct
            {
                uint32_t                 FrameWidthInBlksMinus1                           : __CODEGEN_BITFIELD( 0, 12)    ; //!< Frame Width In Blks Minus 1, Encoder Only
                uint32_t                 Roundingquant                                    : __CODEGEN_BITFIELD(13, 15)    ; //!< ROUNDINGQUANT, Encoder Only
                uint32_t                 FrameHeightInBlksMinus1                          : __CODEGEN_BITFIELD(16, 28)    ; //!< Frame Height In Blks Minus 1, Encoder Only
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved, Encoder Only
            } Obj1;
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_MEDIA                                                = 0, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_COMMON                                               = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_JPEG                                        = 7, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief OUTPUT_MCU_STRUCTURE
        //! \details
        //!     <p></p>
        //!                         Output MCU Structure(<b>OutputMcuStructure</b>) should be set
        //!     accordingly for each Input Surface Format
        //!     YUV(<b>InputSurfaceFormatYUV</b>):
        //!                         <ul>
        //!                             <li>If <b>InputSurfaceFormatYUV</b> is set to NV12,
        //!     <b>OutputMCUStructure</b> is set to YUV420.</li>
        //!                             <li>If <b>InputSurfaceFormatYUV</b> is set to UYVY or YUY2,
        //!     <b>OutputMCUStructure</b> is set to YUV422H_2Y.</li>
        //!                             <li>If <b>InputSurfaceFormatYUV</b> is set to Y8,
        //!     <b>OutputMCuStructure</b> is set to YUV400.</li>
        //!                             <li>If <b>InputSurfaceFormatYUV</b> is set to RGB (or GBR, BGR,
        //!     YUV), <b>OutputMCuStructure</b> is set to RGB.</li>
        //!                             <li>If <b>InputSurfaceFormatYUV</b> is set to RGB, the order of
        //!     encoded blocks in MCU will be same as the order of input image
        //!     components. 
        //!                                 If the order of input image components is RGB (or GBR, BGR, YUV),
        //!     then the order of blocks will be RGB (or GBR, BGR, YUV
        //!     respectively).</li>
        //!                         </ul>
        //!                         <p></p>
        enum OUTPUT_MCU_STRUCTURE
        {
            OUTPUT_MCU_STRUCTURE_YUV_400                                     = 0, //!< Grayscale Image
            OUTPUT_MCU_STRUCTURE_YUV_420                                     = 1, //!< Both horizontally and vertically chroma 2:1 subsampled
            OUTPUT_MCU_STRUCTURE_YUV_422H2Y                                  = 2, //!< Horizontally chroma 2:1 subsampled - horizontal 2 Y-blocks,  1 U and 1 V block
            OUTPUT_MCU_STRUCTURE_RGB                                         = 3, //!< RGB or YUV444: No subsample
            OUTPUT_MCU_STRUCTURE_UNNAMED_4                                   = 4, //!< No additional details
            OUTPUT_MCU_STRUCTURE_UNNAMED5                                    = 5, //!< No additional details
            OUTPUT_MCU_STRUCTURE_UNNAMED6                                    = 6, //!< No additional details
            OUTPUT_MCU_STRUCTURE_UNNAMED7                                    = 7, //!< No additional details
        };

        enum INPUT_FORMAT_YUV
        {
            INPUT_FORMAT_YUV_UNNAMED0                                        = 0, //!< YUV400 (grayscale image)
            INPUT_FORMAT_YUV_UNNAMED1                                        = 1, //!< YUV420
            INPUT_FORMAT_YUV_UNNAMED2                                        = 2, //!< YUV422H_2Y (Horizontally chroma 2:1 subsampled) - horizontal 2 Y-block, 1U and 1V
            INPUT_FORMAT_YUV_UNNAMED3                                        = 3, //!< YUV444
            INPUT_FORMAT_YUV_UNNAMED_4                                       = 4, //!< YUV411
            INPUT_FORMAT_YUV_UNNAMED5                                        = 5, //!< YUV422V_2Y (Vertically chroma 2:1 subsampled) - vertical 2 Y-blocks, 1U and 1V
            INPUT_FORMAT_YUV_UNNAMED6                                        = 6, //!< YUV422H_4Y - 2x2 Y-blocks, vertical 2U and 2V
            INPUT_FORMAT_YUV_UNNAMED7                                        = 7, //!< YUV422V_4Y - 2x2 Y-blocks, horizontal 2U and 2V
        };

        //! \brief ROTATION
        //! \details
        //!     Rotation can be set to 01b, 10b, or 11b when OutputFormatYUV is set to
        //!     0000b. For other OutputFormatYUV, Rotation is not allowed.
        enum ROTATION
        {
            ROTATION_UNNAMED0                                                = 0, //!< no rotation
            ROTATION_UNNAMED1                                                = 1, //!< rotate clockwise 90 degree
            ROTATION_UNNAMED2                                                = 2, //!< rotate counter-clockwise 90 degree (same as rotating 270 degree clockwise)
            ROTATION_UNNAMED3                                                = 3, //!< rotate 180 degree (NOT the same as flipped on the x-axis)
        };

        //! \brief OUTPUT_FORMAT_YUV
        //! \details
        //!     This field specifies the surface format to write the decoded JPEG
        //!     image.Note that any non-interleaved JPEG input should be set to "0000".
        //!     For the interleaved input Scan data, it can be set either "0000" or the
        //!     corresponding format.
        enum OUTPUT_FORMAT_YUV
        {
            OUTPUT_FORMAT_YUV_UNNAMED0                                       = 0, //!< 3 separate plane for Y, U, and V respectively
            OUTPUT_FORMAT_YUV_UNNAMED1                                       = 1, //!< NV12 for chroma 4:2:0
            OUTPUT_FORMAT_YUV_UNNAMED2                                       = 2, //!< UYVY for chroma 4:2:2
            OUTPUT_FORMAT_YUV_UNNAMED3                                       = 3, //!< YUY2 for chroma 4:2:2
        };

        //! \brief INPUT_SURFACE_FORMAT_YUV
        //! \details
        //!     This field specifies the surface format to read a YUV image data
        enum INPUT_SURFACE_FORMAT_YUV
        {
            INPUT_SURFACE_FORMAT_YUV_UNNAMED0                                = 0, //!< Reserved
            INPUT_SURFACE_FORMAT_YUV_NV12                                    = 1, //!< NV12 for chroma 4:2:0
            INPUT_SURFACE_FORMAT_YUV_UYVY                                    = 2, //!< UYVY for chroma 4:2:2
            INPUT_SURFACE_FORMAT_YUV_YUY2                                    = 3, //!< YUY2 for chroma 4:2:2
            INPUT_SURFACE_FORMAT_YUV_Y8                                      = 4, //!< Y8 for chroma400 Y-only image
            INPUT_SURFACE_FORMAT_YUV_RGB                                     = 5, //!< RGB or YUV for chroma 4:4:4
        };

        //! \brief AVERAGE_DOWN_SAMPLING
        //! \details
        //!     This flag is used to select a down-sampling method when
        //!     <b>VertDownSamplingEnb</b> or <b>HoriDownSamplingEnb</b> is set to 1.
        enum AVERAGE_DOWN_SAMPLING
        {
            AVERAGE_DOWN_SAMPLING_UNNAMED0                                   = 0, //!< Drop every other line (or column) pixels
            AVERAGE_DOWN_SAMPLING_UNNAMED1                                   = 1, //!< Average neighboring two pixels
        };

        //! \brief VERTICAL_DOWN_SAMPLING_ENABLE
        //! \details
        //!     Only applied to chroma blocks. This flag is used for 2:1 vertical
        //!     down-sampling for chroma 422 and outputting chroma420 NV21 format. To
        //!     enable this flag, the input should be interleaved Scan,
        //!     <b>InputFormatYUV</b> should be set to YUV422H_2Y or YUV422H_4Y, and
        //!     <b>OutputFormatYUV</b> should be set to NV12.
        enum VERTICAL_DOWN_SAMPLING_ENABLE
        {
            VERTICAL_DOWN_SAMPLING_ENABLE_UNNAMED0                           = 0, //!< no down-sampling
            VERTICAL_DOWN_SAMPLING_ENABLE_UNNAMED1                           = 1, //!< 2:1 vertical down-sampling
        };

        //! \brief HORIZONTAL_DOWN_SAMPLING_ENABLE
        //! \details
        //!     Only applied to chroma blocks. This flag is used for 2:1 horizontal
        //!     down-sampling for chroma 422 and outputting chroma420 NV21 format. To
        //!     enable this flag, the input should be interleaved Scan,
        //!     <b>InputFormatYUV</b> should be set to YUV422V_2Y or YUV422V_4Y, and
        //!     <b>OutputFormatYUV</b> should be set to NV12.
        enum HORIZONTAL_DOWN_SAMPLING_ENABLE
        {
            HORIZONTAL_DOWN_SAMPLING_ENABLE_UNNAMED0                         = 0, //!< no down-sampling
            HORIZONTAL_DOWN_SAMPLING_ENABLE_UNNAMED1                         = 1, //!< 2:1 horizonatl down-sampling
        };

        //! \brief VERTICAL_UP_SAMPLING_ENABLE
        //! \details
        //!     Only applied to chroma blocks. This flag is used for 2:1 vertical
        //!     up-sampling for chroma 420 and outputting chroma422 YUY2 or UYVY format.
        //!     To enable this flag, the input should be interleaved Scan,
        //!     <b>InputFormatYUV</b> should be set to YUV420, and
        //!     <b>OutputFormatYUV</b> should be set to YUY2 or UYVY.
        enum VERTICAL_UP_SAMPLING_ENABLE
        {
            VERTICAL_UP_SAMPLING_ENABLE_UNNAMED0                             = 0, //!< no up-sampling
            VERTICAL_UP_SAMPLING_ENABLE_UNNAMED1                             = 1, //!< 2:1 vertical up-sampling
        };

        //! \brief ROUNDINGQUANT
        //! \details
        //!     Rounding value applied to quantization output
        enum ROUNDINGQUANT
        {
            ROUNDINGQUANT_UNNAMED0                                           = 0, //!< 1/2
            ROUNDINGQUANT_UNNAMED1                                           = 1, //!< (1/2 - 1/128)
            ROUNDINGQUANT_UNNAMED2                                           = 2, //!< (1/2 + 1/128)
            ROUNDINGQUANT_UNNAMED3                                           = 3, //!< (1/2 - 1/64)
            ROUNDINGQUANT_UNNAMED_4                                          = 4, //!< (1/2 + 1/64)
            ROUNDINGQUANT_UNNAMED5                                           = 5, //!< (1/2 - 1/32)
            ROUNDINGQUANT_UNNAMED6                                           = 6, //!< (1/2 - 1/16)
            ROUNDINGQUANT_UNNAMED7                                           = 7, //!< (1/2 - 1/8)
        };

        //! \brief OUTPUT_PIXEL_NORMALIZE
        //! \details
        //!     JPEG decoded output pixels for Y and U/V in order to adjust display YUV
        //!     range.
        enum OUTPUT_PIXEL_NORMALIZE
        {
            OUTPUT_PIXEL_NORMALIZE_UNNAMED0                                  = 0, //!< No Normalization
            OUTPUT_PIXEL_NORMALIZE_UNNAMED1                                  = 1, //!< Normalize output pixels from [0,255] to [16,235]
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_JPEG_PIC_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief MFC_JPEG_HUFF_TABLE_STATE
    //! \details
    //!     This Huffman table commands contains both DC and AC tables for either
    //!     luma or chroma. Once a Huffman table has been defined for a particular
    //!     destination, it replaces the previous tables stored in that destination
    //!     and shall be used in the remaining Scans of the current image.  Two
    //!     Huffman tables for luma and chroma will be sent to H/W, and chroma table
    //!     is used for both U and V.
    //!     
    struct MFC_JPEG_HUFF_TABLE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 HuffTableId                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< HUFF_TABLE_ID
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        uint32_t                         DcTable[12];                                                                     //!< DC_TABLE


        uint32_t                         AcTable[162];                                                                    //!< AC_TABLE


        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_MEDIA                                                = 3, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_COMMON                                               = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_JPEG                                        = 7, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFCJPEGHUFFTABLESTATE                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief HUFF_TABLE_ID
        //! \details
        //!     Huffman table destination identifier will specify one of two
        //!     destinations at the encoder into which the Huffman table must be stored.
        enum HUFF_TABLE_ID
        {
            HUFF_TABLE_ID_UNNAMED0                                           = 0, //!< Huffman table 0
            HUFF_TABLE_ID_UNNAMED1                                           = 1, //!< Huffman table 1
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFC_JPEG_HUFF_TABLE_STATE_CMD();

        static const size_t dwSize = 176;
        static const size_t byteSize = 704;
    };

    //!
    //! \brief MFD_JPEG_BSD_OBJECT
    //! \details
    //!     
    //!     
    struct MFD_JPEG_BSD_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IndirectDataLength                                                               ; //!< Indirect Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 ScanVerticalPosition                             : __CODEGEN_BITFIELD( 0, 12)    ; //!< Scan Vertical Position
                uint32_t                 Reserved109                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 ScanHorizontalPosition                           : __CODEGEN_BITFIELD(16, 28)    ; //!< Scan Horizontal Position
                uint32_t                 Reserved125                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 McuCount                                         : __CODEGEN_BITFIELD( 0, 25)    ; //!< MCU Count
                uint32_t                 Reserved154                                      : __CODEGEN_BITFIELD(26, 26)    ; //!< Reserved
                uint32_t                 ScanComponents                                   : __CODEGEN_BITFIELD(27, 29)    ; //!< Scan Components
                uint32_t                 Interleaved                                      : __CODEGEN_BITFIELD(30, 30)    ; //!< INTERLEAVED
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Restartinterval16Bit                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< RestartInterval(16 bit)
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED8                                             = 8, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED1                                             = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_JPEGDEC                                     = 7, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDJPEGBSDOBJECT                                        = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum INTERLEAVED
        {
            INTERLEAVED_NON_INTERLEAVED                                      = 0, //!< one component in the Scan
            INTERLEAVED_INTERLEAVED                                          = 1, //!< multiple components in the Scan
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_JPEG_BSD_OBJECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief MFC_JPEG_SCAN_OBJECT
    //! \details
    //!     Encoder Only
    //!     
    struct MFC_JPEG_SCAN_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 McuCount                                         : __CODEGEN_BITFIELD( 0, 25)    ; //!< MCU Count
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 RestartInterval                                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Restart Interval
                uint32_t                 IsLastScan                                       : __CODEGEN_BITFIELD(16, 16)    ; //!< IS_LAST_SCAN
                uint32_t                 HeadPresentFlag                                  : __CODEGEN_BITFIELD(17, 17)    ; //!< HEAD_PRESENT_FLAG
                uint32_t                 HuffmanDcTable                                   : __CODEGEN_BITFIELD(18, 20)    ; //!< HUFFMAN_DC_TABLE
                uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                uint32_t                 HuffmanAcTable                                   : __CODEGEN_BITFIELD(22, 24)    ; //!< HUFFMAN_AC_TABLE
                uint32_t                 Reserved89                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED9                                             = 9, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED2                                             = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_JPEGENC                                     = 7, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFCJPEGSCANOBJECT                                       = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief IS_LAST_SCAN
        //! \details
        //!     If this flag is set, then HW will insert EOI (0xFFD9) to the end of Scan
        //!     encoded bitstream.
        enum IS_LAST_SCAN
        {
            IS_LAST_SCAN_UNNAMED0                                            = 0, //!< Not the last Scan. 
            IS_LAST_SCAN_UNNAMED1                                            = 1, //!< Indicates that the current Scan is the last one.
        };

        //! \brief HEAD_PRESENT_FLAG
        //! \details
        //!     If this flag is set to 0, then no MFC_JPEG_PAK_INSERT_OBJECT commands
        //!     will be sent. 
        //!                         If this flag is set to 1, then one or more
        //!     MFC_JPEG_PAK_INSERT_OBJECT commands will be sent after
        //!     MFC_JPEG_SCAN_OBJECT command.
        enum HEAD_PRESENT_FLAG
        {
            HEAD_PRESENT_FLAG_UNNAMED0                                       = 0, //!< No insertion into the output bitstream buffer before Scan encoded bitstream
            HEAD_PRESENT_FLAG_UNNAMED1                                       = 1, //!< Headers, tables, App data insertion into the output bitstream buffer. HW will insert the insertion data before the Scan encoded bitstream.
        };

        //! \brief HUFFMAN_DC_TABLE
        //! \details
        //!     DC Huffman table destination selector specifies one of two possible DC
        //!     table destinations for each Y, U, V, or R, G, B.
        //!                         The DC Huffman tables shall have been loaded in destination 0 and 1
        //!     by the time of issuing MFC_JPEG_HUFF_TABLE_STATE Command.
        enum HUFFMAN_DC_TABLE
        {
            HUFFMAN_DC_TABLE_BIT20V0                                         = 0, //!< The third image component must use the DC table 0.
            HUFFMAN_DC_TABLE_BIT19U0                                         = 0, //!< The second image component must use the DC table 0.
            HUFFMAN_DC_TABLE_BIT18Y0                                         = 0, //!< The first image component must use the DC table 0.
            HUFFMAN_DC_TABLE_BIT18Y1                                         = 1, //!< The first image component must use the DC table 1.
            HUFFMAN_DC_TABLE_BIT19U1                                         = 2, //!< The second image component must use the DC table 1.
            HUFFMAN_DC_TABLE_BIT20V1                                         = 4, //!< The third image component must use the DC table 1.
        };

        //! \brief HUFFMAN_AC_TABLE
        //! \details
        //!     AC Huffman table destination selector specifies one of two possible AC
        //!     table destinations for each Y, U, V, or R, G, B.
        //!                         The AC Huffman tables must have been loaded in destination 0 and 1
        //!     by the time of issuing MFC_JPEG_HUFF_TABLE_STATE Command.
        enum HUFFMAN_AC_TABLE
        {
            HUFFMAN_AC_TABLE_BIT2_4V0                                        = 0, //!< The third image component must use the AC table 0.
            HUFFMAN_AC_TABLE_BIT23U0                                         = 0, //!< The second image component must use the AC table 0.
            HUFFMAN_AC_TABLE_BIT22Y0                                         = 0, //!< The first image component must use the AC table 0.
            HUFFMAN_AC_TABLE_BIT22Y1                                         = 1, //!< The first image component must use the AC table 1.
            HUFFMAN_AC_TABLE_BIT23U1                                         = 2, //!< The second image component must use the AC table 1.
            HUFFMAN_AC_TABLE_BIT2_4V1                                        = 4, //!< The third image component must use the AC table 1.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFC_JPEG_SCAN_OBJECT_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief MFX_VP8_Encoder_CFG
    //! \details
    //!     This must be the very first command to issue after the surface state,
    //!     the pipe select and base address setting commands and must be issued
    //!     before MFX_VP8_PIC_STATE.
    //!     
    struct MFX_VP8_Encoder_CFG_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubOpcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUB_OPCODE_B
                uint32_t                 SubOpcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUB_OPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 PerformanceCounterEnable                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< Performance Counter Enable
                uint32_t                 FinalBitstreamOutputDisable                      : __CODEGEN_BITFIELD( 1,  1)    ; //!< Final Bitstream Output Disable
                uint32_t                 TokenStatisticsOutputEnable                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Token Statistics Output Enable
                uint32_t                 BitstreamStatisticsOutputEnable                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< Bitstream Statistics Output Enable
                uint32_t                 UpdateSegmentFeatureDataFlag                     : __CODEGEN_BITFIELD( 4,  4)    ; //!< Update Segment Feature Data Flag
                uint32_t                 SkipFinalBitstreamWhenOverUnderFlow              : __CODEGEN_BITFIELD( 5,  5)    ; //!< Skip Final Bitstream when Over / Under flow
                uint32_t                 RateControlInitialPass                           : __CODEGEN_BITFIELD( 6,  6)    ; //!< RATE_CONTROL_INITIAL_PASS
                uint32_t                 PerSegmentDeltaQindexLoopfilterDisable           : __CODEGEN_BITFIELD( 7,  7)    ; //!< Per Segment Delta Qindex / LoopFilter Disable
                uint32_t                 FinerBrcEnable                                   : __CODEGEN_BITFIELD( 8,  8)    ; //!< Finer BRC Enable
                uint32_t                 CompressedBitstreamOutputDisable                 : __CODEGEN_BITFIELD( 9,  9)    ; //!< Compressed Bitstream Output Disable
                uint32_t                 VbspunitpowerclockGatingDisable                  : __CODEGEN_BITFIELD(10, 10)    ; //!< VBSPunitPowerClock Gating Disable
                uint32_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 MaxFrameBitCountRateControlEnableMask            : __CODEGEN_BITFIELD( 0,  0)    ; //!< MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
                uint32_t                 MinFrameBitCountRateControlEnableMask            : __CODEGEN_BITFIELD( 1,  1)    ; //!< MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
                uint32_t                 MaxInterMbBitCountCheckEnableMask                : __CODEGEN_BITFIELD( 2,  2)    ; //!< Max Inter MB Bit Count Check Enable Mask
                uint32_t                 MaxIntraMbBitCountCheckEnableMask                : __CODEGEN_BITFIELD( 3,  3)    ; //!< Max Intra MB Bit Count Check Enable Mask
                uint32_t                 IntermediateBitBufferOverrunEnableMask           : __CODEGEN_BITFIELD( 4,  4)    ; //!< Intermediate Bit Buffer Overrun Enable Mask 
                uint32_t                 FinalBistreamBufferOverrunEnableMask             : __CODEGEN_BITFIELD( 5,  5)    ; //!< Final Bistream Buffer Overrun Enable Mask
                uint32_t                 QindexClampHighMaskForUnderflow                  : __CODEGEN_BITFIELD( 6,  6)    ; //!< Qindex_Clamp_High_mask for underflow
                uint32_t                 QindexClampHighMaskForOverflow                   : __CODEGEN_BITFIELD( 7,  7)    ; //!< Qindex_Clamp_High_mask for overflow
                uint32_t                 Reserved72                                       : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MaxInterMbBitCount                               : __CODEGEN_BITFIELD( 0, 11)    ; //!< Max Inter MB bit count
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MaxIntraMbBitCountLimit                          : __CODEGEN_BITFIELD(16, 27)    ; //!< Max Intra MB Bit Count Limit
                uint32_t                 Reserved124                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FrameBitRateMax                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< Frame Bit Rate Max
                uint32_t                 FrameBitRateMaxUnit                              : __CODEGEN_BITFIELD(14, 14)    ; //!< FRAME_BIT_RATE_MAX_UNIT
                uint32_t                 FrameBitrateMaxUnitMode                          : __CODEGEN_BITFIELD(15, 15)    ; //!< FRAME_BITRATE_MAX_UNIT_MODE
                uint32_t                 FrameBitRateMin                                  : __CODEGEN_BITFIELD(16, 29)    ; //!< Frame Bit Rate Min
                uint32_t                 FrameBitRateMinUnit                              : __CODEGEN_BITFIELD(30, 30)    ; //!< FRAME_BIT_RATE_MIN_UNIT
                uint32_t                 FrameBitrateMinUnitMode                          : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAME_BITRATE_MIN_UNIT_MODE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 FrameDeltaQindexMax0                             : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Delta QIndex Max [0]
                uint32_t                 FrameDeltaQindexMax1                             : __CODEGEN_BITFIELD( 8, 15)    ; //!< Frame Delta QIndex Max[1]
                uint32_t                 FrameDeltaqIndexMax2                             : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame DeltaQ Index Max[2]
                uint32_t                 FrameDeltaQindexMax3                             : __CODEGEN_BITFIELD(24, 31)    ; //!< Frame Delta QIndex Max[3]
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 FrameDeltaQindexMin0                             : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Delta QIndex Min[0]
                uint32_t                 FrameDeltaQindexMin1                             : __CODEGEN_BITFIELD( 8, 15)    ; //!< Frame Delta QIndex Min[1]
                uint32_t                 FrameDeltaQindexMin2                             : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame Delta QIndex Min[2]
                uint32_t                 FrameDeltaQindexMin3                             : __CODEGEN_BITFIELD(24, 31)    ; //!< Frame Delta QIndex Min[3]
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMax1                                                   ; //!< Per Segment Frame Delta QIndex Max[1]
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMin1                                                   ; //!< Per Segment Frame Delta QIndex Min[1]
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMax2                                                   ; //!< Per Segment Frame Delta QIndex Max[2]
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMin2                                                   ; //!< Per Segment Frame Delta QIndex Min[2]
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMax3                                                   ; //!< Per Segment Frame Delta QIndex Max[3]
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 PerSegmentFrameDeltaQindexMin3                                                   ; //!< Per Segment Frame Delta QIndex Min[3]
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 FrameDeltaLoopFilterMax0                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Delta Loop Filter Max[0]
                uint32_t                 FramEdeltaLoopFilterMax1                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< Fram eDelta Loop Filter Max[1]
                uint32_t                 FrameDeltaLoopFilterMax2                         : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame Delta Loop Filter Max[2]
                uint32_t                 FrameDeltaLoopFilterMax3                         : __CODEGEN_BITFIELD(24, 31)    ; //!< Frame Delta Loop Filter Max[3]
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 FrameDeltaLoopFilterMin0                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Delta Loop Filter Min[0]
                uint32_t                 FrameDeltaLoopFilterMin1                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< Frame Delta Loop Filter Min[1]
                uint32_t                 FrameDeltaLoopFilterMin2                         : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame Delta Loop Filter Min[2]
                uint32_t                 FrameDeltaLoopFilterMin3                         : __CODEGEN_BITFIELD(24, 31)    ; //!< Frame Delta Loop Filter Min[3]
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMax1                                               ; //!< Per Segment Frame Delta LoopFilter Max[1]
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMin1                                               ; //!< Per Segment Frame Delta LoopFilter Min[1]
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMax2                                               ; //!< Per Segment Frame Delta LoopFilter Max[2]
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMin2                                               ; //!< Per Segment Frame Delta LoopFilter Min[2]
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMax3                                               ; //!< Per Segment Frame Delta LoopFilter Max[3]
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 PerSegmentFrameDeltaLoopfilterMin3                                               ; //!< Per Segment Frame Delta LoopFilter Min[3]
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 FrameBitRateMaxDelta                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< Frame Bit Rate Max Delta
                uint32_t                 Reserved687                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Framebitratemindelta                             : __CODEGEN_BITFIELD(16, 30)    ; //!< FrameBitRateMinDelta
                uint32_t                 Reserved703                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 MinFrameWsize                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Min Frame WSize
                uint32_t                 MinFrameWsizeUnit                                : __CODEGEN_BITFIELD(16, 17)    ; //!< MIN_FRAME_WSIZE_UNIT
                uint32_t                 Reserved722                                      : __CODEGEN_BITFIELD(18, 19)    ; //!< Reserved
                uint32_t                 BitstreamFormatVersion                           : __CODEGEN_BITFIELD(20, 22)    ; //!< Bitstream Format Version
                uint32_t                 ShowFrame                                        : __CODEGEN_BITFIELD(23, 23)    ; //!< Show Frame
                uint32_t                 Reserved728                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 HorizontalSizeCode                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< Horizontal_Size_Code
                uint32_t                 VerticalSizeCode                                 : __CODEGEN_BITFIELD(16, 31)    ; //!< Vertical_Size_Code
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 FrameHeaderBitCount                                                              ; //!< Frame Header Bit Count
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 FrameHeaderBinBufferQindexUpdatePointer                                          ; //!< Frame Header Bin Buffer Qindex Update Pointer
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 FrameHeaderBinBufferLoopfilterUpdatePointer                                      ; //!< Frame Header Bin Buffer LoopFilter Update Pointer
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 FrameHeaderBinBufferTokenUpdatePointer                                           ; //!< Frame Header Bin Buffer Token Update Pointer
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 FrameHeaderBinBufferMvupdatePointer                                              ; //!< Frame Header Bin Buffer MVUpdate Pointer
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 Cv0ClampValue0                                   : __CODEGEN_BITFIELD( 0,  3)    ; //!< CV0 - Clamp Value 0
                uint32_t                 Cv1                                              : __CODEGEN_BITFIELD( 4,  7)    ; //!< CV1
                uint32_t                 Cv2                                              : __CODEGEN_BITFIELD( 8, 11)    ; //!< CV2
                uint32_t                 Cv3                                              : __CODEGEN_BITFIELD(12, 15)    ; //!< CV3
                uint32_t                 Cv4                                              : __CODEGEN_BITFIELD(16, 19)    ; //!< CV4
                uint32_t                 Cv5                                              : __CODEGEN_BITFIELD(20, 23)    ; //!< CV5
                uint32_t                 Cv6                                              : __CODEGEN_BITFIELD(24, 27)    ; //!< CV6
                uint32_t                 ClampvaluesCv7                                   : __CODEGEN_BITFIELD(28, 31)    ; //!< ClampValues - CV7
            };
            uint32_t                     Value;
        } DW29;

        //! \name Local enumerations

        enum SUB_OPCODE_B
        {
            SUB_OPCODE_B_MFXVP8ENCODERCFG                                    = 1, //!< No additional details
        };

        enum SUB_OPCODE_A
        {
            SUB_OPCODE_A_VP8COMMON                                           = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VP8                                         = 4, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_VIDEOCODEC                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum RATE_CONTROL_INITIAL_PASS
        {
            RATE_CONTROL_INITIAL_PASS_SUBSEQUENCEPASSES                      = 0, //!< No additional details
            RATE_CONTROL_INITIAL_PASS_INITIALPASS                            = 1, //!< No additional details
        };

        //! \brief MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
        //! \details
        //!     Enable Max. Frame Rate Control.
        //!                         This is a mask bit controlling if the condition of frame level bit
        //!     count is greater than or equal to FrameBitRateMax.
        enum MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
        {
            MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED0            = 0, //!< Do not update bit[0] of MFX_VP8_IMAGE_STATUS control register.
            MAX_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED1            = 1, //!< If (Total Frame Level Bit Counter) >= (Frame Bit Rate Maximum Limit)Set bit[0] and bit[1] of MFX_VP8_IMAGE_STATUS control register.
        };

        //! \brief MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
        //! \details
        //!     Enable Min. Frame Rate Control. 
        //!                         This is a mask bit controlling if the condition of frame level bit
        //!     count is less than or equal to FrameBitRateMin.
        enum MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK
        {
            MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED0            = 0, //!< Do not update bit[0] of MFX_VP8_IMAGE_STATUS Control Register.
            MIN_FRAME_BIT_COUNT_RATE_CONTROL_ENABLE_MASK_UNNAMED1            = 1, //!< If (Total Frame Level Bit Counter)  =< (Frame Bit Rate Minimum limit)Set bit[0] and bit[1] of MFX_VP8_IMAGE_STATUS Control Register.
        };

        //! \brief FRAME_BIT_RATE_MAX_UNIT
        //! \details
        //!     <i>This field is Frame Bitrate Maximum Mode</i>
        enum FRAME_BIT_RATE_MAX_UNIT
        {
            FRAME_BIT_RATE_MAX_UNIT_32_B                                     = 0, //!< No additional details
            FRAME_BIT_RATE_MAX_UNIT_4_KB                                     = 1, //!< No additional details
        };

        //! \brief FRAME_BITRATE_MAX_UNIT_MODE
        //! \details
        //!     This field is the Frame Bitrate Maximum Limit Units.
        enum FRAME_BITRATE_MAX_UNIT_MODE
        {
            FRAME_BITRATE_MAX_UNIT_MODE_COMPATIBILITYMODE                    = 0, //!< Frame BitRate Max Unit is in old mode (128b/16Kb)
            FRAME_BITRATE_MAX_UNIT_MODE_NEWMODE                              = 1, //!< Frame BitRate Max Unit is in new mode (32byte/4Kb)
        };

        //! \brief FRAME_BIT_RATE_MIN_UNIT
        //! \details
        //!     <i>This field is Frame Bitrate Minimum Mode.</i>
        enum FRAME_BIT_RATE_MIN_UNIT
        {
            FRAME_BIT_RATE_MIN_UNIT_32_B                                     = 0, //!< No additional details
            FRAME_BIT_RATE_MIN_UNIT_4_KB                                     = 1, //!< No additional details
        };

        //! \brief FRAME_BITRATE_MIN_UNIT_MODE
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit Units.
        enum FRAME_BITRATE_MIN_UNIT_MODE
        {
            FRAME_BITRATE_MIN_UNIT_MODE_COMPATIBILITYMODE                    = 0, //!< Frame BitRate Min Unit is in old mode (128b/16Kb)
            FRAME_BITRATE_MIN_UNIT_MODE_NEWMODE                              = 1, //!< Frame BitRate Min Unit is in new mode (32byte/4Kb)
        };

        enum MIN_FRAME_WSIZE_UNIT
        {
            MIN_FRAME_WSIZE_UNIT_COMPATIBILITYMODE                           = 0, //!< MinFrameWSizeUnit is in old mode (128b/16Kb)
            MIN_FRAME_WSIZE_UNIT_NEWMODE                                     = 1, //!< MinFrameWSizeUnit is in new mode (32byte/4Kb)
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_VP8_Encoder_CFG_CMD();

        static const size_t dwSize = 30;
        static const size_t byteSize = 120;
    };

    //!
    //! \brief MFX_VP8_BSP_BUF_BASE_ADDR_STATE
    //! \details
    //!     
    //!     
    struct MFX_VP8_BSP_BUF_BASE_ADDR_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubOpcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUB_OPCODE_B
                uint32_t                 SubOpcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUB_OPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 FrameHeaderBaseAddr                              : __CODEGEN_BITFIELD( 6, 31)    ; //!< Frame Header Base Addr
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FrameHeaderBaseAddrUpperRange                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Frame Header Base Addr - Upper Range
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 FrameHeaderBaseAddrAgeForQuadlruAge              : __CODEGEN_BITFIELD( 0,  1)    ; //!< FRAME_HEADER_BASE_ADDR__AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved98                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 FrameHeaderBaseAddrTargetCacheTc                 : __CODEGEN_BITFIELD( 3,  4)    ; //!< FRAME_HEADER_BASE_ADDR__TARGET_CACHE_TC
                uint32_t                 ForFrameheaderbaseaddrLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; //!< FOR_FRAMEHEADERBASEADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 FrameHeaderBaseAddrArbitrationPriorityControl    : __CODEGEN_BITFIELD( 7,  8)    ; //!< FRAME_HEADER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved105                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 IntermediateBufferBaseAddr                       : __CODEGEN_BITFIELD( 6, 31)    ; //!< Intermediate Buffer Base Addr 
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IntermediateBufferBaseAddrUpperRange             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intermediate Buffer Base Addr - Upper Range
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 IntermediateBufferBaseAddrAgeForQuadlruAge       : __CODEGEN_BITFIELD( 0,  1)    ; //!< INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved194                                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 IntermediateBufferBaseAddrTargetCacheTc          : __CODEGEN_BITFIELD( 3,  4)    ; //!< INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC
                uint32_t                 IntermediateBufferBaseAddrLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; //!< INTERMEDIATE_BUFFER_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 IntermediateBufferBaseAddrArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< INTERMEDIATE_BUFFER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved201                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 IntermediateBufferPartition1Offset                                               ; //!< Intermediate Buffer Partition-1 Offset
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 IntermediateBufferPartition2Offset                                               ; //!< Intermediate Buffer Partition-2 Offset
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 IntermediateBufferPartition3Offset                                               ; //!< Intermediate Buffer Partition-3 Offset
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 IntermediateBufferPartition4Offset                                               ; //!< Intermediate Buffer Partition-4 Offset
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 IntermediateBufferPartition5Offset                                               ; //!< Intermediate Buffer Partition-5 Offset
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 IntermediateBufferPartition6Offset                                               ; //!< Intermediate Buffer Partition-6 Offset
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 IntermediateBufferPartition7Offset                                               ; //!< Intermediate Buffer Partition-7 Offset
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 IntermediateBufferPartition8Offset                                               ; //!< Intermediate Buffer Partition-8 Offset
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 IntermediateBufferMaxSize                                                        ; //!< Intermediate Buffer Max Size
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved512                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 FinalFrameBaseAddr                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Final Frame Base Addr 
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 FinalFrameBaseAddrUpperRange                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Final Frame Base Addr - Upper Range
                uint32_t                 Reserved560                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 FinalFrameBaseAddrAgeForQuadlruAge               : __CODEGEN_BITFIELD( 0,  1)    ; //!< FINAL_FRAME_BASE_ADDR__AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved578                                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 FinalFrameBaseAddrTargetCacheTc                  : __CODEGEN_BITFIELD( 3,  4)    ; //!< FINAL_FRAME_BASE_ADDR__TARGET_CACHE_TC
                uint32_t                 FinalFrameBaseAddrLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; //!< FINAL_FRAME_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 FinalFrameBaseAddrArbitrationPriorityControl     : __CODEGEN_BITFIELD( 7,  8)    ; //!< FINAL_FRAME_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved585                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Finalframebyteoffset                             : __CODEGEN_BITFIELD( 0,  5)    ; //!< FinalFrameByteOffset
                uint32_t                 Reserved614                                      : __CODEGEN_BITFIELD( 6, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 StreamoutBaseAddrCacheabilityControl             : __CODEGEN_BITFIELD( 0,  1)    ; //!< STREAMOUT_BASE_ADDR__CACHEABILITY_CONTROL
                uint32_t                 StreamoutBaseAddrGraphicsDataTypeGfdt            : __CODEGEN_BITFIELD( 2,  2)    ; //!< Streamout Base Addr - Graphics Data Type (GFDT)
                uint32_t                 Reserved643                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 StreamoutBaseAddrArbitrationPriorityControl      : __CODEGEN_BITFIELD( 4,  5)    ; //!< STREAMOUT_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL_
                uint32_t                 StreamoutBaseAddr                                : __CODEGEN_BITFIELD( 6, 31)    ; //!< Streamout Base Addr 
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 StreamoutBaseAddrUpperRange                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Streamout Base Addr - Upper Range
                uint32_t                 Reserved688                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 StreamoutBaseAddrAgeForQuadlruAge                : __CODEGEN_BITFIELD( 0,  1)    ; //!< STREAMOUT_BASE_ADDR__AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved706                                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 StreamoutBaseAddrTargetCacheTc                   : __CODEGEN_BITFIELD( 3,  4)    ; //!< STREAMOUT_BASE_ADDR__TARGET_CACHE_TC
                uint32_t                 StreamoutBaseAddrLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; //!< STREAMOUT_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 StreamoutBaseAddrArbitrationPriorityControl      : __CODEGEN_BITFIELD( 7,  8)    ; //!< STREAMOUT_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved713                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 CoeffProbsStreaminSurfaceCacheabilityControl     : __CODEGEN_BITFIELD( 0,  1)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__CACHEABILITY_CONTROL
                uint32_t                 CoeffProbsStreaminSurfaceGraphicsDataTypeGfdt    : __CODEGEN_BITFIELD( 2,  2)    ; //!< Coeff Probs StreamIn Surface - Graphics Data Type (GFDT)
                uint32_t                 Reserved739                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 CoeffProbsStreaminSurfaceArbitrationPriorityControl : __CODEGEN_BITFIELD( 4,  5)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__ARBITRATION_PRIORITY_CONTROL_
                uint32_t                 CoeffProbsStreaminSurface                        : __CODEGEN_BITFIELD( 6, 31)    ; //!< Coeff Probs StreamIn Surface
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 CoeffProbsStreaminSurfaceUpperRange              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Coeff Probs StreamIn Surface - Upper Range
                uint32_t                 Reserved784                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 CoeffProbsStreaminSurfaceAgeForQuadlruAge        : __CODEGEN_BITFIELD( 0,  1)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved802                                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 CoeffProbsStreaminSurfaceTargetCacheTc           : __CODEGEN_BITFIELD( 3,  4)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__TARGET_CACHE_TC
                uint32_t                 CoeffProbsStreaminSurfaceLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 CoeffProbsStreaminSurfaceArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< COEFF_PROBS_STREAMIN_SURFACE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved809                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 TokenStatisticsSurfaceCacheabilityControl        : __CODEGEN_BITFIELD( 0,  1)    ; //!< TOKEN_STATISTICS_SURFACE__CACHEABILITY_CONTROL
                uint32_t                 TokenStatisticsSurfaceGraphicsDataTypeGfdt       : __CODEGEN_BITFIELD( 2,  2)    ; //!< Token Statistics Surface - Graphics Data Type (GFDT)
                uint32_t                 Reserved835                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 FrameHeaderBaseAddrArbitrationPriorityControl    : __CODEGEN_BITFIELD( 4,  5)    ; //!< FRAME_HEADER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 TokenStatisticsSurface                           : __CODEGEN_BITFIELD( 6, 31)    ; //!< Token Statistics Surface
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 TokenStatisticsSurface                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Token Statistics Surface
                uint32_t                 Reserved880                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 TokenStatisticsSurfaceAgeForQuadlruAge           : __CODEGEN_BITFIELD( 0,  1)    ; //!< TOKEN_STATISTICS_SURFACE__AGE_FOR_QUADLRU_AGE
                uint32_t                 TokenStatisticsSurface                           : __CODEGEN_BITFIELD( 2,  2)    ; //!< Token Statistics Surface
                uint32_t                 TokenStatisticsSurfaceTargetCacheTc              : __CODEGEN_BITFIELD( 3,  4)    ; //!< TOKEN_STATISTICS_SURFACE__TARGET_CACHE_TC
                uint32_t                 MemoryTypeLlcEllcCacheabilityControlLellcccForCoeffprobsStreaminSurface : __CODEGEN_BITFIELD( 5,  6)    ; //!< MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE
                uint32_t                 TokenStatisticsSurfaceArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< TOKEN_STATISTICS_SURFACE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved905                                      : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29..30
            struct
            {
                uint64_t                 MpcRowstoreSurfaceCacheabilityControl            : __CODEGEN_BITFIELD(0, 1); //!< MPC_ROWSTORE_SURFACE__CACHEABILITY_CONTROL
                uint64_t                 MpcRowstoreSurfaceGraphicsDataTypeGfdt           : __CODEGEN_BITFIELD(2, 2); //!< MPC RowStore Surface Graphics Data Type (GFDT)
                uint64_t                 Reserved931                                      : __CODEGEN_BITFIELD(3, 3); //!< Reserved
                uint64_t                 MpcRowstoreBaseAddrArbitrationPriorityControl    : __CODEGEN_BITFIELD(4, 5); //!< MPC_ROWSTORE_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
                uint64_t                 MpcRowstoreSurfaceAddressLow                     : __CODEGEN_BITFIELD(6, 63); //!< MPC RowStore Surface Address Low
            };
            uint32_t                     Value[2];
        } DW29_30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 MpcRowstoreSurfaceAgeForQuadlruAge               : __CODEGEN_BITFIELD( 0,  1)    ; //!< MPC_ROWSTORE_SURFACE__AGE_FOR_QUADLRU_AGE
                uint32_t                 MpcRowstoreSurface                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< MPC RowStore Surface
                uint32_t                 MpcRowstoreTargetCache                           : __CODEGEN_BITFIELD( 3,  4)    ; //!< MPC_ROWSTORE__TARGET_CACHE
                uint32_t                 MpcRowstoreMemoryTypeLlcEllcCacheabilityControl  : __CODEGEN_BITFIELD( 5,  6)    ; //!< MPC_ROWSTORE__MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL
                uint32_t                 MpcRowstoreArbitrationPriorityControl            : __CODEGEN_BITFIELD( 7,  8)    ; //!< MPC_ROWSTORE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved1001                                     : __CODEGEN_BITFIELD( 9, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW31;

        //! \name Local enumerations

        enum SUB_OPCODE_B
        {
            SUB_OPCODE_B_MFXVP8BSPBUFBASEADDRSTATE                           = 3, //!< No additional details
        };

        enum SUB_OPCODE_A
        {
            SUB_OPCODE_A_VP8COMMON                                           = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VP8                                         = 4, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_VIDEOCODEC                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief FRAME_HEADER_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. .
        //!                         If a particular allocation is done at youngest age ("3") it tends
        //!     to stay longer in the cache as compared to older age allocations ("2",
        //!     "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum FRAME_HEADER_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        {
            FRAME_HEADER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 0, //!< No additional details
            FRAME_HEADER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 1, //!< No additional details
            FRAME_HEADER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 2, //!< No additional details
            FRAME_HEADER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief FRAME_HEADER_BASE_ADDR__TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs eLLC for caching
        enum FRAME_HEADER_BASE_ADDR__TARGET_CACHE_TC
        {
            FRAME_HEADER_BASE_ADDR_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGT   = 0, //!< No additional details
            FRAME_HEADER_BASE_ADDR_TARGET_CACHE_TC_LLCONLY                   = 1, //!< No additional details
            FRAME_HEADER_BASE_ADDR_TARGET_CACHE_TC_LLCELLCALLOWED            = 2, //!< No additional details
            FRAME_HEADER_BASE_ADDR_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED        = 3, //!< No additional details
        };

        //! \brief FOR_FRAMEHEADERBASEADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum FOR_FRAMEHEADERBASEADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            FOR_FRAMEHEADERBASEADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_CACHEABLE = 0, //!< Use Cacheability Controls from page table / UC with Fence (if coherent cycle)
            FOR_FRAMEHEADERBASEADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UC  = 1, //!< Uncacheable - non-cacheable
            FOR_FRAMEHEADERBASEADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WT  = 2, //!< Writethrough
            FOR_FRAMEHEADERBASEADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WB  = 3, //!< Writeback
        };

        //! \brief FRAME_HEADER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum FRAME_HEADER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        {
            FRAME_HEADER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            FRAME_HEADER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            FRAME_HEADER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            FRAME_HEADER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. . 
        //!                         If a particular allocation is done at youngest age ("3") it tends
        //!     to stay longer in the cache as compared to older age allocations ("2",
        //!     "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE
        {
            INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 0, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 1, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 2, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs. eLLC for caching
        enum INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC
        {
            INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGT = 0, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC_LLCONLY            = 1, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC_LLCELLCALLOWED     = 2, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED = 3, //!< No additional details
        };

        //! \brief INTERMEDIATE_BUFFER_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum INTERMEDIATE_BUFFER_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            INTERMEDIATE_BUFFER_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_CACHEABLE = 0, //!< Use Cacheability Controls from page table / UC with Fence (if coherent cycle)
            INTERMEDIATE_BUFFER_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UC = 1, //!< Uncacheable - non-cacheable
            INTERMEDIATE_BUFFER_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WT = 2, //!< Writethrough
            INTERMEDIATE_BUFFER_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WB = 3, //!< Writeback
        };

        //! \brief INTERMEDIATE_BUFFER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum INTERMEDIATE_BUFFER_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        {
            INTERMEDIATE_BUFFER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            INTERMEDIATE_BUFFER_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief FINAL_FRAME_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. . 
        //!                         If a particular allocation is done at youngest age ("3") it tends
        //!     to stay longer in the cache as compared to older age allocations ("2",
        //!     "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum FINAL_FRAME_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        {
            FINAL_FRAME_BASE_ADDR_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 0, //!< No additional details
            FINAL_FRAME_BASE_ADDR_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 1, //!< No additional details
            FINAL_FRAME_BASE_ADDR_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 2, //!< No additional details
            FINAL_FRAME_BASE_ADDR_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief FINAL_FRAME_BASE_ADDR__TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs eLLC for caching
        enum FINAL_FRAME_BASE_ADDR__TARGET_CACHE_TC
        {
            FINAL_FRAME_BASE_ADDR_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGT    = 0, //!< No additional details
            FINAL_FRAME_BASE_ADDR_TARGET_CACHE_TC_LLCONLY                    = 1, //!< No additional details
            FINAL_FRAME_BASE_ADDR_TARGET_CACHE_TC_LLCELLCALLOWED             = 2, //!< No additional details
            FINAL_FRAME_BASE_ADDR_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED         = 3, //!< No additional details
        };

        //! \brief FINAL_FRAME_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum FINAL_FRAME_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            FINAL_FRAME_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_CACHEABLE = 0, //!< Use Cacheability Controls from page table / UC with Fence (if coherent cycle)
            FINAL_FRAME_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UC    = 1, //!< Uncacheable - non-cacheable
            FINAL_FRAME_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WT    = 2, //!< Writethrough
            FINAL_FRAME_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WB    = 3, //!< Writeback
        };

        enum FINAL_FRAME_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        {
            FINAL_FRAME_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            FINAL_FRAME_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            FINAL_FRAME_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            FINAL_FRAME_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief STREAMOUT_BASE_ADDR__CACHEABILITY_CONTROL
        //! \details
        //!     This field controls cacheability in the mid-level cache (MLC) and
        //!     last-level cache (LLC)
        enum STREAMOUT_BASE_ADDR__CACHEABILITY_CONTROL
        {
            STREAMOUT_BASE_ADDR_CACHEABILITY_CONTROL_GTTENTRY                = 0, //!< Use cacheability control bits from GTT entry
            STREAMOUT_BASE_ADDR_CACHEABILITY_CONTROL_NOTLLCORMLC             = 1, //!< Data is not cached in LLC or MLC
            STREAMOUT_BASE_ADDR_CACHEABILITY_CONTROL_LLCBUTNOTMLC            = 2, //!< Data is cached in LLC but not MLC
            STREAMOUT_BASE_ADDR_CACHEABILITY_CONTROL_BOTHLLCANDMLC           = 3, //!< Data is cached in both LLC and MLC
        };

        //! \brief STREAMOUT_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL_
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum STREAMOUT_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL_
        {
            STREAMOUT_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            STREAMOUT_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            STREAMOUT_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            STREAMOUT_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY  = 3, //!< No additional details
        };

        //! \brief STREAMOUT_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. . 
        //!                         If a particular allocation is done at youngest age ("3") it tends
        //!     to stay longer in the cache as compared to older age allocations ("2",
        //!     "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum STREAMOUT_BASE_ADDR__AGE_FOR_QUADLRU_AGE
        {
            STREAMOUT_BASE_ADDR_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 0, //!< No additional details
            STREAMOUT_BASE_ADDR_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 1, //!< No additional details
            STREAMOUT_BASE_ADDR_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 2, //!< No additional details
            STREAMOUT_BASE_ADDR_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief STREAMOUT_BASE_ADDR__TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs eLLC for caching
        enum STREAMOUT_BASE_ADDR__TARGET_CACHE_TC
        {
            STREAMOUT_BASE_ADDR_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGT      = 0, //!< No additional details
            STREAMOUT_BASE_ADDR_TARGET_CACHE_TC_LLCONLY                      = 1, //!< No additional details
            STREAMOUT_BASE_ADDR_TARGET_CACHE_TC_LLCELLCALLOWED               = 2, //!< No additional details
            STREAMOUT_BASE_ADDR_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED           = 3, //!< No additional details
        };

        //! \brief STREAMOUT_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum STREAMOUT_BASE_ADDR__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            STREAMOUT_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_CACHEABLE = 0, //!< Use Cacheability Controls from page table / UC with Fence (if coherent cycle)
            STREAMOUT_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UC      = 1, //!< Uncacheable - non-cacheable
            STREAMOUT_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WT      = 2, //!< Writethrough
            STREAMOUT_BASE_ADDR_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WB      = 3, //!< Writeback
        };

        //! \brief COEFF_PROBS_STREAMIN_SURFACE__CACHEABILITY_CONTROL
        //! \details
        //!     This field controls cacheability in the mid-level cache (MLC) and
        //!     last-level cache (LLC)
        enum COEFF_PROBS_STREAMIN_SURFACE__CACHEABILITY_CONTROL
        {
            COEFF_PROBS_STREAMIN_SURFACE_CACHEABILITY_CONTROL_GTTENTRY       = 0, //!< Use cacheability control bits from GTT entry
            COEFF_PROBS_STREAMIN_SURFACE_CACHEABILITY_CONTROL_NOTLLCORMLC    = 1, //!< Data is not cached in LLC or MLC
            COEFF_PROBS_STREAMIN_SURFACE_CACHEABILITY_CONTROL_LLCBUTNOTMLC   = 2, //!< Data is cached in LLC but not MLC
            COEFF_PROBS_STREAMIN_SURFACE_CACHEABILITY_CONTROL_BOTHLLCANDMLC  = 3, //!< Data is cached in both LLC and MLC
        };

        //! \brief COEFF_PROBS_STREAMIN_SURFACE__ARBITRATION_PRIORITY_CONTROL_
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum COEFF_PROBS_STREAMIN_SURFACE__ARBITRATION_PRIORITY_CONTROL_
        {
            COEFF_PROBS_STREAMIN_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief COEFF_PROBS_STREAMIN_SURFACE__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. . 
        //!                         If a particular allocation is done at youngest age ("3") it tends
        //!     to stay longer in the cache as compared to older age allocations ("2",
        //!     "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum COEFF_PROBS_STREAMIN_SURFACE__AGE_FOR_QUADLRU_AGE
        {
            COEFF_PROBS_STREAMIN_SURFACE_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 0, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 1, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 2, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief COEFF_PROBS_STREAMIN_SURFACE__TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs eLLC for caching
        enum COEFF_PROBS_STREAMIN_SURFACE__TARGET_CACHE_TC
        {
            COEFF_PROBS_STREAMIN_SURFACE_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGT = 0, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_TARGET_CACHE_TC_LLCONLY             = 1, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_TARGET_CACHE_TC_LLCELLCALLOWED      = 2, //!< No additional details
            COEFF_PROBS_STREAMIN_SURFACE_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED  = 3, //!< No additional details
        };

        //! \brief COEFF_PROBS_STREAMIN_SURFACE__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum COEFF_PROBS_STREAMIN_SURFACE__LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            COEFF_PROBS_STREAMIN_SURFACE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_CACHEABLE = 0, //!< Use Cacheability Controls from page table / UC with Fence (if coherent cycle)
            COEFF_PROBS_STREAMIN_SURFACE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UC = 1, //!< Uncacheable - non-cacheable
            COEFF_PROBS_STREAMIN_SURFACE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WT = 2, //!< Writethrough
            COEFF_PROBS_STREAMIN_SURFACE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WB = 3, //!< Writeback
        };

        //! \brief TOKEN_STATISTICS_SURFACE__CACHEABILITY_CONTROL
        //! \details
        //!     This field controls cacheability in the mid-level cache (MLC) and
        //!     last-level cache (LLC).
        enum TOKEN_STATISTICS_SURFACE__CACHEABILITY_CONTROL
        {
            TOKEN_STATISTICS_SURFACE_CACHEABILITY_CONTROL_GTTENTRY           = 0, //!< Use cacheability control bits from GTT entry
            TOKEN_STATISTICS_SURFACE_CACHEABILITY_CONTROL_NOTLLCORMLC        = 1, //!< Data is not cached in LLC or MLC
            TOKEN_STATISTICS_SURFACE_CACHEABILITY_CONTROL_LLCBUTNOTMLC       = 2, //!< Data is cached in LLC but not MLC
            TOKEN_STATISTICS_SURFACE_CACHEABILITY_CONTROL_BOTHLLCANDMLC      = 3, //!< Data is cached in both LLC and MLC
        };

        //! \brief TOKEN_STATISTICS_SURFACE__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     This field allows the selection of AGE parameter for a given surface in
        //!     LLC or eLLC. . If a particular allocation is done at youngest age ("3")
        //!     it tends to stay longer in the cache as compared to older age
        //!     allocations ("2", "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.
        enum TOKEN_STATISTICS_SURFACE__AGE_FOR_QUADLRU_AGE
        {
            TOKEN_STATISTICS_SURFACE_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 0, //!< No additional details
            TOKEN_STATISTICS_SURFACE_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 1, //!< No additional details
            TOKEN_STATISTICS_SURFACE_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 2, //!< No additional details
            TOKEN_STATISTICS_SURFACE_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief TOKEN_STATISTICS_SURFACE__TARGET_CACHE_TC
        //! \details
        //!     This field allows the choice of LLC vs eLLC for caching.
        enum TOKEN_STATISTICS_SURFACE__TARGET_CACHE_TC
        {
            TOKEN_STATISTICS_SURFACE_TARGET_CACHE_TC_ELLCONLY_NOTSNOOPEDINGTBDW = 0, //!< No additional details
            TOKEN_STATISTICS_SURFACE_TARGET_CACHE_TC_LLCONLY                 = 1, //!< No additional details
            TOKEN_STATISTICS_SURFACE_TARGET_CACHE_TC_LLCELLCALLOWED          = 2, //!< No additional details
            TOKEN_STATISTICS_SURFACE_TARGET_CACHE_TC_L3_LLC_ELLCALLOWED      = 3, //!< No additional details
        };

        //! \brief MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore.
        //!                         For the cases where the LeLLCCC is set, cacheable transaction are
        //!     generated to enable LLC usage for particular stream.
        enum MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE
        {
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE_USECACHEABILITYCONTROLSFROMPAGETABLEUCWITHFENCEIFCOHERENTCYCLE = 0, //!< No additional details
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE_UC = 1, //!< Uncacheable - non-cacheable
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE_WT = 2, //!< Writethrough
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_FOR_COEFFPROBS_STREAMIN_SURFACE_WB = 3, //!< Writeback
        };

        //! \brief TOKEN_STATISTICS_SURFACE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum TOKEN_STATISTICS_SURFACE__ARBITRATION_PRIORITY_CONTROL
        {
            TOKEN_STATISTICS_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            TOKEN_STATISTICS_SURFACE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            TOKEN_STATISTICS_SURFACE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            TOKEN_STATISTICS_SURFACE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MPC_ROWSTORE_SURFACE__CACHEABILITY_CONTROL
        //! \details
        //!     <b>This field controls cacheability in the mid-level cache (MLC) and
        //!     last-level cache (LLC).</b>
        enum MPC_ROWSTORE_SURFACE__CACHEABILITY_CONTROL
        {
            MPC_ROWSTORE_SURFACE_CACHEABILITY_CONTROL_GTTENTRY               = 0, //!< Use cacheability control bits from GTT entry
            MPC_ROWSTORE_SURFACE_CACHEABILITY_CONTROL_NOTLLCORMLC            = 1, //!< Data is not cached in LLC or MLC
            MPC_ROWSTORE_SURFACE_CACHEABILITY_CONTROL_LLCBUTNOTMLC           = 2, //!< Data is cached in LLC but not MLC
            MPC_ROWSTORE_SURFACE_CACHEABILITY_CONTROL_BOTHLLCANDMLC          = 3, //!< Data is cached in both LLC and MLC
        };

        //! \brief MPC_ROWSTORE_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     <b>This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.</b>
        enum MPC_ROWSTORE_BASE_ADDR__ARBITRATION_PRIORITY_CONTROL
        {
            MPC_ROWSTORE_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MPC_ROWSTORE_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MPC_ROWSTORE_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MPC_ROWSTORE_BASE_ADDR_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MPC_ROWSTORE_SURFACE__AGE_FOR_QUADLRU_AGE
        //! \details
        //!     <b>This field allows the selection of AGE parameter for a given surface
        //!     in LLC or eLLC. . If a particular allocation is done at youngest age
        //!     ("3") it tends to stay longer in the cache as compared to older age
        //!     allocations ("2", "1", or "0").
        //!                         This option is given to driver to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.</b>
        enum MPC_ROWSTORE_SURFACE__AGE_FOR_QUADLRU_AGE
        {
            MPC_ROWSTORE_SURFACE_AGE_FOR_QUADLRU_AGE_POORCHANCEOFGENERATINGHITS = 0, //!< No additional details
            MPC_ROWSTORE_SURFACE_AGE_FOR_QUADLRU_AGE_DECENTCHANCEOFGENERATINGHITS = 1, //!< No additional details
            MPC_ROWSTORE_SURFACE_AGE_FOR_QUADLRU_AGE_NEXTGOODCHANCEOFGENERATINGHITS = 2, //!< No additional details
            MPC_ROWSTORE_SURFACE_AGE_FOR_QUADLRU_AGE_GOODCHANCEOFGENERATINGHITS = 3, //!< No additional details
        };

        //! \brief MPC_ROWSTORE__TARGET_CACHE
        //! \details
        //!     <b>This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.</b>
        enum MPC_ROWSTORE__TARGET_CACHE
        {
            MPC_ROWSTORE_TARGET_CACHE_ELLCONLY                               = 0, //!< No additional details
            MPC_ROWSTORE_TARGET_CACHE_LLCONLY                                = 1, //!< No additional details
            MPC_ROWSTORE_TARGET_CACHE_LLCELLCALLOWED                         = 2, //!< No additional details
            MPC_ROWSTORE_TARGET_CACHE_L3_LLC_ELLCALLOWED                     = 3, //!< No additional details
        };

        //! \brief MPC_ROWSTORE__MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL
        //! \details
        //!     <b>This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.</b>
        enum MPC_ROWSTORE__MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL
        {
            MPC_ROWSTORE_MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_USECACHEABILITYCONTROLSFROMPAGETABLEUCWITHFENCEIFCOHERENTCYCLE = 0, //!< No additional details
            MPC_ROWSTORE_MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_UC         = 1, //!< Uncacheable - non-cacheable
            MPC_ROWSTORE_MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_WT         = 2, //!< Writethrough
            MPC_ROWSTORE_MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_WB         = 3, //!< Writeback
        };

        //! \brief MPC_ROWSTORE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     <b>This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.</b>
        enum MPC_ROWSTORE__ARBITRATION_PRIORITY_CONTROL
        {
            MPC_ROWSTORE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY        = 0, //!< No additional details
            MPC_ROWSTORE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY  = 1, //!< No additional details
            MPC_ROWSTORE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY   = 2, //!< No additional details
            MPC_ROWSTORE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY         = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_VP8_BSP_BUF_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 32;
        static const size_t byteSize = 128;
    };

    //!
    //! \brief MFD_VP8_BSD_OBJECT
    //! \details
    //!     The MFD_VP8_BSD_OBJECT command is the only primitive command for the VP8
    //!     Decoding Pipeline. The Partitions of the bitstream is loaded as indirect
    //!     data object.  Before issuing a MFD_VP8_BSD_OBJECT command, all VP8 frame
    //!     level states of the MFD Engine need to be valid. Therefore the commands
    //!     used to set these states need to have been issued prior to the issue of
    //!     a MFD_VP8_BSD_OBJECT command. Context switch interrupt is not supported
    //!     by this command.
    //!     
    struct MFD_VP8_BSD_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Partition0FirstmbbitoffsetFromFrameHeader        : __CODEGEN_BITFIELD( 0,  2)    ; //!< Partition0 FirstMBBitOffset from Frame Header
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 CodedNumOfCoeffTokenPartitions                   : __CODEGEN_BITFIELD( 4,  5)    ; //!< Coded Num of Coeff Token Partitions
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Partition0CpbacEntropyRange                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Partition0 CPBAC Entropy Range
                uint32_t                 Partition0CpbacEntropyCount                      : __CODEGEN_BITFIELD(16, 20)    ; //!< Partition0 CPBAC Entropy Count
                uint32_t                 Reserved53                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0, 23)    ; //!< Reserved
                uint32_t                 Partition0CpbacEntropyValue                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Partition0 CPBAC Entropy Value
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 IndirectPartition0DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition0 Data Length
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 IndirectPartition0DataStartOffset                                                ; //!< Indirect Partition0 Data Start Offset
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IndirectPartition1DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition1 Data Length
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 IndirectPartition1DataStartOffset                                                ; //!< Indirect Partition1 Data Start Offset
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 IndirectPartition2DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition2 Data Length
                uint32_t                 Reserved248                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 IndirectPartition2DataStartOffset                                                ; //!< Indirect Partition2 Data Start Offset
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 IndirectPartition3DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition3 Data Length
                uint32_t                 Reserved312                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 IndirectPartition3DataStartOffset                                                ; //!< Indirect Partition3 Data Start Offset
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 IndirectPartition4DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition4 Data Length
                uint32_t                 Reserved376                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 IndirectPartition4DataStartOffset                                                ; //!< Indirect Partition4 Data Start Offset
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 IndirectPartition5DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition5 Data Length
                uint32_t                 Reserved440                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 IndirectPartition5DataStartOffset                                                ; //!< Indirect Partition5 Data Start Offset
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 IndirectPartition6DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition6 Data Length
                uint32_t                 Reserved504                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 IndirectPartition6DataStartOffset                                                ; //!< Indirect Partition6 Data Start Offset
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 IndirectPartition7DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition7 Data Length
                uint32_t                 Reserved568                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 IndirectPartition7DataStartOffset                                                ; //!< Indirect Partition7 Data Start Offset
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 IndirectPartition8DataLength                     : __CODEGEN_BITFIELD( 0, 23)    ; //!< Indirect Partition8 Data Length
                uint32_t                 Reserved632                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 IndirectPartition8DataStartOffset                                                ; //!< Indirect Partition8 Data Start Offset
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 Reserved672                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 MbHeaderErrorHandling                            : __CODEGEN_BITFIELD( 8,  8)    ; //!< MB_HEADER_ERROR_HANDLING
                uint32_t                 Reserved681                                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 EntropyErrorHandling                             : __CODEGEN_BITFIELD(10, 10)    ; //!< ENTROPY_ERROR_HANDLING
                uint32_t                 Reserved683                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 MprErrorMvOutOfRangeHandling                     : __CODEGEN_BITFIELD(12, 12)    ; //!< MPR_ERROR_MV_OUT_OF_RANGE_HANDLING
                uint32_t                 Reserved685                                      : __CODEGEN_BITFIELD(13, 13)    ; //!< Reserved
                uint32_t                 BsdprematurecompleteErrorHandling                : __CODEGEN_BITFIELD(14, 14)    ; //!< BSDPREMATURECOMPLETE_ERROR_HANDLING
                uint32_t                 Reserved687                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 ConcealPicIdConcealmentPictureId                 : __CODEGEN_BITFIELD(16, 17)    ; //!< Conceal_Pic_Id (Concealment Picture ID)
                uint32_t                 Reserved690                                      : __CODEGEN_BITFIELD(18, 30)    ; //!< Reserved
                uint32_t                 ConcealmentMethod                                : __CODEGEN_BITFIELD(31, 31)    ; //!< CONCEALMENT_METHOD
            };
            uint32_t                     Value;
        } DW21;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_UNNAMED8                                              = 8, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_UNNAMED1                                              = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VP8DEC                                      = 4, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFDVP8BSDOBJECT                                         = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum MB_HEADER_ERROR_HANDLING
        {
            MB_HEADER_ERROR_HANDLING_IGNORETHEERRORANDCONTINUEMASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING = 0, //!< No additional details
            MB_HEADER_ERROR_HANDLING_SETTHEINTERRUPTTOTHEDRIVERPROVIDEMMIOREGISTERSFORMBADDRESSRW = 1, //!< No additional details
        };

        enum ENTROPY_ERROR_HANDLING
        {
            ENTROPY_ERROR_HANDLING_IGNORETHEERRORANDCONTINUEMASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING = 0, //!< No additional details
            ENTROPY_ERROR_HANDLING_SETTHEINTERRUPTTOTHEDRIVERPROVIDEMMIOREGISTERSFORMBADDRESSRW = 1, //!< No additional details
        };

        enum MPR_ERROR_MV_OUT_OF_RANGE_HANDLING
        {
            MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_IGNORETHEERRORANDCONTINUEMASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING = 0, //!< No additional details
            MPR_ERROR_MV_OUT_OF_RANGE_HANDLING_SETTHEINTERRUPTTOTHEDRIVERPROVIDEMMIOREGISTERSFORMBADDRESSRW = 1, //!< No additional details
        };

        //! \brief BSDPREMATURECOMPLETE_ERROR_HANDLING
        //! \details
        //!     It occurs in situation where the decode is completed but there are still
        //!     data in the bitstream.
        enum BSDPREMATURECOMPLETE_ERROR_HANDLING
        {
            BSDPREMATURECOMPLETE_ERROR_HANDLING_IGNORETHEERRORANDCONTINUEMASKEDTHEINTERRUPT_ASSUMETHEHARDWAREAUTOMATICALLYPERFORMTHEERRORHANDLING = 0, //!< No additional details
            BSDPREMATURECOMPLETE_ERROR_HANDLING_SETTHEINTERRUPTTOTHEDRIVERPROVIDEMMIOREGISTERSFORMBADDRESSRW = 1, //!< No additional details
        };

        //! \brief CONCEALMENT_METHOD
        //! \details
        //!     This field specifies the method used for concealment when error is
        //!     detected.
        enum CONCEALMENT_METHOD
        {
            CONCEALMENT_METHOD_INTRA16X16PREDICTION                          = 0, //!< A copy from the current picture is performed using Intra 16x16 Prediction method.
            CONCEALMENT_METHOD_INTERPCOPY                                    = 1, //!< A copy from collocated macroblock location is performed from the concealment reference indicated by the ConCeal_Pic_Id field.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFD_VP8_BSD_OBJECT_CMD();

        static const size_t dwSize = 22;
        static const size_t byteSize = 88;
    };

    //!
    //! \brief MFX_VP8_PIC_STATE
    //! \details
    //!     This must be the very first command to issue after the surface state,
    //!     the pipe select and base address setting commands and must be issued
    //!     before MFX_VP8_IMG_STATE.
    //!     
    struct MFX_VP8_PIC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubOpcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUB_OPCODE_B
                uint32_t                 SubOpcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUB_OPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 FrameWidthMinus1                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< Frame Width Minus 1
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 FrameHeightMinus1                                : __CODEGEN_BITFIELD(16, 23)    ; //!< Frame Height Minus 1
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 McFilterSelect                                   : __CODEGEN_BITFIELD( 0,  0)    ; //!< MC_FILTER_SELECT, Decoder / Encoder
                uint32_t                 ChromaFullPixelMcFilterMode                      : __CODEGEN_BITFIELD( 1,  1)    ; //!< CHROMA_FULL_PIXEL_MC_FILTER_MODE, Decoder / Encoder
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved, Decoder / Encoder
                uint32_t                 Dblkfiltertype                                   : __CODEGEN_BITFIELD( 4,  4)    ; //!< DBLKFILTERTYPE, Decoder / Encoder
                uint32_t                 Skeyframeflag                                    : __CODEGEN_BITFIELD( 5,  5)    ; //!< SKEYFRAMEFLAG, Decoder / Encoder
                uint32_t                 SegmentationIdStreamoutEnable                    : __CODEGEN_BITFIELD( 6,  6)    ; //!< SEGMENTATION_ID_STREAMOUT_ENABLE, Decoder Only
                uint32_t                 SegmentationIdStreaminEnable                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< SEGMENTATION_ID_STREAMIN_ENABLE, Decoder Only
                uint32_t                 SegmentEnableFlag                                : __CODEGEN_BITFIELD( 8,  8)    ; //!< SEGMENT_ENABLE_FLAG, Decoder / Encoder
                uint32_t                 UpdateMbsegmentMapFlag                           : __CODEGEN_BITFIELD( 9,  9)    ; //!< UPDATE_MBSEGMENT_MAP_FLAG, Decoder / Encoder
                uint32_t                 MbNocoeffSkipflag                                : __CODEGEN_BITFIELD(10, 10)    ; //!< MB_NOCOEFF_SKIPFLAG, Decoder / Encoder
                uint32_t                 ModeReferenceLoopFilterDeltaEnabled              : __CODEGEN_BITFIELD(11, 11)    ; //!< MODE_REFERENCE_LOOP_FILTER_DELTA_ENABLED, Decoder / Encoder
                uint32_t                 GoldenRefPictureMvSignbiasFlag                   : __CODEGEN_BITFIELD(12, 12)    ; //!< Golden Ref Picture MV SignBias Flag, Decoder / Encoder
                uint32_t                 AlternateRefPicMvSignbiasFlag                    : __CODEGEN_BITFIELD(13, 13)    ; //!< Alternate Ref Pic MV SignBias Flag, Decoder / Encoder
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved, Decoder / Encoder
                uint32_t                 DeblockSharpnessLevel                            : __CODEGEN_BITFIELD(16, 18)    ; //!< Deblock Sharpness Level, Decoder / Encoder
                uint32_t                 Reserved83                                       : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved, Decoder / Encoder
                uint32_t                 Log2NumOfPartition                               : __CODEGEN_BITFIELD(24, 25)    ; //!< LOG2_NUM_OF_PARTITION, Decoder / Encoder
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 DblkfilterlevelForSegment0                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< DBLKFILTERLEVEL_FOR_SEGMENT0
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 DblkfilterlevelForSegment1                       : __CODEGEN_BITFIELD( 8, 13)    ; //!< DBLKFILTERLEVEL_FOR_SEGMENT1
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 DblkfilterlevelForSegment2                       : __CODEGEN_BITFIELD(16, 21)    ; //!< DBLKFILTERLEVEL_FOR_SEGMENT2
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 DblkfilterlevelForSegment3                       : __CODEGEN_BITFIELD(24, 29)    ; //!< DBLKFILTERLEVEL_FOR_SEGMENT3
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 QuantizerValue0Blocktype0Y1Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [0][BlockType0=Y1DC], Decoder Only
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue0Blocktype1Y1Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [0][BlockType1=Y1AC], Decoder Only
                uint32_t                 Reserved153                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            } dec;
            struct
            {
                uint32_t                 Seg0Qindex                                       : __CODEGEN_BITFIELD( 0,  6)    ; //!< Seg 0 Qindex, Encoder Only
                uint32_t                 Reserved135                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved, Encoder Only
                uint32_t                 Seg1Qindex                                       : __CODEGEN_BITFIELD( 8, 14)    ; //!< Seg 1 Qindex, Encoder Only
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved, Encoder Only
                uint32_t                 Seg2Qindex                                       : __CODEGEN_BITFIELD(16, 22)    ; //!< Seg 2 Qindex, Encoder Only
                uint32_t                 Reserved151                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved, Encoder Only
                uint32_t                 Seg3Qindex                                       : __CODEGEN_BITFIELD(24, 30)    ; //!< Seg 3 Qindex, Encoder Only
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            } enc;
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 QuantizerValue0Blocktype2Uvdc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [0][BlockType2=UVDC], Decoder Only
                uint32_t                 Reserved169                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue0Blocktype3Uvac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [0][BlockType3=UVAC], Decoder Only
                uint32_t                 Reserved185                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            } dec;
            struct
            {
                uint32_t                 Y2DcQindexDelta                                  : __CODEGEN_BITFIELD( 0,  3)    ; //!< Y2dc Qindex Delta, Encoder Only
                uint32_t                 Y2AcQindexDeltaSign                              : __CODEGEN_BITFIELD( 4,  4)    ; //!< Y2ac Qindex Delta Sign , Encoder Only
                uint32_t                 Reserved165                                      : __CODEGEN_BITFIELD( 5,  7)    ; //!< Reserved, Encoder Only
                uint32_t                 Y2AcQindexDelta                                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< Y2ac Qindex Delta , Encoder Only
                uint32_t                 Y2AcQindexSign                                   : __CODEGEN_BITFIELD(12, 12)    ; //!< Y2ac Qindex Sign, Encoder Only
                uint32_t                 Reserved173                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved, Encoder Only
                uint32_t                 UvdcQindexDelta                                  : __CODEGEN_BITFIELD(16, 19)    ; //!< UVdc Qindex Delta, Encoder Only
                uint32_t                 UvdcQindexDeltaSign                              : __CODEGEN_BITFIELD(20, 20)    ; //!< UVdc Qindex Delta Sign, Encoder Only
                uint32_t                 Reserved181                                      : __CODEGEN_BITFIELD(21, 23)    ; //!< Reserved, Encoder Only
                uint32_t                 UvacQindexdelta                                  : __CODEGEN_BITFIELD(24, 27)    ; //!< UVac QindexDelta, Encoder Only
                uint32_t                 UvacQindexDeltaSign                              : __CODEGEN_BITFIELD(28, 28)    ; //!< UVac Qindex Delta Sign, Encoder Only
                uint32_t                 Reserved189                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            } enc;
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 QuantizerValue0Blocktype4Y2Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [0][BlockType4=Y2DC], Decoder Only
                uint32_t                 Reserved201                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue0Blocktype5Y2Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [0][BlockType5=Y2AC], Decoder Only
                uint32_t                 Reserved217                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            } dec;
            struct
            {
                uint32_t                 Y1DcQindexDelta                                  : __CODEGEN_BITFIELD( 0,  3)    ; //!< Y1dc Qindex Delta, Encoder Only
                uint32_t                 Y1DcQindexDeltaSign                              : __CODEGEN_BITFIELD( 4,  4)    ; //!< Y1dc Qindex Delta Sign , Encoder Only
                uint32_t                 Reserved197                                      : __CODEGEN_BITFIELD( 5, 31)    ; //!< Reserved
            } enc;
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 QuantizerValue1Blocktype0Y1Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [1][BlockType0=Y1DC], Decoder Only
                uint32_t                 Reserved233                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue1Blocktype1Y1Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [1][BlockType1=Y1AC], Decoder Only
                uint32_t                 Reserved249                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            } dec;
            struct
            {
                uint32_t                 ClampQindexLow                                   : __CODEGEN_BITFIELD( 0,  6)    ; //!< Clamp Qindex Low, Encoder Only
                uint32_t                 Reserved231                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved, Encoder Only
                uint32_t                 ClampQindexHigh                                  : __CODEGEN_BITFIELD( 8, 14)    ; //!< Clamp Qindex high, Encoder Only
                uint32_t                 Reserved239                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            } enc;
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 QuantizerValue1Blocktype2Uvdc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [1][BlockType2=UVDC], Decoder Only
                uint32_t                 Reserved265                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue1Blocktype3Uvac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [1][BlockType3=UVAC], Decoder Only
                uint32_t                 Reserved281                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 QuantizerValue1Blocktype4Y2Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [1][BlockType4=Y2DC], Decoder Only
                uint32_t                 Reserved297                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue1Blocktype5Y2Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [1][BlockType5=Y2AC], Decoder Only
                uint32_t                 Reserved313                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 QuantizerValue2Blocktype0Y1Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [2][BlockType0=Y1DC], Decoder Only
                uint32_t                 Reserved329                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue2Blocktype1Y1Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [2][BlockType1=Y1AC], Decoder Only
                uint32_t                 Reserved345                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 QuantizerValue2Blocktype2Uvdc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [2][BlockType2=UVDC], Decoder Only
                uint32_t                 Reserved361                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue2Blocktype3Uvac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [2][BlockType3=UVAC], Decoder Only
                uint32_t                 Reserved377                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 QuantizerValue2Blocktype4Y2Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [2][BlockType4=Y2DC], Decoder Only
                uint32_t                 Reserved393                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue2Blocktype5Y2Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [2][BlockType5=Y2AC], Decoder Only
                uint32_t                 Reserved409                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 QuantizerValue3Blocktype0Y1Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [3][BlockType0=Y1DC], Decoder Only
                uint32_t                 Reserved425                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue3Blocktype1Y1Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [3][BlockType1=Y1AC], Decoder Only
                uint32_t                 Reserved441                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 QuantizerValue3Blocktype2Uvdc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [3][BlockType2=UVDC], Decoder Only
                uint32_t                 Reserved457                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue3Blocktype3Uvac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [3][BlockType3=UVAC], Decoder Only
                uint32_t                 Reserved473                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 QuantizerValue3Blocktype4Y2Dc                    : __CODEGEN_BITFIELD( 0,  8)    ; //!< Quantizer Value [3][BlockType4=Y2DC], Decoder Only
                uint32_t                 Reserved489                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved, Decoder Only
                uint32_t                 QuantizerValue3Blocktype5Y2Ac                    : __CODEGEN_BITFIELD(16, 24)    ; //!< Quantizer Value [3][BlockType5=Y2AC], Decoder Only
                uint32_t                 Reserved505                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved512                                      : __CODEGEN_BITFIELD( 0, 5)     ; //!< Reserved
                uint32_t                 CoeffprobabilityStreaminBaseAddress              : __CODEGEN_BITFIELD( 6, 31)    ; //!< CoeffProbability StreamIn Base Address, Decoder Only
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 CoeffprobabilityStreaminAddress                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< CoeffProbability StreamIn Address, Decoder Only
                uint32_t                 Reserved560                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 Reserved576                                      : __CODEGEN_BITFIELD( 0, 6)     ; //!< Reserved
                uint32_t                 CoeffprobabilityStreaminArbitrationPriorityControl : __CODEGEN_BITFIELD(7, 8); //!< COEFFPROBABILITY_STREAMIN__ARBITRATION_PRIORITY_CONTROL, Decoder Only
                uint32_t                 CoeffprobabilityStreaminMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< CoeffProbability StreamIn - Memory Compression Enable, Decoder Only
                uint32_t                 CoeffprobabilityStreaminMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< COEFFPROBABILITY_STREAMIN__MEMORY_COMPRESSION_MODE, Decoder Only
                uint32_t                 Reserved587                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved, Decoder Only
                uint32_t                 CoeffprobabilityStreaminTiledResourceMode        : __CODEGEN_BITFIELD(13, 14)    ; //!< COEFFPROBABILITY_STREAMIN__TILED_RESOURCE_MODE, Decoder Only
                uint32_t                 Reserved591                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Mbsegmentidtreeprobs0                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< MBSegmentIDTreeProbs[0]
                uint32_t                 Mbsegmentidtreeprobs1                            : __CODEGEN_BITFIELD( 8, 15)    ; //!< MBSegmentIDTreeProbs[1]
                uint32_t                 Mbsegmentidtreeprobs2                            : __CODEGEN_BITFIELD(16, 23)    ; //!< MBSegmentIDTreeProbs[2]
                uint32_t                 Reserved632                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 Interpredfromgrefrefprob                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< InterPredFromGRefRefProb
                uint32_t                 Interpredfromlastrefprob                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< InterPredFromLastRefProb
                uint32_t                 Intrambprob                                      : __CODEGEN_BITFIELD(16, 23)    ; //!< IntraMBProb
                uint32_t                 Mbnocoeffskipfalseprob                           : __CODEGEN_BITFIELD(24, 31)    ; //!< MBNoCoeffSkipFalseProb
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 Ymodeprob0                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< YModeProb[0]
                uint32_t                 Ymodeprob1                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< YModeProb[1]
                uint32_t                 Ymodeprob2                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< YModeProb[2]
                uint32_t                 Ymodeprob3                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< YModeProb[3]
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 Uvmodeprob0                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< UVModeProb[0]
                uint32_t                 Uvmodeprob1                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< UVModeProb[1]
                uint32_t                 Uvmodeprob2                                      : __CODEGEN_BITFIELD(16, 23)    ; //!< UVModeProb[2]
                uint32_t                 Reserved728                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 Mvupdateprobs00                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[0][0]
                uint32_t                 Mvupdateprobs01                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[0][1]
                uint32_t                 Mvupdateprobs02                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[0][2]
                uint32_t                 Mvupdateprobs03                                  : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[0][3]
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 Mvupdateprobs04                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[0][4]
                uint32_t                 Mvupdateprobs05                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[0][5]
                uint32_t                 Mvupdateprobs06                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[0][6]
                uint32_t                 Mvupdateprobs07                                  : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[0][7]
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 Mvupdateprobs08                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[0][8]
                uint32_t                 Mvupdateprobs09                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[0][9]
                uint32_t                 Mvupdateprobs010                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[0][10]
                uint32_t                 Mvupdateprobs011                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[0][11]
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 Mvupdateprobs012                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[0][12]
                uint32_t                 Mvupdateprobs013                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[0][13]
                uint32_t                 Mvupdateprobs014                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[0][14]
                uint32_t                 Mvupdateprobs015                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[0][15]
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 Mvupdateprobs016                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[0][16]
                uint32_t                 Mvupdateprobs017                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[0][17]
                uint32_t                 Mvupdateprobs018                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[0][18]
                uint32_t                 Reserved888                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 Mvupdateprobs10                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[1][0]
                uint32_t                 Mvupdateprobs11                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[1][1]
                uint32_t                 Mvupdateprobs12                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[1][2]
                uint32_t                 Mvupdateprobs13                                  : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[1][3]
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 Mvupdateprobs14                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[1][4]
                uint32_t                 Mvupdateprobs15                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[1][5]
                uint32_t                 Mvupdateprobs16                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[1][6]
                uint32_t                 Mvupdateprobs17                                  : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[1][7]
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 Mvupdateprobs18                                  : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[1][8]
                uint32_t                 Mvupdateprobs19                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[1][9]
                uint32_t                 Mvupdateprobs110                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[1][10]
                uint32_t                 Mvupdateprobs111                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[1][11]
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 Mvupdateprobs112                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[1][12]
                uint32_t                 Mvupdateprobs113                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[1][13]
                uint32_t                 Mvupdateprobs114                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[1][14]
                uint32_t                 Mvupdateprobs115                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< MVUpdateProbs[1][15]
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            //!< DWORD 32
            struct
            {
                uint32_t                 Mvupdateprobs116                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< MVUpdateProbs[1][16]
                uint32_t                 Mvupdateprobs117                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< MVUpdateProbs[1][17]
                uint32_t                 Mvupdateprobs118                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< MVUpdateProbs[1][18]
                uint32_t                 Reserved1048                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 Reflfdelta0ForIntraFrame                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< RefLFDelta0 (for INTRA FRAME)
                uint32_t                 Reserved1063                                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 Reflfdelta1ForLastFrame                          : __CODEGEN_BITFIELD( 8, 14)    ; //!< RefLFDelta1 (for LAST FRAME)
                uint32_t                 Reserved1071                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Reflfdelta2ForGoldenFrame                        : __CODEGEN_BITFIELD(16, 22)    ; //!< RefLFDelta2 (for GOLDEN FRAME)
                uint32_t                 Reserved1079                                     : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 Reflfdelta3ForAltrefFrame                        : __CODEGEN_BITFIELD(24, 30)    ; //!< RefLFDelta3 (for ALTREF FRAME)
                uint32_t                 Reserved1087                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            //!< DWORD 34
            struct
            {
                uint32_t                 Modelfdelta0ForBPredMode                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< ModeLFDelta0 (for B_PRED mode)
                uint32_t                 Reserved1095                                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 Modelfdelta1ForZeromvMode                        : __CODEGEN_BITFIELD( 8, 14)    ; //!< ModeLFDelta1(for ZEROMV mode)
                uint32_t                 Reserved1103                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Modelfdelta2ForNearestNearAndNewMode             : __CODEGEN_BITFIELD(16, 22)    ; //!< ModeLFDelta2 (for Nearest, Near and New mode)
                uint32_t                 Reserved1111                                     : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 Modelfdelta3ForSplitmvMode                       : __CODEGEN_BITFIELD(24, 30)    ; //!< ModeLFDelta3 (for SPLITMV mode)
                uint32_t                 Reserved1119                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            //!< DWORD 35
            struct
            {
                uint32_t                 SegmentationIdStreamBaseAddress                                                  ; //!< Segmentation ID Stream Base Address
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            //!< DWORD 36
            struct
            {
                uint32_t                 SegmentationIdStreamBaseAddress4732              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Segmentation ID Stream Base Address [47:32]
                uint32_t                 Reserved1168                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            //!< DWORD 37
            struct
            {
                uint32_t                 Reserved1184                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 CoeffprobabilityStreaminAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< CoeffProbability StreamIn Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 SegmentationIdStreamArbitrationPriorityControl   : __CODEGEN_BITFIELD( 7,  8)    ; //!< SEGMENTATION_ID_STREAM__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 SegmentationIdStreamMemoryCompressionEnable      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Segmentation ID Stream - Memory Compression Enable
                uint32_t                 SegmentationIdStreamMemoryCompressionMode        : __CODEGEN_BITFIELD(10, 10)    ; //!< SEGMENTATION_ID_STREAM__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1195                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 SegmentationIdStreamTiledResourceMode            : __CODEGEN_BITFIELD(13, 14)    ; //!< SEGMENTATION_ID_STREAM__TILED_RESOURCE_MODE
                uint32_t                 Reserved1199                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW37;

        //! \name Local enumerations

        enum SUB_OPCODE_B
        {
            SUB_OPCODE_B_MFXVP8PICSTATE                                      = 0, //!< No additional details
        };

        enum SUB_OPCODE_A
        {
            SUB_OPCODE_A_VP8COMMON                                           = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VP8                                         = 4, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_VIDEOCODEC                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief MC_FILTER_SELECT
        //! \details
        //!     To specify VP8 Profile of operation.
        enum MC_FILTER_SELECT
        {
            MC_FILTER_SELECT_UNNAMED0                                        = 0, //!< 6-tap filter (regular filter mode)
            MC_FILTER_SELECT_UNNAMED1                                        = 1, //!< 2-tap bilinear filter (simple profile/version mode)
        };

        //! \brief CHROMA_FULL_PIXEL_MC_FILTER_MODE
        //! \details
        //!     To specify VP8 Profile of operation.
        enum CHROMA_FULL_PIXEL_MC_FILTER_MODE
        {
            CHROMA_FULL_PIXEL_MC_FILTER_MODE_UNNAMED0                        = 0, //!< Chroma MC filter operates in sub-pixel mode
            CHROMA_FULL_PIXEL_MC_FILTER_MODE_UNNAMED1                        = 1, //!< Chroma MC filter only operates in full pixel position, i.e. no sub-pixel interpolation.
        };

        //! \brief DBLKFILTERTYPE
        //! \details
        //!     To specify VP8 Profile of operation.
        enum DBLKFILTERTYPE
        {
            DBLKFILTERTYPE_UNNAMED0                                          = 0, //!< Use a full feature normal deblocking filter
            DBLKFILTERTYPE_UNNAMED1                                          = 1, //!< Use a simple filter for deblocking
        };

        enum SKEYFRAMEFLAG
        {
            SKEYFRAMEFLAG_NON_KEYFRAMEP_FRAME                                = 0, //!< No additional details
            SKEYFRAMEFLAG_KEYFRAMEI_FRAME                                    = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_ID_STREAMOUT_ENABLE
        //! \details
        //!     When 0, no output needed.
        enum SEGMENTATION_ID_STREAMOUT_ENABLE
        {
            SEGMENTATION_ID_STREAMOUT_ENABLE_STREAMOUTDISABLED               = 0, //!< No additional details
            SEGMENTATION_ID_STREAMOUT_ENABLE_STREAMOUTENABLED                = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_ID_STREAMIN_ENABLE
        //! \details
        //!     When 0, no input needed.
        enum SEGMENTATION_ID_STREAMIN_ENABLE
        {
            SEGMENTATION_ID_STREAMIN_ENABLE_STREAMINDISABLED                 = 0, //!< No additional details
            SEGMENTATION_ID_STREAMIN_ENABLE_STREAMINENABLED                  = 1, //!< No additional details
        };

        enum SEGMENT_ENABLE_FLAG
        {
            SEGMENT_ENABLE_FLAG_UNNAMED0                                     = 0, //!< Disable Segmentation processing in the current frame
            SEGMENT_ENABLE_FLAG_UNNAMED1                                     = 1, //!< Enable Segmentation processing in the current frame
        };

        enum UPDATE_MBSEGMENT_MAP_FLAG
        {
            UPDATE_MBSEGMENT_MAP_FLAG_UNNAMED0                               = 0, //!< Disable segmentation update
            UPDATE_MBSEGMENT_MAP_FLAG_UNNAMED1                               = 1, //!< Enable segmentation update, and to enable reading segment_id for each MB.
        };

        //! \brief MB_NOCOEFF_SKIPFLAG
        //! \details
        //!     Frame level control if Skip MB (with no non-zero coefficient) is allowed
        //!     or not.
        enum MB_NOCOEFF_SKIPFLAG
        {
            MB_NOCOEFF_SKIPFLAG_UNNAMED0                                     = 0, //!< All MBs will have its MB level signaling mb_skip_coeff forced to 0.  That is, no skip of coefficient record in the bitstream (even their values are all 0s)
            MB_NOCOEFF_SKIPFLAG_UNNAMED1                                     = 1, //!< Skip MB is enabled in the per MB record.
        };

        enum MODE_REFERENCE_LOOP_FILTER_DELTA_ENABLED
        {
            MODE_REFERENCE_LOOP_FILTER_DELTA_ENABLED_UNNAMED0                = 0, //!< Mode or Reference Loop Filter Delta Adjustment for current frame is disabled.
            MODE_REFERENCE_LOOP_FILTER_DELTA_ENABLED_UNNAMED1                = 1, //!< Mode or Reference Loop Filter Delta Adjustment for current frame is enabled.
        };

        enum LOG2_NUM_OF_PARTITION
        {
            LOG2_NUM_OF_PARTITION_1TOKENPARTITION                            = 0, //!< No additional details
            LOG2_NUM_OF_PARTITION_2TOKENPARTITION                            = 1, //!< No additional details
            LOG2_NUM_OF_PARTITION_4TOKENPARTITION                            = 2, //!< No additional details
            LOG2_NUM_OF_PARTITION_8TOKENPARTITION                            = 3, //!< No additional details
        };

        //! \brief DBLKFILTERLEVEL_FOR_SEGMENT0
        //! \details
        //!     There are max 4 segments per frame, each segment can have its own
        //!     deblocking filter level.  When segmentation is disabled, only segment 0
        //!     parameter is used for the entire frame.
        enum DBLKFILTERLEVEL_FOR_SEGMENT0
        {
            DBLKFILTERLEVEL_FOR_SEGMENT0_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION = 0, //!< This is used to set a VP8 profile without in loop deblocker.
        };

        //! \brief DBLKFILTERLEVEL_FOR_SEGMENT1
        //! \details
        //!     There are max 4 segments per frame, each segment can have its own
        //!     deblocking filter level.  When segmentation is disabled, only segment 0
        //!     parameter is used for the entire frame.
        enum DBLKFILTERLEVEL_FOR_SEGMENT1
        {
            DBLKFILTERLEVEL_FOR_SEGMENT1_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION = 0, //!< This is used to set a VP8 profile without in loop deblocker.
        };

        //! \brief DBLKFILTERLEVEL_FOR_SEGMENT2
        //! \details
        //!     There are max 4 segments per frame, each segment can have its own
        //!     deblocking filter level.  When segmentation is disabled, only segment 0
        //!     parameter is used for the entire frame.
        enum DBLKFILTERLEVEL_FOR_SEGMENT2
        {
            DBLKFILTERLEVEL_FOR_SEGMENT2_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION = 0, //!< This is used to set a VP8 profile without in loop deblocker.
        };

        //! \brief DBLKFILTERLEVEL_FOR_SEGMENT3
        //! \details
        //!     There are max 4 segments per frame, each segment can have its own
        //!     deblocking filter level.  When segmentation is disabled, only segment 0
        //!     parameter is used for the entire frame.
        enum DBLKFILTERLEVEL_FOR_SEGMENT3
        {
            DBLKFILTERLEVEL_FOR_SEGMENT3_SIGNIFIESDISABLEINLOOPDEBLOCKINGOPERATION = 0, //!< This is used to set a VP8 profile without in loop deblocker.
        };

        //! \brief COEFFPROBABILITY_STREAMIN__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum COEFFPROBABILITY_STREAMIN__ARBITRATION_PRIORITY_CONTROL
        {
            COEFFPROBABILITY_STREAMIN_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            COEFFPROBABILITY_STREAMIN_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            COEFFPROBABILITY_STREAMIN_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            COEFFPROBABILITY_STREAMIN_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief COEFFPROBABILITY_STREAMIN__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a
        //!                         <b>Memory Data Formats</b> chapter, <b>Media Memory Compression</b>
        //!     section for more details.
        enum COEFFPROBABILITY_STREAMIN__MEMORY_COMPRESSION_MODE
        {
            COEFFPROBABILITY_STREAMIN_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            COEFFPROBABILITY_STREAMIN_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief COEFFPROBABILITY_STREAMIN__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum COEFFPROBABILITY_STREAMIN__TILED_RESOURCE_MODE
        {
            COEFFPROBABILITY_STREAMIN_TILED_RESOURCE_MODE_TRMODENONE         = 0, //!< No tiled resource
            COEFFPROBABILITY_STREAMIN_TILED_RESOURCE_MODE_TRMODETILEYF       = 1, //!< 4KB tiled resources
            COEFFPROBABILITY_STREAMIN_TILED_RESOURCE_MODE_TRMODETILEYS       = 2, //!< 64KB tiled resources
        };

        //! \brief SEGMENTATION_ID_STREAM__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum SEGMENTATION_ID_STREAM__ARBITRATION_PRIORITY_CONTROL
        {
            SEGMENTATION_ID_STREAM_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            SEGMENTATION_ID_STREAM_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            SEGMENTATION_ID_STREAM_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            SEGMENTATION_ID_STREAM_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief SEGMENTATION_ID_STREAM__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a
        //!                         <b>Memory Data Formats</b> chapter, <b>Media Memory Compression</b>
        //!     section for more details.
        enum SEGMENTATION_ID_STREAM__MEMORY_COMPRESSION_MODE
        {
            SEGMENTATION_ID_STREAM_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            SEGMENTATION_ID_STREAM_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_ID_STREAM__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum SEGMENTATION_ID_STREAM__TILED_RESOURCE_MODE
        {
            SEGMENTATION_ID_STREAM_TILED_RESOURCE_MODE_TRMODENONE            = 0, //!< No tiled resource
            SEGMENTATION_ID_STREAM_TILED_RESOURCE_MODE_TRMODETILEYF          = 1, //!< 4KB tiled resources
            SEGMENTATION_ID_STREAM_TILED_RESOURCE_MODE_TRMODETILEYS          = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_VP8_PIC_STATE_CMD();

        static const size_t dwSize = 38;
        static const size_t byteSize = 152;
    };

    //!
    //! \brief MFX_JPEG_HUFF_TABLE_STATE
    //! \details
    //!     This Huffman table commands contains both DC and AC tables for either
    //!     luma or chroma. Once a Huffman table has been defined for a particular
    //!     destination, it replaces the previous tables stored in that destination
    //!     and shall be used in the remaining Scans of the current image. A Huffman
    //!     table will be sent to H/W only when it is loaded from bitstream.
    //!     
    struct MFX_JPEG_HUFF_TABLE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Hufftableid1Bit                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< HUFFTABLEID_1_BIT
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        uint32_t                         DcBits128BitArray[3];                                                            //!< DC_BITS (12 8-bit array)


        uint32_t                         DcHuffval128BitArray[3];                                                         //!< DC_HUFFVAL (12 8-bit array)


        uint32_t                         AcBits168BitArray[4];                                                            //!< AC_BITS (16 8-bit array)


        uint32_t                         AcHuffval1608BitArray[40];                                                       //!< AC_HUFFVAL (160 8-bit array)

        union
        {
            //!< DWORD 52
            struct
            {
                uint32_t                 AcHuffval28BitArray                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< AC_HUFFVAL(2-8 bit array)
                uint32_t                 Reserved1680                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW52;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED2                                             = 2, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_JPEGCOMMON                                  = 7, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXMULTIDW                                              = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief HUFFTABLEID_1_BIT
        //! \details
        //!     Identifies the huffman table.
        enum HUFFTABLEID_1_BIT
        {
            HUFFTABLEID_1_BIT_Y                                              = 0, //!< Huffman table for Y
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_JPEG_HUFF_TABLE_STATE_CMD();

        static const size_t dwSize = 53;
        static const size_t byteSize = 212;
    };

    //!
    //! \brief GRAPHICSADDRESS63_6
    //! \details
    //!     This structure is intended to define the upper bits of the
    //!     GraphicsAddress, when bits 5:0 are already defined in the referring
    //!     register. So bit 0 of this structure should correspond to bit 6 of the
    //!     full GraphicsAddress.
    //!     
    struct GRAPHICSADDRESS63_6_CMD
    {
        union
        {
            //!< DWORD 0..1
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint64_t                 Graphicsaddress476                               : __CODEGEN_BITFIELD( 6, 47)    ; //!< GraphicsAddress47-6
                uint64_t                 Reserved48                                       : __CODEGEN_BITFIELD(48, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        GRAPHICSADDRESS63_6_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief MFX_PIPE_BUF_ADDR_STATE
    //! \details
    //!     This state command provides the memory base addresses for all row
    //!     stores, StreamOut buffer and reconstructed picture output buffers
    //!     required by the MFD or MFC Engine (that are in addition to the row
    //!     stores of the Bit Stream Decoding/Encoding Unit (BSD/BSE) and the
    //!     reference picture buffers).
    //!     This is a picture level state command and is common among all codec
    //!     standards and for both encoder and decoder operating modes. However,
    //!     some fields may only applicable to a specific codec standard. All Pixel
    //!     Surfaces (original, reference frame and reconstructed frame) in the
    //!     Encoder are programmed with the same surface state (NV12 and TileY
    //!     format), except each has its own frame buffer base address. In the tile
    //!     format, there is no need to provide buffer offset for each slice; since
    //!     from each MB address, the hardware can calculated the corresponding
    //!     memory location within the frame buffer directly.
    //!     
    struct MFX_PIPE_BUF_ADDR_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 CommonOpcode                                     : __CODEGEN_BITFIELD(24, 26)    ; //!< COMMON_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 PreDeblockingDestinationAddress                  : __CODEGEN_BITFIELD( 6, 31)    ; //!< Pre Deblocking Destination Address
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 PreDeblockingDestinationAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Pre Deblocking Destination Address High
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 PreDeblockingArbitrationPriorityControl          : __CODEGEN_BITFIELD( 7,  8)    ; //!< PRE_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 PreDeblockingMemoryCompressionEnable             : __CODEGEN_BITFIELD( 9,  9)    ; //!< PRE_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
                uint32_t                 PreDeblockingMemoryCompressionType               : __CODEGEN_BITFIELD(10, 10)    ; //!< Pre Deblocking - Memory Compression Type
                uint32_t                 Reserved107                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 PreDeblockingTiledResourceMode                   : __CODEGEN_BITFIELD(13, 14)    ; //!< PRE_DEBLOCKING__TILED_RESOURCE_MODE
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 PostDeblockingDestinationAddress                 : __CODEGEN_BITFIELD( 6, 31)    ; //!< Post Deblocking Destination Address
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 PostDeblockingDestinationAddressHigh             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Post Deblocking Destination Address High
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 PostDeblockingArbitrationPriorityControl         : __CODEGEN_BITFIELD( 7,  8)    ; //!< POST_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 PostDeblockingMemoryCompressionEnable            : __CODEGEN_BITFIELD( 9,  9)    ; //!< POST_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
                uint32_t                 PostDeblockingMemoryCompressionType              : __CODEGEN_BITFIELD(10, 10)    ; //!< POST_DEBLOCKING__MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved203                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 PostDeblockingTiledResourceMode                  : __CODEGEN_BITFIELD(13, 14)    ; //!< POST_DEBLOCKING__TILED_RESOURCE_MODE
                uint32_t                 Reserved207                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved224                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 OriginalUncompressedPictureSourceAddress         : __CODEGEN_BITFIELD( 6, 31)    ; //!< Original Uncompressed Picture Source Address
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 OriginalUncompressedPictureSourceAddressHigh     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Original Uncompressed Picture Source Address High
                uint32_t                 Reserved272                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 OriginalUncompressedPictureSourceArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 OriginalUncompressedPictureMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_ENABLE
                uint32_t                 OriginalUncompressedPictureMemoryCompressionType : __CODEGEN_BITFIELD(10, 10)    ; //!< ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved299                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 OriginalUncompressedPictureTiledResourceMode     : __CODEGEN_BITFIELD(13, 14)    ; //!< ORIGINAL_UNCOMPRESSED_PICTURE__TILED_RESOURCE_MODE
                uint32_t                 Reserved303                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 StreamoutDataDestinationBaseAddress              : __CODEGEN_BITFIELD( 6, 31)    ; //!< StreamOut Data Destination Base Address
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 StreamoutDataDestinationBaseAddressHigh          : __CODEGEN_BITFIELD( 0, 15)    ; //!< StreamOut Data Destination Base Address High
                uint32_t                 Reserved368                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 StreamoutDataDestinationArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 StreamoutDataDestinationMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved394                                      : __CODEGEN_BITFIELD(10, 10)    ; //!< STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved395                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 StreamoutDataDestinationTiledResourceMode        : __CODEGEN_BITFIELD(13, 14)    ; //!< STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
                uint32_t                 Reserved399                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 Reserved416                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 IntraRowStoreScratchBufferBaseAddress            : __CODEGEN_BITFIELD( 6, 31)    ; //!< Intra Row Store Scratch Buffer Base Address
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 IntraRowStoreScratchBufferBaseAddressHigh        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra Row Store Scratch Buffer Base Address High
                uint32_t                 Reserved464                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 IntraRowStoreScratchBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< INTRA_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 IntraRowStoreScratchBufferMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved490                                      : __CODEGEN_BITFIELD(10, 10)    ; //!< INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved491                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 IntraRowStoreScratchBufferCacheSelect            : __CODEGEN_BITFIELD(12, 12)    ; //!< INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 IntraRowStoreScratchBufferTiledResourceMode      : __CODEGEN_BITFIELD(13, 14)    ; //!< INTRA_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved495                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved512                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 DeblockingFilterRowStoreScratchBaseAddress       : __CODEGEN_BITFIELD( 6, 31)    ; //!< Deblocking Filter Row Store Scratch Base Address
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 DeblockingFilterRowStoreScratchBaseAddressHigh   : __CODEGEN_BITFIELD( 0, 15)    ; //!< Deblocking Filter Row Store Scratch Base Address High
                uint32_t                 Reserved560                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 DeblockingFilterRowStoreScratchArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< DEBLOCKING_FILTER_ROW_STORE_SCRATCH__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 DeblockingFilterRowStoreScratchMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved586                                      : __CODEGEN_BITFIELD(10, 10)    ; //!< DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved587                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 DeblockingFilterRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 DeblockingFilterRowStoreTiledResourceMode        : __CODEGEN_BITFIELD(13, 14)    ; //!< DEBLOCKING_FILTER_ROW_STORE__TILED_RESOURCE_MODE
                uint32_t                 Reserved591                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;

        mhw_vdbox_mfx_g12_X::GRAPHICSADDRESS63_6_CMD          Refpicbaseaddr[16];                                         //!< RefPicBaseAddr

        union
        {
            //!< DWORD 51
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 ReferencePictureArbitrationPriorityControl       : __CODEGEN_BITFIELD( 7,  8)    ; //!< REFERENCE_PICTURE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 Reserved1641                                     : __CODEGEN_BITFIELD( 9, 12)    ; //!< Reserved
                uint32_t                 ReferencePictureTiledResourceMode                : __CODEGEN_BITFIELD(13, 14)    ; //!< REFERENCE_PICTURE__TILED_RESOURCE_MODE
                uint32_t                 Reserved1647                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW51;
        union
        {
            //!< DWORD 52
            struct
            {
                uint32_t                 Reserved1664                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 MacroblockBufferBaseAddressOrDecodedPictureErrorStatusBufferBaseAddress : __CODEGEN_BITFIELD( 6, 31)    ; //!< Macroblock Buffer Base Address or Decoded Picture Error/Status Buffer Base Address
            };
            uint32_t                     Value;
        } DW52;
        union
        {
            //!< DWORD 53
            struct
            {
                uint32_t                 MacroblockBufferBaseAddressOrDecodedPictureErrorStatusBufferBaseAddressHigh : __CODEGEN_BITFIELD( 0, 15)    ; //!< Macroblock Buffer Base Address or Decoded Picture Error/Status Buffer Base Address High
                uint32_t                 Reserved1712                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW53;
        union
        {
            //!< DWORD 54
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MacroblockStatusBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MACROBLOCK_STATUS_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 MacroblockStatusBufferMemoryCompressionEnable    : __CODEGEN_BITFIELD( 9,  9)    ; //!< MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved1738                                     : __CODEGEN_BITFIELD(10, 10)    ; //!< MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1739                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 MacroblockStatusBufferTiledResourceMode          : __CODEGEN_BITFIELD(13, 14)    ; //!< MACROBLOCK_STATUS_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved1743                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW54;
        union
        {
            //!< DWORD 55
            struct
            {
                uint32_t                 Reserved1760                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 MacroblockIldbStreamoutBufferBaseAddress         : __CODEGEN_BITFIELD( 6, 31)    ; //!< Macroblock ILDB StreamOut Buffer Base Address
            };
            uint32_t                     Value;
        } DW55;
        union
        {
            //!< DWORD 56
            struct
            {
                uint32_t                 MacroblockIldbStreamoutBufferBaseAddressHigh     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Macroblock ILDB StreamOut Buffer Base Address High
                uint32_t                 Reserved1808                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW56;
        union
        {
            //!< DWORD 57
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 MacroblockIldbStreamoutBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< MACROBLOCK_ILDB_STREAMOUT_BUFFER__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 MacroblockIldbStreamoutBufferMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved1834                                     : __CODEGEN_BITFIELD(10, 10)    ; //!< MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1835                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 MacroblockIldbStreamoutTiledResourceMode         : __CODEGEN_BITFIELD(13, 14)    ; //!< MACROBLOCK_ILDB_STREAMOUT__TILED_RESOURCE_MODE
                uint32_t                 Reserved1839                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW57;
        union
        {
            //!< DWORD 58
            struct
            {
                uint32_t                 Reserved1856                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 SecondMacroblockIldbStreamoutBufferBaseAddress   : __CODEGEN_BITFIELD( 6, 31)    ; //!< Second Macroblock ILDB StreamOut Buffer Base Address
            };
            uint32_t                     Value;
        } DW58;
        union
        {
            //!< DWORD 59
            struct
            {
                uint32_t                 SecondMacroblockIldbStreamoutBufferBaseAddressHigh : __CODEGEN_BITFIELD( 0, 15)    ; //!< Second Macroblock ILDB StreamOut Buffer Base Address High
                uint32_t                 Reserved1904                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW59;
        union
        {
            //!< DWORD 60
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 SecondMacroblockIldbStreamoutBufferArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< Second Macroblock ILDB StreamOut Buffer - Arbitration Priority Control
                uint32_t                 SecondMacroblockIldbStreamoutBufferMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
                uint32_t                 Reserved1930                                     : __CODEGEN_BITFIELD(10, 10)    ; //!< SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1931                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 SecondMacroblockIldbStreamoutBufferTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__TILED_RESOURCE_MODE
                uint32_t                 Reserved1935                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW60;
        union
        {
            //!< DWORD 61
            struct
            {
                uint32_t                 ReferencePicture0MemoryCompressionEnable         : __CODEGEN_BITFIELD( 0,  0)    ; //!< REFERENCE_PICTURE_0__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture0MemoryCompressionType           : __CODEGEN_BITFIELD( 1,  1)    ; //!< REFERENCE_PICTURE_0__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture1MemoryCompressionEnable         : __CODEGEN_BITFIELD( 2,  2)    ; //!< REFERENCE_PICTURE_1__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture1MemoryCompressionType           : __CODEGEN_BITFIELD( 3,  3)    ; //!< REFERENCE_PICTURE_1__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture2MemoryCompressionEnable         : __CODEGEN_BITFIELD( 4,  4)    ; //!< REFERENCE_PICTURE_2__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture2MemoryCompressionType           : __CODEGEN_BITFIELD( 5,  5)    ; //!< REFERENCE_PICTURE_2__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture3MemoryCompressionEnable         : __CODEGEN_BITFIELD( 6,  6)    ; //!< REFERENCE_PICTURE_3__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture3MemoryCompressionType           : __CODEGEN_BITFIELD( 7,  7)    ; //!< REFERENCE_PICTURE_3__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture4MemoryCompressionEnable         : __CODEGEN_BITFIELD( 8,  8)    ; //!< REFERENCE_PICTURE_4__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture4MemoryCompressionType           : __CODEGEN_BITFIELD( 9,  9)    ; //!< REFERENCE_PICTURE_4__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture5MemoryCompressionEnable         : __CODEGEN_BITFIELD(10, 10)    ; //!< REFERENCE_PICTURE_5__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture5MemoryCompressionType           : __CODEGEN_BITFIELD(11, 11)    ; //!< REFERENCE_PICTURE_5__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture6MemoryCompressionEnable         : __CODEGEN_BITFIELD(12, 12)    ; //!< REFERENCE_PICTURE_6__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture6MemoryCompressionType           : __CODEGEN_BITFIELD(13, 13)    ; //!< REFERENCE_PICTURE_6__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture7MemoryCompressionEnable         : __CODEGEN_BITFIELD(14, 14)    ; //!< REFERENCE_PICTURE_7__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture7MemoryCompressionType           : __CODEGEN_BITFIELD(15, 15)    ; //!< REFERENCE_PICTURE_7__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture8MemoryCompressionEnable         : __CODEGEN_BITFIELD(16, 16)    ; //!< REFERENCE_PICTURE_8__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture8MemoryCompressionType           : __CODEGEN_BITFIELD(17, 17)    ; //!< REFERENCE_PICTURE_8__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture9MemoryCompressionEnable         : __CODEGEN_BITFIELD(18, 18)    ; //!< REFERENCE_PICTURE_9__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture9MemoryCompressionType           : __CODEGEN_BITFIELD(19, 19)    ; //!< REFERENCE_PICTURE_9__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture10MemoryCompressionEnable        : __CODEGEN_BITFIELD(20, 20)    ; //!< REFERENCE_PICTURE_10__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture10MemoryCompressionType          : __CODEGEN_BITFIELD(21, 21)    ; //!< REFERENCE_PICTURE_10__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture11MemoryCompressionEnable        : __CODEGEN_BITFIELD(22, 22)    ; //!< REFERENCE_PICTURE_11__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture11MemoryCompressionType          : __CODEGEN_BITFIELD(23, 23)    ; //!< REFERENCE_PICTURE_11__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture12MemoryCompressionEnable        : __CODEGEN_BITFIELD(24, 24)    ; //!< REFERENCE_PICTURE_12__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture12MemoryCompressionType          : __CODEGEN_BITFIELD(25, 25)    ; //!< REFERENCE_PICTURE_12__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture13MemoryCompressionEnable        : __CODEGEN_BITFIELD(26, 26)    ; //!< REFERENCE_PICTURE_13__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture13MemoryCompressionType          : __CODEGEN_BITFIELD(27, 27)    ; //!< REFERENCE_PICTURE_13__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture14MemoryCompressionEnable        : __CODEGEN_BITFIELD(28, 28)    ; //!< REFERENCE_PICTURE_14__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture14MemoryCompressionType          : __CODEGEN_BITFIELD(29, 29)    ; //!< REFERENCE_PICTURE_14__MEMORY_COMPRESSION_MODE
                uint32_t                 ReferencePicture15MemoryCompressionEnable        : __CODEGEN_BITFIELD(30, 30)    ; //!< REFERENCE_PICTURE_15__MEMORY_COMPRESSION_ENABLE
                uint32_t                 ReferencePicture15MemoryCompressionType          : __CODEGEN_BITFIELD(31, 31)    ; //!< REFERENCE_PICTURE_15__MEMORY_COMPRESSION_MODE
            };
            uint32_t                     Value;
        } DW61;
        union
        {
            //!< DWORD 62
            struct
            {
                uint32_t                 Reserved1984                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 ScaledReferenceSurfaceBaseAddress                : __CODEGEN_BITFIELD( 6, 31)    ; //!< Scaled Reference Surface Base Address
            };
            uint32_t                     Value;
        } DW62;
        union
        {
            //!< DWORD 63
            struct
            {
                uint32_t                 ScaledReferenceSurfaceBaseAddressHigh            : __CODEGEN_BITFIELD( 0, 15)    ; //!< Scaled Reference Surface Base Address High
                uint32_t                 Reserved2032                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW63;
        union
        {
            //!< DWORD 64
            struct
            {
                uint32_t                 Reserved2048                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 ScaledReferenceSurfaceIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Scaled Reference Surface - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 ScaleReferenceSurfaceArbitrationPriorityControl  : __CODEGEN_BITFIELD( 7,  8)    ; //!< SCALE_REFERENCE_SURFACE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 ScaledReferenceSurfaceMemoryCompressionEnable    : __CODEGEN_BITFIELD( 9,  9)    ; //!< Scaled Reference Surface - Memory Compression Enable
                uint32_t                 ScaledReferenceSurfaceMemoryCompressionType      : __CODEGEN_BITFIELD(10, 10)    ; //!< SCALED_REFERENCE_SURFACE__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved2059                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 ScaledReferenceSurfaceTiledResourceMode          : __CODEGEN_BITFIELD(13, 14)    ; //!< SCALED_REFERENCE_SURFACE__TILED_RESOURCE_MODE
                uint32_t                 Reserved2063                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW64;
         union
        {
            //!< DWORD 65
            struct
            {
                uint32_t                 Reserved2080                                     : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 SlicesizeStreamoutDataDestinationBaseAddress     : __CODEGEN_BITFIELD( 6, 31)    ; //!< SliceSize StreamOut Data Destination Base Address
            };
            uint32_t                     Value;
        } DW65;
        union
        {
            //!< DWORD 66
            struct
            {
                uint32_t                 SlicesizeStreamoutDataDestinationBaseAddressHigh : __CODEGEN_BITFIELD( 0, 15)    ; //!< SliceSize StreamOut Data Destination Base Address High
                uint32_t                 Reserved2128                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW66;
        union
        {
            //!< DWORD 67
            struct
            {
                uint32_t                 Reserved2144                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 SlicesizeStreamoutDataDestinationIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< SliceSize StreamOut Data Destination - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 SlicesizeStreamoutDataDestinationArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< SLICESIZE_STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 SlicesizeStreamoutDataDestinationMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< SliceSize StreamOut Data Destination - Memory Compression Enable
                uint32_t                 Reserved2154                                     : __CODEGEN_BITFIELD(10, 10)    ; //!< SLICESIZE_STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved2155                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 SlicesizeStreamoutDataDestinationTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< SLICESIZE_STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
                uint32_t                 Reserved2159                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW67;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED2                                             = 2, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum COMMON_OPCODE
        {
            COMMON_OPCODE_MFXCOMMONSTATE                                     = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXPIPEBUFADDRSTATE                                     = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief PRE_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum PRE_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
        {
            PRE_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY      = 0, //!< No additional details
            PRE_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            PRE_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            PRE_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY       = 3, //!< No additional details
        };

        //! \brief PRE_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     Memory compression will be attempted for this surface.
        enum PRE_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
        {
            PRE_DEBLOCKING_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE      = 0, //!< No additional details
            PRE_DEBLOCKING_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE       = 1, //!< No additional details
        };

        //! \brief PRE_DEBLOCKING__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum PRE_DEBLOCKING__TILED_RESOURCE_MODE
        {
            PRE_DEBLOCKING_TILED_RESOURCE_MODE_TRMODENONE                    = 0, //!< No tiled resource
            PRE_DEBLOCKING_TILED_RESOURCE_MODE_TRMODETILEYF                  = 1, //!< 4KB tiled resources
            PRE_DEBLOCKING_TILED_RESOURCE_MODE_TRMODETILEYS                  = 2, //!< 64KB tiled resources
        };

        //! \brief POST_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum POST_DEBLOCKING__ARBITRATION_PRIORITY_CONTROL
        {
            POST_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY     = 0, //!< No additional details
            POST_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            POST_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            POST_DEBLOCKING_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY      = 3, //!< No additional details
        };

        //! \brief POST_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     Memory compression will be attempted for this surface.
        enum POST_DEBLOCKING__MEMORY_COMPRESSION_ENABLE
        {
            POST_DEBLOCKING_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE     = 0, //!< No additional details
            POST_DEBLOCKING_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE      = 1, //!< No additional details
        };

        //! \brief POST_DEBLOCKING__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a <b>Memory Data Formats chapter -section</b> Media Memory
        //!     Compression <b>for more details.</b>
        enum POST_DEBLOCKING__MEMORY_COMPRESSION_MODE
        {
            POST_DEBLOCKING_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
        };

        //! \brief POST_DEBLOCKING__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum POST_DEBLOCKING__TILED_RESOURCE_MODE
        {
            POST_DEBLOCKING_TILED_RESOURCE_MODE_TRMODENONE                   = 0, //!< No tiled resource
            POST_DEBLOCKING_TILED_RESOURCE_MODE_TRMODETILEYF                 = 1, //!< 4KB tiled resources
            POST_DEBLOCKING_TILED_RESOURCE_MODE_TRMODETILEYS                 = 2, //!< 64KB tiled resources
        };

        //! \brief ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE__ARBITRATION_PRIORITY_CONTROL
        {
            ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            ORIGINAL_UNCOMPRESSED_PICTURE_SOURCE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     Note:  This is a READ Surface.  The setting of this bit should match the
        //!     settings on how this is written out before.
        enum ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_ENABLE
        {
            ORIGINAL_UNCOMPRESSED_PICTURE_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before
        enum ORIGINAL_UNCOMPRESSED_PICTURE__MEMORY_COMPRESSION_MODE
        {
            ORIGINAL_UNCOMPRESSED_PICTURE_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            ORIGINAL_UNCOMPRESSED_PICTURE_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief ORIGINAL_UNCOMPRESSED_PICTURE__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum ORIGINAL_UNCOMPRESSED_PICTURE__TILED_RESOURCE_MODE
        {
            ORIGINAL_UNCOMPRESSED_PICTURE_TILED_RESOURCE_MODE_TRMODENONE     = 0, //!< No tiled resource
            ORIGINAL_UNCOMPRESSED_PICTURE_TILED_RESOURCE_MODE_TRMODETILEYF   = 1, //!< 4KB tiled resources
            ORIGINAL_UNCOMPRESSED_PICTURE_TILED_RESOURCE_MODE_TRMODETILEYS   = 2, //!< 64KB tiled resources
        };

        //! \brief STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
        {
            STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     Note:  This is a READ Surface.  The setting of this bit should match the
        //!     settings on how this is written out before.
        enum STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_ENABLE
        {
            STREAMOUT_DATA_DESTINATION_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before
        enum STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_MODE
        {
            STREAMOUT_DATA_DESTINATION_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            STREAMOUT_DATA_DESTINATION_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
        {
            STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODENONE        = 0, //!< No tiled resource
            STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYF      = 1, //!< 4KB tiled resources
            STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYS      = 2, //!< 64KB tiled resources
        };

        //! \brief INTRA_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum INTRA_ROW_STORE_SCRATCH_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            INTRA_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            INTRA_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            INTRA_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            INTRA_ROW_STORE_SCRATCH_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_ENABLE
        {
            INTRA_ROW_STORE_SCRATCH_BUFFER_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1"
        enum INTRA_ROW_STORE_SCRATCH_BUFFER__MEMORY_COMPRESSION_MODE
        {
            INTRA_ROW_STORE_SCRATCH_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            INTRA_ROW_STORE_SCRATCH_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if Intra Row Store is going to store inside Media
        //!     Cache or to LLC.
        enum INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0             = 0, //!< Buffer going to LLC.
            INTRA_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1             = 1, //!< Buffer going to Internal Media Storage
        };

        //! \brief INTRA_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum INTRA_ROW_STORE_SCRATCH_BUFFER__TILED_RESOURCE_MODE
        {
            INTRA_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODENONE    = 0, //!< No tiled resource
            INTRA_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF  = 1, //!< 4KB tiled resources
            INTRA_ROW_STORE_SCRATCH_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS  = 2, //!< 64KB tiled resources
        };

        //! \brief DEBLOCKING_FILTER_ROW_STORE_SCRATCH__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum DEBLOCKING_FILTER_ROW_STORE_SCRATCH__ARBITRATION_PRIORITY_CONTROL
        {
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_ENABLE
        {
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum DEBLOCKING_FILTER_ROW_STORE_SCRATCH__MEMORY_COMPRESSION_MODE
        {
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if Intra Row Store is going to store inside Media
        //!     Internal Storage or to LLC.
        enum DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0 = 0, //!< Buffer going to LLC
            DEBLOCKING_FILTER_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1 = 1, //!< Buffer going to Media Internal Storage
        };

        //! \brief DEBLOCKING_FILTER_ROW_STORE__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum DEBLOCKING_FILTER_ROW_STORE__TILED_RESOURCE_MODE
        {
            DEBLOCKING_FILTER_ROW_STORE_TILED_RESOURCE_MODE_TRMODENONE       = 0, //!< No tiled resource
            DEBLOCKING_FILTER_ROW_STORE_TILED_RESOURCE_MODE_TRMODETILEYF     = 1, //!< 4KB tiled resources
            DEBLOCKING_FILTER_ROW_STORE_TILED_RESOURCE_MODE_TRMODETILEYS     = 2, //!< 64KB tiled resources
        };

        //! \brief REFERENCE_PICTURE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum REFERENCE_PICTURE__ARBITRATION_PRIORITY_CONTROL
        {
            REFERENCE_PICTURE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY   = 0, //!< No additional details
            REFERENCE_PICTURE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            REFERENCE_PICTURE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            REFERENCE_PICTURE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY    = 3, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum REFERENCE_PICTURE__TILED_RESOURCE_MODE
        {
            REFERENCE_PICTURE_TILED_RESOURCE_MODE_TRMODENONE                 = 0, //!< No tiled resource
            REFERENCE_PICTURE_TILED_RESOURCE_MODE_TRMODETILEYF               = 1, //!< 4KB tiled resources
            REFERENCE_PICTURE_TILED_RESOURCE_MODE_TRMODETILEYS               = 2, //!< 64KB tiled resources
        };

        //! \brief MACROBLOCK_STATUS_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MACROBLOCK_STATUS_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            MACROBLOCK_STATUS_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MACROBLOCK_STATUS_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MACROBLOCK_STATUS_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MACROBLOCK_STATUS_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_ENABLE
        {
            MACROBLOCK_STATUS_BUFFER_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum MACROBLOCK_STATUS_BUFFER__MEMORY_COMPRESSION_MODE
        {
            MACROBLOCK_STATUS_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            MACROBLOCK_STATUS_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_STATUS_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MACROBLOCK_STATUS_BUFFER__TILED_RESOURCE_MODE
        {
            MACROBLOCK_STATUS_BUFFER_TILED_RESOURCE_MODE_TRMODENONE          = 0, //!< No tiled resource
            MACROBLOCK_STATUS_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF        = 1, //!< 4KB tiled resources
            MACROBLOCK_STATUS_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS        = 2, //!< 64KB tiled resources
        };

        //! \brief MACROBLOCK_ILDB_STREAMOUT_BUFFER__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum MACROBLOCK_ILDB_STREAMOUT_BUFFER__ARBITRATION_PRIORITY_CONTROL
        {
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
        {
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
        {
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief MACROBLOCK_ILDB_STREAMOUT__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum MACROBLOCK_ILDB_STREAMOUT__TILED_RESOURCE_MODE
        {
            MACROBLOCK_ILDB_STREAMOUT_TILED_RESOURCE_MODE_TRMODENONE         = 0, //!< No tiled resource
            MACROBLOCK_ILDB_STREAMOUT_TILED_RESOURCE_MODE_TRMODETILEYF       = 1, //!< 4KB tiled resources
            MACROBLOCK_ILDB_STREAMOUT_TILED_RESOURCE_MODE_TRMODETILEYS       = 2, //!< 64KB tiled resources
        };

        //! \brief SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_ENABLE
        {
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__MEMORY_COMPRESSION_MODE
        {
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER__TILED_RESOURCE_MODE
        {
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            SECOND_MACROBLOCK_ILDB_STREAMOUT_BUFFER_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        enum REFERENCE_PICTURE_0__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_0_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_0_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_0__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_0__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_0_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_0_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_1__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_1_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_1_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_1__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_1__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_1_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_1_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_2__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_2_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_2_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_2__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_2__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_2_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_2_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_3__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_3_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_3_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_3__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_3__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_3_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_3_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_4__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_4_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_4_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_4__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_4__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_4_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_4_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_5__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_5_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_5_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_5__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_5__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_5_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_5_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_6__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_6_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_6_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_6__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_6__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_6_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_6_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_7__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_7_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_7_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_7__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_7__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_7_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_7_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_8__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_8_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_8_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_8__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_8__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_8_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_8_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_9__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_9_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_9_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE  = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_9__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_9__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_9_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_9_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_10__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_10_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_10_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_10__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_10__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_10_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_10_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_11__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_11_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_11_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_11__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details.
        //!                         Note:  This bit is not used unless Memory Compression Enable is set
        //!     to "1"
        //!                         Note:  This is a READ Surface.  The setting of this bit should
        //!     match the settings on how this is written out before.
        enum REFERENCE_PICTURE_11__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_11_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_11_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_12__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_12_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_12_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_12__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_12__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_12_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_12_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_13__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_13_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_13_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_13__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_13__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_13_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_13_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_14__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_14_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_14_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_14__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_14__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_14_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_14_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        enum REFERENCE_PICTURE_15__MEMORY_COMPRESSION_ENABLE
        {
            REFERENCE_PICTURE_15_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
            REFERENCE_PICTURE_15_MEMORY_COMPRESSION_ENABLE_COMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief REFERENCE_PICTURE_15__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details. Note: This bit is not used unless Memory Compression
        //!     Enable is set to "1" Note: This is a READ Surface. The setting of this
        //!     bit should match the settings on how this is written out before.
        enum REFERENCE_PICTURE_15__MEMORY_COMPRESSION_MODE
        {
            REFERENCE_PICTURE_15_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            REFERENCE_PICTURE_15_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief SCALE_REFERENCE_SURFACE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum SCALE_REFERENCE_SURFACE__ARBITRATION_PRIORITY_CONTROL
        {
            SCALE_REFERENCE_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            SCALE_REFERENCE_SURFACE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            SCALE_REFERENCE_SURFACE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            SCALE_REFERENCE_SURFACE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief SCALED_REFERENCE_SURFACE__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details.
        enum SCALED_REFERENCE_SURFACE__MEMORY_COMPRESSION_MODE
        {
            SCALED_REFERENCE_SURFACE_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            SCALED_REFERENCE_SURFACE_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief SCALED_REFERENCE_SURFACE__TILED_RESOURCE_MODE
        //! \details
        //!     For Media Surfaces:
        //!                         This field specifies the tiled resource mode
        enum SCALED_REFERENCE_SURFACE__TILED_RESOURCE_MODE
        {
            SCALED_REFERENCE_SURFACE_TILED_RESOURCE_MODE_TRMODENONE          = 0, //!< No tiled resource
            SCALED_REFERENCE_SURFACE_TILED_RESOURCE_MODE_TRMODETILEYF        = 1, //!< No tiled resource
            SCALED_REFERENCE_SURFACE_TILED_RESOURCE_MODE_TRMODETILEYS        = 2, //!< No tiled resource
        };

        //! \brief SLICESIZE_STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum SLICESIZE_STREAMOUT_DATA_DESTINATION__ARBITRATION_PRIORITY_CONTROL
        {
            SLICESIZE_STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            SLICESIZE_STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            SLICESIZE_STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            SLICESIZE_STREAMOUT_DATA_DESTINATION_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief SLICESIZE_STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details.
        enum SLICESIZE_STREAMOUT_DATA_DESTINATION__MEMORY_COMPRESSION_MODE
        {
            SLICESIZE_STREAMOUT_DATA_DESTINATION_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            SLICESIZE_STREAMOUT_DATA_DESTINATION_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief SLICESIZE_STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
        //! \details
        //!     For Media Surfaces: This Surface is never tiled.
        enum SLICESIZE_STREAMOUT_DATA_DESTINATION__TILED_RESOURCE_MODE
        {
            SLICESIZE_STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            SLICESIZE_STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< No tiled resource
            SLICESIZE_STREAMOUT_DATA_DESTINATION_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< No tiled resource
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_PIPE_BUF_ADDR_STATE_CMD();

        static const size_t dwSize = 68;
        static const size_t byteSize = 272;
    };

    //!
    //! \brief MFX_AVC_DIRECTMODE_STATE
    //! \details
    //!     This is a picture level command and is issued once per picture. All DMV
    //!     buffers are treated as standard media surfaces, in which the lower 6
    //!     bits are used for conveying surface states.Current Pic POC number is
    //!     assumed to be available in POCList[32 and 33] of the
    //!     MFX_AVC_DIRECTMODE_STATE Command.This command is only valid in the AVC
    //!     decoding in VLD and IT modes, and AVC encoder mode. The same command
    //!     supports both Long and Short AVC Interface. The DMV buffers are
    //!     not required to be programmed for encoder mode.
    //!     
    struct MFX_AVC_DIRECTMODE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        mhw_vdbox_mfx_g12_X::GRAPHICSADDRESS63_6_CMD          DirectMvBufferBaseAddress[16];                              //!< Direct MV Buffer Base Address

        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 DirectMvBufferBaseAddressForReferenceFrameArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 DirectMvBufferBaseAddressForReferenceFrameMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_ENABLE
                uint32_t                 DirectMvBufferBaseAddressForReferenceFrameMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1067                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 DirectMvBufferBaseAddressForReferenceFrameTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__TILED_RESOURCE_MODE
                uint32_t                 Reserved1071                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;

        mhw_vdbox_mfx_g12_X::GRAPHICSADDRESS63_6_CMD          DirectMvBufferBaseAddressForWrite[1];                       //!< Direct MV Buffer Base Address for Write

        union
        {
            //!< DWORD 36
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Memory Object Control State
                uint32_t                 DirectMvBufferBaseAddressForWriteArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__ARBITRATION_PRIORITY_CONTROL
                uint32_t                 DirectMvBufferBaseAddressForWriteMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_ENABLE
                uint32_t                 DirectMvBufferBaseAddressForWriteMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1163                                     : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 DirectMvBufferBaseAddressForWriteTiledResourceMode : __CODEGEN_BITFIELD(13, 14)    ; //!< DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__TILED_RESOURCE_MODE
                uint32_t                 Reserved1167                                     : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;

        uint32_t                         PocList[34];                                                                     //!< POC List


        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_UNNAMED2                                              = 2, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_UNNAMED0                                              = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_AVCCOMMON                                   = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXSINGLEDW                                             = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__ARBITRATION_PRIORITY_CONTROL
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_ENABLE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details.
        //!                         Note:  This bit is not used unless Memory Compression Enable is set
        //!     to "1"
        //!                         Note:  This is a READ Surface.  The setting of this bit should
        //!     match the settings on how this is written out before.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__MEMORY_COMPRESSION_MODE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME__TILED_RESOURCE_MODE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_REFERENCE_FRAME_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__ARBITRATION_PRIORITY_CONTROL
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY = 0, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY = 1, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY = 2, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY = 3, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This surface is linear surface.  This bit must be set to "0" since only
        //!     TileY/TileYf/TileYs surface is allowed to be compressed
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_ENABLE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_MEMORY_COMPRESSION_ENABLE_COMPRESSIONDISABLE = 0, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a Memory Data Formats chapter -section Media Memory Compression for
        //!     more details.
        //!                         Note:  This bit is not used unless Memory Compression Enable is set
        //!     to "1"
        //!                         Note:  This is a READ Surface.  The setting of this bit should
        //!     match the settings on how this is written out before.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__MEMORY_COMPRESSION_MODE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE = 1, //!< No additional details
        };

        //! \brief DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE__TILED_RESOURCE_MODE
        {
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_TILED_RESOURCE_MODE_TRMODENONE = 0, //!< No tiled resource
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_TILED_RESOURCE_MODE_TRMODETILEYF = 1, //!< 4KB tiled resources
            DIRECT_MV_BUFFER_BASE_ADDRESS_FOR_WRITE_TILED_RESOURCE_MODE_TRMODETILEYS = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MFX_AVC_DIRECTMODE_STATE_CMD();

        static const size_t dwSize = 71;
        static const size_t byteSize = 284;
    };

};

#pragma pack()

#endif  // __MHW_VDBOX_MFX_HWCMD_G12_X_H__