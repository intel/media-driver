
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
//! \file     mhw_vdbox_vvcp_hwcmd_xe2_lpm_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of m15_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_VDBOX_VVCP_HWCMD_XE2_LPM_X_H__
#define __MHW_VDBOX_VVCP_HWCMD_XE2_LPM_X_H__

#pragma once
#pragma pack(1)

#include "mhw_hwcmd.h"
#include <cstdint>
#include <cstddef>
#include "media_class_trace.h"

namespace mhw
{
namespace vdbox
{
namespace vvcp
{
namespace xe2_lpm_base
{
namespace xe2_lpm
{
class Cmd
{

public:
    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief MEMORYADDRESSATTRIBUTES
    //! \details
    //!     This field controls the priority of arbitration used in the GAC/GAM
    //!     pipeline for this surface. It defines the attributes for VDBOX addresses
    //!     on BDW+.
    //!
    struct MEMORYADDRESSATTRIBUTES_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD(0, 0);       //!< Reserved
                uint32_t                 BaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 BaseAddressArbitrationPriorityControl            : __CODEGEN_BITFIELD( 7,  8)    ; //!< Base Address - Arbitration Priority Control
                uint32_t                 BaseAddressMemoryCompressionEnable               : __CODEGEN_BITFIELD( 9,  9)    ; //!< Base Address - Memory Compression Enable
                uint32_t                 CompressionType                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< COMPRESSION_TYPE
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 BaseAddressRowStoreScratchBufferCacheSelect      : __CODEGEN_BITFIELD(12, 12)    ; //!< BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Tilemode                                         : __CODEGEN_BITFIELD(13, 14)    ; //!< TILEMODE
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief COMPRESSION_TYPE
        //! \details
        //!     Indicates if buffer is render/media compressed.
        enum COMPRESSION_TYPE
        {
            COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE                          = 0, //!< No additional details
            COMPRESSION_TYPE_RENDERCOMPRESSIONENABLE                         = 1, //!< Only support rendered compression with unified memory
        };

        //! \brief BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if the Row Store is going to store inside Media
        //!     Cache (rowstore cache) or to LLC.
        enum BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0      = 0, //!< Buffer going to LLC.
            BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1      = 1, //!< Buffer going to Internal Media Storage.
        };

