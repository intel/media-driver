/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_reference_frames.h
//! \brief    Defines reference list related logic for encode vp9
//!
#ifndef __ENCODE_VP9_REFERENCE_FRAMES_H__
#define __ENCODE_VP9_REFERENCE_FRAMES_H__

#include "codec_def_common.h"
#include "codec_def_common_vp9.h"
#include "codec_def_encode_vp9.h"
#include "mhw_vdbox.h"
#include "encode_utils.h"
#include "mhw_vdbox_hcp_itf.h"
#include "mhw_vdbox_vdenc_itf.h"

namespace encode
{

class Vp9VdencPipeline;

//!
//! \enum     DYS_REF_FLAGS
//! \brief    DYS reference flags
//!
enum DYS_REF_FLAGS
{
    DYS_REF_NONE   = 0,
    DYS_REF_LAST   = (1 << 0),
    DYS_REF_GOLDEN = (1 << 1),
    DYS_REF_ALT    = (1 << 2),
};

class Vp9BasicFeature;

class Vp9ReferenceFrames : public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9ReferenceFrames constructor
    //!
    Vp9ReferenceFrames(){};

    //!
    //! \brief  Vp9ReferenceFrames destructor
    //!
    ~Vp9ReferenceFrames();

    //!
    //! \brief  Initialize reference frame
    //! \param  [in] params
    //!         Pointer to Vp9BasicFeature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Vp9BasicFeature *basicFeature);

    //!
    //! \brief  Update reference frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update();

    //!
    //! \brief  Get picture index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return CODEC_PIC_ID
    //!         CODEC_PIC_ID refer to the picture index
    //!
    CODEC_PIC_ID GetPicIndex(uint8_t idx) { return m_picIdx[idx]; };

    //!
    //! \brief  Get reference list index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return uint8_t
    //!         reference list index
    //!
    uint8_t GetRefListIndex(uint8_t idx) { return m_picIdx[idx].ucPicIdx; };

    //!
    //! \brief  Get current reference list
    //! \return PCODEC_REF_LIST
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST GetCurrRefList() { return m_currRefList; };

    //!
    //! \brief  Get reference list
    //! \return PCODEC_REF_LIST*
    //!         Pointer of reference list
    //!
    PCODEC_REF_LIST* GetRefList() { return m_refList; };

    //!
    //! \brief  Set Hcp surface parameters
    //! \param  [in] surfaceParams
    //!         Pointer to MHW_VDBOX_SURFACE_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetHcpSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams);

    //!
    //! \brief  Set Hcp surface parameters (DYS)
    //! \param  [in] surfaceParams
    //!         Pointer to MHW_VDBOX_SURFACE_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetDysHcpSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams);

    //!
    //! \brief  Set Hcp pipe buffer address parameter (DYS)
    //! \param  [in] pipeBufAddrParams
    //!         Pointer to MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetDysHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS *pipeBufAddrParams);

    //!
    //! \brief  Set Vdenc surface parameters
    //! \param  [in] surfaceParams
    //!         Pointer to MHW_VDBOX_SURFACE_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVdencSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams);

    //!
    //! \brief  Set VDEnc pipe buffer address parameter
    //! \param  [in] pipeBufAddrParams
    //!         Pointer to MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS *pipeBufAddrParams);

    //!
    //! \brief  Reference frame flags
    //! \return Refefence frame flags
    //!
    uint8_t RefFrameFlags() { return m_refFrameFlags; }

    //!
    //! \brief  Number of reference frames
    //! \return Number of reference frames
    //!
    uint8_t NumRefFrames() const { return m_numRefFrames; }

    //!
    //! \brief  Dynamic scaling reference frame flags
    //! \return Dynamic scaling Refefence frame flags
    //!
    uint8_t DysRefFrameFlags() const { return m_dysRefFrameFlags; }

    //!
    //! \brief  Set dynamic scaling reference frame flags
    //! \param  [in] dysRefFrameFlags
    //!         Dynamic scaling reference frame flags
    //!
    void SetDysRefFrameFlags(uint8_t dysRefFrameFlags) { m_dysRefFrameFlags = dysRefFrameFlags; }

