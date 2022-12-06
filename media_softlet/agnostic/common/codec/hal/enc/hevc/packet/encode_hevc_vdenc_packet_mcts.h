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
//! \file     encode_hevc_vdenc_packet_rsvd.h
//! \brief    Defines the interface to adapt to HEVC VDENC pipeline
//!


//#ifndef __CODECHAL_HEVC_VDENC_PACKET_RSVD_H__
//#define __CODECHAL_HEVC_VDENC_PACKET_RSVD_H__

#include "encode_hevc_vdenc_packet.h"

namespace encode
{
#define MINFRAMESIZE_OFFSET_RSVD  0x3D
static constexpr uint32_t OPCODE = 0x73a10000;
struct SIDE_BAND
{
    uint32_t packetEnd : 1;
    uint32_t realDw : 1;   // Not padded DWs for CL alignment
    uint32_t dwCount : 3;  // Reset at LCU boundary and CU boundaries
    uint32_t dwType : 2;   // 01: LCU Header Info, 10: CU Info, 00: Default
    uint32_t mbz : 9;
    uint32_t cuSad : 16;
};

struct LCU_HDR
{
    uint32_t  dw0;
    SIDE_BAND sb0;

    uint32_t  rsv0 : 30;
    uint32_t  lastCtbOfTileFlag : 1;
    uint32_t  lastCtbOfSliceFlag : 1;
    SIDE_BAND sb1;

    uint32_t  lcuX : 16;
    uint32_t  lcuY : 16;
    SIDE_BAND sb2;

    uint32_t  estimatedLcuSizeInBytes : 14;
    uint32_t  rsv1 : 18;
    SIDE_BAND sb3;

    uint32_t  rsv2 : 16;
    uint32_t  timeBudgetOverflowOccurred : 1;
    uint32_t  sliceEnd : 1;
    uint32_t  sliceStart : 1;
    uint32_t  reconSkip : 1;
    uint32_t  firstNonStaticRowOfTile : 1;
    uint32_t  rsv3 : 11;
    SIDE_BAND sb4;
};

struct CU_RECORD
{
    int16_t  l0Mv0X : 16;
    int16_t  l0Mv0Y : 16;
    SIDE_BAND sb0;

    int16_t  l0Mv1X : 16;
    int16_t  l0Mv1Y : 16;
    SIDE_BAND sb1;

    int16_t  l1Mv0X : 16;
    int16_t  l1Mv0Y : 16;
    SIDE_BAND sb2;

    int16_t  l1Mv1X : 16;
    int16_t  l1Mv1Y : 16;
    SIDE_BAND sb3;

    uint32_t  l0Mv0RefId : 4;
    uint32_t  l0Mv1RefId : 4;
    uint32_t  l1Mv0RefId : 4;
    uint32_t  l1Mv1RefId : 4;
    uint32_t  sseClassId32x32 : 4;
    uint32_t  EstimatedBytesPer32x32 : 12;
    SIDE_BAND sb4;

    uint32_t  tuSize;
    SIDE_BAND sb5;

    uint32_t  lumaIntraMode4x4_1 : 6;
    uint32_t  lumaIntraMode4x4_2 : 6;
    uint32_t  lumaIntraMode4x4_3 : 6;
    uint32_t  roundingSelect : 4;
    uint32_t  lastCuOf32x32 : 1;
    uint32_t  lastCuOfLcu : 1;
    uint32_t  tuCountM1 : 4;
    uint32_t  escapePresentFlag : 1;
    uint32_t  paletteTransposeFlag : 1;
    uint32_t  paletteMode : 1;
    uint32_t  ibcMode : 1;
    SIDE_BAND sb6;

