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
//! \file     mhw_vdbox_mfx_itf.h
//! \brief    MHW VDBOX MFX interface common base
//! \details
//!
#ifndef __MHW_VDBOX_MFX_ITF_H__
#define __MHW_VDBOX_MFX_ITF_H__
#include "mhw_itf.h"
#include "mhw_vdbox_mfx_cmdpar.h"
#include "mhw_vdbox.h"

#define AVC_MPR_ROWSTORE_BASEADDRESS 256
#define AVC_MPR_ROWSTORE_BASEADDRESS_MBAFF 512
#define AVC_IP_ROWSTORE_BASEADDRESS 512
#define AVC_IP_ROWSTORE_BASEADDRESS_MBAFF 1024
#define AVC_VLF_ROWSTORE_BASEADDRESS 768
#define VP8_IP_ROWSTORE_BASEADDRESS 256
#define VP8_VLF_ROWSTORE_BASEADDRESS 512

#define _MFX_CMD_DEF(DEF) \
        DEF(MFX_QM_STATE);\
        DEF(MFX_FQM_STATE);\
        DEF(MFX_PIPE_MODE_SELECT);\
        DEF(MFX_SURFACE_STATE);\
        DEF(MFX_PIPE_BUF_ADDR_STATE);\
        DEF(MFX_IND_OBJ_BASE_ADDR_STATE);\
        DEF(MFX_BSP_BUF_BASE_ADDR_STATE);\
        DEF(MFX_PAK_INSERT_OBJECT);\
        DEF(MFX_AVC_IMG_STATE);\
        DEF(MFX_AVC_REF_IDX_STATE);\
        DEF(MFX_AVC_WEIGHTOFFSET_STATE);\
        DEF(MFX_AVC_SLICE_STATE);\
        DEF(MFX_AVC_DIRECTMODE_STATE);\
        DEF(MFD_AVC_PICID_STATE);\
        DEF(MFD_AVC_DPB_STATE);\
        DEF(MFD_AVC_SLICEADDR);\
        DEF(MFD_AVC_BSD_OBJECT);\
        DEF(MFX_JPEG_PIC_STATE);\
        DEF(MFC_JPEG_HUFF_TABLE_STATE);\
        DEF(MFC_JPEG_SCAN_OBJECT);\
        DEF(MFD_JPEG_BSD_OBJECT);\
        DEF(MFX_JPEG_HUFF_TABLE_STATE);\
        DEF(MFX_MPEG2_PIC_STATE);\
        DEF(MFD_MPEG2_BSD_OBJECT);\
        DEF(MFX_VP8_PIC_STATE);\
        DEF(MFD_VP8_BSD_OBJECT);\
        DEF(MFD_IT_OBJECT);\
        DEF(MFD_IT_OBJECT_MPEG2_INLINE_DATA);

