/*
* Copyright (c) 2014-2020, Intel Corporation
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

//! \file     mhw_vdbox_hcp_interface.h
//! \brief    MHW interface for constructing HCP commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox HCP commands across all platforms
//!

#ifndef _MHW_VDBOX_HCP_INTERFACE_H_
#define _MHW_VDBOX_HCP_INTERFACE_H_

#include "mhw_vdbox.h"
#include "mhw_mi.h"
#include "codec_def_encode_hevc.h"
#include "mhw_vdbox_hcp_def.h"

#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO        (26 * MHW_CACHELINE_SIZE) // 18+4+4
#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO_REXT   (35 * MHW_CACHELINE_SIZE) // 27+4+4

struct MHW_VDBOX_HEVC_PIC_STATE
{
    // Decode
    PCODEC_HEVC_PIC_PARAMS                  pHevcPicParams = nullptr;

    // Encode
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS      pHevcEncSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS       pHevcEncPicParams = nullptr;
    bool                                    bSAOEnable = false;
    bool                                    bNotFirstPass = false;
    bool                                    bHevcRdoqEnabled = false;
    bool                                    bUseVDEnc = false;
    bool                                    sseEnabledInVmeEncode = false;
    PMHW_BATCH_BUFFER                       pBatchBuffer = nullptr;
    bool                                    bBatchBufferInUse = false;
    bool                                    bRDOQIntraTUDisable = false;
    uint16_t                                wRDOQIntraTUThreshold = 0;
    uint32_t                                brcNumPakPasses = 0;
    bool                                    rhodomainRCEnable = false;
    bool                                    bTransformSkipEnable = false;

    //FEI multiple passes PAK ---max frame size
    uint16_t                                currPass = 0;
    uint8_t                                *deltaQp = nullptr;
    uint32_t                                maxFrameSize = 0;
    virtual ~MHW_VDBOX_HEVC_PIC_STATE(){}
};
using PMHW_VDBOX_HEVC_PIC_STATE = MHW_VDBOX_HEVC_PIC_STATE * ;

typedef struct _MHW_VDBOX_HEVC_TILE_STATE
{
    PCODEC_HEVC_PIC_PARAMS          pHevcPicParams;
    uint16_t                       *pTileColWidth;
    uint16_t                       *pTileRowHeight;
} MHW_VDBOX_HEVC_TILE_STATE, *PMHW_VDBOX_HEVC_TILE_STATE;

typedef struct _MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS
{
    uint8_t    ucMaxBitDepth;
    uint8_t    ucChromaFormat;
    uint32_t   dwCtbLog2SizeY;
    uint32_t   dwPicWidth;
    uint32_t   dwPicHeight;
    uint32_t   dwBufferSize;
    uint32_t   dwMaxFrameSize;
}MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS, *PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS;

typedef struct _MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS
{
    uint8_t    ucMaxBitDepth;
    uint8_t    ucChromaFormat;
    uint32_t   dwPicWidth;
    uint32_t   dwPicHeight;
    uint32_t   dwPicWidthAlloced;
    uint32_t   dwPicHeightAlloced;
    uint32_t   dwCtbLog2SizeY;
    uint32_t   dwCtbLog2SizeYMax;
    uint32_t   dwFrameSize;
    uint32_t   dwFrameSizeAlloced;
    bool       bNeedBiggerSize;
}MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS, *PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS;

struct MHW_VDBOX_HEVC_REF_IDX_PARAMS
{
    CODEC_PICTURE                   CurrPic = {};
    bool                            isEncode = false;
    uint8_t                         ucList = 0;
    uint8_t                         ucNumRefForList = 0;
    CODEC_PICTURE                   RefPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
    void                            **hevcRefList = nullptr;
    int32_t                         poc_curr_pic = 0;
    int32_t                         poc_list[CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
    int8_t                         *pRefIdxMapping = 0;
    uint16_t                        RefFieldPicFlag = 0;
    uint16_t                        RefBottomFieldFlag = 0;
    bool                            bDummyReference = false;
    virtual ~MHW_VDBOX_HEVC_REF_IDX_PARAMS(){}
};
using PMHW_VDBOX_HEVC_REF_IDX_PARAMS = MHW_VDBOX_HEVC_REF_IDX_PARAMS * ;

typedef struct _MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS
{
    uint8_t                         ucList;
    char                            LumaWeights[2][15];
    int16_t                         LumaOffsets[2][15];
    char                            ChromaWeights[2][15][2];
    int16_t                         ChromaOffsets[2][15][2];
} MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS, *PMHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS;

typedef struct _MHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS
{
    bool     Transformskip_enabled;
    uint16_t Transformskip_lambda;
    uint8_t  Transformskip_Numzerocoeffs_Factor0;
    uint8_t  Transformskip_Numnonzerocoeffs_Factor0;
    uint8_t  Transformskip_Numzerocoeffs_Factor1;
    uint8_t  Transformskip_Numnonzerocoeffs_Factor1;
}MHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS, *PMHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS;

struct MHW_VDBOX_HEVC_SLICE_STATE
{
    PCODEC_PIC_ID                   pHevcPicIdx = nullptr;
    PMOS_RESOURCE                   presDataBuffer = nullptr;
    uint32_t                        dwDataBufferOffset = 0;
    uint32_t                        dwOffset = 0;
    uint32_t                        dwLength = 0;
    uint32_t                        dwSliceIndex = 0;
    bool                            bLastSlice = false;
    bool                            bLastSliceInTile = false;
    bool                            bLastSliceInTileColumn = false;
    bool                            bHucStreamOut = false;
    int8_t                         *pRefIdxMapping = nullptr;
    bool                            bSaoLumaFlag = false;
    bool                            bSaoChromaFlag = false;

    PCODEC_HEVC_SLICE_PARAMS        pHevcSliceParams = nullptr;
    PCODEC_HEVC_PIC_PARAMS          pHevcPicParams = nullptr;

    // Encoding Only
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS      pEncodeHevcSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS       pEncodeHevcPicParams = nullptr;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS         pEncodeHevcSliceParams = nullptr;
    PBSBuffer                               pBsBuffer = nullptr;
    PCODECHAL_NAL_UNIT_PARAMS              *ppNalUnitParams = nullptr;
    bool                                    bFirstPass = false;
    bool                                    bLastPass = false;
    bool                                    bIntraRefFetchDisable = false;
    bool                                    bBrcEnabled = false;
    uint32_t                                dwHeaderBytesInserted = 0;
    uint32_t                                dwHeaderDummyBytes = 0;
    uint32_t                                uiSkipEmulationCheckCount = 0;
    bool                                    bInsertBeforeSliceHeaders = false;
    bool                                    bIsLowDelay = false;
    // Encoding + BRC only
    PMHW_BATCH_BUFFER                       pBatchBufferForPakSlices = nullptr;
    bool                                    bSingleTaskPhaseSupported = false;
    uint32_t                                dwBatchBufferForPakSlicesStartOffset = 0;
    bool                                    bVdencInUse = false;
    bool                                    bVdencHucInUse = false;
    bool                                    bWeightedPredInUse = false;
    PMHW_BATCH_BUFFER                       pVdencBatchBuffer = nullptr;

    //Pak related params
    MHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS EncodeHevcTransformSkipParams = {};
    bool                                    DeblockingFilterDisable = false;
    char                                    TcOffsetDiv2 = 0;
    char                                    BetaOffsetDiv2 = 0;

    uint8_t                                 RoundingIntra = 0;
    uint8_t                                 RoundingInter = 0;
    virtual ~MHW_VDBOX_HEVC_SLICE_STATE(){}
};
using PMHW_VDBOX_HEVC_SLICE_STATE = MHW_VDBOX_HEVC_SLICE_STATE * ;

typedef struct _MHW_VDBOX_VP9_ENCODE_PIC_STATE
{
    PCODEC_VP9_ENCODE_PIC_PARAMS        pVp9PicParams;
    PCODEC_REF_LIST                    *ppVp9RefList;
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS   pVp9SeqParams;

    union
    {
        struct
        {
            uint8_t                    KeyFrame : 1;        // [0..1]
            uint8_t                    IntraOnly : 1;        // [0..1]
            uint8_t                    Display : 1;        // [0..1]
            uint8_t                    ReservedField : 5;        // [0]
        } fields;
        uint8_t                        value;
    } PrevFrameParams;

    uint32_t                        dwPrevFrmWidth;
    uint32_t                        dwPrevFrmHeight;
    uint8_t                         ucTxMode;
    bool                            bUseDysRefSurface;
    bool                            bNonFirstPassFlag;
    bool                            bSSEEnable;
    bool                            bVdencPakOnlyPassFlag;
    uint32_t                        uiMaxBitRate;
    uint32_t                        uiMinBitRate;

} MHW_VDBOX_VP9_ENCODE_PIC_STATE, *PMHW_VDBOX_VP9_ENCODE_PIC_STATE;

typedef struct _MHW_VDBOX_VP9_PIC_STATE
{
    // Decode
    PCODEC_VP9_PIC_PARAMS              pVp9PicParams;
    PCODEC_REF_LIST                   *ppVp9RefList;

    union
    {
        struct
        {
            uint8_t                    KeyFrame : 1;        // [0..1]
            uint8_t                    IntraOnly : 1;        // [0..1]
            uint8_t                    Display : 1;        // [0..1]
            uint8_t                    ReservedField : 5;        // [0]
        } fields;
        uint8_t                        value;
    } PrevFrameParams;

    uint32_t                           dwPrevFrmWidth;
    uint32_t                           dwPrevFrmHeight;

} MHW_VDBOX_VP9_PIC_STATE, *PMHW_VDBOX_VP9_PIC_STATE;

typedef struct _MHW_VDBOX_IMAGE_STATUS_CONTROL
{
    union
    {
        struct
        {
            uint32_t   MaxMbConformanceFlag : 1;
            uint32_t   FrameBitcountFlag : 1;
            uint32_t   Panic : 1;
            uint32_t   MissingHuffmanCode : 1; // new addition for JPEG encode
            uint32_t   : 4;
            uint32_t   TotalNumPass : 4;
            uint32_t   VDENCSliceOverflowErrorOccurred : 1;
            uint32_t   NumPassPolarityChange : 2;
            uint32_t   CumulativeSliceQpPolarityChange : 1;
            uint32_t   SuggestedSliceQPDelta : 8;
            uint32_t   CumulativeSliceDeltaQP : 8;
        };

        struct
        {
            uint32_t   hcpLCUMaxSizeViolate : 1;
            uint32_t   hcpFrameBitCountViolateOverRun : 1;
            uint32_t   hcpFrameBitCountViolateUnderRun : 1;
            uint32_t   : 5;
            uint32_t   hcpTotalPass : 4;
            uint32_t   : 4;
            uint32_t   hcpCumulativeFrameDeltaLF : 7;
            uint32_t   : 1;
            uint32_t   hcpCumulativeFrameDeltaQp : 8;
        };

        struct
        {
            uint32_t   Value;
        };
    };
}MHW_VDBOX_IMAGE_STATUS_CONTROL, *PMHW_VDBOX_IMAGE_STATUS_CONTROL;

typedef struct _MHW_VDBOX_PAK_NUM_OF_SLICES
{
    // Num Slices
    union
    {
        struct
        {
            uint32_t   NumberOfSlices : 16;
            uint32_t   Reserved : 16;
        };

        struct
        {
            uint32_t   Value;
        };
    };
}MHW_VDBOX_PAK_NUM_OF_SLICES, *PMHW_VDBOX_PAK_NUM_OF_SLICES;

// definition for HEVC/VP9 internal buffer type
typedef enum _MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE
{
    MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE = 0x0,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TR_NBR,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_HSSE_RS,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_HSAO_RS,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_SLC_STATE_STREAMOUT,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RIGHT_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL,
    MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID,
    MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE,
    MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE
} MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE;



//!  MHW Vdbox Hcp interface
/*!
This class defines the interfaces for constructing Vdbox Hcp commands across all platforms
*/
class MhwVdboxHcpInterface
{
public:
    //!
    //! \enum     HevcSliceType
    //! \brief    HEVC slice type
    //!
    enum HevcSliceType
    {
        hevcSliceB  = 0,
        hevcSliceP  = 1,
        hevcSliceI  = 2
    };
    //!
    //! \brief    Get max vdbox index
    //!
    //! \return   MHW_VDBOX_NODE_IND
    //!           max vdbox index got
    //!
    inline MHW_VDBOX_NODE_IND GetMaxVdboxIndex()
    {
        return MEDIA_IS_SKU(m_skuTable, FtrVcs2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;
    }

protected:
    PMOS_INTERFACE              m_osInterface = nullptr; //!< Pointer to OS interface
    MhwMiInterface              *m_miInterface = nullptr; //!< Pointer to MI interface
    MhwCpInterface              *m_cpInterface = nullptr; //!< Pointer to CP interface
    MEDIA_FEATURE_TABLE         *m_skuTable = nullptr; //!< Pointer to SKU table
    MEDIA_WA_TABLE              *m_waTable = nullptr; //!< Pointer to WA table
    bool                        m_decodeInUse = false; //!< Flag to indicate if the interface is for decoder or encoder use

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {}; //!< Cacheability settings

    bool                        m_rhoDomainStatsEnabled = false; //!< Flag to indicate if Rho domain stats is enabled
    bool                        m_rowstoreCachingSupported = false; //!< Flag to indicate if row store cache is supported
    uint32_t                    m_brcNumPakPasses = 4; //!< Number of brc pak passes

    MHW_VDBOX_ROWSTORE_CACHE    m_hevcDatRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_hevcDfRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_hevcSaoRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_hevcHSaoRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_vp9HvdRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_vp9DfRowStoreCache = {};
    MHW_VDBOX_ROWSTORE_CACHE    m_vp9DatRowStoreCache = {};

    uint32_t                    m_hevcEncCuRecordSize = 0; //!< size of hevc enc cu record
    uint32_t                    m_pakHWTileSizeRecordSize = 0; //! pak HW tile size recored size

    static const uint32_t       m_timeStampCountsPerMillisecond = (12000048 / 1000);  //<! Time stamp coounts per millisecond
    static const uint32_t       m_hcpCabacErrorFlagsMask = 0x0879; //<! Hcp CABAC error flags mask

    MmioRegistersHcp            m_mmioRegisters[MHW_VDBOX_NODE_MAX] = {};  //!< hcp mmio registers

    static const HevcSliceType  m_hevcBsdSliceType[3]; //!< HEVC Slice Types for Long Format

    std::shared_ptr<void> m_hcpItfNew = nullptr;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    bool bMMCReported = false;
#endif

    //!
    //! \brief    Constructor
    //!
    MhwVdboxHcpInterface(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse);

    //!
    //! \brief    Add a resource to the command buffer
    //! \details  Internal function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer
    //!
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           Command buffer to which resource is added
    //! \param    [in] params
    //!           Parameters necessary to add the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*pfnAddResourceToCmd) (
        PMOS_INTERFACE osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_RESOURCE_PARAMS params);