    uint32_t  lumaIntraMode : 6;
    uint32_t  cuSize : 2;//8
    uint32_t  chromaIntraMode : 3;//11
    uint32_t  cuTransquantBypassFlag : 1;//12
    uint32_t  cuPartMode : 3;//15
    uint32_t  cuPredMode : 1;//16
    uint32_t  interPredIdcMv0 : 2;//18
    uint32_t  interPredIdcMv1 : 2;//20
    uint32_t  rsv0 : 2;//22
    uint32_t  ipcmEnable : 1;//23
    uint32_t  zeroOutCoefficients : 1;//24
    uint32_t  cuQp : 7;//31
    uint32_t  cuQpSign : 1;//32
    SIDE_BAND sb7;
};

struct PAKOBJVDEnc
{
    uint32_t Pak_Info;//dump下来要改的东西在pak_info里面，所以还是上面那种定义更加友好
    uint32_t DW1_PacketEnd  : MOS_BITFIELD_BIT(0);
    uint32_t DW1_RealDW     : MOS_BITFIELD_BIT(1);
    uint32_t DW1_DWCount    : MOS_BITFIELD_RANGE(2, 4);
    uint32_t DW1_DWType     : MOS_BITFIELD_RANGE(5, 6); //(01: LCU Header Info, 10: CU Info, 00: Default)
    uint32_t DW1_MBZ        : MOS_BITFIELD_RANGE(7, 15);
    uint32_t DW1_CUSAD      : MOS_BITFIELD_RANGE(16, 31);
};

struct VDEncLcuHeaderInfo
{
    PAKOBJVDEnc DW[5];
};

struct VDEncCuRecordInfo
{
    PAKOBJVDEnc DW[8];
};

struct VMECuRecordInfo
{
    //DWORD 0
    union
    {
        struct
        {
            uint32_t DW0_L0_Mv0x : MOS_BITFIELD_RANGE(0, 15);
            uint32_t DW0_L0_Mv0y : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t DW0_LumaIntraModeSecondBest : MOS_BITFIELD_RANGE(0, 5);
            uint32_t DW0_LumaIntraModeSecondBest4x4_1 : MOS_BITFIELD_RANGE(6, 11);
            uint32_t DW0_LumaIntraModeSecondBest4x4_2 : MOS_BITFIELD_RANGE(12, 17);
            uint32_t DW0_LumaIntraModeSecondBest4x4_3 : MOS_BITFIELD_RANGE(18, 23);
            uint32_t DW0_ChromaIntraModeSecondBest_o3a_p : MOS_BITFIELD_RANGE(24, 26);
            uint32_t DW0_Reserved0 : MOS_BITFIELD_RANGE(27, 31);
        };
    };

    //DWORD 1
    uint32_t DW1_L0_Mv1x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW1_L0_Mv1y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 2
    uint32_t DW2_L1_Mv0x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW2_L1_Mv0y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 3
    uint32_t DW3_L1_Mv1x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW3_L1_Mv1y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 4
    uint32_t DW4_L0Mv0RefIdx : MOS_BITFIELD_RANGE(0, 3);
    uint32_t DW4_L0Mv1RefIdx_ChromaIntraMode3 : MOS_BITFIELD_RANGE(4, 7);
    uint32_t DW4_L1Mv0RefIdx_ChromaIntraMode2 : MOS_BITFIELD_RANGE(8, 11);
    uint32_t DW4_L1Mv1RefIdx_ChromaIntraMode1 : MOS_BITFIELD_RANGE(12, 15);
    uint32_t DW4_Tu_Yuv_TransformSkip : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 5
    uint32_t DW5_TuSize : MOS_BITFIELD_RANGE(0, 31);

    //DWORD 6
    uint32_t DW6_LumaIntraMode4x4_1 : MOS_BITFIELD_RANGE(0, 5);
    uint32_t DW6_LumaIntraMode4x4_2 : MOS_BITFIELD_RANGE(6, 11);
    uint32_t DW6_LumaIntraMode4x4_3 : MOS_BITFIELD_RANGE(12, 17);
    uint32_t DW6_RoundingSelect : MOS_BITFIELD_RANGE(18, 21); // seems VDEnc only
    uint32_t DW6_zero_out_coefficients_V_LastCUof32x32 : MOS_BITFIELD_BIT(22);
    uint32_t DW6_zero_out_coefficients_U_LastCUofLCU : MOS_BITFIELD_BIT(23);
    uint32_t DW6_TuCountMinus1 : MOS_BITFIELD_RANGE(24, 27);
    uint32_t DW6_Reserved1 : MOS_BITFIELD_RANGE(28, 31);// VDEnc only

