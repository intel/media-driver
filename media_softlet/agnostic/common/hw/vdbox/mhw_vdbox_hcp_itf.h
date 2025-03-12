/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     mhw_vdbox_hcp_itf.h
//! \brief    MHW VDBOX HCP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_HCP_ITF_H__
#define __MHW_VDBOX_HCP_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox_hcp_cmdpar.h"

#define _HCP_CMD_DEF(DEF)             \
    DEF(HCP_SURFACE_STATE);           \
    DEF(HCP_PIC_STATE);               \
    DEF(HCP_SLICE_STATE);             \
    DEF(HCP_IND_OBJ_BASE_ADDR_STATE); \
    DEF(HCP_QM_STATE);                \
    DEF(HCP_BSD_OBJECT);              \
    DEF(HCP_TILE_STATE);              \
    DEF(HCP_REF_IDX_STATE);           \
    DEF(HCP_WEIGHTOFFSET_STATE);      \
    DEF(HCP_PIPE_MODE_SELECT);        \
    DEF(HCP_PIPE_BUF_ADDR_STATE);     \
    DEF(HCP_FQM_STATE);               \
    DEF(HCP_PAK_INSERT_OBJECT);       \
    DEF(HCP_VP9_PIC_STATE);           \
    DEF(HCP_VP9_SEGMENT_STATE);       \
    DEF(HEVC_VP9_RDOQ_STATE);         \
    DEF(HCP_TILE_CODING);             \
    DEF(HCP_PALETTE_INITIALIZER_STATE)

namespace mhw
{
namespace vdbox
{
namespace hcp
{
enum HevcSliceType
{
    hevcSliceB  = 0,
    hevcSliceP  = 1,
    hevcSliceI  = 2
};

class Itf
{
public:
    enum CommandsNumberOfAddresses
    {
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                        =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES    =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                    =  4, //  4 DW for  2 address fields
        MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address fields
        MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                          =  1, //  2 DW for  1 address field

        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                           =  0, //  0 DW for    address fields
    
        HCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        HCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES            = 45, //           45 address fields
        HCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES        = 11, // 22 DW for 11 address field
        HCP_QM_STATE_CMD_NUMBER_OF_ADDRESSES                       =  0, //  0 DW for    address fields
        HCP_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                      =  0, //  0 DW for    address fields
        HCP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                      =  0, //  0 DW for    address fields
        HCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        HCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES                    =  0, //  0 DW for    address fields
        HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        HCP_TILE_STATE_CMD_NUMBER_OF_ADDRESSES                     =  0, //  0 DW for    address fields
        HCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES                     =  0, //  0 DW for    address fields
        HCP_VP9_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        HCP_VP9_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES                =  1, //  0 DW for    address fields
        HCP_PALETTE_INITIALIZER_STATE_CMD_NUMBER_OF_ADDRESSES      =  0, //  0 DW for    address fields
    
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES          = 12, // 12 DW for 12 address fields
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for  0 address fields
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _HCP_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    virtual MOS_STATUS GetHcpBufSize(const HcpBufferSizePar &par, uint32_t &size) = 0;

    virtual MOS_STATUS GetVP9BufSize(const HcpBufferSizePar &par, uint32_t &size) = 0;

    virtual const HcpMmioRegisters *GetMmioRegisters(const MHW_VDBOX_NODE_IND index) const = 0;

    virtual uint32_t GetEncCuRecordSize() = 0;

    virtual uint32_t GetHcpPakObjSize() = 0;

    virtual bool IsRowStoreCachingSupported() = 0;

    virtual uint32_t GetPakHWTileSizeRecordSize() = 0;

    virtual MOS_STATUS SetRowstoreCachingOffsets(const HcpVdboxRowStorePar &rowstoreParams) = 0;

    virtual uint32_t   GetHcpVp9PicStateCommandSize() = 0;

    virtual uint32_t   GetHcpVp9SegmentStateCommandSize() = 0;

    virtual MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t *                      commandsSize,
        uint32_t *                      patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;

    virtual MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific) = 0;
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
    //! \brief    Judge if hevc sao row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcSaoRowstoreCacheEnabled()
    {
        return m_hevcSaoRowStoreCache.enabled ? true : false;
    }

    //!
    //! \brief    Judge if hevc df row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcDfRowstoreCacheEnabled()
    {
        return m_hevcDfRowStoreCache.enabled ? true : false;
    }

    //!
    //! \brief    Judge if hevc dat store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsHevcDatRowstoreCacheEnabled()
    {
        return m_hevcDatRowStoreCache.enabled ? true : false;
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
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ? (m_hevcBsdSliceType[sliceType] == hevcSliceI) : false;
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
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ? (m_hevcBsdSliceType[sliceType] == hevcSliceP) : false;
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
        return (sliceType < MHW_ARRAY_SIZE(m_hevcBsdSliceType)) ? (m_hevcBsdSliceType[sliceType] == hevcSliceB) : false;
    }

    bool IsVp9DfRowstoreCacheEnabled()
    {
        return m_vp9DfRowStoreCache.enabled ? true : false;
    }

