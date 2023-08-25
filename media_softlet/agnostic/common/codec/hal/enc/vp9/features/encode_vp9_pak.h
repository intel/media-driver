/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_vp9_pak.h
//! \brief    Defines the common interface for vp9 vdenc\pak features
//!
#ifndef __ENCODE_VP9_PAK_H__
#define __ENCODE_VP9_PAK_H__

#include "media_cmd_packet.h"
#include "encode_pipeline.h"
#include "encode_vp9_basic_feature.h"
#include "encode_vp9_pipeline.h"

#include "media_pipeline.h"
#include "encode_utils.h"

namespace encode
{
#define CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE    4
#define CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM (CODECHAL_ENCODE_RECYCLED_BUFFER_NUM * CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE) // for salability, need 1 buffer per pipe,
//!
//! \struct    CU_DATA
//! \brief     CU data
//!
struct CU_DATA
{
    // DW0
    uint32_t        cu_size : 2;
    uint32_t        Res_DW0_2_3 : 2;
    uint32_t        cu_part_mode : 2;    // 0=2Nx2N,1=2NxN,2=Nx2N,3=NxN(8x8 only)
    uint32_t        Res_DW0_6_7 : 2;
    uint32_t        intra_chroma_mode0 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW0_12_15 : 4;
    uint32_t        intra_chroma_mode1 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        cu_pred_mode0 : 1;    // 1=Intra,0=Inter
    uint32_t        cu_pred_mode1 : 1;
    uint32_t        Res_DW0_23_22 : 2;
    uint32_t        interpred_comp0 : 1;    // 0=single,1=compound
    uint32_t        interpred_comp1 : 1;
    uint32_t        Res_DW0_31_26 : 6;

    //DW1
    uint32_t        intra_mode0 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_4_7 : 4;
    uint32_t        intra_mode1 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_12_15 : 4;
    uint32_t        intra_mode2 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_20_23 : 4;
    uint32_t        intra_mode3 : 4;    // 0=DC,1=V,2=H,3=TM,4=D45,5=D135,6=D117,7=D153,8=D207,9=D63
    uint32_t        Res_DW1_28_31 : 4;

    //DW2
    int16_t        mvx_l0_part0 : 16;
    int16_t        mvy_l0_part0 : 16;

    //DW3
    int16_t        mvx_l0_part1 : 16;
    int16_t        mvy_l0_part1 : 16;

    //DW4
    int16_t        mvx_l0_part2 : 16;
    int16_t        mvy_l0_part2 : 16;

    //DW5
    int16_t        mvx_l0_part3 : 16;
    int16_t        mvy_l0_part3 : 16;

    //DW6
    int16_t        mvx_l1_part0 : 16;
    int16_t        mvy_l1_part0 : 16;

    //DW7
    int16_t        mvx_l1_part1 : 16;
    int16_t        mvy_l1_part1 : 16;

    //DW8
    int16_t        mvx_l1_part2 : 16;
    int16_t        mvy_l1_part2 : 16;

    //DW9
    int16_t        mvx_l1_part3 : 16;
    int16_t        mvy_l1_part3 : 16;

    //DW10
    uint32_t        refframe_part0_l0 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_2_3 : 2;
    uint32_t        refframe_part1_l0 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_6_7 : 2;
    uint32_t        refframe_part0_l1 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_10_11 : 2;
    uint32_t        refframe_part1_l1 : 2;    // 0=intra,1=last,2=golden,3=altref
    uint32_t        Res_DW10_14_15 : 2;
    uint32_t        round_part0 : 3;
    uint32_t        Res_DW10_19 : 1;
    uint32_t        round_part1 : 3;
    uint32_t        Res_DW10_23_31 : 9;

    //DW11
    uint32_t        tu_size0 : 2;
    uint32_t        tu_size1 : 2;
    uint32_t        Res_DW11_4_13 : 10;
    uint32_t        segidx_pred0 : 1;
    uint32_t        segidx_pred1 : 1;
    uint32_t        segidx_part0 : 3;
    uint32_t        segidx_part1 : 3;
    uint32_t        mc_filtertype_part0 : 2;
    uint32_t        mc_filtertype_part1 : 2;
    uint32_t        Res_DW11_26_31 : 6;

    uint32_t        Res_DW12 : 32;

    uint32_t        Res_DW13 : 32;

    uint32_t        Res_DW14 : 32;

    uint32_t        Res_DW15 : 32;

};
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CU_DATA)) == 16);