    //DWORD 7
    uint32_t DW7_LumaIntraMode : MOS_BITFIELD_RANGE(0, 5);
    uint32_t DW7_CuSize : MOS_BITFIELD_RANGE(6, 7);
    uint32_t DW7_ChromaIntraMode : MOS_BITFIELD_RANGE(8, 10);
    uint32_t DW7_CuTransquantBypassFlag : MOS_BITFIELD_BIT(11);
    uint32_t DW7_CuPartMode : MOS_BITFIELD_RANGE(12, 14);
    uint32_t DW7_CuPredMode : MOS_BITFIELD_BIT(15);
    uint32_t DW7_InterPredIdcMv0 : MOS_BITFIELD_RANGE(16, 17);
    uint32_t DW7_InterPredIdcMv1 : MOS_BITFIELD_RANGE(18, 19);
    uint32_t DW7_ModifiedFlag : MOS_BITFIELD_BIT(20);
    uint32_t DW7_LastCUofLCUFlag : MOS_BITFIELD_BIT(21);//vdenc only
    uint32_t DW7_IPCMenable : MOS_BITFIELD_BIT(22);
    uint32_t DW7_ForceZeroCoeffY : MOS_BITFIELD_BIT(23);
    uint32_t DW7_CuQp : MOS_BITFIELD_RANGE(24, 30);
    uint32_t DW7_CuQpSign : MOS_BITFIELD_BIT(31);
};

struct HcpPakObject//Instruct _HCP_PAK_OBJECT Index/57779 LCU级，也就是一个LCU要发一个
{
    // DW0
    struct
    {
        uint32_t DwordLength : 16;
        uint32_t SubOp : 7;
        uint32_t Opcode : 6;
        uint32_t Type : 3;
    } DW0;

    //DW1
    struct
    {
        uint32_t Split_flag_level2_level1part0 : 4;
        uint32_t Split_flag_level2_level1part1 : 4;
        uint32_t Split_flag_level2_level1part2 : 4;
        uint32_t Split_flag_level2_level1part3 : 4;
        uint32_t Split_flag_level1 : 4;
        uint32_t Split_flag_level0 : 1;
        uint32_t Reserved21_23 : 3;
        uint32_t CU_count_minus1 : 6;
        uint32_t LastCtbOfTileFlag : 1;
        uint32_t LastCtbOfSliceFlag : 1;
    } DW1;

    //DW2
    struct
    {
        uint32_t Current_LCU_X_Addr : 16;
        uint32_t Current_LCU_Y_Addr : 16;
    } DW2;

    //DW3
    struct
    {
        uint32_t Estimated_LCU_Size_In_Bytes : 32;
    } DW3;

    //DW4
    struct
    {
        uint32_t SSE_ClassID_32x32_0 : 4;
        uint32_t SSE_ClassID_32x32_1 : 4;
        uint32_t SSE_ClassID_32x32_2 : 4;
        uint32_t SSE_ClassID_32x32_3 : 4;
        uint32_t LCUForceZeroCoeff : 1;
        uint32_t Disable_SAO_On_LCU_Flag : 1;
        uint32_t Reserve18_31 : 14;
    } DW4;

    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;
};

class HevcVdencPktMcts : public HevcVdencPkt
{
public:
    HevcVdencPktMcts(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface) :
        HevcVdencPkt(pipeline, task, hwInterface) {}
    virtual ~HevcVdencPktMcts() {}

    MOS_STATUS AllocateResources() override;
    MOS_STATUS FreeResources() override; 
    MOS_STATUS ForceMCTS();

    MOS_STATUS Prepare() override;
    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;
MOS_STATUS AddSlicesCommandsInTile(
            MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddHcpPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer) override;
     MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

        //!
        //! \brief  Construct 3rd level batch buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Construct3rdLevelBatch() override;
       //!
        //! \brief  Add picture state with tile
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddPicStateWithTile(
            MOS_COMMAND_BUFFER &cmdBuffer) override;
        //!
        //! \brief  Patch tile level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase) override;

        //!
        //! \brief  Add one tile commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] tileRow
        //!         tile row
        //! \param  [in] tileCol
        //!         tile column
        //! \param  [in] tileRowPass
        //!         tile row pass
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AddOneTileCommands(
            MOS_COMMAND_BUFFER  &cmdBuffer,
            uint32_t            tileRow,
            uint32_t            tileCol,
            uint32_t            tileRowPass) override;
    virtual std::string GetPacketName() override
    {
        return "STANDALONE_PAK_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS OutPutVME422(const char *outFilePath, uint8_t *data, uint32_t size);
    MOS_STATUS LoadPakCommandAndCuRecordFromFile(PMOS_INTERFACE osInterface, HevcBasicFeature *basicFeature);
#endif

protected:
    PMOS_RESOURCE      m_newMbCodeBuffer = nullptr;  //!< Pointer to MOS_RESOURCE of MbCode buffer
    uint32_t           m_newmbCodeSize      = 0;        //!< MB code buffer size
   uint32_t            m_newmvOffset = 0;    
    uint32_t           m_numCTUs      = 0;        //!< MB code buffer size

#if USE_CODECHAL_DEBUG_TOOL
    bool               m_bDumpConvertedBuf  = false;    //!< refine later
    bool               m_bLoadFromFile      = false;    //!< refine later
#endif

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPktMcts)
};

}  // namespace encode
//#endif
