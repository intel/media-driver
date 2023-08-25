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
//! \file     encode_vp9_hpu.h
//! \brief    Defines the common interface for vp9 encode hpu (header probabiloity update) features
//!
#ifndef __ENCODE_VP9_HPU_H__
#define __ENCODE_VP9_HPU_H__

#include "encode_vp9_basic_feature.h"
#include "media_vp9_packet_defs.h"
#include "codec_def_vp9_probs.h"

namespace encode
{

class Vp9EncodeHpu : public MediaFeature, public mhw::vdbox::huc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9EncodeHpu feature constructor
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
    Vp9EncodeHpu(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    //!
    //! \brief  Vp9EncodeHpu feature destructor
    //!
    virtual ~Vp9EncodeHpu() {}

    //!
    //! \brief  Init CQP basic features related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update CQP basic features related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Set regions for huc prob
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegions(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const;

    //!
    //! \brief  Get probability buffer
    //! \param  [in] idx
    //!         Index of the probability buffer
    //! \param  [out] buffer
    //!         Reference to the buffer get from Brc feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetProbabilityBuffer(
        uint32_t       idx,
        PMOS_RESOURCE &buffer);

    //!
    //! \brief  Get huc probability dmem buffer
    //! \param  [in] idx
    //!         Index of the huc probability dmem buffer
    //! \param  [out] buffer
    //!         Reference to the buffer get from Brc feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucProbDmemBuffer(
        uint32_t       idx,
        PMOS_RESOURCE &buffer);

    //!
    //! \brief  Set Last Pass flag
    //! \param  [in] bool
    //!         Last Pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetIsLastPass(bool isLastPass);

    //!
    //! \brief    Set default tx probs
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultTxProbs(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt) const;

    //!
    //! \brief    Set default coeff probs
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultCoeffProbs(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt) const;

    //!
    //! \brief    Set default mb skip probs
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultMbskipProbs(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt) const;

    //!
    //! \brief    Populate prob values which are different between Key and Non-Key frame
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CtxBufDiffInit(
        uint8_t *ctxBuffer,
        bool     setToKey) const;

