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
//! \file     encode_av1_stream_in.h
//! \brief    Defines the common interface for encode av1 stream in utility
//!
#ifndef __ENCODE_AV1_STREAM_IN_H__
#define __ENCODE_AV1_STREAM_IN_H__
#include "mhw_vdbox.h"
#include "encode_allocator.h"
#include "codec_def_encode_av1.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"

namespace encode
{
struct VdencStreamInState
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
            uint32_t MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
            uint32_t MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
            uint32_t NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
            uint32_t Reserved_0 : MOS_BITFIELD_RANGE(16, 20);
            uint32_t ForceQPDelta : MOS_BITFIELD_BIT(21);
            uint32_t PaletteDisable : MOS_BITFIELD_BIT(22);
            uint32_t Reserved_1 : MOS_BITFIELD_BIT(23);
            uint32_t PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW0;

    // DWORD 1-4
    union
    {
        struct
        {
            uint32_t ForceMvX : MOS_BITFIELD_RANGE(0, 15);
            uint32_t ForceMvY : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW1[4];

    // DWORD 5
    union
    {
        struct
        {
            uint32_t Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            uint32_t ForceRefIdx : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t NumMergeCandidateCu8x8 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t NumMergeCandidateCu16x16 : MOS_BITFIELD_RANGE(20, 23);
            uint32_t NumMergeCandidateCu32x32 : MOS_BITFIELD_RANGE(24, 27);
            uint32_t NumMergeCandidateCu64x64 : MOS_BITFIELD_RANGE(28, 31);
        };
        uint32_t Value;
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            uint32_t SegID : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t QpEnable : MOS_BITFIELD_RANGE(16, 19);
            uint32_t SegIDEnable : MOS_BITFIELD_RANGE(20, 20);
            uint32_t Reserved : MOS_BITFIELD_RANGE(21, 22);
            uint32_t ForceRefIdEnable : MOS_BITFIELD_RANGE(23, 23);
            uint32_t ImePredictorSelect : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW7;

    // DWORD 8-11
    union
    {
        struct
        {
            uint32_t ImePredictorMvX : MOS_BITFIELD_RANGE(0, 15);
            uint32_t ImePredictorMvY : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW8[4];

    // DWORD 12
    union
    {
        struct
        {
            uint32_t ImePredictorRefIdx : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            uint32_t PanicModeLCUThreshold : MOS_BITFIELD_RANGE(0, 15);
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            uint32_t ForceQp_0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t ForceQp_1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t ForceQp_2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t ForceQp_3 : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            uint32_t Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW15;

    inline bool operator==(const VdencStreamInState& ps) const
    {

        if ((this->DW0.Value == ps.DW0.Value) &&
            (this->DW1[0].Value == ps.DW1[0].Value) &&
            this->DW1[1].Value == ps.DW1[1].Value &&
            this->DW1[2].Value == ps.DW1[2].Value &&
            this->DW1[3].Value == ps.DW1[3].Value &&
            this->DW5.Value == ps.DW5.Value &&
            this->DW6.Value == ps.DW6.Value &&
            this->DW7.Value == ps.DW7.Value &&
            this->DW8[0].Value == ps.DW8[0].Value &&
            this->DW8[1].Value == ps.DW8[1].Value &&
            this->DW8[2].Value == ps.DW8[2].Value &&
            this->DW8[3].Value == ps.DW8[3].Value &&
            this->DW12.Value == ps.DW12.Value &&
            this->DW13.Value == ps.DW13.Value &&
            this->DW14.Value == ps.DW14.Value &&
            this->DW15.Value == ps.DW15.Value)
            return true;
        return false;
    }
};

struct CommonStreamInParams
{
    uint8_t MaxCuSize;
    uint8_t MaxTuSize;
    uint8_t NumImePredictors;
    uint8_t NumMergeCandidateCu8x8;
    uint8_t NumMergeCandidateCu16x16;
    uint8_t NumMergeCandidateCu32x32;
    uint8_t NumMergeCandidateCu64x64;
};

inline uint32_t AlignRectCoordFloor(uint32_t blockSize, uint32_t coord) { return blockSize * ((coord % 2 != 0) ? coord - 1 : coord); }
inline uint32_t AlignRectCoordCeil(uint32_t blockSize, uint32_t coord) { return blockSize * ((coord % 2 != 0) ? coord + 1 : coord); }

class Av1BasicFeature;
class Av1StreamIn : public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    //!
    //! \brief  Av1StreamIn constructor
    //!
    Av1StreamIn() {};