namespace mhw
{
namespace vdbox
{
namespace mfx
{
class Itf
{
public:
#define PATCH_LIST_COMMAND(x) (x##_NUMBER_OF_ADDRESSES)
    //!
    //! \enum     CommandsNumberOfAddresses
    //! \brief    Commands number of addresses
    //!
    enum CommandsNumberOfAddresses
    {
        // MFX Engine Commands
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           = 1,   //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,   //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,   //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,   //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,   //  2 DW for  1 address field
        MFX_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES            = 0,   //  0 DW for    address fields
        MFX_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
        MFX_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES         = 27,  // 50 DW for 25 address fields, added 2 for DownScaledReconPicAddr
        MFX_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 5,   // 10 DW for  5 address fields
        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                        = 0,   //  0 DW for    address fields
        MFX_BSP_BUF_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 3,   //  2 DW for  3 address fields
        MFD_AVC_PICID_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
        MFX_AVC_DIRECTMODE_STATE_CMD_NUMBER_OF_ADDRESSES        = 17,  // 50 DW for 17 address fields
        MFX_AVC_IMG_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
        MFX_QM_STATE_CMD_NUMBER_OF_ADDRESSES                    = 0,   //  0 DW for    address fields
        MFX_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                   = 0,   //  0 DW for    address fields
        MFX_MPEG2_PIC_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
        MFX_DBK_OBJECT_CMD_NUMBER_OF_ADDRESSES                  = 4,   //  2 DW for  4 address fields
        MFX_VP8_PIC_STATE_CMD_NUMBER_OF_ADDRESSES               = 2,   //  2 DW for  2 address fields
        MFX_AVC_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
        MFD_AVC_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
        MFD_AVC_DPB_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
        MFD_AVC_SLICEADDR_CMD_NUMBER_OF_ADDRESSES               = 0,   //  0 DW for    address fields
        MFX_AVC_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,   //  0 DW for    address fields
        MFX_AVC_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES      = 0,   //  0 DW for    address fields
        MFC_AVC_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES       = 0,   //  0 DW for    address fields
        MFD_MPEG2_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES            = 0,   //  0 DW for    address fields
        MFD_MPEG2_IT_OBJECT_CMD_NUMBER_OF_ADDRESSES             = 0,   //  0 DW for    address fields
        MFD_VP8_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
    };

    static __inline uint32_t MosGetHWTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
    {
        uint32_t tileMode = 0;

        if (gmmTileEnabled)
        {
            return tileModeGMM;
        }

        switch (tileType)
        {
            case MOS_TILE_LINEAR:
                tileMode = 0;
                break;
            case MOS_TILE_YS:
                tileMode = 1;
                break;
            case MOS_TILE_X:
                tileMode = 2;
                break;
            default:
                tileMode = 3;
                break;
        }
        return tileMode;
    }

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _MFX_CMD_DEF(_MHW_SETPAR_DEF);
    };
    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    virtual MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    virtual bool IsRowStoreCachingSupported() = 0;

    virtual bool IsDeblockingFilterRowstoreCacheEnabled() = 0;

    virtual bool IsIntraRowstoreCacheEnabled() = 0;

    virtual bool IsBsdMpcRowstoreCacheEnabled() = 0;

    virtual bool IsMprRowstoreCacheEnabled() = 0;

    virtual MHW_VDBOX_NODE_IND GetMaxVdboxIndex() = 0;

    virtual uint8_t GetNumVdbox() = 0;

    virtual MOS_STATUS FindGpuNodeToUse(PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit) = 0;

    virtual MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat) = 0;

    virtual MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isModeSpecific) = 0;

    inline uint32_t GetMfxErrorFlagsMask() { return m_mfxErrorFlagsMask; }

    vdbox::RowStoreCache m_intraRowstoreCache            = {};  //!< Intra rowstore cache
    vdbox::RowStoreCache m_deblockingFilterRowstoreCache = {};  //!< Deblocking filter row store cache
    vdbox::RowStoreCache m_bsdMpcRowstoreCache           = {};  //!< BSD/MPC row store cache
    vdbox::RowStoreCache m_mprRowstoreCache              = {};  //!< MPR row store cache
    bool                 m_rowstoreCachingSupported      = false;
    MHW_VDBOX_NODE_IND   m_maxVdboxIndex                 = MHW_VDBOX_NODE_1;  //!< max vdbox index
    uint8_t              m_numVdbox                      = 1;                 //!< vdbox num
    bool                 m_scalabilitySupported          = false;             //!< Indicate if scalability supported
    static const uint32_t m_mfxErrorFlagsMask            = 0xFBFF;            //!< Mfx error flags mask
    static const uint32_t m_mpeg2WeightScaleSize          = 16;                //!< Size of MPEG2 weight scale
                                                                               //!< Bit 10 of MFD_ERROR_STATUS register is set to a random value during RC6, so it is not used
    _MFX_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__Itf)
};
}//namespace mfx
}//namespace vdbox
}//namespace mhw

#endif  //__MHW_VDBOX_MFX_ITF_H__

