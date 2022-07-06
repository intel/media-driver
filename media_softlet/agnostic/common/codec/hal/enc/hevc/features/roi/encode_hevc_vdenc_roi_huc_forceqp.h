/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_vdenc_huc_forceqp.h
//! \brief    Defines of the HUC ForceQP of HEVC VDENC
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_HUC_FORCEQP_H__
#define __CODECHAL_HEVC_VDENC_ROI_HUC_FORCEQP_H__

#include "encode_hevc_vdenc_roi_strategy.h"

namespace encode
{

class HucForceQpROI : public RoiStrategy
{
public:
    HucForceQpROI(
        EncodeAllocator *allocator,
        MediaFeatureManager *featureManager,
        PMOS_INTERFACE osInterface);

    virtual ~HucForceQpROI() {}

    //!
    //! \brief    Setup the ROI regione
    //!
    //! \param    [in] overlap
    //!           Overlap between ROI and dirty ROI
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupRoi(RoiOverlap &overlap) override;

    //!
    //! \brief    Set VDENC_PIPE_BUF_ADDR parameters
    //!
    //! \param    [in] streamIn
    //!           Stream in buffer
    //! \param    [out] PipeBufAddrParams
    //!           Pipe buf addr parameters
    //!
    //! \return   void
    //!
    void SetVdencPipeBufAddrParams(PMOS_RESOURCE streamin,
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams) override
    {
        pipeBufAddrParams.presVdencStreamInBuffer = m_hucRoiOutput;
        return;
    }

    PMOS_RESOURCE GetStreamInBuf() const override { return m_hucRoiOutput; }

    //!
    //! \brief    Setup HuC BRC init/reset parameters
    //!
    //! \param    [out] hucVdencBrcInitDmem
    //!           pointer of PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemHuCBrcInitReset(
        VdencHevcHucBrcInitDmem *hucVdencBrcInitDmem) override
    {
        ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);

        hucVdencBrcInitDmem->StreamInROIEnable_U8     = 1;
        hucVdencBrcInitDmem->StreamInSurfaceEnable_U8 = 1;
        return MOS_STATUS_SUCCESS;
    }

private:
    static constexpr uint32_t m_roiStreamInBufferSize = 
        65536 * CODECHAL_CACHELINE_SIZE; //!< ROI Streamin buffer size (part of BRC Update)

    static constexpr uint32_t m_deltaQpBufferSize = 65536;

    
    uint32_t m_deltaQpRoiBufferSize = m_deltaQpBufferSize; //!< VDEnc DeltaQp for ROI buffer size
    uint32_t m_HucForceQpROIBufferSize = m_roiStreamInBufferSize; //!< BRC ROI input buffer size

    MOS_RESOURCE *m_deltaQpBuffer = nullptr;
    MOS_RESOURCE *m_hucRoiOutput = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HucForceQpROI)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_HUC_FORCEQP_H__