    //!
    //! \brief  Av1StreamIn deconstructor
    //!
    virtual ~Av1StreamIn();

    //!
    //! \brief  Init stream in instance
    //! \param  [in] basicFeature
    //!         Pointer to basic feature
    //! \param  [in] allocator
    //!         Pointer to allocator
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(Av1BasicFeature *basicFeature, EncodeAllocator *allocator, PMOS_INTERFACE osInterface);

    //!
    //! \brief  update stream in buffer for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update();

    //!
    //! \brief  Get stream in buffer offset for each CU32x32
    //! \return uint32_t
    //!         offset for each cu
    //!
    uint32_t GetCuOffset(uint32_t xIdx, uint32_t yIdx) const;

    //!
    //! \brief  Get stream in buffer base locked addrress
    //! \return VdencStreamInState*
    //!         pointer to stream in buffer locked address
    //!
    virtual VdencStreamInState *GetStreamInBuffer();

    //!
    //! \brief  Return stream in buffer base locked addrress
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReturnStreamInBuffer();

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    //!
    //! \brief  Reset stream in after frame programming is done
    //!
    virtual void Reset();

    const CommonStreamInParams& GetCommonParams() const;

    static const uint8_t m_streamInBlockSize = 32;              //!< size of stream in block in one dimension

protected:
    //!
    //! \brief  Set up LCU map for stream in
    //! \param  [in] params
    //!         Pointer to encode parameter
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupLCUMap();

    //!
    //! \brief  Get LCU Addr for each (x, y)
    //! \param  [in] x
    //!         horizontal coordination for current block
    //! \param  [in] y
    //!         vetical coordination for current blockn
    //! \return uint32_t
    //!         LCU offset for current block
    //!
    uint32_t GetLCUAddr(uint32_t x, uint32_t y) const;

    //!
    //! \brief  Initialize all stream in blocks
    //! \param  [in] streamInBuffer
    //!         pointer to stream in buffer locked address
    //!
    MOS_STATUS StreamInInit(uint8_t *streamInBuffer);

    Av1BasicFeature *m_basicFeature = nullptr;        //!< AV1 paramter
    EncodeAllocator *m_allocator = nullptr;           //!< Encode allocator
    PMOS_INTERFACE   m_osInterface    = nullptr;      //!< Pointer to OS interface
    PMOS_RESOURCE    m_streamInBuffer = nullptr;      //!< Stream in buffer
    bool             m_enabled = false;               //!< Indicate stream in enabled or not
    bool             m_initialized = false;           //!< Indicate stream in buffer initialized or not

    uint32_t m_widthInLCU = 0;           //!< Frame width in LCU unit
    uint32_t m_heightInLCU = 0;          //!<  Frame height in LCU unit
    static const uint8_t m_num32x32BlocksInLCU = 4;             //!< Number of 32x32 blocks in one LCU
    static const uint8_t m_num32x32BlocksInLCUOnedimension = 2; //!< Number of 32x32 blocks in one LCU in one dimension

    uint32_t *m_LcuMap = nullptr;

    CommonStreamInParams m_commonPar = {};

    uint8_t *m_streamInTemp = nullptr;
    uint32_t m_streamInSize = 0;

MEDIA_CLASS_DEFINE_END(encode__Av1StreamIn)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_STREAM_IN_H__
