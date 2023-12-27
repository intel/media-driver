/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_decode_vc1.h
//! \brief    Defines the decode interface extension for VC1.
//! \details  Defines all types, macros, and functions required by CodecHal for VC1 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_VC1_H__
#define __CODECHAL_DECODER_VC1_H__

#include "codechal_decoder.h"

//!
//! \def CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES
//! Unequal Field Surface max index
//!
#define CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES   4

//!
//! \def CODECHAL_DECODE_VC1_NUM_SYNC_TAGS
//! Sync Tags Number for StateHeap Settings
//!
#define CODECHAL_DECODE_VC1_NUM_SYNC_TAGS               36

//!
//! \def CODECHAL_DECODE_VC1_INITIAL_DSH_SIZE
//! Initial Dsh Size for StateHeap Settings
//!
#define CODECHAL_DECODE_VC1_INITIAL_DSH_SIZE            (MHW_PAGE_SIZE * 2)

//!
//! \def CODECHAL_DECODE_VC1_FAST_CHROMA_MV
//! Fast Chroma Mv calculation
//!
#define CODECHAL_DECODE_VC1_FAST_CHROMA_MV(cmv)         ((cmv) - ((cmv) % 2))

//!
//! \def CODECHAL_DECODE_VC1_CHROMA_MV
//! Chroma Mv calculation
//!
#define CODECHAL_DECODE_VC1_CHROMA_MV(lmv)              (((lmv) + CODECHAL_DECODE_VC1_RndTb[(lmv) & 3]) >> 1)

//!
//! \def CODECHAL_DECODE_VC1_BITSTRM_BUF_LEN
//! Bitstream Buffer Length
//!
#define CODECHAL_DECODE_VC1_BITSTRM_BUF_LEN             8

//!
//! \def CODECHAL_DECODE_VC1_STUFFING_BYTES
//!
#define CODECHAL_DECODE_VC1_STUFFING_BYTES              64

//!
//! \def CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH
//! Sc prefix lengith
//!
#define CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH            3

//!
//! \struct CODECHAL_VC1_VLD_SLICE_RECORD 
//! VC1 slice record 
//!
typedef struct _CODECHAL_VC1_VLD_SLICE_RECORD
{
    uint32_t   dwSkip;
    uint32_t   dwOffset;
    uint32_t   dwLength;
    uint32_t   dwSliceYOffset;
    uint32_t   dwNextSliceYOffset;
} CODECHAL_VC1_VLD_SLICE_RECORD, *PCODECHAL_VC1_VLD_SLICE_RECORD;

//!
//! \enum CODECHAL_DECODE_VC1_BINDING_TABLE_OFFSET_OLP
//! VC1 OLP Binding Table Offset
//!
typedef enum _CODECHAL_DECODE_VC1_BINDING_TABLE_OFFSET_OLP
{
    CODECHAL_DECODE_VC1_OLP_SRC_Y = 0,
    CODECHAL_DECODE_VC1_OLP_SRC_UV = 1,
    CODECHAL_DECODE_VC1_OLP_DST_Y = 3,
    CODECHAL_DECODE_VC1_OLP_DST_UV = 4,
    CODECHAL_DECODE_VC1_OLP_NUM_SURFACES = 6
}CODECHAL_DECODE_VC1_BINDING_TABLE_OFFSET_OLP;

//!
//! \enum CODECHAL_DECODE_VC1_DMV_INDEX
//! VC1 DMV index
//!
typedef enum _CODECHAL_DECODE_VC1_DMV_INDEX
{
    CODECHAL_DECODE_VC1_DMV_EVEN = 0,
    CODECHAL_DECODE_VC1_DMV_ODD = 1,
    CODECHAL_DECODE_VC1_DMV_MAX = 2
}CODECHAL_DECODE_VC1_DMV_INDEX;

//!
//! \struct _CODECHAL_DECODE_VC1_I_LUMA_BLOCKS
//! \brief  Define Look Up Table Structure for Luma Polarity of Interlaced Picture
//!
typedef struct _CODECHAL_DECODE_VC1_I_LUMA_BLOCKS
{
    uint8_t u8NumSamePolarity;
    union
    {
        uint8_t u8Polarity;
        uint8_t u8MvIndex0;
    };
    uint8_t u8MvIndex1;
    uint8_t u8MvIndex2;
    uint8_t u8MvIndex3;
}CODECHAL_DECODE_VC1_I_LUMA_BLOCKS;

