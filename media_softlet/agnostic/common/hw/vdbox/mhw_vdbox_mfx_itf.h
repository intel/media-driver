/*
* Copyright (c) 2021, Intel Corporation
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
    bool IsVPlanePresent(MOS_FORMAT format)
    {
        switch (format)
        {
            case Format_NV12:
            case Format_NV11:
            case Format_P208:
            case Format_IMC1:
            case Format_IMC3:
            case Format_YUY2:
            case Format_YUYV:
            case Format_YVYU:
            case Format_UYVY:
            case Format_VYUY:
            case Format_422H:
            case Format_422V:
                // Adding RGB formats because RGB is treated like YUV for JPEG encode and decode
            case Format_RGBP:
            case Format_BGRP:
            case Format_A8R8G8B8:
            case Format_X8R8G8B8:
            case Format_A8B8G8R8:
            case Format_411P:
            case Format_411R:
            case Format_444P:
            case Format_IMC2:
            case Format_IMC4:
                return true;
            default:
                return false;
        }
    }

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

    uint8_t MosToMediaStateFormat(MOS_FORMAT format)
    {
        switch (format)
        {
            case Format_A8R8G8B8:
            case Format_X8R8G8B8:
            case Format_A8B8G8R8:
                return MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
            case Format_422H:
            case Format_422V:
                return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8;
            case Format_AYUV:
            case Format_AUYV:
                return MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM;
            case Format_NV12:
            case Format_NV11:
            case Format_P208:
            case Format_IMC1:
            case Format_IMC3:
                return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8;
            case Format_400P:
            case Format_P8:
                return MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM;
            case Format_411P:
            case Format_411R:
                return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8;
            case Format_UYVY:
                return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY;
            case Format_YVYU:
                return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV;
            case Format_VYUY:
                return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY;
            case Format_YUY2:
            case Format_YUYV:
            case Format_444P:
            case Format_IMC2:
            case Format_IMC4:
            default:
                return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
        }

        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
    }

    MHW_VDBOX_DECODE_JPEG_FORMAT_CODE GetJpegDecodeFormat(MOS_FORMAT format)
    {
        switch (format)
        {
        case Format_NV12:
            return MHW_VDBOX_DECODE_JPEG_FORMAT_NV12;
        case Format_UYVY:
            return MHW_VDBOX_DECODE_JPEG_FORMAT_UYVY;
        case Format_YUY2:
            return MHW_VDBOX_DECODE_JPEG_FORMAT_YUY2;
        default:
            return MHW_VDBOX_DECODE_JPEG_FORMAT_SEPARATE_PLANE;
        }

        return MHW_VDBOX_DECODE_JPEG_FORMAT_SEPARATE_PLANE;
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

    virtual MHW_VDBOX_NODE_IND GetMaxVdboxIndex() = 0;

    virtual uint8_t GetNumVdbox() = 0;

    virtual MOS_STATUS FindGpuNodeToUse(PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit) = 0;

    vdbox::RowStoreCache m_intraRowstoreCache            = {};  //!< Intra rowstore cache
    vdbox::RowStoreCache m_deblockingFilterRowstoreCache = {};  //!< Deblocking filter row store cache
    vdbox::RowStoreCache m_bsdMpcRowstoreCache           = {};  //!< BSD/MPC row store cache
    vdbox::RowStoreCache m_mprRowstoreCache              = {};  //!< MPR row store cache
    bool                 m_rowstoreCachingSupported      = false;
    MHW_VDBOX_NODE_IND   m_maxVdboxIndex                 = MHW_VDBOX_NODE_1;  //!< max vdbox index
    uint8_t              m_numVdbox                      = 1;                 //!< vdbox num
    bool                 m_scalabilitySupported          = false;             //!< Indicate if scalability supported
    _MFX_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

};
}//namespace mfx
}//namespace vdbox
}//namespace mhw

#endif  //__MHW_VDBOX_MFX_ITF_H__