    enum HevcSliceType
    {
        hevcSliceB = 0,
        hevcSliceP = 1,
        hevcSliceI = 2
    };

    MOS_STATUS GetVp9BufferSize(
        HCP_INTERNAL_BUFFER_TYPE bufferType,
        HcpBufferSizePar        *hcpBufSizeParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hcpBufSizeParam);

        uint32_t bufferSize            = 0;
        uint32_t dblkRsbSizeMultiplier = 0;
        uint32_t dblkCsbSizeMultiplier = 0;
        uint32_t intraPredMultiplier   = 0;

        uint8_t               maxBitDepth   = hcpBufSizeParam->ucMaxBitDepth;
        uint32_t              widthInSb     = hcpBufSizeParam->dwPicWidth;
        uint32_t              heightInSb    = hcpBufSizeParam->dwPicHeight;
        uint32_t              widthInMinCb  = widthInSb * 64 / 8;  //using smallest cb to get max width
        uint32_t              heightInMinCb = heightInSb * 64 / 8;
        HCP_CHROMA_FORMAT_IDC chromaFormat  = (HCP_CHROMA_FORMAT_IDC)hcpBufSizeParam->ucChromaFormat;
        uint32_t              maxFrameSize  = hcpBufSizeParam->dwMaxFrameSize;

        if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
        {
            dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 36 : 18;
            dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 34 : 17;
            intraPredMultiplier   = (maxBitDepth > 8) ? 4 : 2;
        }
        else if (chromaFormat == HCP_CHROMA_FORMAT_YUV444)
        {
            dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 54 : 27;
            dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 50 : 25;
            intraPredMultiplier   = (maxBitDepth > 8) ? 6 : 3;
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("Format not supported.");
            return eStatus;
        }

        switch (bufferType)
        {
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE:
            bufferSize = widthInSb * dblkRsbSizeMultiplier * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL:
            bufferSize = heightInSb * dblkCsbSizeMultiplier * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE:
            bufferSize = widthInSb * 5 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL:
            bufferSize = heightInSb * 5 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::CURR_MV_TEMPORAL:
        case HCP_INTERNAL_BUFFER_TYPE::COLL_MV_TEMPORAL:
            bufferSize = widthInSb * heightInSb * 9 * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::SEGMENT_ID:
            bufferSize = widthInSb * heightInSb * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::HVD_LINE:
        case HCP_INTERNAL_BUFFER_TYPE::HVD_TILE:
            bufferSize = widthInSb * MHW_CACHELINE_SIZE;
            break;
            //scalable mode specific buffers
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_UP_RIGHT_COL:
        case HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_LFT_RECON_COL:
            bufferSize = intraPredMultiplier * heightInSb * MHW_CACHELINE_SIZE;
            break;
        case HCP_INTERNAL_BUFFER_TYPE::CABAC_STREAMOUT:
            //From sas, cabac stream out buffer size =
            //(#LCU) in picture * (Worst case LCU_CU_TU_info) + 1 byte aligned per LCU + Bitstream Size * 3
            if ((chromaFormat == HCP_CHROMA_FORMAT_YUV420) && (maxBitDepth == 8))
            {
                bufferSize = widthInMinCb * heightInMinCb * m_hcpWorstCaseCuTuInfo + widthInMinCb * heightInMinCb + maxFrameSize * 3;
            }
            else
            {
                bufferSize = widthInMinCb * heightInMinCb * m_hcpWorstCaseCuTuInfoRext + widthInMinCb * heightInMinCb + maxFrameSize * 3;
            }
            bufferSize = MOS_ALIGN_CEIL(bufferSize, MHW_CACHELINE_SIZE);
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        hcpBufSizeParam->dwBufferSize = bufferSize;

        return eStatus;
    }

    inline uint32_t GetOpLength(uint32_t uiLength) 
    {
        #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
        #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
        #define __CODEGEN_OP_LENGTH_BIAS 2
        #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

        return __CODEGEN_OP_LENGTH(uiLength);
    }

    protected:
        RowStoreCache m_hevcDatRowStoreCache  = {};
        RowStoreCache m_hevcDfRowStoreCache   = {};
        RowStoreCache m_hevcSaoRowStoreCache  = {};
        RowStoreCache m_hevcHSaoRowStoreCache = {};
        RowStoreCache m_vp9HvdRowStoreCache   = {};
        RowStoreCache m_vp9DfRowStoreCache    = {};
        RowStoreCache m_vp9DatRowStoreCache   = {};

        static const uint32_t m_hcpCabacErrorFlagsMask = 0x0879;  //<! Hcp CABAC error flags mask

        static const HevcSliceType m_hevcBsdSliceType[3];  //!< HEVC Slice Types for Long Format

        static const uint32_t m_hcpWorstCaseCuTuInfo = 4 * MHW_CACHELINE_SIZE;

        static const uint32_t m_hcpWorstCaseCuTuInfoRext = 6 * MHW_CACHELINE_SIZE;

    _HCP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
MEDIA_CLASS_DEFINE_END(mhw__vdbox__hcp__Itf)
};
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_ITF_H__