//!
//! \struct _CODECHAL_DECODE_VC1_P_LUMA_BLOCKS
//! \brief  Define Look Up Table for Luma Inter-coded Blocks of Progressive Picture
//!
typedef struct _CODECHAL_DECODE_VC1_P_LUMA_BLOCKS
{
    uint8_t u8NumIntercodedBlocks;
    uint8_t u8MvIndex1;
    uint8_t u8MvIndex2;
    uint8_t u8MvIndex3;
}CODECHAL_DECODE_VC1_P_LUMA_BLOCKS;

//!
//! \struct _CODECHAL_DECODE_VC1_BITSTREAM
//! \brief  Define variables for VC1 bitstream
//!
typedef struct _CODECHAL_DECODE_VC1_BITSTREAM
{
    uint8_t*    pOriginalBitBuffer;                                   // pointer to the original capsuted bitstream
    uint8_t*    pOriginalBufferEnd;                                   // pointer to the end of the original uncapsuted bitstream
    uint32_t    u32ZeroNum;                                           // number of continuous zeros before the current bype.
    uint32_t    u32ProcessedBitNum;                                   // number of bits being processed from initiation
    uint8_t     CacheBuffer[CODECHAL_DECODE_VC1_BITSTRM_BUF_LEN + 4]; // cache buffer of uncapsuted raw bitstream
    uint32_t*   pu32Cache;                                            // pointer to the cache buffer
    uint32_t*   pu32CacheEnd;                                         // pointer to the updating end of the cache buffer
    uint32_t*   pu32CacheDataEnd;                                     // pointer to the last valid uint32_t of the cache buffer
    int32_t     iBitOffset;                                           // offset = 32 is the MSB, offset = 1 is the LSB.
    int32_t     iBitOffsetEnd;                                        // bit offset of the last valid uint32_t
    bool        bIsEBDU;                                              // 1 if it is EBDU and emulation prevention bytes are present.
} CODECHAL_DECODE_VC1_BITSTREAM, *PCODECHAL_DECODE_VC1_BITSTREAM;

//!
//! \struct _CODECHAL_DECODE_VC1_OLP_PARAMS
//! \brief  Define variables of VC1 Olp params for hw cmd
//!
typedef struct _CODECHAL_DECODE_VC1_OLP_PARAMS
{
    PMOS_COMMAND_BUFFER         pCmdBuffer;
    PMHW_PIPE_CONTROL_PARAMS    pPipeControlParams;
    PMHW_STATE_BASE_ADDR_PARAMS pStateBaseAddrParams;
    PMHW_VFE_PARAMS             pVfeParams;
    PMHW_CURBE_LOAD_PARAMS      pCurbeLoadParams;
    PMHW_ID_LOAD_PARAMS         pIdLoadParams;
}CODECHAL_DECODE_VC1_OLP_PARAMS, *PCODECHAL_DECODE_VC1_OLP_PARAMS;

//!
//! \struct _CODECHAL_DECODE_VC1_OLP_STATIC_DATA
//! \brief  Define VC1 OLP Static Data
//!
typedef struct _CODECHAL_DECODE_VC1_OLP_STATIC_DATA
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   BlockWidth                      : 16;   // in byte
            uint32_t   BlockHeight                     : 16;   // in byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t   Profile                         : 1;
            uint32_t   RangeExpansionFlag              : 1;    // Simple & Main Profile only
            uint32_t   PictureUpsamplingFlag           : 2;    // 2:H, 3:V
            uint32_t                                   : 1;
            uint32_t   InterlaceFieldFlag              : 1;
            uint32_t                                   : 2;
            uint32_t   RangeMapUV                      : 3;
            uint32_t   RangeMapUVFlag                  : 1;
            uint32_t   RangeMapY                       : 3;
            uint32_t   RangeMapYFlag                   : 1;
            uint32_t                                   : 4;
            uint32_t   ComponentFlag                   : 1;
            uint32_t                                   : 11;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t   SourceDataBindingIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t   DestDataBindingIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

} CODECHAL_DECODE_VC1_OLP_STATIC_DATA, *PCODECHAL_DECODE_VC1_OLP_STATIC_DATA;