class Vp9EncodePak : public MediaFeature, 
    public mhw::vdbox::huc::Itf::ParSetting,
    public mhw::vdbox::hcp::Itf::ParSetting,
    public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9EncodePak feature constructor
    //!
    //! \param  [in] featureManager
    //!         Pointer to MediaFeatureManager
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] constSettings
    //!         Pointer to const settings
    //!
    Vp9EncodePak(
        MediaFeatureManager *featureManager,
        EncodeAllocator     *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                *constSettings);

    //!
    //! \brief  Vp9EncodePak feature destructor
    //!
    virtual ~Vp9EncodePak() {}

    //!
    //! \brief  Init vdenc pak features related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update vdenc pak features related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Set regions for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \param  [in] currPass
    //!         current pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsForBrcUpdate(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params,
        uint32_t                                    currPass) const;

    //!
    //! \brief  Set regions for huc prob
    //! \param  [in] params
    //!         Pointer to parameters
    //! \param  [in] currPass
    //!         current pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsForHucProb(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params,
        uint32_t                                    currPass) const;

    //!
    //! \brief  Construct pic state batch buffer
    //! \param  [in] pipeline
    //!         Pointer to the pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructPicStateBatchBuffer(
        EncodePipeline *pipeline);

    //!
    //! \brief  Construct pak insert object batch buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructPakInsertObjBatchBuffer();

    //!
    //! \brief    Set huc pak insert object batch buffer
    //! \param    [in, out] secondLevelBatchBuffer
    //!           Pointer to huc pak insert object batch buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetHucPakInsertObjBatchBuffer(
        MHW_BATCH_BUFFER &secondLevelBatchBuffer);

    //!
    //! \brief  Set vdenc second level batch buffer
    //! \param  [in] currPass
    //!         Current pass
    //! \param  [in, out] secondLevelBatchBuffer
    //!         Second level batch buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencSecondLevelBatchBuffer(
        uint32_t          currPass,
        MHW_BATCH_BUFFER &secondLevelBatchBuffer);

    //!
    //! \brief    Get vdenc picture state second level batch buffer size
    //! \param  [out] size
    //!         Vdenc picture state second level batch buffer size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVdencPictureState2ndLevelBatchBufferSize(
        uint32_t &size);

    //!
    //! \brief  Update parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateParameters();

    //!
    //! \brief      Pak construct picture state batch buffer
    //!
    //! \param      [in] picStateBuffer
    //!             Pointer to MOS surface
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PakConstructPicStateBatchBuffer(
        PMOS_RESOURCE picStateBuffer);

    //!
    //! \brief  Get compressed header buffer
    //! \return Pointer to the buffer
    //!
    PMOS_RESOURCE GetCompressedHeaderBuffer();

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_PAK_INSERT_OBJECT);
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    //!
    //! \brief    Prepare PAK object and CU record for dynamic scaling
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PreparePakObjAndCuRecord();

    //!
    //! \brief  Add the VDenc CMD1 command.
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencCmd1Command(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add the VDenc CMD2 command.
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencCmd2Command(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Calculate QP Lambda values
    //! \param  sadQpLambda
    //!         SAD QP Lambda value
    //! \param  rdQpLambda
    //!         RD QP Lambda value
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void CalculateQpLambdaValues(double &sadQpLambda, double &rdQpLambda);

    CodechalHwInterfaceNext *   m_hwInterface    = nullptr;
    EncodeAllocator *       m_allocator      = nullptr;
    Vp9BasicFeature *       m_basicFeature   = nullptr;

    std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpInterfaceNew   = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencInterfaceNew = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf             = nullptr;

    // Picture state resources
    bool     m_picState2ndLevelBBConstructed[3];             //!< Indicated picture state 2nd level batch buffer has been contructed
    uint32_t m_vdencPicStateSecondLevelBatchBufferSize = 0;  //!< VDENC picture state second level batch buffer size
    uint16_t m_vdencPictureState2ndLevelBBIndex        = 0;
    uint16_t m_lastVdencPictureState2ndLevelBBIndex    = 0;

    MOS_RESOURCE m_resVdencPictureState2ndLevelBatchBufferRead[3][CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM] = {0};
    MOS_RESOURCE m_resVdencPictureState2ndLevelBatchBufferWrite[CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM]   = {0};

    // Vdenc/Pak resources
    MOS_RESOURCE m_resHucPakInsertUncompressedHeaderReadBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]  = {0};  //!< Huc VP9 pak insert uncompressed header read buffer
    MOS_RESOURCE m_resHucPakInsertUncompressedHeaderWriteBuffer                                      = {0};  //!< Huc VP9 pak insert uncompressed header write buffer

    MOS_RESOURCE m_resCompressedHeaderBuffer   = {0};  //!< Compressed heander buffer
    MOS_RESOURCE m_resVdencDataExtensionBuffer = {0};  //!< Data extension buffer

    // PAK resources
    PMOS_RESOURCE m_resMetadataLineBuffer       = nullptr;  //!< Metadata line data buffer
    PMOS_RESOURCE m_resMetadataTileLineBuffer   = nullptr;  //!< Metadata tile line data buffer
    PMOS_RESOURCE m_resMetadataTileColumnBuffer = nullptr;  //!< Metadata tile column data buffer

    // DYS
    MOS_RESOURCE m_resVdencDysPictureState2ndLevelBatchBuffer = {0};

MEDIA_CLASS_DEFINE_END(encode__Vp9EncodePak)
};

}  // namespace encode

#endif  // __ENCODE_VP9_PAK_H__