    //!
    //! \brief    Set default inter mode probs
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultInterModeProbs(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default switchable interprediction Prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultSwitchableInterpProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default intra-inter prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultIntraInterProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default comp inter prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultCompInterProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default single reference prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultSingleRefProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default comp reference prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultCompRefProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default Y mode prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultYModeProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default partition prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultPartitionProb(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default NMV prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultNmvContext(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Set default UV mode prob
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in,out] byteCnt
    //!           Numbe rof bytes counter
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDefaultUVModeProbs(
        uint8_t  *ctxBuffer,
        uint32_t &byteCnt,
        bool      setToKey) const;

    //!
    //! \brief    Init context buffer
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ContextBufferInit(
        uint8_t *ctxBuffer,
        bool     setToKey) const;

    //!
    //! \brief  Refresh frame interlnal buffers
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RefreshFrameInternalBuffers();

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources() override;

    static constexpr uint32_t m_probabilityCounterBufferSize = 193 * CODECHAL_CACHELINE_SIZE;
    static const uint32_t     m_probDmem[320];

    EncodeAllocator *     m_allocator    = nullptr;
    Vp9BasicFeature *     m_basicFeature = nullptr;

    // HuC Prob resoruces/buffers
    MOS_RESOURCE m_resProbabilityDeltaBuffer             = {0};                        //!< Probability delta buffer
    MOS_RESOURCE m_resProbabilityCounterBuffer           = {0};                        //!< Probability counter buffer
    MOS_RESOURCE m_resHucProbDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][3] = {0}; //!< VDENC HuC Prob DMEM buffer
    MOS_RESOURCE m_resHucProbOutputBuffer                = {0};                        //!< HuC Prob output buffer
    MOS_RESOURCE m_resProbBuffer[CODEC_VP9_NUM_CONTEXTS] = {0};                        //!< Probability buffer

    mutable bool m_isLastPass = false;

    //!
    //! \struct    CompressedHeader
    //! \brief     Compressed header
    //!
    struct CompressedHeader
    {
        union
        {
            struct
            {
                uint8_t valid : 1;         // valid =1, invalid = 0
                uint8_t bin_probdiff : 1;  // 1= bin, 0 = prob diff
                uint8_t prob : 1;          // 0 = 128, 1 = 252
                uint8_t bin : 1;
                uint8_t reserved : 4;
            } fields;
            uint8_t value;
        };
    };

    /* BinIdx for compressed header generation for PAK */
    /* The first value indicates previous SE index and second value indicates the size of the previous SE*/
    static constexpr uint32_t PAK_TX_MODE_IDX                 = 0;                                     //idx=0
    static constexpr uint32_t PAK_TX_MODE_SELECT_IDX          = (PAK_TX_MODE_IDX + 2);                 //idx=2
    static constexpr uint32_t PAK_TX_8x8_PROB_IDX             = (PAK_TX_MODE_SELECT_IDX + 1);          //idx=3
    static constexpr uint32_t PAK_TX_16x16_PROB_IDX           = (PAK_TX_8x8_PROB_IDX + 4);             //idx=7
    static constexpr uint32_t PAK_TX_32x32_PROB_IDX           = (PAK_TX_16x16_PROB_IDX + 8);           //idx=15
    static constexpr uint32_t PAK_TX_4x4_COEFF_PROB_IDX       = (PAK_TX_32x32_PROB_IDX + 12);          //idx=27
    static constexpr uint32_t PAK_TX_8x8_COEFF_PROB_IDX       = (PAK_TX_4x4_COEFF_PROB_IDX + 793);     //idx=820
    static constexpr uint32_t PAK_TX_16x16_COEFF_PROB_IDX     = (PAK_TX_8x8_COEFF_PROB_IDX + 793);     //idx=1613
    static constexpr uint32_t PAK_TX_32x32_COEFF_PROB_IDX     = (PAK_TX_16x16_COEFF_PROB_IDX + 793);   //idx=2406
    static constexpr uint32_t PAK_SKIP_CONTEXT_IDX            = (PAK_TX_32x32_COEFF_PROB_IDX + 793);   //idx=3199
    static constexpr uint32_t PAK_INTER_MODE_CTX_IDX          = (PAK_SKIP_CONTEXT_IDX + 6);            //idx=3205
    static constexpr uint32_t PAK_SWITCHABLE_FILTER_CTX_IDX   = (PAK_INTER_MODE_CTX_IDX + 42);         //idx=3247
    static constexpr uint32_t PAK_INTRA_INTER_CTX_IDX         = (PAK_SWITCHABLE_FILTER_CTX_IDX + 16);  //idx=3263
    static constexpr uint32_t PAK_COMPOUND_PRED_MODE_IDX      = (PAK_INTRA_INTER_CTX_IDX + 8);         //idx=3271
    static constexpr uint32_t PAK_HYBRID_PRED_CTX_IDX         = (PAK_COMPOUND_PRED_MODE_IDX + 2);      //idx=3273
    static constexpr uint32_t PAK_SINGLE_REF_PRED_CTX_IDX     = (PAK_HYBRID_PRED_CTX_IDX + 10);        //idx=3283
    static constexpr uint32_t PAK_CMPUND_PRED_CTX_IDX         = (PAK_SINGLE_REF_PRED_CTX_IDX + 20);    //idx=3303
    static constexpr uint32_t PAK_INTRA_MODE_PROB_CTX_IDX     = (PAK_CMPUND_PRED_CTX_IDX + 10);        //idx=3313
    static constexpr uint32_t PAK_PARTITION_PROB_IDX          = (PAK_INTRA_MODE_PROB_CTX_IDX + 72);    //idx=3385
    static constexpr uint32_t PAK_MVJOINTS_PROB_IDX           = (PAK_PARTITION_PROB_IDX + 96);         //idx=3481
    static constexpr uint32_t PAK_MVCOMP0_IDX                 = (PAK_MVJOINTS_PROB_IDX + 24);          //idx=3505
    static constexpr uint32_t PAK_MVCOMP1_IDX                 = (PAK_MVCOMP0_IDX + 176);               //idx=3681
    static constexpr uint32_t PAK_MVFRAC_COMP0_IDX            = (PAK_MVCOMP1_IDX + 176);               //idx=3857
    static constexpr uint32_t PAK_MVFRAC_COMP1_IDX            = (PAK_MVFRAC_COMP0_IDX + 72);           //idx=3929
    static constexpr uint32_t PAK_MVHP_COMP0_IDX              = (PAK_MVFRAC_COMP1_IDX + 72);           //idx=4001
    static constexpr uint32_t PAK_MVHP_COMP1_IDX              = (PAK_MVHP_COMP0_IDX + 16);             //idx=4017
    static constexpr uint32_t PAK_COMPRESSED_HDR_SYNTAX_ELEMS = (PAK_MVHP_COMP1_IDX + 16);             //=4033

    bool    m_clearAllToKey[CODEC_VP9_NUM_CONTEXTS]               = {false};
    bool    m_isPreCtx0InterProbSaved                             = false;
    uint8_t m_preCtx0InterProbSaved[CODECHAL_VP9_INTER_PROB_SIZE] = {0};

    //!
    //! \enum     PRED_MODE
    //! \brief    Pred mode
    //!
    enum PRED_MODE
    {
        PRED_MODE_SINGLE   = 0,
        PRED_MODE_COMPOUND = 1,
        PRED_MODE_HYBRID   = 2
    };

    //!
    //! \brief      Put data for compressed header
    //!
    //! \param      [in] compressedHdr
    //!             Compressed header
    //! \param      [in] bit
    //!             Bit
    //! \param      [in] prob
    //!             Prob
    //! \param      [in] binIdx
    //!             Bin index
    //!
    void PutDataForCompressedHdr(
        CompressedHeader *compressedHdr,
        uint32_t          bit,
        uint32_t          prob,
        uint32_t          binIdx);

MEDIA_CLASS_DEFINE_END(encode__Vp9EncodeHpu)
};

}  // namespace encode

#endif  // __ENCODE_VP9_CQP_H__