//!
//! \def CODECHAL_DECODE_VC1_CURBE_SIZE_OLP
//! VC1 Curbe Size for Olp
//!
#define CODECHAL_DECODE_VC1_CURBE_SIZE_OLP          (sizeof(CODECHAL_DECODE_VC1_OLP_STATIC_DATA))

//!
//! \struct _CODECHAL_DECODE_VC1_KERNEL_HEADER_CM
//! \brief Define VC1 Kernel Header CM
//!
typedef struct _CODECHAL_DECODE_VC1_KERNEL_HEADER_CM {
    int nKernelCount;

    CODECHAL_KERNEL_HEADER OLP;
    CODECHAL_KERNEL_HEADER IC;
} CODECHAL_DECODE_VC1_KERNEL_HEADER_CM, *PCODECHAL_DECODE_VC1_KERNEL_HEADER_CM;

//*------------------------------------------------------------------------------
//* Codec Definitions
//*------------------------------------------------------------------------------

//!
//! \class CodechalDecodeVc1
//! \brief This class defines the member fields, functions etc used by VC1 decoder.
//!
class CodechalDecodeVc1 : public CodechalDecode
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeVc1(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeVc1(const CodechalDecodeVc1&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeVc1& operator=(const CodechalDecodeVc1&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeVc1();

    //!
    //! \brief    Allocate and initialize VC1 decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard(
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for VC1 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates() override;

    //!
    //! \brief    VC1 decoder state level function
    //! \details  State level function for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel() override;

    //!
    //! \brief    VC1 decoder primitive level function
    //! \details  Primitive level function for GEN specific VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel() override;

    MOS_STATUS  InitMmcState() override;

    //!
    //! \brief    VC1 decoder primitive level function for VLD mode
    //! \details  Primitive level function for GEN specific VC1 decoder for VLD mode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual  MOS_STATUS  DecodePrimitiveLevelVLD();

    //!
    //! \brief    VC1 decoder primitive level function for IT mode
    //! \details  Primitive level function for GEN specific VC1 decoder for IT mode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  DecodePrimitiveLevelIT();

    // no downsampling

    //!
    //! \brief    Allocate resources for VC1 decoder
    //! \details  Allocate resources for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Set GEN specific Curbe data for VC1 OLP
    //! \details  Configure Curbe data for VC1 OLP Y / UV component
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeOlp();

    //!
    //! \brief    Update VC1 Kernel State
    //! \details  Get Decode Kernel and Update Kernel State
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateVc1KernelState();

    virtual MOS_STATUS AddVc1OlpCmd(
        PCODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams);

    //!
    //! \brief    Return if Olp needed
    //! \details  Return value of member bOlpNeeded
    //! \return   bool
    //!           true if Olp needed, else false
    //!
    bool IsOlpNeeded() { return m_olpNeeded; };

    PCODEC_VC1_PIC_PARAMS m_vc1PicParams = nullptr;                           //!< VC1 Picture Params
    MOS_SURFACE           m_destSurface;                                      //!< Pointer to MOS_SURFACE of render surface
    PMOS_RESOURCE         m_presReferences[CODEC_MAX_NUM_REF_FRAME_NON_AVC];  //!< Reference Resources Handle list
    bool                  m_deblockingEnabled   = false;                      //!< Indicator of deblocking enabling
    bool                  m_unequalFieldWaInUse = false;                      //!< Indicator of Unequal Field WA

protected:
    //!
    //! \brief    Construct VC1 decode bitstream buffer
    //! \details  For WaVC1ShortFormat. Construct VC1 decode bistream buffer by
    //            adding a stuffing byte ahead of frame bitstream data. It's for
    //            simple & main profile short format only.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructBistreamBuffer();

    //!
    //! \brief    Handle VC1 skipped frame
    //! \details  For skipped frame, use reference frame instead
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HandleSkipFrame();

    //!
    //! \brief    Initialize Unequal Field Surface
    //! \details  Initialize Unequal Field Surface for VC1 decoder
    //! \param    [in] refListIdx
    //!           Index for pic in RefList
    //! \param    [in] nullHwInUse
    //!           Indicate if null HW is in use or not
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeUnequalFieldSurface(
        uint8_t                     refListIdx,
        bool                        nullHwInUse);

    //!
    //! \brief    Formats destination surface for VC1 decoder
    //! \details  Formats the destination surface, in the pack case the UV surface
    //            is moved to be adjacent to the UV surface such that NV12
    //            formatting is maintained when the surface is returned to SW,
    //            in the unpack case the UV surface is moved to be 32 - pixel rows
    //            away from the Y surface so that during decoding HW will not
    //            overwrite the UV surface
    //! \param    [in] srcSurface
    //!           Source Surface
    //! \param    [in] dstSurface
    //!           Destiny Surface
    //! \param    [in] pack
    //!           Indicate pack case or unpack case
    //! \param    [in] nullHwInUse
    //!           Indicate if null HW is in use or not
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FormatUnequalFieldPicture(
        MOS_SURFACE                 srcSurface,
        MOS_SURFACE                 dstSurface,
        bool                        pack,
        bool                        nullHwInUse);

    //!
    //! \brief    Parse Picture Header for VC1 decoder
    //! \details  Parse Picture Header in bitstream for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureHeader();

    //!
    //! \brief    Parse Picture Header for VC1 decoder Advanced profile
    //! \details  Parse Picture Header in bitstream for VC1 decoder Advaced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureHeaderAdvanced();

    //!
    //! \brief    Parse Picture Header for VC1 decoder Simple profile
    //! \details  Parse Picture Header in bitstream for VC1 decoder Simple profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureHeaderMainSimple();

    //!
    //! \brief    Initialise bitstream for VC1 decoder
    //! \details  Initialise members' value of bitstream struct for VC1 decoder
    //! \param    [in] buffer
    //!           Original bitstream buffer
    //! \param    [in] length
    //!           Original bitstream length
    //! \param    [in] isEBDU
    //!           Indicate if it is EBDU
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitialiseBitstream(
        uint8_t*                           buffer,
        uint32_t                           length,
        bool                               isEBDU);

    //!
    //! \brief    Parse bitplane for VC1 decoder
    //! \details  Parse bitplane according to bitplane mode for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseBitplane();

    //!
    //! \brief    Parse bitplane in Norm2 Mode for VC1 decoder
    //! \details  Parse bitplane in Norm2 Mode for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BitplaneNorm2Mode();

    //!
    //! \brief    Parse bitplane in Norm6 Mode for VC1 decoder
    //! \details  Parse bitplane in Norm6 Mode for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BitplaneNorm6Mode();

    //!
    //! \brief    Parse bitplane in Rowskip Mode for VC1 decoder
    //! \details  Parse bitplane in Rowskip Mode for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BitplaneRowskipMode();

    //!
    //! \brief    Parse bitplane in Colskip Mode for VC1 decoder
    //! \details  Parse bitplane in Colskip Mode for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BitplaneColskipMode();

    //!
    //! \brief    Parse bitplane quantization for VC1 decoder
    //! \details  Parse bitplane quantization for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseVopDquant();

    //!
    //! \brief    Parse Mv Range for VC1 decoder
    //! \details  Parse Mv Range for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseMvRange();

    //!
    //! \brief    Parse Progressive Mv Mode for VC1 decoder
    //! \details  Parse Progressive Mv Mode for VC1 decoder
    //! \param    [in] mvModeTable[]
    //!           const MV Mode Table
    //! \param    [out] mvMode
    //!           pointer to Mv Mode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseProgressiveMvMode(
        const uint32_t                     mvModeTable[],
        uint32_t*                          mvMode);

    //!
    //! \brief    Parse Interlace Mv Mode for VC1 decoder
    //! \details  Parse Interlace Mv Mode for VC1 decoder
    //! \param    [in] isPPicture
    //!           indicate if it is P picture
    //! \param    [out] mvmode
    //!           pointer to Mv Mode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseInterlaceMVMode(
        bool                               isPPicture,
        uint32_t*                          mvmode);

    //!
    //! \brief    Parse I Picture Layer for VC1 decoder
    //! \details  Parse I Picture Layer for VC1 decoder advanced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureLayerIAdvanced();

    //!
    //! \brief    Parse P Picture Layer for VC1 decoder
    //! \details  Parse P Picture Layer for VC1 decoder advanced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureLayerPAdvanced();

    //!
    //! \brief    Parse B Picture Layer for VC1 decoder
    //! \details  Parse B Picture Layer for VC1 decoder advanced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParsePictureLayerBAdvanced();

    //!
    //! \brief    Parse P Field Picture Layer for VC1 decoder
    //! \details  Parse P Field Picture Layer for VC1 decoder advanced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseFieldPictureLayerPAdvanced();

    //!
    //! \brief    Parse B Field Picture Layer for VC1 decoder
    //! \details  Parse B Field Picture Layer for VC1 decoder advanced profile
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ParseFieldPictureLayerBAdvanced();

    //!
    //! \brief    Get Macroblock Offset for VC1 decoder
    //! \details  Get Macroblock Offset for VC1 decoder slice params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSliceMbDataOffset();

    //!
    //! \brief    Perform Olp for VC1 decoder
    //! \details  Perform Olp for VC1 decoder
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PerformVc1Olp();

    //!
    //! \brief    Initializes the VC1 OLP state
    //! \details  Initializes the VC1 OLP state based on parameters saved in InitInterface
    //!           command buffer or indirect state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateVc1Olp();

    //!
    //! \brief    Pack Motion Vectors in Macro Block State
    //! \param    [in] vc1MbState
    //!           Pointer to Vc1 Macro Block State
    //! \param    [in] mv
    //!           Pointer to Motion Vector
    //! \param    [out] packedLumaMvs
    //!           Pointer to Packed Luma Motion Vectors
    //! \param    [out] packedChromaMv
    //!           Pointer to Packed Chroma Motion Vectors
    //! \return   void
    //!
    virtual void PackMotionVectors(
        PMHW_VDBOX_VC1_MB_STATE vc1MbState,
        int16_t                 *mv,
        int16_t                 *packedLumaMvs,
        int16_t                 *packedChromaMv);

    // Parameters passed by application
    uint16_t                m_picWidthInMb         = 0;        //!< Picture Width in MB width count
    uint16_t                m_picHeightInMb        = 0;        //!< Picture Height in MB height count
    bool                    m_intelEntrypointInUse = false;    //!< Indicator of using a Intel-specific entrypoint.
    bool                    m_shortFormatInUse     = false;    //!< Short format slice data
    bool                    m_vc1OddFrameHeight    = false;    //!< VC1 Odd Frame Height
    uint32_t                m_dataSize             = 0;        //!< Size of the data contained in presDataBuffer
    uint32_t                m_dataOffset           = 0;        //!< Offset of the data contained in presDataBuffer
    uint32_t                m_numSlices            = 0;        //!< [VLD mode] Number of slices to be decoded
    uint32_t                m_numMacroblocks       = 0;        //!< [IT mode] Number of MBs to be decoded
    uint32_t                m_numMacroblocksUv     = 0;        //!< [IT mode] Number of UV MBs to be decoded
    PCODEC_VC1_SLICE_PARAMS m_vc1SliceParams       = nullptr;  //!< VC1 Slice Params
    PCODEC_VC1_MB_PARAMS    m_vc1MbParams          = nullptr;  //!< VC1 Macro Block Params
    MOS_SURFACE             m_deblockSurface;                  //!< Deblock Surface
    MOS_RESOURCE            m_resDataBuffer;                   //!< Handle of residual difference surface
    MOS_RESOURCE            m_resBitplaneBuffer;               //!< Handle of Bitplane buffer
    uint8_t *               m_deblockDataBuffer = nullptr;     //!< Pointer to the deblock data

    // Internally maintained
    MOS_RESOURCE                   m_resMfdDeblockingFilterRowStoreScratchBuffer;        //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE                   m_resBsdMpcRowStoreScratchBuffer;                     //!< Handle of BSD/MPC Row Store Scratch data surface
    MOS_RESOURCE                   m_resVc1BsdMvData[CODECHAL_DECODE_VC1_DMV_MAX];       //!< Handle of VC1 BSD MV Data
    PCODECHAL_VC1_VLD_SLICE_RECORD m_vldSliceRecord = nullptr;                           //!< [VLD mode] Slice record
    uint32_t                       m_numVldSliceRecord = 0;
    PCODEC_REF_LIST                m_vc1RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1];  //!< VC1 Reference List
    MOS_RESOURCE                   m_resSyncObject;                                      //!< Handle of Sync Object
    MOS_RESOURCE                   m_resPrivateBistreamBuffer;                           //!< Handle of Private Bistream Buffer
    uint32_t                       m_privateBistreamBufferSize = 0;                      //!< Size of Private Bistream Buffer
    CODECHAL_DECODE_VC1_BITSTREAM  m_bitstream;                                          //!< VC1 Bitstream
    // PCODECHAL_DECODE_VC1_BITSTREAM  pBitstream;                                     //!< Pointer to Bitstream

    uint16_t m_prevAnchorPictureTff     = 0;      //!< Previous Anchor Picture Top Field First(TFF)
    bool     m_prevEvenAnchorPictureIsP = false;  //!< Indicator of Previous Even Anchor Picture P frame
    bool     m_prevOddAnchorPictureIsP  = false;  //!< Indicator of Previous Odd Anchor Picture P frame
    uint16_t m_referenceDistance        = 0;      //!< REFDIST.

    // OLP related
    MHW_KERNEL_STATE m_olpKernelState;                      //!< Olp Kernel State
    uint8_t *        m_olpKernelBase            = nullptr;  //!< Pointer to Kernel Base Address
    uint32_t         m_olpKernelSize            = 0;        //!< Olp Kernel Size
    bool             m_olpNeeded                = false;    //!< Indicator if Olp Needed
    uint16_t         m_olpPicWidthInMb          = 0;        //!< Width of Olp Pic in Macro block
    uint16_t         m_olpPicHeightInMb         = 0;        //!< Height of Olp Pic in Macro block
    uint32_t         m_olpCurbeStaticDataLength = 0;        //!< Olp Curbe Static Data Length
    uint32_t         m_olpDshSize               = 0;        //!< Olp DSH Size

    // IT mode related
    MHW_BATCH_BUFFER m_itObjectBatchBuffer;  //!< IT mode Object Batch Buffer
    uint8_t          m_fieldPolarity = 0;    //!< Field Polarity Offset

    MOS_SURFACE m_unequalFieldSurface[CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES];     //!< Handle of Unequal Field Surface
    uint8_t     m_unequalFieldRefListIdx[CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES];  //!< Reference list of Unequal Field Surface
    uint8_t     m_unequalFieldSurfaceForBType = 0;                                        //!< Unequal Field Surface Index for B frame
    uint8_t     m_currUnequalFieldSurface     = 0;                                        //!< Current Unequal Field Surface Index

    // HuC copy related
    bool         m_huCCopyInUse;                    //!< a sync flag used when huc copy and decoder run in the different VDBOX
    MOS_RESOURCE m_resSyncObjectWaContextInUse;     //!< signals on the video WA context
    MOS_RESOURCE m_resSyncObjectVideoContextInUse;  //!< signals on the video context