    //!
    //! \brief    Convert to Sign Magnitude
    //! \details  Implementation of signed bitfield as specified
    //!
    //! \param    [in] val
    //!           Input value to calculate
    //! \param    [in] signBitPos
    //!           The position of sign bit
    //!
    //! \return   uint16_t
    //!           value calculated
    //!
    uint16_t Convert2SignMagnitude(
        int32_t val,
        uint32_t signBitPos);

public:

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHcpInterface() {}

    //!
    //! \brief    Get new HCP interface, temporal solution before switching from
    //!           old interface to new one
    //!
    //! \return   pointer to new HCP interface
    //!
    virtual std::shared_ptr<void> GetNewHcpInterface() { return nullptr; }

    //!
    //! \brief    Get OS interface
    //!
    //! \return   [out] PMOS_INTERFACE
    //!           Pointer to the OS interface.
    //!
    inline PMOS_INTERFACE GetOsInterface()
    {
        return m_osInterface;
    }

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //!
    //! \return   [out] MmioRegistersHcp*
    //!           mmio registers got.
    //!
    inline MmioRegistersHcp* GetMmioRegisters(MHW_VDBOX_NODE_IND index)
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

    //!
    //! \brief    Get Hcp Cabac Error Flags Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    virtual inline uint32_t GetHcpCabacErrorFlagsMask()
    {
        return m_hcpCabacErrorFlagsMask;
    }