    //!
    //! \brief  Get current dynamic scaling reference list
    //! \return PCODEC_REF_LIST
    //!         Pointer of current dynamic scaling reference list
    //!
    PCODEC_REF_LIST GetCurrDysRefList() { return m_currDysRefList; };

    //!
    //! \brief  Get dynamic scaling reference index.
    //!         For CU_DATA use, 0=intra, 1=Last, 2=Golden, 3=Alt
    //! \return uint8_t
    //!         Dynamic scaling reference list index
    //!
    uint8_t GetDysRefIndex() { return m_dysRefIndex; };

    //!
    //! \brief  Dump input resources or infomation before submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpInput(Vp9VdencPipeline *pipeline);

    //!
    //! \brief  Set DynamicScaling flag
    //! \param  [in] bool
    //!         Dynamic Scaling value
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDysValue(bool value);

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_VP9_PIC_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

protected:

    //!
    //! \brief   Set up internal reference frame flags
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupRefFlags();

    //!
    //! \brief   Set up internal reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupRefPic();

    //!
    //! \brief   Set up internal dynamic scaling reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupDysRefPic();

    //!
    //! \brief   Set up reference picture index array
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupRefIndex();

    uint32_t usePrevInFindMvRef() const;

    //!
    //! \enum     Vp9ReferenceId
    //! \brief    VP9 referebce frame identify
    //!
    enum Vp9ReferenceId
    {
        lastFrame = 0,
        goldenFrame,
        altFrame,
        maxReferenceIds
    };

    Vp9BasicFeature *m_basicFeature                                   = nullptr;  //!< VP9 parameter
    CODEC_PIC_ID     m_picIdx[CODEC_VP9_NUM_REF_FRAMES]               = {};       //!< Reference picture index array
    PCODEC_REF_LIST  m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9] = {};       //!< Pointer to reference pictures
    PCODEC_REF_LIST  m_currRefList                                    = nullptr;  //!< Current reference list
    PCODEC_REF_LIST  m_currDysRefList                                 = nullptr;  //!< Current dynamic scaling reference list

    // m_refFrameFlags is to indicate which frames to be used as reference
    // m_refFrameFlags & 0x01 != 0: Last ref frames used as reference
    // m_refFrameFlags & 0x02 != 0: Golden ref frames used as reference
    // m_refFrameFlags & 0x04 != 0: Alternate ref frames used as reference
    uint8_t m_refFrameFlags    = 0;  //!< m_refFrameFlags is to indicate which frames to be used as reference
    uint8_t m_numRefFrames     = 0;  //!< number of current used reference surfaces
    uint8_t m_dysRefFrameFlags = 0;  //!< Dynamic scaling reference frame flags
    uint8_t m_dysCurrFrameFlag = 0;  //!< Current dynamic scaling current reference frame flags
    uint8_t m_dysRefIndex      = 0;  //!< Dynamic reference index (for CU_DATA use, 0=intra, 1=Last, 2=Golden, 3=Alt)

    // pointer to the reference surfaces
    PMOS_SURFACE m_lastRefPic   = nullptr;
    PMOS_SURFACE m_goldenRefPic = nullptr;
    PMOS_SURFACE m_altRefPic    = nullptr;

    PMOS_SURFACE m_refSurface[maxReferenceIds]          = {};
    PMOS_SURFACE m_refSurfaceNonScaled[maxReferenceIds] = {};
    PMOS_SURFACE m_dsRefSurface4x[maxReferenceIds]      = {};
    PMOS_SURFACE m_dsRefSurface8x[maxReferenceIds]      = {};
    
    // Dynamic scaling reference surfaces
    PMOS_SURFACE m_dysRefSurface[maxReferenceIds] = {};

    mutable bool m_dysEnabled = false;
MEDIA_CLASS_DEFINE_END(encode__Vp9ReferenceFrames)
};

}  // namespace

#endif