private:
    //!
    //! \brief    Wrapper function to read bits from VC1 bitstream
    //! \param    [in] bitsRead
    //!           Number of bits to be read
    //! \param    [out] value
    //!           VC1 bitstream status, EOS if reaching end of stream, else bitstream value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetBits(uint32_t bitsRead, uint32_t &value);

    //!
    //! \brief    Wrapper function to get VLC from VC1 bitstream according to VLC Table
    //! \param    [in] table
    //!           Pointer to VLC Table
    //! \param    [out] value
    //!           VC1 bitstream status, EOS if reaching end of stream, else bitstream value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetVLC(const uint32_t* table, uint32_t & value);

    //!
    //! \brief    Wrapper function to skip words from VC1 bitstream
    //! \param    [in] dwordNumber
    //!           Number of Dword to be skipped
    //! \param    [out] value
    //!           VC1 bitstream status, EOS if reaching end of stream, else bitstream value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SkipWords(uint32_t dwordNumber, uint32_t & value);

    //!
    //! \brief    Wrapper function to skip bits from VC1 bitstream
    //! \param    [in] bits
    //!           Number of bits to be skipped
    //! \param    [out] value
    //!           VC1 bitstream status, EOS if reaching end of stream, else bitstream value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SkipBits(uint32_t bits, uint32_t & value);

    //!
    //! \brief    Read bits from VC1 bitstream
    //! \param    [in] bitsRead
    //!           Number of bits to be read
    //! \return   uint32_t
    //!           EOS if reaching end of stream, else bitstream value
    //!
    uint32_t GetBits(uint32_t bitsRead);

    //!
    //! \brief    Update VC1 bitstream memeber value
    //! \return   uint32_t
    //!           EOS if reaching end of stream, else bitstream value
    //!
    uint32_t UpdateBitstreamBuffer();

    //!
    //! \brief    Get VLC from VC1 bitstream according to VLC Table
    //! \param    [in] table
    //!           Pointer to VLC Table
    //! \return   uint32_t
    //!           EOS if reaching end of stream, else bitstream value
    //!
    uint32_t GetVLC(const uint32_t *table);

    //!
    //! \brief    Read bits from VC1 bitstream and don't update bitstream pointer
    //! \param    [in] bitsRead
    //!           Number of bits to be read
    //! \return   uint32_t
    //!           EOS if reaching end of stream, else bitstream value
    //!
    uint32_t PeekBits(uint32_t bitsRead);

    //!
    //! \brief    Skip bits from VC1 bitstream
    //! \param    [in] bits
    //!           Number of bits to be skipped
    //! \return   uint32_t
    //!           EOS if reaching end of stream, else bitstream value
    //!
    uint32_t SkipBits(uint32_t bitsRead);

    //!
    //! \brief    Pack Chroma/Luma Motion Vectors for Interlaced frame
    //! \param    [in] fieldSelect
    //!           Field Select Index
    //! \param    [in] currentField
    //!           Current Filed Indicator
    //! \param    [in] fastUVMotionCompensation
    //!           Fast UV Motion Compensation Indicator
    //! \param    [out] lmv
    //!           Pointer to Adjusted Luma Motion Vectors
    //! \param    [out] cmv
    //!           Pointer to Adjusted Chroma Motion Vectors
    //! \return   void
    //!
    uint8_t PackMotionVectorsChroma4MvI(
        uint16_t    fieldSelect,
        uint16_t    currentField,
        bool        fastUVMotionCompensation,
        int16_t      *lmv,
        int16_t      *cmv);

    //!
    //! \brief    Pack Chroma/Luma Motion Vectors for Picture frame
    //! \param    [in] intraFlags
    //!           Intra Flag Index
    //! \param    [out] lmv
    //!           Pointer to Adjusted Luma Motion Vectors
    //! \param    [out] cmv
    //!           Pointer to Adjusted Chroma Motion Vectors
    //! \return   void
    //!
    void PackMotionVectorsChroma4MvP(uint16_t intraFlags, int16_t *lmv, int16_t *cmv);

    //!
    //! \brief    Find Median for 3 Motion Vectors
    //! \param    [in] mv#
    //!           Motion Vectors
    //! \return   int16_t
    //!           return median for 3 Motion Vectors
    //!
    int16_t PackMotionVectorsMedian3(int16_t mv1, int16_t mv2, int16_t mv3);
    //!
    //! \brief    Find Median for 4 Motion Vectors
    //! \param    [in] mv#
    //!           Motion Vectors
    //! \return   int16_t
    //!           return median for 4 Motion Vectors
    //!
    int16_t PackMotionVectorsMedian4(int16_t mv1, int16_t mv2, int16_t mv3, int16_t mv4);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_VC1_PIC_PARAMS vc1PicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_VC1_SLICE_PARAMS sliceControl);

    MOS_STATUS DumpMbParams(
        PCODEC_VC1_MB_PARAMS mbParams);
#endif
};

#endif  // __CODECHAL_DECODER_VC1_H__