    //!
    //! \brief    Get Watch Dog Timer Threhold
    //!
    //! \return   [out] uint32_t
    //!           Threhold got.
    //!
    virtual inline uint32_t GetWatchDogTimerThrehold()
    {
        return 0;
    }

    //!
    //! \brief    Get Time Stamp Counts Per Millisecond
    //!
    //! \return   [out] uint32_t
    //!           Counts got.
    //!
    inline uint32_t GetTimeStampCountsPerMillisecond()
    {
        return m_timeStampCountsPerMillisecond;
    }

    //!
    //! \brief    Get hevc enc cu record size
    //! \return   [out] uint32_t
    //!           Brc pak passes num.
    //!
    inline uint32_t GetHevcEncCuRecordSize()
    {
        return m_hevcEncCuRecordSize;
    }

    //!
    //! \brief    Get pak HW tile size record size
    //! \return   [out] uint32_t
    //!           Brc pak passes num.
    //!
    inline uint32_t GetPakHWTileSizeRecordSize()
    {
        return m_pakHWTileSizeRecordSize;
    }

    //!
    //! \brief    Judge if row store caching supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsRowStoreCachingSupported()
    {
        return m_rowstoreCachingSupported;
    }

    //!
    //! \brief    Judge if hevc dat store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcDatRowstoreCacheEnabled()
    {
        return m_hevcDatRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if hevc df row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcDfRowstoreCacheEnabled()
    {
        return m_hevcDfRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if hevc sao row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcSaoRowstoreCacheEnabled()
    {
        return m_hevcSaoRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if vp9 hvd row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsVp9HvdRowstoreCacheEnabled()
    {
        return m_vp9HvdRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if vp9 df row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsVp9DfRowstoreCacheEnabled()
    {
        return m_vp9DfRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if vp9 dat row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsVp9DatRowstoreCacheEnabled()
    {
        return m_vp9DatRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] cacheabilitySettings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
    {
        MHW_FUNCTION_ENTER;

        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);
        MOS_STATUS eStatus = MOS_SecureMemcpy(m_cacheabilitySettings, size,
            cacheabilitySettings, size);

        return eStatus;
    }

    //!
    //! \brief    Determines if the slice is P slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's P slice, otherwise return false
    //!
    inline bool IsHevcPSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ?
            (m_hevcBsdSliceType[sliceType] == hevcSliceP) : false;
    }

    //!
    //! \brief    Determines if the slice is B slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's B slice, otherwise return false
    //!
    inline bool IsHevcBSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ?
            (m_hevcBsdSliceType[sliceType] == hevcSliceB) : false;
    }

    //!
    //! \brief    Determines if the slice is I slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's I slice, otherwise return false
    //!
    inline bool IsHevcISlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ?
            (m_hevcBsdSliceType[sliceType] == hevcSliceI) : false;
    }

    //!
    //! \brief    Get pak object size
    //!
    //! \return   uint32_t
    //!           The size got
    //!
    virtual uint32_t GetHcpPakObjSize() = 0;

    //!
    //! \brief    Calculates the maximum size for HCP picture level commands
    //! \details  Client facing function to calculate the maximum size for HCP picture level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] params
    //!           Indicate the command size parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;

    //!
    //! \brief    Calculates maximum size for HCP slice/MB level commands
    //! \details  Client facing function to calculate maximum size for HCP slice/MB level commands
    //! \param    [in] mode
    //!           Indicate the codec mode
    //! \param    [out] commandsSize
    //!            The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] modeSpecific
    //!           Indicate the long or short format
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        bool                            modeSpecific) = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of HEVC_VP9_RDOQ_STATE_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetHcpHevcVp9RdoqStateCommandSize() = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of HCP_VP9_PIC_STATE_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetHcpVp9PicStateCommandSize() = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of HCP_VP9_SEGMENT_STATE_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetHcpVp9SegmentStateCommandSize() = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to get required buffer size for HEVC codec (both dual pipe and vdenc, enc and dec)
    //!
    //! \param    [in] bufferType
    //!           HEVC Buffer type
    //! \param    [in, out] hcpBufSizeParam
    //!           HCP buffer size parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to get required buffer size for VP9 codec
    //!
    //! \param    [in] bufferType
    //!           HEVC Buffer type
    //! \param    [in, out] hcpBufSizeParam
    //!           HCP buffer size parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVp9BufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to judge if buffer realloc is needed for HEVC codec
    //!
    //! \param    [in] bufferType
    //!          HEVC Buffer type
    //! \param    [in, out] reallocParam
    //!          HCP Re-allocate parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam) = 0;