        enum TILEMODE
        {
            TILEMODE_LINEAR                                                  = 0, //!< No additional details
            TILEMODE_TILES_64K                                               = 1, //!< No additional details
            TILEMODE_TILEX                                                   = 2, //!< No additional details
            TILEMODE_TILEF                                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEMORYADDRESSATTRIBUTES_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief SPLITBASEADDRESS4KBYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 4K-byte aligned memory base
    //!     address. GraphicsAddress is a 64-bit value [63:0], but only a portion of
    //!     it is used by hardware. The upper reserved bits are ignored and MBZ.
    //!
    //!     Bits 63:48 must be zero.
    //!
    struct SPLITBASEADDRESS4KBYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD(12, 56)    ; //!< Base Address
                uint64_t                 Reserved57                                       : __CODEGEN_BITFIELD(57, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS4KBYTEALIGNED_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SPLITBASEADDRESS64BYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 64-byte aligned memory base
    //!     address.
    //!
    //!     Bits 63:48 must be zero.
    //!
    struct SPLITBASEADDRESS64BYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD( 6, 56)    ; //!< Base Address
                uint64_t                 Reserved57                                       : __CODEGEN_BITFIELD(57, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS64BYTEALIGNED_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    struct ALF_LUMA_CLIP_IDX_ENTRY
    {
        union
        {
            struct
            {
                uint32_t                 alf_luma_clip_idx0                             : __CODEGEN_BITFIELD( 0, 1)    ;
                uint32_t                 alf_luma_clip_idx1                             : __CODEGEN_BITFIELD( 2, 3)    ;
                uint32_t                 alf_luma_clip_idx2                             : __CODEGEN_BITFIELD( 4, 5)    ;
                uint32_t                 alf_luma_clip_idx3                             : __CODEGEN_BITFIELD( 6, 7)    ;
                uint32_t                 alf_luma_clip_idx4                             : __CODEGEN_BITFIELD( 8, 9)    ;
                uint32_t                 alf_luma_clip_idx5                             : __CODEGEN_BITFIELD( 10, 11)    ;
                uint32_t                 alf_luma_clip_idx6                             : __CODEGEN_BITFIELD( 12, 13)    ;
                uint32_t                 alf_luma_clip_idx7                             : __CODEGEN_BITFIELD( 14, 15)    ;
                uint32_t                 alf_luma_clip_idx8                             : __CODEGEN_BITFIELD( 16, 17)    ;
                uint32_t                 alf_luma_clip_idx9                             : __CODEGEN_BITFIELD( 18, 19)    ;
                uint32_t                 alf_luma_clip_idx10                             : __CODEGEN_BITFIELD( 20, 21)    ;
                uint32_t                 alf_luma_clip_idx11                             : __CODEGEN_BITFIELD( 22, 23)    ;
                uint32_t                 Reserved28                                      : __CODEGEN_BITFIELD( 24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        };
    };

    struct VVCP_APS_ALF_PARAMSET
    {
        union
        {
            struct
            {
                uint32_t                 AlfLumaCoeffDeltaIdx0                             : __CODEGEN_BITFIELD( 0, 4)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx1                             : __CODEGEN_BITFIELD( 5, 9)    ; //!< lmcs_min_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx2                             : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx3                             : __CODEGEN_BITFIELD(15, 19)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx4                             : __CODEGEN_BITFIELD(20, 24)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx5                             : __CODEGEN_BITFIELD(25, 29)    ; //!< Reserved
                uint32_t                 Reserved28                                        : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 AlfLumaCoeffDeltaIdx6                             : __CODEGEN_BITFIELD( 0, 4)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx7                             : __CODEGEN_BITFIELD( 5, 9)    ; //!< lmcs_min_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx8                             : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx9                             : __CODEGEN_BITFIELD(15, 19)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx10                             : __CODEGEN_BITFIELD(20, 24)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx11                             : __CODEGEN_BITFIELD(25, 29)    ; //!< Reserved
                uint32_t                 Reserved28                                        : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 AlfLumaCoeffDeltaIdx12                             : __CODEGEN_BITFIELD( 0, 4)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx13                             : __CODEGEN_BITFIELD( 5, 9)    ; //!< lmcs_min_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx14                             : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx15                             : __CODEGEN_BITFIELD(15, 19)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx16                             : __CODEGEN_BITFIELD(20, 24)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx17                             : __CODEGEN_BITFIELD(25, 29)    ; //!< Reserved
                uint32_t                 Reserved28                                        : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 AlfLumaCoeffDeltaIdx18                             : __CODEGEN_BITFIELD( 0, 4)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx19                             : __CODEGEN_BITFIELD( 5, 9)    ; //!< lmcs_min_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx20                             : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx21                             : __CODEGEN_BITFIELD(15, 19)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 AlfLumaCoeffDeltaIdx22                             : __CODEGEN_BITFIELD(20, 24)    ; //!< Reserved
                uint32_t                 AlfLumaCoeffDeltaIdx23                             : __CODEGEN_BITFIELD(25, 29)    ; //!< Reserved
                uint32_t                 Reserved28                                        : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 AlfLumaCoeffDeltaIdx24                             : __CODEGEN_BITFIELD( 0, 4)    ; //!< Reserved
                uint32_t                 Reserved28                                         : __CODEGEN_BITFIELD( 5, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        ALF_LUMA_CLIP_IDX_ENTRY AlfLumaClipIdx[25]; //DW5-29
        uint32_t DW30_Reserved;//DW30, reserved
        uint32_t DW31_Reserved;//DW31, reserved
        uint32_t AlfCoeffL[80];//DW32..111
        uint32_t AlfCoeffC[12];//DW112..123
        uint32_t AlfChromaClipIdx[3];// alf_chroma_clip_idx;//DW124..126
        uint32_t D127_Reserved;

        union
        {
            struct
            {
                uint32_t                 AlfCcCbFiltersSignalledMinus1                             : __CODEGEN_BITFIELD( 0, 1)    ; //!< Reserved
                uint32_t                 Reserved32                                         : __CODEGEN_BITFIELD( 2, 3)    ; //!< Reserved
                uint32_t                 AlfCcCbCoeffSign                                   :__CODEGEN_BITFIELD( 4, 31)    ;
            };
            uint32_t                     Value;
        } DW128;

        uint32_t AlfCcCbMappedCoeffAbs[7];// DW129..135, alf_cc_cb_mapped_coeff_abs[ 4 ][ 7 ]

        union
        {
            struct
            {
                uint32_t                 AlfCcCrFiltersSignalledMinus1                             : __CODEGEN_BITFIELD( 0, 1)    ; //!< Reserved
                uint32_t                 Reserved32                                         : __CODEGEN_BITFIELD( 2, 3)    ; //!< Reserved
                uint32_t                 AlfCcCrCoeffSign                                   :__CODEGEN_BITFIELD( 4, 31)    ;
            };
            uint32_t                     Value;
        } DW136;

        uint32_t AlfCcCrMappedCoeffAbs[7];// DW137..143, alf_cc_cr_mapped_coeff_abs[4][7]




        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_APS_ALF_PARAMSET();

        static const size_t dwSize = 144;
        static const size_t byteSize = 576;
    };


        //!
    //! \brief VVCP_APS_LMCS_PARAMSET
    //! \details
    //!     The lmcs_data() generic syntax arrays are replaced by the corresponding
    //!     derived LmcsPivot[17, InvScaleCoeff[ 16] and ChromaScaleCoeff[ 16]
    //!     arrays in the VVC spec. Driver sends only one set of LMCS Parameters to
    //!     HW in the APS LMCS Data Buffer. The set being sent corresponds to the
    //!     APS ID specified in the Slice Header.
    //!
    //!     This APS LMCS Data Buffer is required to be cacheline (CL) aligned. An
    //!     APS LMCS Parameter Settakes up 1 CL(16 DWords).
    //!
    struct VVCP_APS_LMCS_PARAMSET_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 LmcsMinBinIdx                                    : __CODEGEN_BITFIELD(16, 19)    ; //!< lmcs_min_bin_idx
                uint32_t                 Reserved20                                       : __CODEGEN_BITFIELD(20, 23)    ; //!< Reserved
                uint32_t                 LmcsDeltaMaxBinIdx                               : __CODEGEN_BITFIELD(24, 27)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 Reserved28                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint16_t                 Scalecoeff[16]                                                                     ; //!< ScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW1_8;
        union
        {
            struct
            {
                uint16_t                 Invscalecoeff[16]                                                                  ; //!< InvScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW9_16;
        union
        {
            struct
            {
                uint16_t                 Chromascalecoeff[16]                                                               ; //!< ChromaScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW17_24;
        union
        {
            struct
            {
                uint16_t                 Lmcspivot[16]                                                               ; //!< LmcsPivot[ 16 ]
            };
            uint32_t                     Value[8];
        } DW25_32;


        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_APS_LMCS_PARAMSET_CMD();

        static const size_t dwSize = 33;
        static const size_t byteSize = 132;
    };

    //!
    //! \brief VVCP_APS_SCALINGLIST_PARAMSET
    //! \details
    //!     The scaling_list_data() generic syntax arrays are replaced by the
    //!     corresponding derived ScalingMatrix arrays in the VVC spec. Driver sends
    //!     only one set of Scaling Matrices to HW in the APS ScalingList Data
    //!     Buffer. The set being sent corresponds to the APS ID specified in the
    //!     Slice Header. The set being sent contains the following 4 arrays:
    //!     1) ScalingMatrixDCRec[ 14 ];
    //!     2) ScalingMatrixRec2x2[ 2 ][ 2 ][ 2 ];
    //!     3) ScalingMatrixRec4x4[ 6 ][ 4 ][ 4 ]; and
    //!     4) ScalingMatrixRec8x8[ 20 ][ 8 ][ 8 ].
    //!     This APS ScalingList Data Buffer is required to be cacheline (CL)
    //!     aligned. The set of Scaling Matrices takes up 22 CLs (350DWords).
    //!
    struct VVCP_APS_SCALINGLIST_PARAMSET_CMD
    {
        uint32_t                                 Onescalingmatrixset[351];                                                //!< OneScalingMatrixSet

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_APS_SCALINGLIST_PARAMSET_CMD();

        static const size_t dwSize = 351;
        static const size_t byteSize = 1404;
    };

    //!
    //! \brief VVCP_DPB_ENTRY
    //! \details
    //!     This structure define one entry of the array VVCP_DPB_ENTRY[i=0 .. 14].
    //!     Each entry of VVCP_DPB_ENTRY[i=0..14] is associated with the
    //!     corresponding DPB buffer defined as Reference Picture Base Address
    //!     (RefAddr[i=..14]) in the VVCP_PIPE_BUF_ADDR_STATE Command.
    //!
    struct VVCP_DPB_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DRefscalingwinleftoffsetI                          : __CODEGEN_BITFIELD( 0, 19)    ; //!< D_refScalingWinLeftOffset[ i ]
                uint32_t                 Reserved                                           : __CODEGEN_BITFIELD(20, 31)    ; //!< reserved bits
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 DRefscalingwinrightoffsetI                         : __CODEGEN_BITFIELD( 0, 19)    ; //!< D_Refscalingwinrightoffset[ i ]
                uint32_t                 Reserved                                           : __CODEGEN_BITFIELD(20, 31)    ; //!< reserved bits
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 DRefscalingwintopoffsetI                           : __CODEGEN_BITFIELD( 0, 19)    ; //!< D_Refscalingwintopoffset[ i ]
                uint32_t                 Reserved                                           : __CODEGEN_BITFIELD(20, 31)    ; //!< reserved bits
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DRefscalingwinbottomoffsetI                        : __CODEGEN_BITFIELD( 0, 19)    ; //!< D_Refscalingwinbottomoffset[ i ]
                uint32_t                 Reserved                                           : __CODEGEN_BITFIELD(20, 31)    ; //!< reserved bits
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 DRefpicscalex0J                                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_RefPicScaleX0[ j ]
                uint32_t                 DRefpicscalex1J                                   : __CODEGEN_BITFIELD(16, 31)    ; //!< D_RefPicScaleX1[ j ]
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 DRefpicwidthI                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_refPicWidth[ j ]
                uint32_t                 DRefpicheightI                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< D_refPicHeight[ j ]
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_DPB_ENTRY_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VVCP_PIC_ALF_PARAMETER_ENTRY
    //! \details
    //!     This structure define one entry of the array
    //!     VVCP_PIC_ALF_PARAMETER_ENTRY[i=0 .. 7].
    //!     Each entry is 16-bits, and only the lower 14-bits [13:0] are defined,
    //!     [15:14] are not used and are undefined.
    //!     Each entry of the array VVCP_PIC_ALF_PARAMETER_ENTRY[i] corresponds to
    //!     one set of selected APS ALF framelevel parametersthat have moved from
    //!     APS_ALF_DATA_BUFFER to PIC_STATE instead.
    //!
    struct VVCP_PIC_ALF_PARAMETER_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint16_t                 AlfLumaFilterSignalFlag                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< alf_luma_filter_signal_flag
                uint16_t                 AlfChromaFilterSignalFlag                        : __CODEGEN_BITFIELD( 1,  1)    ; //!< alf_chroma_filter_signal_flag
                uint16_t                 AlfCcCbFilterSignalFlag                          : __CODEGEN_BITFIELD( 2,  2)    ; //!< alf_cc_cb_filter_signal_flag
                uint16_t                 AlfCcCrFilterSignalFlag                          : __CODEGEN_BITFIELD( 3,  3)    ; //!< alf_cc_cr_filter_signal_flag
                uint16_t                 AlfLumaClipFlag                                  : __CODEGEN_BITFIELD( 4,  4)    ; //!< alf_luma_clip_flag
                uint16_t                 AlfChromaClipFlag                                : __CODEGEN_BITFIELD( 5,  5)    ; //!< alf_chroma_clip_flag
                uint16_t                 Reserved6                                        : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint16_t                 AlfLumaNumFiltersSignalledMinus1                 : __CODEGEN_BITFIELD( 8, 12)    ; //!< alf_luma_num_filters_signalled_minus1
                uint16_t                 AlfChromaNumAltFiltersMinus1                     : __CODEGEN_BITFIELD(13, 15)    ; //!< alf_chroma_num_alt_filters_minus1
            };
            uint16_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_PIC_ALF_PARAMETER_ENTRY_CMD();

        static const size_t dwSize = 0;
        static const size_t byteSize = 2;
    };

    //!
    //! \brief VVCP_REF_LIST_ENTRY
    //! \details
    //!     This structure define one entry of the array VVCP_REF_LIST_ENTRY[i=0 ..
    //!     14].
    //!     Each entry of VVCP_REF_LIST_ENTRY[i] corresponds to a [listIdx][i]
    //!     element of the corresponding variable defined in the REF_IDX_STATE
    //!     Command.
    //!
    //!
    struct VVCP_REF_LIST_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 RefpiclistListidxI                               : __CODEGEN_BITFIELD( 0,  3)    ; //!< RefPicList[listIdx][i]
                uint32_t                 StRefPicFlagListidxRplsidxI                      : __CODEGEN_BITFIELD( 4,  4)    ; //!< ST_REF_PIC_FLAG_LISTIDX__RPLSIDX__I_
                uint32_t                 RprconstraintsactiveflagListidxI                 : __CODEGEN_BITFIELD( 5,  5)    ; //!< RprConstraintsActiveFlag[listIdx][ i ]
                uint32_t                 Reserved6                                        : __CODEGEN_BITFIELD( 6, 14)    ; //!< Reserved
                uint32_t                 DUnavailablerefpicListidxI                       : __CODEGEN_BITFIELD(15, 15)    ; //!< D_UnavailableRefPic[listIdx][ i ]
                uint32_t                 DDiffpicordercntListidxI                         : __CODEGEN_BITFIELD(16, 31)    ; //!< D_DiffPicOrderCnt[listIdx][ i ]
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief ST_REF_PIC_FLAG_LISTIDX__RPLSIDX__I_
        //! \details
        //!     listIdx = 0 for L0, 1 for L1, and i = [0 .. 14] is that of an entry
        //!     in the array VVCP_REF_LIST_ENTRY[i]. rplsIdx is ignored by HW and is set
        //!     by Driver to pick a single flag from thearrayst_ref_pic_flag[ listIdx ][
        //!     rplsIdx ][ i ].
        //!     st_ref_pic_flag equalto 1, specifies, the current reference frame
        //!     (i)is an STRP entry. st_ref_pic_flag equal to 0, specifies that it is an
        //!     LTRP entry.
        enum ST_REF_PIC_FLAG_LISTIDX__RPLSIDX__I_
        {
            ST_REF_PIC_FLAG_LISTIDX_RPLSIDX_I_LONGTERMREFERENCE              = 0, //!< No additional details
            ST_REF_PIC_FLAG_LISTIDX_RPLSIDX_I_SHORTTERMREFERENCE             = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_REF_LIST_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VVCP_SPS_CHROMAQPTABLE
    //! \details
    //!     This is the exact same array variable ChromaQpTable[i=0/1/2][j=-12 to
    //!     63] derived in the VVC Spec at SPS frame level.
    //!     This surface must be cacheline (CL) aligned. The size of this data
    //!     structure is 4 CLs (which is 64 DWords).
    //!
    //!
    struct VVCP_SPS_CHROMAQPTABLE_CMD
    {
        uint32_t                                 Chromaqptable[57];                                                       //!< ChromaQpTable[][]

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_SPS_CHROMAQPTABLE_CMD();

        static const size_t dwSize = 57;
        static const size_t byteSize = 228;
    };

    //!
    //! \brief VD_CONTROL_STATE_BODY
    //! \details
    //!
    //!
    struct VD_CONTROL_STATE_BODY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 PipelineInitialization                           : __CODEGEN_BITFIELD( 0,  0)    ; //!< Pipeline Initialization
                uint32_t                 VdboxPipelineArchitectureClockgateDisable        : __CODEGEN_BITFIELD( 1,  1)    ; //!< VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
                uint32_t                 Reserved2                                        : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 PipeScalableModePipeLock                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< Pipe Scalable Mode Pipe Lock
                uint32_t                 PipeScalableModePipeUnlock                       : __CODEGEN_BITFIELD( 1,  1)    ; //!< Pipe Scalable Mode Pipe Unlock
                uint32_t                 MemoryImplicitFlush                              : __CODEGEN_BITFIELD( 2,  2)    ; //!< Memory Implicit Flush
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        //! \brief VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
        //! \details
        //!     This is used to disable the architecture clockgate for AWM (in AVP
        //!     pipeline) or HWM (in HCP pipeline) if needed.
        enum VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
        {
            VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE_ENABLE             = 0, //!< No additional details
            VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE_DISABLE            = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VD_CONTROL_STATE_BODY_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VVCP_WEIGHTOFFSET_LUMA_ENTRY
    //! \details
    //!
    //!
    struct VVCP_WEIGHTOFFSET_LUMA_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DeltaLumaWeightLxI                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< delta_luma_weight_lX[i]
                uint32_t                 LumaOffsetLxI                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< luma_offset_lX[i]
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_WEIGHTOFFSET_LUMA_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VVCP_WEIGHTOFFSET_CHROMA_ENTRY
    //! \details
    //!
    //!
    struct VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DeltaChromaWeightLxIJ                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< delta_chroma_weight_lX[ i ][ j ]
                uint32_t                 DeltaChromaOffsetLxIJ                            : __CODEGEN_BITFIELD(16, 31)    ; //!< delta_chroma_offset_lX[ i ][ j ]
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VVCP_BSD_OBJECT
    //! \details
    //!     The VVCP_BSD_OBJECT command fetches the VVC bit stream for a slice
    //!     starting with the first byte in the slice. The bit stream ends with the
    //!     last non-zero bit of the frame and does not include any zero-padding at
    //!     the end of the bit stream (??? is this restriction needed). There can be
    //!     multiple slices in a VVC frame and thus this command can be issued
    //!     multiple times per frame.
    //!     The VVCP_BSD_OBJECT command must be the last command issued in the
    //!     sequence of batch commands before the VVCP starts decoding. Prior to
    //!     issuing this command, it is assumed that all configuration parameters of
    //!     VVC have been loaded including workload configuration registers and
    //!     configuration tables. When this command is issued, the VVCP is waiting
    //!     for bit stream data to be presented to the shift register.
    //!
    struct VVCP_BSD_OBJECT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 IndirectBsdDataLength                                                            ; //!< Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 IndirectDataStartAddress                                                         ; //!< Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPBSDOBJECTSTATE                     = 32, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_BSD_OBJECT_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VVCP_DPB_STATE
    //! \details
    //!     The VVCP_DPB_STATE command is used to provide global information to all
    //!     reference pictures in the DPB, regardless of Reference Picture List L0
    //!     and L1 definitions. Parameters that are associated with Reference
    //!     Picture List L0 and L1 , should be kept in the VVCP_REF_IDX_STATE
    //!     Command.
    //!     This command is associated with the corresponding DPB buffer defined
    //!     asReference Picture Base Address (RefAddr[0-14]) in the
    //!     VVCP_PIPE_BUF_ADDR_STATE Command.
    //!     This is a picture level state command and is issued in both encoding and
    //!     decoding processes.
    //!
    struct VVCP_DPB_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        VVCP_DPB_ENTRY_CMD                       Entries[15];                                                             //!< DW1..90, Entries

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPDPBSTATE                           = 4, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_DPB_STATE_CMD();

        static const size_t dwSize = 91;
        static const size_t byteSize = 364;
    };

    //!
    //! \brief VVCP_IND_OBJ_BASE_ADDR_STATE
    //! \details
    //!     The VVCP_IND_OBJ_BASE_ADDR_STATE command is used to define the indirect
    //!     object base address of the VVCcompressed bitstream in graphics memory.
    //!     This is a frame level command issued in both encoding and decoding
    //!     processes.
    //!     Although a frame is coded as separate slices, all compressed slice
    //!     bitstreams are still required to line up sequentially as one VVC
    //!     bitstream. Hence, there is only one Indirect Object Base Address for the
    //!     entire VVC codedframe. If the frame contains more than 1 slice, the BSD
    //!     Object Command will be issued multiple times, once for each slice and
    //!     with its own slice bitstream starting memory address.
    //!
    struct VVCP_IND_OBJ_BASE_ADDR_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        VvcpIndirectBitstreamObjectBaseAddress;                                  //!< DW1..2, VVCP Indirect Bitstream Object Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VvcpIndirectBitstreamObjectMemoryAddressAttributes;                      //!< DW3, VVCP Indirect Bitstream Object Memory Address Attributes

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPINDOBJBASEADDRSTATE                = 3, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_IND_OBJ_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief VVCP_MEM_DATA_ACCESS
    //! \details
    //!     This command is used to modify the control of VVCP pipe (as well as HCP
    //!     and AVP Pipes). It can be inserted anywhere within a frame. It can be
    //!     inserted multiple times within a frame as well.
    //!
    struct VVCP_MEM_DATA_ACCESS_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 DataId                                           : __CODEGEN_BITFIELD( 0,  7)    ; //!< DATA_ID
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 30)    ; //!< Reserved
                uint32_t                 AccessType                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< ACCESS_TYPE
            };
            uint32_t                     Value;
        } DW1;
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        DataAccessBaseAddress;                                                   //!< DW2..3, Data Access Base Address
        MEMORYADDRESSATTRIBUTES_CMD              DataAccessMemoryAddressAttributes;                                       //!< DW4, Data Access Memory Address Attributes
        union
        {
            struct
            {
                uint32_t                 ImmediateValue                                                                   ; //!< Immediate Value
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPMEMDATAACCESS                      = 15, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/EngineName = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVVCP                  = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief DATA_ID
        //! \details
        //!     Indicate the requested read/write data
        enum DATA_ID
        {
            DATA_ID_VVCPUNITDONE                                             = 0, //!< This register contains VVCP unit dones.This register must be write-only.
            DATA_ID_VVCPSTATUS                                               = 1, //!< This register contains VVCP unit status.This register must be write-only.
            DATA_ID_VVCPERRORSTATUS                                          = 2, //!< This register contains VVCP error status.This register must be write-only.
            DATA_ID_VVCPSLICEPERFCNT                                         = 3, //!< This register contains VVCP slice perf.This register must be write-only. 
            DATA_ID_VVCPVCWMCRC                                              = 4, //!< This register contains VVCP VCWM output CRC.This register must be write-only. 
            DATA_ID_VVCPVCMXCRC                                              = 5, //!< This register contains VVCP VCMX output CRC.This register must be write-only. 
            DATA_ID_VVCPVCEDCRC                                              = 6, //!< This register contains VVCP VCED output CRC.This register must be write-only. 
            DATA_ID_VVCPVCMCCRC                                              = 7, //!< This register contains VVCP VCMC output CRC.This register must be write-only. 
            DATA_ID_VVCPVCPRCRC                                              = 8, //!< This register contains VVCP VCPR output CRC.This register must be write-only. 
            DATA_ID_VVCPVCLFCRC                                              = 9, //!< This register contains VVCP VCLF output CRC.This register must be write-only. 
            DATA_ID_VVCPVCALFCRC                                             = 10, //!< This register contains VVCP VCALF output CRC.This register must be write-only. 
            DATA_ID_VVCPMEMORYLATENCYCOUNT1                                  = 11, //!< This register contains VVCP Motion Comp Memory Latency counter 1.This register must be write-only. 
            DATA_ID_VVCPMEMORYLATENCYCOUNT2                                  = 12, //!< This register contains VVCP Motion Comp Memory Latency counter 2.This register must be write-only. 
            DATA_ID_VVCPMOTIONCOMPMISSCOUNT                                  = 13, //!< This register contains VVCP Motion Comp Miss Counter.This register must be write-only. 
            DATA_ID_VVCPMOTIONCOMPREADCOUNT                                  = 14, //!< This register contains VVCP Motion Comp Read Counter.This register must be write-only. 
            DATA_ID_IMMEDIATE                                                = 255, //!< Use the 32-bit immediate values (provided in this instruction below) and write it to memory.This register must be write-only.
        };

        //! \brief ACCESS_TYPE
        //! \details
        //!     Indicate if the data is read or write to memory
        enum ACCESS_TYPE
        {
            ACCESS_TYPE_READ                                                 = 0, //!< No additional details
            ACCESS_TYPE_WRITE                                                = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_MEM_DATA_ACCESS_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VVCP_PIC_STATE
    //! \details
    //!     This is a frame level command and is issued only once per workload for
    //!     both VVC encoding and decoding processes.
    //!
    struct VVCP_PIC_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Lengthfield                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< LENGTHFIELD
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 SpsSubpicInfoPresentFlag                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< sps_subpic_info_present_flag
                uint32_t                 SpsIndependentSubpicsFlag                        : __CODEGEN_BITFIELD( 1,  1)    ; //!< sps_independent_subpics_flag
                uint32_t                 SpsSubpicSameSizeFlag                            : __CODEGEN_BITFIELD( 2,  2)    ; //!< sps_subpic_same_size_flag
                uint32_t                 SpsEntropyCodingSyncEnabledFlag                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< sps_entropy_coding_sync_enabled_flag
                uint32_t                 SpsQtbttDualTreeIntraFlag                        : __CODEGEN_BITFIELD( 4,  4)    ; //!< sps_qtbtt_dual_tree_intra_flag
                uint32_t                 SpsMaxLumaTransformSize64Flag                    : __CODEGEN_BITFIELD( 5,  5)    ; //!< sps_max_luma_transform_size_64_flag
                uint32_t                 SpsTransformSkipEnabledFlag                      : __CODEGEN_BITFIELD( 6,  6)    ; //!< sps_transform_skip_enabled_flag
                uint32_t                 SpsBdpcmEnabledFlag                              : __CODEGEN_BITFIELD( 7,  7)    ; //!< sps_bdpcm_enabled_flag
                uint32_t                 SpsMtsEnabledFlag                                : __CODEGEN_BITFIELD( 8,  8)    ; //!< sps_mts_enabled_flag
                uint32_t                 SpsExplicitMtsIntraEnabledFlag                   : __CODEGEN_BITFIELD( 9,  9)    ; //!< sps_explicit_mts_intra_enabled_flag
                uint32_t                 SpsExplicitMtsInterEnabledFlag                   : __CODEGEN_BITFIELD(10, 10)    ; //!< sps_explicit_mts_inter_enabled_flag
                uint32_t                 SpsLfnstEnabledFlag                              : __CODEGEN_BITFIELD(11, 11)    ; //!< sps_lfnst_enabled_flag
                uint32_t                 SpsJointCbcrEnabledFlag                          : __CODEGEN_BITFIELD(12, 12)    ; //!< sps_joint_cbcr_enabled_flag
                uint32_t                 SpsSameQpTableForChromaFlag                      : __CODEGEN_BITFIELD(13, 13)    ; //!< sps_same_qp_table_for_chroma_flag
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved
                uint32_t                 DLmcsDisabledFlag                                : __CODEGEN_BITFIELD(15, 15)    ; //!< D_lmcs_disabled_flag
                uint32_t                 DDblkDisabledFlag                                : __CODEGEN_BITFIELD(16, 16)    ; //!< D_dblk_disabled_flag
                uint32_t                 DSaoLumaDisabledFlag                             : __CODEGEN_BITFIELD(17, 17)    ; //!< D_sao_luma_disabled_flag
                uint32_t                 DSaoChromaDisabledFlag                           : __CODEGEN_BITFIELD(18, 18)    ; //!< D_sao_chroma_disabled_flag
                uint32_t                 DAlfDisabledFlag                                 : __CODEGEN_BITFIELD(19, 19)    ; //!< D_alf_disabled_flag
                uint32_t                 DAlfCbDisabledFlag                               : __CODEGEN_BITFIELD(20, 20)    ; //!< D_alf_cb_disabled_flag
                uint32_t                 DAlfCrDisabledFlag                               : __CODEGEN_BITFIELD(21, 21)    ; //!< D_alf_cr_disabled_flag
                uint32_t                 DAlfCcCbDisabledFlag                             : __CODEGEN_BITFIELD(22, 22)    ; //!< D_alf_cc_cb_disabled_flag
                uint32_t                 DAlfCcCrDisabledFlag                             : __CODEGEN_BITFIELD(23, 23)    ; //!< D_alf_cc_cr_disabled_flag
                uint32_t                 DSingleSliceFrameFlag                            : __CODEGEN_BITFIELD(24, 24)    ; //!< D_single_slice_frame_flag
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 SpsSbtmvpEnabledFlag                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< sps_sbtmvp_enabled_flag
                uint32_t                 SpsAmvrEnabledFlag                               : __CODEGEN_BITFIELD( 1,  1)    ; //!< sps_amvr_enabled_flag
                uint32_t                 SpsSmvdEnabledFlag                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< sps_smvd_enabled_flag
                uint32_t                 SpsMmvdEnabledFlag                               : __CODEGEN_BITFIELD( 3,  3)    ; //!< sps_mmvd_enabled_flag
                uint32_t                 SpsSbtEnabledFlag                                : __CODEGEN_BITFIELD( 4,  4)    ; //!< sps_sbt_enabled_flag
                uint32_t                 SpsAffineEnabledFlag                             : __CODEGEN_BITFIELD( 5,  5)    ; //!< sps_affine_enabled_flag
                uint32_t                 Sps6ParamAffineEnabledFlag                       : __CODEGEN_BITFIELD( 6,  6)    ; //!< sps_6param_affine_enabled_flag
                uint32_t                 SpsAffineAmvrEnabledFlag                         : __CODEGEN_BITFIELD( 7,  7)    ; //!< sps_affine_amvr_enabled_flag
                uint32_t                 SpsBcwEnabledFlag                                : __CODEGEN_BITFIELD( 8,  8)    ; //!< sps_bcw_enabled_flag
                uint32_t                 SpsCiipEnabledFlag                               : __CODEGEN_BITFIELD( 9,  9)    ; //!< sps_ciip_enabled_flag
                uint32_t                 SpsGpmEnabledFlag                                : __CODEGEN_BITFIELD(10, 10)    ; //!< sps_gpm_enabled_flag
                uint32_t                 SpsIspEnabledFlag                                : __CODEGEN_BITFIELD(11, 11)    ; //!< sps_isp_enabled_flag
                uint32_t                 SpsMrlEnabledFlag                                : __CODEGEN_BITFIELD(12, 12)    ; //!< sps_mrl_enabled_flag
                uint32_t                 SpsMipEnabledFlag                                : __CODEGEN_BITFIELD(13, 13)    ; //!< sps_mip_enabled_flag
                uint32_t                 SpsCclmEnabledFlag                               : __CODEGEN_BITFIELD(14, 14)    ; //!< sps_cclm_enabled_flag
                uint32_t                 SpsChromaHorizontalCollocatedFlag                : __CODEGEN_BITFIELD(15, 15)    ; //!< sps_chroma_horizontal_collocated_flag
                uint32_t                 SpsChromaVerticalCollocatedFlag                  : __CODEGEN_BITFIELD(16, 16)    ; //!< sps_chroma_vertical_collocated_flag
                uint32_t                 SpsTemporalMvpEnabledFlag                        : __CODEGEN_BITFIELD(17, 17)    ; //!< sps_temporal_mvp_enabled_flag
                uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(18, 23)    ; //!< Reserved
                uint32_t                 SpsPaletteEnabledFlag                            : __CODEGEN_BITFIELD(24, 24)    ; //!< sps_palette_enabled_flag
                uint32_t                 SpsActEnabledFlag                                : __CODEGEN_BITFIELD(25, 25)    ; //!< sps_act_enabled_flag
                uint32_t                 SpsIbcEnabledFlag                                : __CODEGEN_BITFIELD(26, 26)    ; //!< sps_ibc_enabled_flag
                uint32_t                 SpsLadfEnabledFlag                               : __CODEGEN_BITFIELD(27, 27)    ; //!< sps_ladf_enabled_flag
                uint32_t                 SpsScalingMatrixForLfnstDisabledFlag             : __CODEGEN_BITFIELD(28, 28)    ; //!< sps_scaling_matrix_for_lfnst_disabled_flag
                uint32_t                 SpsScalingMatrixForAlternativeColorSpaceDisabledFlag : __CODEGEN_BITFIELD(29, 29)    ; //!< sps_scaling_matrix_for_alternative_color_space_disabled_flag
                uint32_t                 SpsScalingMatrixDesignatedColorSpaceFlag         : __CODEGEN_BITFIELD(30, 30)    ; //!< sps_scaling_matrix_designated_color_space_flag
                uint32_t                 Reserved95                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 PpsLoopFilterAcrossTilesEnabledFlag              : __CODEGEN_BITFIELD( 0,  0)    ; //!< pps_loop_filter_across_tiles_enabled_flag
                uint32_t                 PpsRectSliceFlag                                 : __CODEGEN_BITFIELD( 1,  1)    ; //!< pps_rect_slice_flag
                uint32_t                 PpsSingleSlicePerSubpicFlag                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< pps_single_slice_per_subpic_flag
                uint32_t                 PpsLoopFilterAcrossSlicesEnabledFlag             : __CODEGEN_BITFIELD( 3,  3)    ; //!< pps_loop_filter_across_slices_enabled_flag
                uint32_t                 PpsWeightedPredFlag                              : __CODEGEN_BITFIELD( 4,  4)    ; //!< pps_weighted_pred_flag
                uint32_t                 PpsWeightedBipredFlag                            : __CODEGEN_BITFIELD( 5,  5)    ; //!< pps_weighted_bipred_flag
                uint32_t                 PpsRefWraparoundEnabledFlag                      : __CODEGEN_BITFIELD( 6,  6)    ; //!< pps_ref_wraparound_enabled_flag
                uint32_t                 PpsCuQpDeltaEnabledFlag                          : __CODEGEN_BITFIELD( 7,  7)    ; //!< pps_cu_qp_delta_enabled_flag
                uint32_t                 Reserved104                                      : __CODEGEN_BITFIELD( 8, 21)    ; //!< Reserved
                uint32_t                 Virtualboundariespresentflag                     : __CODEGEN_BITFIELD(22, 22)    ; //!< VirtualBoundariesPresentFlag
                uint32_t                 PhNonRefPicFlag                                  : __CODEGEN_BITFIELD(23, 23)    ; //!< ph_non_ref_pic_flag
                uint32_t                 PhChromaResidualScaleFlag                        : __CODEGEN_BITFIELD(24, 24)    ; //!< ph_chroma_residual_scale_flag
                uint32_t                 PhTemporalMvpEnabledFlag                         : __CODEGEN_BITFIELD(25, 25)    ; //!< ph_temporal_mvp_enabled_flag
                uint32_t                 PhMmvdFullpelOnlyFlag                            : __CODEGEN_BITFIELD(26, 26)    ; //!< ph_mmvd_fullpel_only_flag
                uint32_t                 PhMvdL1ZeroFlag                                  : __CODEGEN_BITFIELD(27, 27)    ; //!< ph_mvd_l1_zero_flag
                uint32_t                 PhBdofDisabledFlag                               : __CODEGEN_BITFIELD(28, 28)    ; //!< ph_bdof_disabled_flag
                uint32_t                 PhDmvrDisabledFlag                               : __CODEGEN_BITFIELD(29, 29)    ; //!< ph_dmvr_disabled_flag
                uint32_t                 PhProfDisabledFlag                               : __CODEGEN_BITFIELD(30, 30)    ; //!< ph_prof_disabled_flag
                uint32_t                 PhJointCbcrSignFlag                              : __CODEGEN_BITFIELD(31, 31)    ; //!< ph_joint_cbcr_sign_flag
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SpsChromaFormatIdc                               : __CODEGEN_BITFIELD( 0,  3)    ; //!< SPS_CHROMA_FORMAT_IDC
                uint32_t                 SpsLog2CtuSizeMinus5                             : __CODEGEN_BITFIELD( 4,  7)    ; //!< SPS_LOG2_CTU_SIZE_MINUS5
                uint32_t                 SpsBitdepthMinus8                                : __CODEGEN_BITFIELD( 8, 11)    ; //!< SPS_BITDEPTH_MINUS8
                uint32_t                 SpsLog2MinLumaCodingBlockSizeMinus2              : __CODEGEN_BITFIELD(12, 15)    ; //!< SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2
                uint32_t                 SpsNumSubpicsMinus1                              : __CODEGEN_BITFIELD(16, 31)    ; //!< sps_num_subpics_minus1
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 SpsLog2TransformSkipMaxSizeMinus2                : __CODEGEN_BITFIELD( 0,  3)    ; //!< sps_log2_transform_skip_max_size_minus2
                uint32_t                 SpsSixMinusMaxNumMergeCand                       : __CODEGEN_BITFIELD( 4,  7)    ; //!< sps_six_minus_max_num_merge_cand
                uint32_t                 SpsFiveMinusMaxNumSubblockMergeCand              : __CODEGEN_BITFIELD( 8, 11)    ; //!< sps_five_minus_max_num_subblock_merge_cand
                uint32_t                 DMaxNumGpmMergeCand                               : __CODEGEN_BITFIELD(12, 15)    ; //!< sps_max_num_merge_cand_minus_max_num_gpm_cand
                uint32_t                 SpsLog2ParallelMergeLevelMinus2                  : __CODEGEN_BITFIELD(16, 19)    ; //!< sps_log2_parallel_merge_level_minus2
                uint32_t                 SpsMinQpPrimeTs                                  : __CODEGEN_BITFIELD(20, 23)    ; //!< sps_min_qp_prime_ts
                uint32_t                 SpsSixMinusMaxNumIbcMergeCand                    : __CODEGEN_BITFIELD(24, 27)    ; //!< sps_six_minus_max_num_ibc_merge_cand
                uint32_t                 Reserved188                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 SpsLadfQpOffset0                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< sps_ladf_qp_offset[ 0 ]
                uint32_t                 SpsLadfQpOffset1                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< sps_ladf_qp_offset[ 1 ]
                uint32_t                 SpsLadfQpOffset2                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< sps_ladf_qp_offset[ 2 ]
                uint32_t                 SpsLadfQpOffset3                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< sps_ladf_qp_offset[ 3 ]
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 SpsLadfDeltaThresholdMinus10                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< sps_ladf_delta_threshold_minus1[ 0 ]
                uint32_t                 SpsLadfDeltaThresholdMinus11                     : __CODEGEN_BITFIELD(12, 23)    ; //!< sps_ladf_delta_threshold_minus1[ 1 ]
                uint32_t                 SpsLadfLowestIntervalQpOffset                    : __CODEGEN_BITFIELD(24, 31)    ; //!< sps_ladf_lowest_interval_qp_offset
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 SpsLadfDeltaThresholdMinus12                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< sps_ladf_delta_threshold_minus1[ 2 ]
                uint32_t                 SpsLadfDeltaThresholdMinus13                     : __CODEGEN_BITFIELD(12, 23)    ; //!< sps_ladf_delta_threshold_minus1[ 3 ]
                uint32_t                 Reserved280                                      : __CODEGEN_BITFIELD(24, 29)    ; //!< Reserved
                uint32_t                 SpsNumLadfIntervalsMinus2                        : __CODEGEN_BITFIELD(30, 31)    ; //!< sps_num_ladf_intervals_minus2
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 PpsPicWidthInLumaSamples                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< pps_pic_width_in_luma_samples
                uint32_t                 PpsPicHeightInLumaSamples                        : __CODEGEN_BITFIELD(16, 31)    ; //!< pps_pic_height_in_luma_samples
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 PpsScalingWinLeftOffset                          : __CODEGEN_BITFIELD( 0, 19)    ; //!< pps_scaling_win_left_offset
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 PpsScalingWinRightOffset                         : __CODEGEN_BITFIELD( 0, 19)    ; //!< pps_scaling_win_right_offset
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 PpsScalingWinTopOffset                           : __CODEGEN_BITFIELD( 0, 19)    ; //!< pps_scaling_win_top_offset
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 PpsScalingWinBottomOffset                        : __CODEGEN_BITFIELD( 0, 19)    ; //!< pps_scaling_win_bottom_offset
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 DNumtilerowsminus1                               : __CODEGEN_BITFIELD(0, 11)    ; //!< D_NumTileRowsMinus1
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 DNumtilecolumnsminus1                            : __CODEGEN_BITFIELD(16, 27)    ; //!< D_NumTileColumnsMinus1
                uint32_t                 Reserved428                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;

        union
        {
            struct
            {
                uint32_t                 PpsCbQpOffset                                    : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_cb_qp_offset
                uint32_t                 PpsCrQpOffset                                    : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_cr_qp_offset
                uint32_t                 PpsJointCbcrQpOffsetValue                        : __CODEGEN_BITFIELD(16, 23)    ; //!< pps_joint_cbcr_qp_offset_value
                uint32_t                 PpsChromaQpOffsetListLenMinus1                   : __CODEGEN_BITFIELD(24, 27)    ; //!< pps_chroma_qp_offset_list_len_minus1
                uint32_t                 Reserved460                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;

        union
        {
            struct
            {
                uint32_t                 PpsCbQpOffsetList0                               : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_cb_qp_offset_list[ 0 ]
                uint32_t                 PpsCbQpOffsetList1                               : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_cb_qp_offset_list[ 1 ]
                uint32_t                 PpsCbQpOffsetList2                               : __CODEGEN_BITFIELD(16, 23)    ; //!< pps_cb_qp_offset_list[ 2 ]
                uint32_t                 PpsCbQpOffsetList3                               : __CODEGEN_BITFIELD(24, 31)    ; //!< pps_cb_qp_offset_list[ 3 ]
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 PpsCbQpOffsetList4                               : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_cb_qp_offset_list[ 4 ]
                uint32_t                 PpsCbQpOffsetList5                               : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_cb_qp_offset_list[ 5 ]
                uint32_t                 PpsPicWidthMinusWraparoundOffset                 : __CODEGEN_BITFIELD(16, 31)    ; //!< pps_pic_width_minus_wraparound_offset
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 PpsCrQpOffsetList0                               : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_cr_qp_offset_list[ 0 ]
                uint32_t                 PpsCrQpOffsetList1                               : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_cr_qp_offset_list[ 1 ]
                uint32_t                 PpsCrQpOffsetList2                               : __CODEGEN_BITFIELD(16, 23)    ; //!< pps_cr_qp_offset_list[ 2 ]
                uint32_t                 PpsCrQpOffsetList3                               : __CODEGEN_BITFIELD( 24, 31)    ; //!< pps_cr_qp_offset_list[ 3 ]
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 PpsCrQpOffsetList4                               : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_cr_qp_offset_list[ 4 ]
                uint32_t                 PpsCrQpOffsetList5                               : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_cr_qp_offset_list[ 5 ]
                uint32_t                 Reserved576                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 PpsJointCbcrQpOffsetList0                        : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_joint_cbcr_qp_offset_list[ 0 ]
                uint32_t                 PpsJointCbcrQpOffsetList1                        : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_joint_cbcr_qp_offset_list[ 1 ]
                uint32_t                 PpsJointCbcrQpOffsetList2                        : __CODEGEN_BITFIELD(16, 23)    ; //!< pps_joint_cbcr_qp_offset_list[ 2 ]
                uint32_t                 PpsJointCbcrQpOffsetList3                        : __CODEGEN_BITFIELD(24, 31)    ; //!< pps_joint_cbcr_qp_offset_list[ 3 ]
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 PpsJointCbcrQpOffsetList4                        : __CODEGEN_BITFIELD(0, 7)    ; //!< pps_joint_cbcr_qp_offset_list[ 4 ]
                uint32_t                 PpsJointCbcrQpOffsetList5                        : __CODEGEN_BITFIELD(8, 15)    ; //!< pps_joint_cbcr_qp_offset_list[ 5 ]
                uint32_t                 Reserved640                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            struct
            {
                uint32_t                 Numvervirtualboundaries                          : __CODEGEN_BITFIELD(0, 3)    ; //!< NumVerVirtualBoundaries
                uint32_t                 Numhorvirtualboundaries                          : __CODEGEN_BITFIELD(4, 7)    ; //!< NumHorVirtualBoundaries
                uint32_t                 PhLog2DiffMinQtMinCbIntraSliceLuma               : __CODEGEN_BITFIELD(8, 11)    ; //!< ph_log2_diff_min_qt_min_cb_intra_slice_luma
                uint32_t                 PhMaxMttHierarchyDepthIntraSliceLuma             : __CODEGEN_BITFIELD(12, 15)    ; //!< ph_max_mtt_hierarchy_depth_intra_slice_luma
                uint32_t                 PhLog2DiffMaxBtMinQtIntraSliceLuma               : __CODEGEN_BITFIELD(16, 19)    ; //!< ph_log2_diff_max_bt_min_qt_intra_slice_luma
                uint32_t                 PhLog2DiffMaxTtMinQtIntraSliceLuma               : __CODEGEN_BITFIELD(20, 23)    ; //!< ph_log2_diff_max_tt_min_qt_intra_slice_luma
                uint32_t                 PhLog2DiffMinQtMinCbIntraSliceChroma             : __CODEGEN_BITFIELD(24, 27)    ; //!< ph_log2_diff_min_qt_min_cb_intra_slice_chroma
                uint32_t                 PhMaxMttHierarchyDepthIntraSliceChroma           : __CODEGEN_BITFIELD(28, 31)    ; //!< ph_max_mtt_hierarchy_depth_intra_slice_chroma
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            struct
            {
                uint32_t                 DVirtualboundaryposxminus10                      : __CODEGEN_BITFIELD(0, 12)    ; //!< D_VirtualBoundaryPosXMinus1[ 0 ] 
                uint32_t                 Reserved701                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 DVirtualboundaryposyminus10                      : __CODEGEN_BITFIELD(16, 28)    ; //!< D_VirtualBoundaryPosYMinus1[ 0 ]
                uint32_t                 Reserved717                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            struct
            {
                uint32_t                 DVirtualboundaryposxminus11                      : __CODEGEN_BITFIELD(0, 12)    ; //!< D_VirtualBoundaryPosXMinus1[ 1 ]
                uint32_t                 Reserved733                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 DVirtualboundaryposyminus11                      : __CODEGEN_BITFIELD( 16, 28)    ; //!< D_VirtualBoundaryPosYMinus1[ 1 ]
                uint32_t                 Reserved749                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            struct
            {
                uint32_t                 DVirtualboundaryposxminus12                      : __CODEGEN_BITFIELD(0, 12)    ; //!< D_VirtualBoundaryPosXMinus1[ 2 ]
                uint32_t                 Reserved765                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 DVirtualboundaryposyminus12                      : __CODEGEN_BITFIELD( 16, 28)    ; //!< D_VirtualBoundaryPosYMinus1[ 2 ]
                uint32_t                 Reserved781                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            struct
            {
                uint32_t                 PhLog2DiffMaxBtMinQtIntraSliceChroma             : __CODEGEN_BITFIELD(0, 3)    ; //!< ph_log2_diff_max_bt_min_qt_intra_slice_chroma
                uint32_t                 PhLog2DiffMaxTtMinQtIntraSliceChroma             : __CODEGEN_BITFIELD(4, 7)    ; //!< ph_log2_diff_max_tt_min_qt_intra_slice_chroma
                uint32_t                 PhCuQpDeltaSubdivIntraSlice                      : __CODEGEN_BITFIELD(8, 15)    ; //!< ph_cu_qp_delta_subdiv_intra_slice
                uint32_t                 PhCuChromaQpOffsetSubdivIntraSlice               : __CODEGEN_BITFIELD( 16,  23)    ; //!< ph_cu_chroma_qp_offset_subdiv_intra_slice
                uint32_t                 PhLog2DiffMinQtMinCbInterSlice                   : __CODEGEN_BITFIELD( 24, 27)    ; //!< ph_log2_diff_min_qt_min_cb_inter_slice
                uint32_t                 PhMaxMttHierarchyDepthInterSlice                 : __CODEGEN_BITFIELD(28, 31)    ; //!< ph_max_mtt_hierarchy_depth_inter_slice
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            struct
            {
                uint32_t                 PhLog2DiffMaxBtMinQtInterSlice                   : __CODEGEN_BITFIELD(0, 3)    ; //!< ph_log2_diff_max_bt_min_qt_inter_slice
                uint32_t                 PhLog2DiffMaxTtMinQtInterSlice                   : __CODEGEN_BITFIELD(4, 7)    ; //!< ph_log2_diff_max_tt_min_qt_inter_slice
                uint32_t                 PhCuQpDeltaSubdivInterSlice                      : __CODEGEN_BITFIELD(8, 15)    ; //!< ph_cu_qp_delta_subdiv_inter_slice
                uint32_t                 PhCuChromaQpOffsetSubdivInterSlice               : __CODEGEN_BITFIELD( 16,  23)    ; //!< ph_cu_chroma_qp_offset_subdiv_inter_slice
                uint32_t                 Reserved840                                      : __CODEGEN_BITFIELD( 24, 31)    ; //!< Reserved
            };

            uint32_t                     Value;
        } DW27;

        union
        {
            struct
            {
                uint32_t                 DActiveapsid                                     : __CODEGEN_BITFIELD(0, 7)    ; //!< D_ActiveAPSID
                uint32_t                 Reserved856                                      : __CODEGEN_BITFIELD(8, 31)    ; //!< Reserved
            };

            uint32_t                     Value;
        } DW28;

        VVCP_PIC_ALF_PARAMETER_ENTRY_CMD         DPicApsalfparamsets[8];

        //! \name Local enumerations

        //! \brief LENGTHFIELD
        //! \details
        //!     (Excludes Dwords 0, 1).
        enum LENGTHFIELD
        {
            LENGTHFIELD_DWORDLENGTH_LENGTHBIAS                               = 29, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPPICSTATE                           = 16, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC                      = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SPS_CHROMA_FORMAT_IDC
        //! \details
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%"><span
        //!     style="font-family:Calibri,sans-serif">specifies the chroma sampling
        //!     relative to the luma sampling.
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%">0
        //!     - Monochrome (not supported yet)
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%">1
        //!     - 4:2:0
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%">2
        //!     - 4:2:2 (not supported yet)
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%">3
        //!     - 4:4:4 (not supported yet)
        enum SPS_CHROMA_FORMAT_IDC
        {
            SPS_CHROMA_FORMAT_IDC_420                                        = 1, //!< No additional details
        };

        //! \brief SPS_LOG2_CTU_SIZE_MINUS5
        //! \details
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%"><span
        //!     style="font-family:Calibri,sans-serif">(sps_log2_ctu_size_minus5 + 5)
        //!     specifies the luma coding tree block size of each CTU. The value of
        //!     sps_log2_ctu_size_minus5 shall be in the range of 0 to 2, inclusive. The
        //!     value 3 for sps_log2_ctu_size_minus5 is reserved for future
        //!     use.
        enum SPS_LOG2_CTU_SIZE_MINUS5
        {
            SPS_LOG2_CTU_SIZE_MINUS5_CTUSIZE32                               = 0, //!< Indicates CTU Size is 32x32.
            SPS_LOG2_CTU_SIZE_MINUS5_CTUSIZE64                               = 1, //!< Indicates CTU Size is 64x64.
            SPS_LOG2_CTU_SIZE_MINUS5_CTUSIZE128                              = 2, //!< Indicates CTU Size is 128x128.
        };

        //! \brief SPS_BITDEPTH_MINUS8
        //! \details
        //!     <span
        //!     style="font-family:&quot;Calibri&quot;,sans-serif">(sps_bitdepth_minus8
        //!     + 8) specifies the bit depth of the samples of the luma and chroma
        //!     arrays, BitDepth, and the value of the luma and chroma quantization
        //!     parameter range offset, QpBdOffset.
        enum SPS_BITDEPTH_MINUS8
        {
            SPS_BITDEPTH_MINUS8_8_BIT                                        = 0, //!< No additional details
            SPS_BITDEPTH_MINUS8_10_BIT                                       = 2, //!< No additional details
        };

        //! \brief SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2
        //! \details
        //!     style="margin-bottom:11px"><span
        //!     style="line-height:107%"><span
        //!     style="font-family:Calibri,sans-serif">(sps_log2_min_luma_coding_block_size_minus2
        //!     + 2) specifies the minimum luma coding block size. The value range of
        //!     sps_log2_min_luma_coding_block_size_minus2 shall be in the range of 0 to
        //!     Min(4,sps_log2_ctu_size_minus5+3), inclusive.
        //!     style="margin-bottom:11px">0 - 4x4 pixel block size;
        //!     style="margin-bottom:11px">1 - 8x8pixel block size;
        //!     style="margin-bottom:11px">2 - 16x16pixel block size;
        //!     style="margin-bottom:11px">3 - 32x32pixel block size;
        //!     style="margin-bottom:11px">4 - 64x64pixel block size.
        enum SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2
        {
            SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_4X4PIXELBLOCKSIZE     = 0, //!< No additional details
            SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_8X8PIXELBLOCKSIZE     = 1, //!< No additional details
            SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_16X16PIXELBLOCKSIZE   = 2, //!< No additional details
            SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_32X32PIXELBLOCKSIZE   = 3, //!< No additional details
            SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_64X64PIXELBLOCKSIZE   = 4, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_PIC_STATE_CMD();

        static const size_t dwSize = 33;
        static const size_t byteSize = 132;
    };

    struct VVCP_REFERENCE_PICTURE_ENTRY_CMD
    {
        SPLITBASEADDRESS64BYTEALIGNED_CMD        ReferencePictureBaseAddress0Refaddr;
        MEMORYADDRESSATTRIBUTES_CMD              ReferencePictureMemoryAddressAttributes;                                //!< DW6, Reference Picture 0 Memory Address Attributes

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_REFERENCE_PICTURE_ENTRY_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VVCP_PIPE_BUF_ADDR_STATE
    //! \details
    //!     This state command provides the memory base addresses for the row store
    //!     buffer and reconstructed pictureoutput buffers required by the VVCP.
    //!     This is a picture level state command and is shared by both encoding and
    //!     decoding processes.
    //!     All pixel surface addresses must be 4K byte aligned. There is a max of 8
    //!     Reference Picture BufferAddresses, and all share the same third address
    //!     DW in specifying 57-bit address.
    //!
    struct VVCP_PIPE_BUF_ADDR_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        DecodedPictureBaseAddress;                                               //!< DW1..2, Decoded Picture Base Address
        MEMORYADDRESSATTRIBUTES_CMD              DecodedPictureMemoryAddressAttributes;                                   //!< DW3, Decoded Picture Memory Address Attributes

        VVCP_REFERENCE_PICTURE_ENTRY_CMD         ReferencePicture[15];                                                    //!< DW4..48, Reference Pictures [0..14] Base Address and Memory Address Attributes
        VVCP_REFERENCE_PICTURE_ENTRY_CMD         CollocatedMotionVectorTemporalBuffer[15];                                //!< DW49..93, Collocated Motion Vector Temporal Buffer [0..14] Base Address and Memory Address Attributes

        SPLITBASEADDRESS64BYTEALIGNED_CMD        CurrentMotionVectorTemporalBufferBaseAddress;                            //!< DW94..95, Current Motion Vector Temporal Buffer Base Address 
        MEMORYADDRESSATTRIBUTES_CMD              CurrentMotionVectorTemporalBufferMemoryAddressAttributes;                //!< DW96, Current Motion Vector Temporal Buffer Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        ApsScalinglistDataBufferBaseAddress;                                     //!< DW97..98, APS ScalingList Data Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              ApsScalinglistDataBufferMemoryAddressAttributes;                         //!< DW99, APS ScalingList Data Buffer Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        ApsAlfDataBufferBaseAddress;                                             //!< DW100..101, APS ALF Data Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              ApsAlfDataBufferMemoryAddressAttributes;                                 //!< DW102, APS ALF Data Buffer Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        SpsChromaqpTableBufferBaseAddress;                                       //!< DW103..104, SPS ChromaQP Table Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              SpsChromaqpTableBufferMemoryAddressAttributes;                           //!< DW105, SPS ChromaQP Table Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcedLineBufferBaseAddress;                                               //!< DW106..107, VCED Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcedLineBufferMemoryAddressAttributes;                                   //!< DW108, VCED Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcmvLineBufferBaseAddress;                                               //!< DW109..110, VCMV Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcmvLineBufferMemoryAddressAttributes;                                   //!< DW111, VCMV Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcprLineBufferBaseAddress;                                               //!< DW112..113, VCPR Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcprLineBufferMemoryAddressAttributes;                                   //!< DW114, VCPR Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfYLineBufferBaseAddress;                                              //!< DW115..116, VCLF Y Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfYLineBufferMemoryAddressAttributes;                                  //!< DW117, VCLF Y Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfYTileRowBufferBaseAddress;                                           //!< DW118..119, VCLF Y Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfYTileRowBufferMemoryAddressAttributes;                               //!< DW120, VCLF Y Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfYTileColumnBufferBaseAddress;                                        //!< DW121..122, VCLF Y Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfYTileColumnBufferMemoryAddressAttributes;                            //!< DW123, VCLF Y Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfULineBufferBaseAddress;                                              //!< DW124..125, VCLF U Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfULineBufferMemoryAddressAttributes;                                  //!< DW126, VCLF U Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfUTileRowBufferBaseAddress;                                           //!< DW127..128, VCLF U Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfUTileRowBufferMemoryAddressAttributes;                               //!< DW129, VCLF U Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfUTileColumnBufferBaseAddress;                                        //!< DW130..131, VCLF U Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfUTileColumnBufferMemoryAddressAttributes;                            //!< DW132, VCLF U Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfVLineBufferBaseAddress;                                              //!< DW133..134, VCLF V Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfVLineBufferMemoryAddressAttributes;                                  //!< DW135, VCLF V Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfVTileRowBufferBaseAddress;                                           //!< DW136..137, VCLF V Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfVTileRowBufferMemoryAddressAttributes;                               //!< DW138, VCLF V Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VclfVTileColumnBufferBaseAddress;                                        //!< DW139..140, VCLF V Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VclfVTileColumnBufferMemoryAddressAttributes;                            //!< DW141, VCLF V Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoYLineBufferBaseAddress;                                             //!< DW142..143, VCSAO Y Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoYLineBufferMemoryAddressAttributes;                                 //!< DW144, VCSAO Y Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoYTileRowBufferBaseAddress;                                          //!< DW145..146, VCSAO Y Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoYTileRowBufferMemoryAddressAttributes;                              //!< DW147, VCSAO Y Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoYTileColumnBufferBaseAddress;                                       //!< DW148..149, VCSAO Y Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoYTileColumnBufferMemoryAddressAttributes;                           //!< DW150, VCSAO Y Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoULineBufferBaseAddress;                                             //!< DW151..152, VCSAO U Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoULineBufferMemoryAddressAttributes;                                 //!< DW153, VCSAO U Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoUTileRowBufferBaseAddress;                                          //!< DW154..155, VCSAO U Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoUTileRowBufferMemoryAddressAttributes;                              //!< DW156, VCSAO U Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoUTileColumnBufferBaseAddress;                                       //!< DW157..158, VCSAO U Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoUTileColumnBufferMemoryAddressAttributes;                           //!< DW159, VCSAO U Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoVLineBufferBaseAddress;                                             //!< DW160..161, VCSAO V Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoVLineBufferMemoryAddressAttributes;                                 //!< DW162, VCSAO V Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoVTileRowBufferBaseAddress;                                          //!< DW163..164, VCSAO V Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoVTileRowBufferMemoryAddressAttributes;                              //!< DW165, VCSAO V Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcsaoVTileColumnBufferBaseAddress;                                       //!< DW166..167, VCSAO V Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcsaoVTileColumnBufferMemoryAddressAttributes;                           //!< DW168, VCSAO V Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcalfLineBufferBaseAddress;                                              //!< DW169..170, VCALF Line Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcalfLineBufferMemoryAddressAttributes;                                  //!< DW171, VCALF Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcalfTileRowBufferBaseAddress;                                           //!< DW172..173, VCALF Tile Row Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcalfTileRowBufferMemoryAddressAttributes;                               //!< DW174, VCALF Tile Row Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcalfYTileColumnBufferBaseAddress;                                       //!< DW175..176, VCALF Y Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcalfYTileColumnBufferMemoryAddressAttributes;                           //!< DW177, VCALF Y Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcalfUTileColumnBufferBaseAddress;                                       //!< DW178..179, VCALF U Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcalfUTileColumnBufferMemoryAddressAttributes;                           //!< DW180, VCALF U Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        VcalfVTileColumnBufferBaseAddress;                                       //!< DW181..182, VCALF V Tile Column Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              VcalfVTileColumnBufferMemoryAddressAttributes;                           //!< DW183, VCALF V Tile Column Buffer Memory Address Attributes

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPPIPEBUFADDRSTATE                   = 2, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_PIPE_BUF_ADDR_STATE_CMD();

        static const size_t dwSize = 184;
        static const size_t byteSize = 736;
    };

    //!
    //! \brief VVCP_PIPE_MODE_SELECT
    //! \details
    //!     The workload for the VVCP is based upon a single slice decode. There is
    //!     no states saved between slice and frame decoding/encoding in the VVCP.
    //!     Once the bit stream DMA is configured with the VVCP_BSD_OBJECT command,
    //!     and the bitstream is presented to the VVCP Pipeline, the slice decode
    //!     will begin.
    //!     The VVCP_PIPE_MODE_SELECT command is responsible for general pipeline
    //!     level configuration that would normally be set once for a single stream
    //!     encode or decode and would not be modified on a frame and slice workload
    //!     basis.
    //!     This is a picture level state command and is shared by both encoding and
    //!     decoding processes.
    //!
    struct VVCP_PIPE_MODE_SELECT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 CodecSelect                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< CODEC_SELECT
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1,  2)    ; //!< Reserved
                uint32_t                 PicStatusErrorReportEnable                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< PIC_STATUSERROR_REPORT_ENABLE
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 CodecStandardSelect                              : __CODEGEN_BITFIELD( 5,  7)    ; //!< CODEC_STANDARD_SELECT
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 12)    ; //!< Reserved
                uint32_t                 MultiEngineMode                                  : __CODEGEN_BITFIELD(13, 14)    ; //!< MULTI_ENGINE_MODE
                uint32_t                 PipeWorkingMode                                  : __CODEGEN_BITFIELD(15, 16)    ; //!< PIPE_WORKING_MODE
                uint32_t                 Reserved49                                       : __CODEGEN_BITFIELD(17, 22)    ; //!< Reserved
                uint32_t                 MotionCompMemoryTrackerCounterEnable             : __CODEGEN_BITFIELD(23, 23)    ; //!< Motion Comp Memory Tracker Counter Enable
                uint32_t                 Reserved51                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 PicStatusErrorReportId                                                           ; //!< Pic Status/Error Report ID
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved97                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 Reserved98                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPPIPEMODESELECT                     = 0, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum CODEC_SELECT
        {
            CODEC_SELECT_DECODE                                              = 0, //!< No additional details
        };

        enum PIC_STATUSERROR_REPORT_ENABLE
        {
            PIC_STATUSERROR_REPORT_ENABLE_DISABLE                            = 0, //!< Disable status/error reporting
            PIC_STATUSERROR_REPORT_ENABLE_ENABLE                             = 1, //!< Status/Error reporting is written out once per picture. The Pic Status/Error Report ID in DWord3 along with the status/error status bits are packed into one cache line and written to the Status/Error Buffer address in the VVCP_PIPE_BUF_ADDR_STATE command. Must be zero for encoder mode.
        };

        enum CODEC_STANDARD_SELECT
        {
            CODEC_STANDARD_SELECT_VVC                                        = 3, //!< No additional details
        };

        //! \brief MULTI_ENGINE_MODE
        //! \details
        //!     This indicates the current pipe is in single pipe mode or if in pipe
        //!     scalable mode is in left/right/middle pipe in multi-engine mode.
        enum MULTI_ENGINE_MODE
        {
            MULTI_ENGINE_MODE_SINGLEENGINEMODE                               = 0, //!< This is for single engine mode
        };

        //! \brief PIPE_WORKING_MODE
        //! \details
        //!     This programs the working mode for VVCP pipe.
        enum PIPE_WORKING_MODE
        {
            PIPE_WORKING_MODE_LEGACYDECODERENCODERMODE_SINGLEPIPE            = 0, //!< This is for single pipe mode without pipe scalability. It is used by both decoder and encoder.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VVCP_REF_IDX_STATE
    //! \details
    //!     VVCP allows max 15reference idx entries in each of the L0 and L1
    //!     Reference Picture list for a progressive picture. Each field picture is
    //!     handled as if it were a progressive picture.
    //!     For P-Slice, this command is issued only once, representing L0 list. For
    //!     B-Slice, this command can be issued up to two times, one for L0 list and
    //!     one for L1 list.
    //!     This is a slice level command used in both encoding and decoding
    //!     processes.
    //!
    struct VVCP_REF_IDX_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Listidx                                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< LISTIDX
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1,  7)    ; //!< Reserved
                uint32_t                 Refidxsymlx                                      : __CODEGEN_BITFIELD( 8, 11)    ; //!< RefIdxSymLX
                uint32_t                 Reserved44                                       : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        VVCP_REF_LIST_ENTRY_CMD                  Entries[15];                                                             //!< DW2..16, Entries

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPREFIDXSTATE                        = 18, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief LISTIDX
        //! \details
        //!     There are max 2 possible reference picture list L0 and L1, so listIdx
        //!     can be 0 or 1 only.
        //!     listIdx is set to 0, when this Ref_IDX_STATE command is issued for
        //!     L0.
        //!     listIdx is set to 1, when this Ref_IDX_STATE command is issued for
        //!     L1.
        enum LISTIDX
        {
            LISTIDX_REFERENCEPICTURELIST0                                    = 0, //!< No additional details
            LISTIDX_REFERENCEPICTURELIST1                                    = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_REF_IDX_STATE_CMD();

        static const size_t dwSize = 17;
        static const size_t byteSize = 68;
    };

    //!
    //! \brief VVCP_SLICE_STATE
    //! \details
    //!
    //!
    struct VVCP_SLICE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Lengthfield                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< LENGTHFIELD
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 ShAlfEnabledFlag                                 : __CODEGEN_BITFIELD( 0,  0)    ; //!< sh_alf_enabled_flag
                uint32_t                 ShAlfCbEnabledFlag                               : __CODEGEN_BITFIELD( 1,  1)    ; //!< sh_alf_cb_enabled_flag
                uint32_t                 ShAlfCrEnabledFlag                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< sh_alf_cr_enabled_flag
                uint32_t                 ShAlfCcCbEnabledFlag                             : __CODEGEN_BITFIELD( 3,  3)    ; //!< sh_alf_cc_cb_enabled_flag
                uint32_t                 ShAlfCcCrEnabledFlag                             : __CODEGEN_BITFIELD( 4,  4)    ; //!< sh_alf_cc_cr_enabled_flag
                uint32_t                 ShLmcsUsedFlag                                   : __CODEGEN_BITFIELD( 5,  5)    ; //!< sh_lmcs_used_flag
                uint32_t                 ShExplicitScalingListUsedFlag                    : __CODEGEN_BITFIELD( 6,  6)    ; //!< sh_explicit_scaling_list_used_flag
                uint32_t                 ShCabacInitFlag                                  : __CODEGEN_BITFIELD( 7,  7)    ; //!< sh_cabac_init_flag
                uint32_t                 ShCollocatedFromL0Flag                           : __CODEGEN_BITFIELD( 8,  8)    ; //!< sh_collocated_from_l0_flag
                uint32_t                 ShCuChromaQpOffsetEnabledFlag                    : __CODEGEN_BITFIELD( 9,  9)    ; //!< sh_cu_chroma_qp_offset_enabled_flag
                uint32_t                 ShSaoLumaUsedFlag                                : __CODEGEN_BITFIELD(10, 10)    ; //!< sh_sao_luma_used_flag
                uint32_t                 ShSaoChromaUsedFlag                              : __CODEGEN_BITFIELD(11, 11)    ; //!< sh_sao_chroma_used_flag
                uint32_t                 ShDeblockingFilterDisabledFlag                   : __CODEGEN_BITFIELD(12, 12)    ; //!< sh_deblocking_filter_disabled_flag
                uint32_t                 ShDepQuantUsedFlag                               : __CODEGEN_BITFIELD(13, 13)    ; //!< sh_dep_quant_used_flag
                uint32_t                 ShSignDataHidingUsedFlag                         : __CODEGEN_BITFIELD(14, 14)    ; //!< sh_sign_data_hiding_used_flag
                uint32_t                 ShTsResidualCodingDisabledFlag                   : __CODEGEN_BITFIELD(15, 15)    ; //!< sh_ts_residual_coding_disabled_flag
                uint32_t                 Nobackwardpredflag                               : __CODEGEN_BITFIELD(16, 16)    ; //!< NoBackwardPredFlag
                uint32_t                 PVvcpDebugEnable                                 : __CODEGEN_BITFIELD(17, 17)    ; //!< P_vvcp_debug_enable
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(18, 21)    ; //!< Reserved
                uint32_t                 DIsRightMostSliceOfSubpicFlag                    : __CODEGEN_BITFIELD(22, 22)    ; //!< D_IsRightMostSliceOfSubpic_flag
                uint32_t                 DIsLeftMostSliceOfSubpicFlag                     : __CODEGEN_BITFIELD(23, 23)    ; //!< D_IsLeftMostSliceOfSubpic_flag
                uint32_t                 DIsBottomMostSliceOfSubpicFlag                   : __CODEGEN_BITFIELD(24, 24)    ; //!< D_IsBottomMostSliceOfSubpic_flag
                uint32_t                 DIsTopMostSliceOfSubpicFlag                      : __CODEGEN_BITFIELD(25, 25)    ; //!< D_IsTopMostSliceOfSubpic_flag
                uint32_t                 DMultipleSlicesInTileFlag                        : __CODEGEN_BITFIELD(26, 26)    ; //!< D_multiple_slices_in_tile flag
                uint32_t                 DIsbottommostsliceoftileFlag                     : __CODEGEN_BITFIELD(27, 27)    ; //!< D_IsBottomMostSliceOfTile_flag
                uint32_t                 DIstopmostsliceoftileFlag                        : __CODEGEN_BITFIELD(28, 28)    ; //!< D_IsTopMostSliceOfTile_flag
                uint32_t                 DSubpicTreatedAsPicFlag                          : __CODEGEN_BITFIELD(29, 29)    ; //!< D_subpic_treated_as_pic_flag
                uint32_t                 DLoopFilterAcrossSubpicEnabledFlag               : __CODEGEN_BITFIELD(30, 30)    ; //!< D_loop_filter_across_subpic_enabled_flag
                uint32_t                 DLastsliceofpicFlag                              : __CODEGEN_BITFIELD(31, 31)    ; //!< D_LastSliceOfPic_flag
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Numctusincurrslice                                                               ; //!< NumCtusInCurrSlice
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 ShNumTilesInSliceMinus1                          : __CODEGEN_BITFIELD( 0, 15)    ; //!< sh_num_tiles_in_slice_minus1
                uint32_t                 ShSliceType                                      : __CODEGEN_BITFIELD(16, 19)    ; //!< SH_SLICE_TYPE
                uint32_t                 ShNumAlfApsIdsLuma                               : __CODEGEN_BITFIELD(20, 23)    ; //!< sh_num_alf_aps_ids_luma
                uint32_t                 AlfChromaNumAltFiltersMinus1                     : __CODEGEN_BITFIELD(24, 26)    ; //!< alf_chroma_num_alt_filters_minus1
                uint32_t                 Reserved123                                      : __CODEGEN_BITFIELD(27, 27)    ; //!< Reserved
                uint32_t                 AlfCcCbFiltersSignalledMinus1                    : __CODEGEN_BITFIELD(28, 29)    ; //!< alf_cc_cb_filters_signalled_minus1
                uint32_t                 AlfCcCrFiltersSignalledMinus1                    : __CODEGEN_BITFIELD(30, 31)    ; //!< alf_cc_cr_filters_signalled_minus1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 ShAlfApsIdLuma0                                  : __CODEGEN_BITFIELD( 0,  3)    ; //!< sh_alf_aps_id_luma[ 0 ]
                uint32_t                 ShAlfApsIdLuma1                                  : __CODEGEN_BITFIELD( 4,  7)    ; //!< sh_alf_aps_id_luma[ 1 ]
                uint32_t                 ShAlfApsIdLuma2                                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< sh_alf_aps_id_luma[ 2 ]
                uint32_t                 ShAlfApsIdLuma3                                  : __CODEGEN_BITFIELD(12, 15)    ; //!< sh_alf_aps_id_luma[ 3 ]
                uint32_t                 ShAlfApsIdLuma4                                  : __CODEGEN_BITFIELD(16, 19)    ; //!< sh_alf_aps_id_luma[ 4 ]
                uint32_t                 ShAlfApsIdLuma5                                  : __CODEGEN_BITFIELD(20, 23)    ; //!< sh_alf_aps_id_luma[ 5 ]
                uint32_t                 ShAlfApsIdLuma6                                  : __CODEGEN_BITFIELD(24, 27)    ; //!< sh_alf_aps_id_luma[ 6 ]
                uint32_t                 Reserved156                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 ShAlfApsIdChroma                                 : __CODEGEN_BITFIELD( 0,  3)    ; //!< sh_alf_aps_id_chroma
                uint32_t                 ShAlfCcCbApsId                                   : __CODEGEN_BITFIELD( 4,  7)    ; //!< sh_alf_cc_cb_aps_id
                uint32_t                 ShAlfCcCrApsId                                   : __CODEGEN_BITFIELD( 8, 11)    ; //!< sh_alf_cc_cr_aps_id
                uint32_t                 Numrefidxactive0                                 : __CODEGEN_BITFIELD(12, 15)    ; //!< NumRefIdxActive[ 0 ]
                uint32_t                 Numrefidxactive1                                 : __CODEGEN_BITFIELD(16, 19)    ; //!< NumRefIdxActive[ 1 ]
                uint32_t                 Reserved180                                      : __CODEGEN_BITFIELD(20, 23)    ; //!< Reserved
                uint32_t                 ShCollocatedRefIdx                               : __CODEGEN_BITFIELD(24, 31)    ; //!< sh_collocated_ref_idx
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 Sliceqpy                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< SliceQpY
                uint32_t                 ShCbQpOffset                                     : __CODEGEN_BITFIELD( 8, 15)    ; //!< sh_cb_qp_offset
                uint32_t                 ShCrQpOffset                                     : __CODEGEN_BITFIELD(16, 23)    ; //!< sh_cr_qp_offset
                uint32_t                 ShJointCbcrQpOffset                              : __CODEGEN_BITFIELD(24, 31)    ; //!< sh_joint_cbcr_qp_offset
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 ShLumaBetaOffsetDiv2                             : __CODEGEN_BITFIELD( 0,  7)    ; //!< sh_luma_beta_offset_div2
                uint32_t                 ShLumaTcOffsetDiv2                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< sh_luma_tc_offset_div2
                uint32_t                 ShCbBetaOffsetDiv2                               : __CODEGEN_BITFIELD(16, 23)    ; //!< sh_cb_beta_offset_div2
                uint32_t                 ShCbTcOffsetDiv2                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< sh_cb_tc_offset_div2
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 ShCrBetaOffsetDiv2                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< sh_cr_beta_offset_div2
                uint32_t                 ShCrTcOffsetDiv2                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< sh_cr_tc_offset_div2
                uint32_t                 Reserved272                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 Reserved288                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 LmcsMinBinIdx                                    : __CODEGEN_BITFIELD(16, 19)    ; //!< lmcs_min_bin_idx
                uint32_t                 Reserved308                                      : __CODEGEN_BITFIELD(20, 23)    ; //!< Reserved
                uint32_t                 LmcsDeltaMaxBinIdx                               : __CODEGEN_BITFIELD(24, 27)    ; //!< lmcs_delta_max_bin_idx
                uint32_t                 Reserved316                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint16_t                 Scalecoeff[16]                                                                     ; //!< ScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW10_17;
        union
        {
            struct
            {
                uint16_t                 Invscalecoeff[16]                                                                  ; //!< InvScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW18_25;
        union
        {
            struct
            {
                uint16_t                 Chromascalecoeff[16]                                                               ; //!< ChromaScaleCoeff[ 16 ]
            };
            uint32_t                     Value[8];
        } DW26_33;

        uint32_t                                 Lmcspivot161[8];                                                         //!< LmcsPivot[ 16:1 ]

        union
        {
            struct
            {
                uint32_t                 DSubpicCtuTopLeftX                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_subpic_ctu_top_left_x
                uint32_t                 DSubpicCtuTopLeftY                               : __CODEGEN_BITFIELD(16, 31)    ; //!< D_subpic_ctu_top_left_y
            };
            uint32_t                     Value;
        } DW42;
        union
        {
            struct
            {
                uint32_t                 DSubpicWidthMinus1                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_subpic_width_minus1
                uint32_t                 DSubpicHeightMinus1                              : __CODEGEN_BITFIELD(16, 31)    ; //!< D_subpic_height_minus1
            };
            uint32_t                     Value;
        } DW43;
        union
        {
            struct
            {
                uint32_t                 Reserved1415                                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 Reserved1416                                     : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Reserved1417                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW44;
        union
        {
            struct
            {
                uint32_t                 DToplefttilex                                    : __CODEGEN_BITFIELD( 0,  7)    ; //!< D_TopLeftTileX
                uint32_t                 Reserved1448                                     : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 DToplefttiley                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< D_TopLeftTileY
            };
            uint32_t                     Value;
        } DW45;
        union
        {
            struct
            {
                uint32_t                 DSliceheightinctus                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_sliceHeightInCtus
                uint32_t                 Reserved1488                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW46;
        union
        {
            struct
            {
                uint32_t                 DSlicestartctbx                                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< D_SliceStartCtbX
                uint32_t                 DSlicestartctby                                  : __CODEGEN_BITFIELD(16, 31)    ; //!< D_SliceStartCtbY
            };
            uint32_t                     Value;
        } DW47;

        //! \name Local enumerations

        //! \brief LENGTHFIELD
        //! \details
        //!     (Excludes Dwords 0, 1).
        enum LENGTHFIELD
        {
            LENGTHFIELD_DWORDLENGTH_LENGTHBIAS                               = 46, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPSLICESTATE                         = 20, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC                      = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SH_SLICE_TYPE
        //! \details
        //!     style="margin-bottom:11px">specifies the coding type of the slice, 0
        //!     = B slice, 1 = P slice, 2 = I slice.  For more detail refers to the VVC
        //!     Spec.
        enum SH_SLICE_TYPE
        {
            SH_SLICE_TYPE_B_SLICE                                            = 0, //!< No additional details
            SH_SLICE_TYPE_P_SLICE                                            = 1, //!< No additional details
            SH_SLICE_TYPE_I_SLICE                                            = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_SLICE_STATE_CMD();

        static const size_t dwSize = 48;
        static const size_t byteSize = 192;
    };

    //!
    //! \brief VVCP_SURFACE_STATE
    //! \details
    //!     The VVCP_SURFACE_STATE command is responsible for defining the frame
    //!     buffer pitch and the offset of the chroma component.
    //!     This is a picture level state command and is shared by both encoding and
    //!     decoding processes.
    //!     For Decoder, this command is issued once per surface type. There is one
    //!     reconstructed surface, 8 reference pictures surfaces.
    //!     For all tiling surface, Tile4/Tile64 are supported and indicated on the
    //!     MemoryAttributes field on its corresponding address field.
    //!     Note : When NV12 and Tile4/Tile64are being used, full pitch and
    //!     interleaved UV is always in use. U and V Xoffset must be set to 0; U and
    //!     V Yoffset must be 8-pixel aligned. For 10-bit pixel, P010 surface
    //!     definition is being used.
    //!
    struct VVCP_SURFACE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 SurfacePitchMinus1                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Surface Pitch Minus1
                uint32_t                 Reserved49                                       : __CODEGEN_BITFIELD(17, 26)    ; //!< Reserved
                uint32_t                 SurfaceId                                        : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_ID
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 YOffsetForUCbInPixel                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for U(Cb) in pixel
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 26)    ; //!< Reserved
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 CompressionFormat                                : __CODEGEN_BITFIELD(16, 20)    ; //!< Compression Format
                uint32_t                 Reserved149                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPSURFACESTATE                       = 1, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum SURFACE_ID
        {
            SURFACE_ID_RECONSTRUCTEDPICTURE                                  = 0, //!< This is for the reconstructed picture surface state
            SURFACE_ID_REFERENCEPICTURE0                                     = 17, //!< This is for Reference Picture 0
            SURFACE_ID_REFERENCEPICTURE1                                     = 18, //!< This is for Reference Picture 1
            SURFACE_ID_REFERENCEPICTURE2                                     = 19, //!< This is for Reference Picture 2
            SURFACE_ID_REFERENCEPICTURE3                                     = 20, //!< This is for Reference Picture 3
            SURFACE_ID_REFERENCEPICTURE4                                     = 21, //!< This is for Reference Picture 4
            SURFACE_ID_REFERENCEPICTURE5                                     = 22, //!< This is for Reference Picture 5
            SURFACE_ID_REFERENCEPICTURE6                                     = 23, //!< This is for Reference Picture 6
            SURFACE_ID_REFERENCEPICTURE7                                     = 24, //!< This is for Reference Picture 7
            SURFACE_ID_REFERENCEPICTURE8                                     = 25, //!< This is for Reference Picture 8
            SURFACE_ID_REFERENCEPICTURE9                                     = 26, //!< This is for Reference Picture 9
            SURFACE_ID_REFERENCEPICTURE10                                    = 27, //!< This is for Reference Picture 10
            SURFACE_ID_REFERENCEPICTURE11                                    = 28, //!< This is for Reference Picture 11
            SURFACE_ID_REFERENCEPICTURE12                                    = 29, //!< This is for Reference Picture 12
            SURFACE_ID_REFERENCEPICTURE13                                    = 30, //!< This is for Reference Picture 13
            SURFACE_ID_REFERENCEPICTURE14                                    = 31, //!< This is for Reference Picture 14
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface.
        //!
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< No additional details
            SURFACE_FORMAT_P010                                              = 13, //!< This format can be used for 8, 9, 10 bit 420 format
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_SURFACE_STATE_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief VVCP_TILE_CODING
    //! \details
    //!
    //!
    struct VVCP_TILE_CODING_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Tilecolbdval                                     : __CODEGEN_BITFIELD( 0, 10)    ; //!< TileColBdVal
                uint32_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 Tilerowbdval                                     : __CODEGEN_BITFIELD(16, 26)    ; //!< TileRowBdVal
                uint32_t                 Reserved59                                       : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Colwidthval                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< ColWidthVal
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Rowheightval                                     : __CODEGEN_BITFIELD(16, 27)    ; //!< RowHeightVal
                uint32_t                 Reserved92                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DCurrenttilecolumnposition                       : __CODEGEN_BITFIELD( 0, 10)    ; //!< D_CurrentTileColumnPosition
                uint32_t                 Reserved107                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 DCurrenttilerowposition                          : __CODEGEN_BITFIELD(16, 26)    ; //!< D_CurrentTileRowPosition
                uint32_t                 Reserved123                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 19)    ; //!< Reserved
                uint32_t                 DIsrightmosttileofsliceFlag                      : __CODEGEN_BITFIELD(20, 20)    ; //!< D_IsRightMostTileOfSlice_flag
                uint32_t                 DIsleftmosttileofsliceFlag                       : __CODEGEN_BITFIELD(21, 21)    ; //!< D_IsLeftMostTileOfSlice_flag
                uint32_t                 DIsbottommosttileofsliceFlag                     : __CODEGEN_BITFIELD(22, 22)    ; //!< D_IsBottomMostTileOfSlice_flag
                uint32_t                 DIstopmosttileofsliceFlag                        : __CODEGEN_BITFIELD(23, 23)    ; //!< D_IsTopMostTileOfSlice_flag
                uint32_t                 Reserved129                                      : __CODEGEN_BITFIELD(24, 27)    ; //!< Reserved
                uint32_t                 DIsrightmosttileofframeFlag                      : __CODEGEN_BITFIELD(28, 28)    ; //!< D_IsRightMostTileOfFrame_flag
                uint32_t                 DIsleftmosttileofframeFlag                       : __CODEGEN_BITFIELD(29, 29)    ; //!< D_IsLeftMostTileOfFrame_flag
                uint32_t                 DIsbottommosttileofframeFlag                     : __CODEGEN_BITFIELD(30, 30)    ; //!< D_IsBottomMostTileOfFrame_flag
                uint32_t                 DIstopmosttileofframeFlag                        : __CODEGEN_BITFIELD(31, 31)    ; //!< D_IsTopMostTileOfFrame_flag
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPTILECODING                         = 21, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC                      = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_TILE_CODING_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief VVCP_VD_CONTROL_STATE
    //! \details
    //!     This command is used to modify the control of VVCP pipe (as well as HCP
    //!     and AVP Pipes). It can be inserted anywhere within a frame. It can be
    //!     inserted multiple times within a frame as well.
    //!
    struct VVCP_VD_CONTROL_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        VD_CONTROL_STATE_BODY_CMD                VdControlStateBody;                                                      //!< DW1..2, VD Control State Body

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATE                         = 10, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/EngineName = VVC
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVVCP                  = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_VD_CONTROL_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VVCP_WEIGHTOFFSET_STATE
    //! \details
    //!     This slice level command is issued in both the encoding and decoding
    //!     processes, if the weighted_pred_flag orweighted_bipred_flag equals one.
    //!     If zero, then this command is not issued.
    //!     Weight Prediction Values are provided in this command. Only Explicit
    //!     Weight Prediction is supported inencoder.
    //!     For P-Slice, this command is issued only once together with
    //!     VVCP_REF_IDX_STATE Command for L0 list. ForB-Slice, this command can be
    //!     issued up to two times together with VVCP_REF_IDX_STATE Command, one
    //!     forL0 list and one for L1 list.
    //!
    struct VVCP_WEIGHTOFFSET_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 21)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(22, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Listidx                                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< LISTIDX
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1,  7)    ; //!< Reserved
                uint32_t                 LumaLog2WeightDenom                              : __CODEGEN_BITFIELD( 8, 11)    ; //!< luma_log2_weight_denom
                uint32_t                 Chromalog2Weightdenom                            : __CODEGEN_BITFIELD(12, 15)    ; //!< ChromaLog2WeightDenom
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 LumaWeightLxFlagI                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< luma_weight_lX_flag[ i ]
                uint32_t                 ChromaWeightLxFlagI                              : __CODEGEN_BITFIELD(16, 31)    ; //!< chroma_weight_lX_flag[ i ]
            };
            uint32_t                     Value;
        } DW2;
        VVCP_WEIGHTOFFSET_LUMA_ENTRY_CMD         DLumaweightsoffsets[15];                                                 //!< DW3..17, D_LumaWeightsOffsets
        VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD       DChromacbweightsoffsets[15];                                             //!< DW18..32, D_ChromaCbWeightsOffsets
        VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD       DChromacrweightsoffsets[15];                                             //!< DW33..47, D_ChromaCrWeightsOffsets

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_VVCPWEIGHTOFFSETSTATE                  = 19, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = VVC = 0Fh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 15, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief LISTIDX
        //! \details
        //!     There are max 2 possible reference picture list L0 and L1, so listIdx
        //!     can be 0 or 1 only.
        //!     listIdx is set to 0, when this VVCP_WEIGHTOFFSET_STATE command is
        //!     issued for L0.
        //!     listIdx is set to 1, when this VVCP_WEIGHTOFFSET_STATE command is
        //!     issued for L1.
        enum LISTIDX
        {
            LISTIDX_REFERENCEPICTURELIST0                                    = 0, //!< No additional details
            LISTIDX_REFERENCEPICTURELIST1                                    = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VVCP_WEIGHTOFFSET_STATE_CMD();

        static const size_t dwSize = 48;
        static const size_t byteSize = 192;
    };

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vvcp__xe2_lpm_base__xe2_lpm__Cmd)
};

}  // namespace xe2_lpm
}  // namespace xe2_lpm_base
}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#pragma pack()

#endif  // __MHW_VDBOX_VVCP_HWCMD_XE2_LPM_X_H__