    //!
    //! \brief    Get the required buffer size for VDBOX
    //! \details  Internal function to judge if buffer realloc is needed for VP9 codec
    //!
    //! \param    [in] bufferType
    //!           HEVC Buffer type
    //! \param    [in, out] reallocParam
    //!           HCP Re-allocate parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam) = 0;

    //!
    //! \brief    Adds HCP Pipe Pipe Mode Select command in command buffer
    //! \details  Client facing function to add HCP Pipe Mode Select command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpSurfaceCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    //!
    //! \brief    Adds HCP slice state command in command buffer
    //! \details  Client facing function to add HCP slice state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] hevcSliceState
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState);

    //!
    //! \brief    Adds HCP picture State command in command buffer
    //! \details  Client facing function to add HCP picture State command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpPicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params);

    //!
    //! \brief    Adds HCP Protect State command in command buffer
    //! \details  Client facing function to add HCP picture State command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] hevcSliceState
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpProtectStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState);

    //!
    //! \brief    Programs base address of rowstore scratch buffers
    //! \details  Internal function to get base address of rowstore scratch buffers
    //!
    //! \param    [in] rowstoreParams
    //!           Rowstore parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    //!
    //! \brief    Adds HCP Pipe Pipe Mode Select command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params) = 0;

    //!
    //! \brief    Adds HCP Surface State command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params) = 0;

    //!
    //! \brief    Adds HCP Surface State command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpEncodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params) = 0;

    //!
    //! \brief    Adds HCP Pipe Buffer Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS      params) = 0;

    //!
    //! \brief    Adds HCP Indirect Object Base Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params) = 0;

    //!
    //! \brief    Adds HCP QM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!          Command buffer to which HW command is added
    //! \param    [in] params
    //!          Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpQmStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_QM_PARAMS                 params) = 0;

    //!
    //! \brief    Adds HCP FQM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpFqmStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_QM_PARAMS                 params) = 0;

    //!
    //! \brief    Adds HCP picture State command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpDecodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params) = 0;

    //!
    //! \brief    Adds HCP picture State command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpEncodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params) = 0;

    //!
    //! \brief    Adds HCP tile State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpTileStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_TILE_STATE       params) = 0;

    //!
    //! \brief    Adds HCP reference frame index command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpRefIdxStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_HEVC_REF_IDX_PARAMS   params) = 0;

    //!
    //! \brief    Adds HCP weight offset state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS  params) = 0;

    //!
    //! \brief    Adds HCP slice state command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] hevcSliceState
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpDecodeSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState) = 0;

    //!
    //! \brief    Adds HCP slice state command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] hevcSliceState
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpEncodeSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState) = 0;

    //!
    //! \brief    Adds HCP Protect state command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] hevcSliceState
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpDecodeProtectStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState) = 0;

    //!
    //! \brief    Adds HCP BSD object state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpBsdObjectCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HCP_BSD_PARAMS        params) = 0;

    //!
    //! \brief    Adds HCP Pak insert object State command foe encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPakInsertObject(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS     params) = 0;

    //!
    //! \brief    Adds VP9 picture state command for decode in command buffer
    //!
    //! \param    [in]  cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in]  batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in]  params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpVp9PicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_PIC_STATE         params) = 0;

    //!
    //! \brief    Adds VP9 picture state command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpVp9PicStateEncCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_ENCODE_PIC_STATE  params) = 0;

    //!
    //! \brief    Adds VP9 segment state command for decode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpVp9SegmentStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_SEGMENT_STATE     params) = 0;

    //!
    //! \brief    Adds VP9 RDOQ state command for encode in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpHevcVp9RdoqStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params) = 0;

    //!
    //! \brief    Adds HEVC Brc buffer
    //!
    //! \param    [in] hcpImgStates
    //!           Resource to which Brc buffer is added
    //! \param    [in] hevcPicState
    //!           Params structure used to add the Brc buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpHevcPicBrcBuffer(
        PMOS_RESOURCE                    hcpImgStates,
        PMHW_VDBOX_HEVC_PIC_STATE         hevcPicState) = 0;

    //!
    //! \brief    Get OsResLaceOrAceOrRgbHistogramBuffer Size
    //!
    //! \param    [in] width
    //!           Width of Surface
    //! \param    [in] height
    //!           Height of Surface
    //! \param    [out] size
    //!           Size of OsResLaceOrAceOrRgbHistogramBuffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetOsResLaceOrAceOrRgbHistogramBufferSize(
        uint32_t                         width,
        uint32_t                         height,
        uint32_t                        *size) = 0;

    //!
    //! \brief    Get OsResStatisticsOutputBuffer Size
    //!
    //! \param    [in] width
    //!           Width of Surface
    //! \param    [in] height
    //!           Height of Surface
    //! \param    [out] size
    //!           Size of OsResStatisticsOutputBuffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetOsResStatisticsOutputBufferSize(
        uint32_t                         width,
        uint32_t                         height,
        uint32_t                        *size) = 0;
};

#endif
