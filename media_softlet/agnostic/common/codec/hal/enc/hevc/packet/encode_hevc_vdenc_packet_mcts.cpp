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
//! \file     encode_hevc_vdenc_packet_mcts.cpp
//! \brief    Defines the interface to adapt to HEVC VDENC pipeline
//!

#include "encode_hevc_vdenc_packet_mcts.h"

namespace encode
{
MOS_STATUS HevcVdencPktMcts::AllocateResources()
{

    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::AllocateResources());

    const uint32_t picWidthInLCU  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 64);
    const uint32_t picHeightInLCU = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, 64);
    m_numCTUs    = picWidthInLCU * picHeightInLCU;
    //m_newmbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (m_numCTUs * 5 + m_numCTUs * 64 * 8), CODECHAL_PAGE_SIZE);
   // m_newmbCodeSize +=m_newmvOffset;
    //m_newmvOffset =0;
    m_newmvOffset                 = MOS_ALIGN_CEIL((m_numCTUs * (MOS_BYTES_TO_DWORDS(sizeof(HcpPakObject))) * sizeof(uint32_t)), CODECHAL_PAGE_SIZE);
    m_newmbCodeSize               = m_newmvOffset + MOS_ALIGN_CEIL((m_numCTUs * 64 * sizeof(VMECuRecordInfo)), CODECHAL_PAGE_SIZE);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type             = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType         = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format           = Format_Buffer;
    allocParamsForBufferLinear.dwMemType        = MOS_MEMPOOL_SYSTEMMEMORY;
    allocParamsForBufferLinear.Flags.bCacheable = true;
    // for now we set ResUsageType to MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM rather than
    // MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE to enable coherency in gmm
    allocParamsForBufferLinear.ResUsageType     = MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM;

    allocParamsForBufferLinear.dwBytes  = m_newmbCodeSize;
    allocParamsForBufferLinear.pBufName = "Standalone PAK Input Buffer";
    m_newMbCodeBuffer                = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    ENCODE_CHK_NULL_RETURN(m_newMbCodeBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::FreeResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);

    if(m_newMbCodeBuffer)
    {
    //    printf("free resources\n");
        ENCODE_CHK_STATUS_RETURN(m_allocator->DestroyResource(m_newMbCodeBuffer));
        m_newMbCodeBuffer = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}


int cu_num_per_ctu=10;
int qt_width[4]={8,16,32,64};

void update_coord(int &x0,int &y0,int qt_count,int cu_part_mode)
{
        if(cu_part_mode==3){
                return;
        }
        if(qt_count>0x3e){
                //cout<<"end"<<endl;
                return;
        }
        int qt_count_tmp = (qt_count >>(cu_part_mode<<1))&0x3;
        switch(qt_count_tmp){
                case 0:
                        x0+=qt_width[cu_part_mode];
                        y0+=0;
                        break;
                case 1:
                        x0-=qt_width[cu_part_mode];
                        y0+=qt_width[cu_part_mode];
                        break;
                case 2:
                        x0+=qt_width[cu_part_mode];
                        y0+=0;
                        break;
                case 3:
                {
                        x0-=qt_width[cu_part_mode];
                        y0-=qt_width[cu_part_mode];
                        update_coord(x0,y0,qt_count,cu_part_mode+1);
                        break;
                }
        }
}

int mv_check(int up,int left,int tile_w,int tile_h,int CUx0,int CUy0,int CU_width,int16_t &mvx,int16_t &mvy,int pic_width,int pic_height){
    int down = up + tile_h-1;
    int right = left + tile_w-1;

    const int lRangeXLeft = left;
    const int lRangeYTop = up;
    const int lRangeXRight =(right)<(pic_width-1)?(right):(pic_width-1);
    const int lRangeYBottom =(down)<(pic_height-1)?(down):(pic_height-1);
    int PredXLeft=CUx0+mvx;
    int PredXRight=CUx0+CU_width+mvx;
    int PredYTop=CUy0+mvy;
    int PredYBottom=CUy0+CU_width+mvy;
    int return_flag=0;
    if (PredXLeft < lRangeXLeft ||PredXRight < lRangeXLeft )
    {
        mvx=lRangeXLeft-CUx0;
        return_flag= 1;
    }
   else if(PredXLeft > lRangeXRight || PredXRight > lRangeXRight)
    {
        mvx=lRangeXRight-(CUx0+CU_width);
        return_flag= 1;
    }
    if (PredYTop < lRangeYTop || PredYBottom < lRangeYTop)
    {
        mvy=lRangeYTop-CUy0;
        return_flag= 1;
    }
   else if ( PredYTop > lRangeYBottom|| PredYBottom > lRangeYBottom)
    {
        mvy=lRangeYBottom-(CUy0+CU_width);
        return_flag= 1;
    }
    return return_flag;
}
MOS_STATUS HevcVdencPktMcts::ForceMCTS()
{
    //printf("MCTS\n");

    ENCODE_FUNC_CALL();
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly       = 1;

    uint8_t *oldmbCodeBuffer = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, m_basicFeature->m_resMbCodeBuffer, &lockFlags);
     ENCODE_CHK_NULL_RETURN(oldmbCodeBuffer);

    lockFlags.ReadOnly       = 0;
    lockFlags.WriteOnly      = 1;
    uint8_t *newmbCodeBuffer = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, m_newMbCodeBuffer, &lockFlags);
    ENCODE_CHK_NULL_RETURN(newmbCodeBuffer);
    uint8_t *newdata   = newmbCodeBuffer;

    uint8_t *data         = oldmbCodeBuffer;
    uint8_t *tempPakObj   = newmbCodeBuffer;//lcuheader
    uint8_t *tempCuRecord = newmbCodeBuffer + m_newmvOffset;//vdenc cu record buffer

    uint32_t bufferSize = m_basicFeature->m_mbCodeSize;
    while (*((uint32_t *)data) != OPCODE)  // add check to prevent infinite loop
    {
        data += sizeof(uint32_t);
        bufferSize -= sizeof(uint32_t);
        if (bufferSize == 0)
        {
            ENCODE_ASSERTMESSAGE("Failed to find OPCODE.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    uint16_t ctuIdxInTile=0;
    uint16_t TileRowIdx=0;
    uint16_t TileColumIdx=0;
    int numTileColumns=m_basicFeature->m_hevcPicParams->num_tile_columns_minus1+1;
    int numTileRows=m_basicFeature->m_hevcPicParams->num_tile_rows_minus1+1;
    int numTile=numTileColumns*numTileRows;
    int tileidx=0;
    int tile_w=0;
    int tile_h=0;
    EncodeTileData curTileData = {};
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, curTileData,tileidx);
    for (uint32_t i = 0; i < m_numCTUs; i++)
    {
        tile_w=m_basicFeature->m_hevcPicParams->tile_column_width[TileColumIdx];
        tile_h=m_basicFeature->m_hevcPicParams->tile_row_height[TileRowIdx];

        while (*((uint32_t *)data) != OPCODE)  // add check to prevent infinite loop
        {
            data += sizeof(uint32_t);
            bufferSize -= sizeof(uint32_t);
            if (bufferSize == 0)
            {
                ENCODE_ASSERTMESSAGE("VDEnc 420 pass not finished.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        VDEncLcuHeaderInfo *lcuInfo = (VDEncLcuHeaderInfo *)data;
        data += sizeof(VDEncLcuHeaderInfo);
        for (int j = 0; j < 5; j++)
        {
            uint32_t *tempdata = (uint32_t *)tempPakObj + j;
            *tempdata          = j ? lcuInfo->DW[j].Pak_Info : 0x73A10003;
        }

        HcpPakObject *vmeLcuInfo = (HcpPakObject *)tempPakObj;
        tempPakObj += sizeof(HcpPakObject);

        // Add 0x05000000 to indicate each slice end
        if (vmeLcuInfo->DW1.LastCtbOfTileFlag)
        {   
                vmeLcuInfo->DW5 = 0x05000000;
        }
        uint32_t cuCount = 0;
        int x0 = vmeLcuInfo->DW2.Current_LCU_X_Addr<<6;
        int y0 = vmeLcuInfo->DW2.Current_LCU_Y_Addr<<6;
        int qt_count=0x0;

        for (; cuCount < 64; cuCount++)
        {
            VDEncCuRecordInfo *cuInfo = (VDEncCuRecordInfo *)data;
                        auto cuRecord = (CU_RECORD *)data; 
            //printf("x0:%d,y0:%d,qt_count:%x,partmode:%d\n",x0,y0,qt_count,cuRecord->cuSize);
   // printf("cu(%d,%d),mv(%d,%d)(%d,%d)(%d,%d)(%d,%d)\n",x0,y0,cuRecord->l0Mv0X,cuRecord->l0Mv0Y,cuRecord->l0Mv1X,cuRecord->l0Mv1Y,cuRecord->l1Mv0X,cuRecord->l1Mv0Y,cuRecord->l1Mv1X,cuRecord->l1Mv1Y);

int picwidth=(m_basicFeature->m_hevcSeqParams->wFrameWidthInMinCbMinus1+1)<<3;
int picheight=(m_basicFeature->m_hevcSeqParams->wFrameHeightInMinCbMinus1+1)<<3;
//printf("%d,%d\n",m_basicFeature->m_hevcSeqParams->wFrameWidthInMinCbMinus1,m_basicFeature->m_hevcSeqParams->wFrameHeightInMinCbMinus1);
int16_t mvx=cuRecord->l0Mv0X;
int16_t mvy=cuRecord->l0Mv0Y;
    if((cuRecord->l0Mv0X||cuRecord->l0Mv0Y)&&mv_check(curTileData.tileStartY<<6,curTileData.tileStartX<<6,tile_w<<6,tile_h<<6,x0,y0,qt_width[cuRecord->cuSize]-1,mvx,mvy,picwidth,picheight))
    {
   cuRecord->l0Mv0X=mvx;
  cuRecord->l0Mv0Y=mvy;
    }
   mvx=cuRecord->l0Mv1X;
mvy=cuRecord->l0Mv1Y; 
    if((cuRecord->l0Mv1X||cuRecord->l0Mv1Y)&&mv_check(curTileData.tileStartY<<6,curTileData.tileStartX<<6,tile_w<<6,tile_h<<6,x0,y0,qt_width[cuRecord->cuSize]-1,mvx,mvy,picwidth,picheight))
    {
   cuRecord->l0Mv1X=mvx;
  cuRecord->l0Mv1Y=mvy;
    }
    mvx=cuRecord->l1Mv0X;
    mvy=cuRecord->l1Mv0Y; 
    if((cuRecord->l1Mv0X||cuRecord->l1Mv0Y)&&mv_check(curTileData.tileStartY<<6,curTileData.tileStartX<<6,tile_w<<6,tile_h<<6,x0,y0,qt_width[cuRecord->cuSize]-1,mvx,mvy,picwidth,picheight))
    {
   cuRecord->l1Mv0X=mvx;
    cuRecord->l1Mv0Y=mvy;
    }
        mvx=cuRecord->l1Mv1X;
    mvy=cuRecord->l1Mv1Y; 
    if((cuRecord->l1Mv1X||cuRecord->l1Mv1Y)&&mv_check(curTileData.tileStartY<<6,curTileData.tileStartX<<6,tile_w<<6,tile_h<<6,x0,y0,qt_width[cuRecord->cuSize]-1,mvx,mvy,picwidth,picheight))
    {
    cuRecord->l1Mv1X=mvx;
    cuRecord->l1Mv1Y=mvy;
    }

   
   data += sizeof(VDEncCuRecordInfo);

            for (int j = 0; j < 8; j++)
            {
                uint32_t *tempdata = (uint32_t *)tempCuRecord + j;
                *tempdata          = cuInfo->DW[j].Pak_Info;
            }
       //     put cu level infomation to tempCuRecord
            VMECuRecordInfo *vmeCuInfo = (VMECuRecordInfo *)tempCuRecord;
            tempCuRecord += sizeof(VMECuRecordInfo);
            vmeCuInfo->DW7_ForceZeroCoeffY                       = 0;
            vmeCuInfo->DW6_zero_out_coefficients_V_LastCUof32x32 = 0;
            vmeCuInfo->DW4_Tu_Yuv_TransformSkip                  = 0;
            if (vmeCuInfo->DW6_zero_out_coefficients_U_LastCUofLCU || cuCount == 63)
            {
                vmeLcuInfo->DW1.CU_count_minus1                    = cuCount;
                vmeCuInfo->DW6_zero_out_coefficients_U_LastCUofLCU = 0;
                break;
            }
            update_coord(x0,y0,qt_count,cuRecord->cuSize);
            qt_count+=(1<<(cuRecord->cuSize<<1));
            while(y0>=picheight||x0>=picwidth){
                update_coord(x0,y0,qt_count,cuRecord->cuSize);
                qt_count+=(1<<(cuRecord->cuSize<<1));
            }
            
            vmeCuInfo->DW6_zero_out_coefficients_U_LastCUofLCU = 0;
        }
        tempCuRecord += (63 - cuCount) * sizeof(VMECuRecordInfo);

        if(ctuIdxInTile+1==tile_w*tile_h)
        {
            assert(vmeLcuInfo->DW1.LastCtbOfTileFlag);
            ctuIdxInTile=0;
            if(tileidx+1!=numTile){
                while (*((uint32_t *)data) != OPCODE)  // add check to prevent infinite loop
                {
                    data += sizeof(uint32_t);
                    bufferSize -= sizeof(uint32_t);
                    if (bufferSize == 0)
                    {
                        ENCODE_ASSERTMESSAGE("Failed to find OPCODE.");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
            }
            if(TileColumIdx+1==numTileColumns)
            {
                TileColumIdx=0;
                TileRowIdx++;
            }else{
                TileColumIdx++;
            }
            tileidx++;
            if(tileidx<numTile)
                RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, curTileData,tileidx);
        }else{
            ctuIdxInTile++;
        }
    }

    HcpPakObject *vmeLcuInfo = (HcpPakObject *)tempPakObj;
    vmeLcuInfo--;
    vmeLcuInfo->DW5 = 0x05000000;  // Add 0x05000000 to indicate frame end

    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_basicFeature->m_resMbCodeBuffer));
    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_newMbCodeBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::Prepare()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::Prepare());

    ENCODE_CHK_STATUS_RETURN(ForceMCTS());


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcPicParams);


   if (m_basicFeature->m_hevcPicParams->tiles_enabled_flag &&m_basicFeature->m_hevcPicParams->constrained_mv_in_tile )
    {
        ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::Completed(mfxStatus, rcsStatus, statusReport));
    }
    else
    {
        // When 422 wa feature is not enabled, not need above complete options
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));


    SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

    ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

    // Send command buffer header at the beginning (OS dependent)
    ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPictureHcpCommands(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPicStateWithNoTile(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::Construct3rdLevelBatch()
{
    ENCODE_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;


    // Begin patching 3rd level batch cmds
    MOS_COMMAND_BUFFER constructedCmdBuf;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, BeginPatch3rdLevelBatch, constructedCmdBuf);


    SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &constructedCmdBuf);

    // set MI_BATCH_BUFFER_END command
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

    // End patching 3rd level batch cmds
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, EndPatch3rdLevelBatch);

    return eStatus;
}

MOS_STATUS HevcVdencPktMcts::AddPicStateWithTile(
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();


    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
    if (!tileEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    PMHW_BATCH_BUFFER thirdLevelBatchBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetThirdLevelBatchBuffer, thirdLevelBatchBuffer);
    ENCODE_CHK_NULL_RETURN(thirdLevelBatchBuffer);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, thirdLevelBatchBuffer)));

        auto rdoqFeature = dynamic_cast<HevcEncodeCqp *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcCqpFeature));
        ENCODE_CHK_NULL_RETURN(rdoqFeature);
        if (rdoqFeature->IsRDOQEnabled())
        {
            SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);
        }
    return MOS_STATUS_SUCCESS;
}
    
MOS_STATUS HevcVdencPktMcts::AddOneTileCommands(
MOS_COMMAND_BUFFER &cmdBuffer,
uint32_t            tileRow,
uint32_t            tileCol,
uint32_t            tileRowPass)
{
    ENCODE_FUNC_CALL();
    PMOS_COMMAND_BUFFER tempCmdBuffer         = &cmdBuffer;
    PMHW_BATCH_BUFFER   tileLevelBatchBuffer  = nullptr;
    auto                eStatus               = MOS_STATUS_SUCCESS;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);


    if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!m_osInterface->bUsesPatchList)
    {
        MOS_COMMAND_BUFFER constructTileBatchBuf = {};

        // Begin patching tile level batch cmds
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

        // Add batch buffer start for tile
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

        tempCmdBuffer = &constructTileBatchBuf;
    }

    // HCP Lock for multiple pipe mode
    if (m_pipeline->GetPipeNum() > 1)
    {
        auto &vdControlStateParams                = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                      = {};
        vdControlStateParams.scalableModePipeLock = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
    }

    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, tempCmdBuffer);

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
  //  auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
   // mfxWaitParams.iStallVdboxPipeline   = true;
   // ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(tempCmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPicStateWithTile(*tempCmdBuffer));

    SETPAR_AND_ADDCMD(HCP_TILE_CODING, m_hcpItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddSlicesCommandsInTile(*tempCmdBuffer));

    //HCP unLock for multiple pipe mode
    if (m_pipeline->GetPipeNum() > 1)
    {
        auto &vdControlStateParams                  = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                        = {};
        vdControlStateParams.scalableModePipeUnlock = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
    }

    if (!m_osInterface->bUsesPatchList)
    {
        // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
        ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
        tileLevelBatchBuffer->iCurrent   = tempCmdBuffer->iOffset;
        tileLevelBatchBuffer->iRemaining = tempCmdBuffer->iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));
    }

    // End patching tile level batch cmds
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, EndPatchTileLevelBatch);

    return eStatus;
}

MOS_STATUS HevcVdencPktMcts::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;
    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
                /*d                              part vdenc                                   */


    // multi tiles cases on Liunx, 3rd level batch buffer is 2nd level.
    ENCODE_CHK_STATUS_RETURN(Construct3rdLevelBatch());
                /*d                              part vdenc                                   */

    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        uint32_t Pass = m_pipeline->GetCurrentPass();
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
        {
            ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                cmdBuffer,
                tileRow,
                tileCol,
                Pass));
        }
    }

    // Insert end of sequence/stream if set
    if ((m_basicFeature->m_lastPicInSeq || m_basicFeature->m_lastPicInStream) && m_pipeline->IsLastPipe())
    {
        ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
    }

    // post-operations are done by pak integrate pkt

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::AddSlicesCommandsInTile(
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    PCODEC_ENCODER_SLCDATA         slcData = m_basicFeature->m_slcData;


    uint32_t slcCount, sliceNumInTile = 0;
    for (slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
    {
        m_basicFeature->m_curNumSlices = slcCount;
        bool sliceInTile               = false;
        m_lastSliceInTile              = false;

        EncodeTileData curTileData = {};
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetCurrentTile, curTileData);
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsSliceInTile, slcCount, &curTileData, &sliceInTile, &m_lastSliceInTile);

        m_basicFeature->m_lastSliceInTile = m_lastSliceInTile;
        if (!sliceInTile)
        {
            continue;
        }
        //slcData[slcCount].CmdOffset = m_basicFeature->tilelcuoffset * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
       // m_basicFeature->tilelcuoffset += (curTileData.tileEndXInLCU-curTileData.tileStartXInLCU)*(curTileData.tileEndYInLCU-curTileData.tileStartYInLCU);
        ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(slcData, slcCount, cmdBuffer));

        sliceNumInTile++;
    }  // end of slice

    if (0 == sliceNumInTile)
    {
        // One tile must have at least one slice
        ENCODE_ASSERT(false);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint16_t numTileRows    = 1;
    uint16_t numTileColumns = 1;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    if (sliceNumInTile > 1 && (numTileColumns > 1 || numTileRows > 1))
    {
        ENCODE_ASSERTMESSAGE("Multi-slices in a tile is not supported!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
    

    if (tileEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &cmdBuffer);


            SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::AddAllCmds_HCP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);


    bool bLastPicInSeq    = m_basicFeature->m_lastPicInSeq;
    bool bLastPicInStream = m_basicFeature->m_lastPicInStream;
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
    params       = {};

    if (bLastPicInSeq && bLastPicInStream)
    {
        params = {};

        uint32_t dwPadding[3];

        params.dwPadding                   = sizeof(dwPadding) / sizeof(dwPadding[0]);
        params.bHeaderLengthExcludeFrmSize = 0;
        params.bEndOfSlice                 = 1;
        params.bLastHeader                 = 1;
        params.bEmulationByteBitsInsert    = 0;
        params.uiSkipEmulationCheckCount   = 0;
        params.dataBitsInLastDw            = 16;
        params.databyteoffset              = 0;
        params.bIndirectPayloadEnable      = 0;

        m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);

        dwPadding[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
        dwPadding[1] = (1L | (1L << 24));
        dwPadding[2] = (HEVC_NAL_UT_EOB << 1) | (1L << 8);
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, nullptr, &dwPadding[0], sizeof(dwPadding)));
    }
    else if (bLastPicInSeq || bLastPicInStream)
    {
        params = {};
        uint32_t dwLastPicInSeqData[2], dwLastPicInStreamData[2];

        params.dwPadding                   = bLastPicInSeq * 2 + bLastPicInStream * 2;
        params.bHeaderLengthExcludeFrmSize = 0;
        params.bEndOfSlice                 = 1;
        params.bLastHeader                 = 1;
        params.bEmulationByteBitsInsert    = 0;
        params.uiSkipEmulationCheckCount   = 0;
        params.dataBitsInLastDw            = 8;
        params.databyteoffset              = 0;
        params.bIndirectPayloadEnable      = 0;

        m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);

        if (bLastPicInSeq)
        {
            dwLastPicInSeqData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
            dwLastPicInSeqData[1] = 1;  // nuh_temporal_id_plus1
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, nullptr, &dwLastPicInSeqData[0], sizeof(dwLastPicInSeqData)));
        }

        if (bLastPicInStream)
        {
            dwLastPicInStreamData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOB << 1) << 24));
            dwLastPicInStreamData[1] = 1;  // nuh_temporal_id_plus1
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, nullptr, &dwLastPicInStreamData[0], sizeof(dwLastPicInStreamData)));
        }
    }
    else
    {
        PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_nalUnitParams;


        PBSBuffer         pBsBuffer   = &(m_basicFeature->m_bsBuffer);
        uint32_t          bitSize     = 0;
        uint32_t          offSet      = 0;

        if (cmdBuffer == nullptr )
        {
            ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        //insert AU, SPS, PSP headers before first slice header
        if (m_basicFeature->m_curNumSlices == 0)
        {
            uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for Length field in PAK_INSERT_OBJ cmd

            for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
            {
                uint32_t nalunitPosiSize   = ppNalUnitParams[i]->uiSize;
                uint32_t nalunitPosiOffset = ppNalUnitParams[i]->uiOffset;

                while (nalunitPosiSize > 0)
                {
                    bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                    offSet  = nalunitPosiOffset;

                    params = {};

                    params.dwPadding                 = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                    params.bEmulationByteBitsInsert  = ppNalUnitParams[i]->bInsertEmulationBytes;
                    params.uiSkipEmulationCheckCount = ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                    params.dataBitsInLastDw          = bitSize % 32;
                    if (params.dataBitsInLastDw == 0)
                    {
                        params.dataBitsInLastDw = 32;
                    }

                    if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                    {
                        nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                        nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                    }
                    else
                    {
                        nalunitPosiSize = 0;
                    }
                    m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                    uint32_t byteSize = (bitSize + 7) >> 3;
                    if (byteSize)
                    {
                        MHW_MI_CHK_NULL(pBsBuffer);
                        MHW_MI_CHK_NULL(pBsBuffer->pBase);
                        uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, NULL, data, byteSize));
                    }
                }
            }
        }

        params = {};
        // Insert slice header
        params.bLastHeader              = true;
        params.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        PCODEC_ENCODER_SLCDATA slcData    = m_basicFeature->m_slcData;
        uint32_t               currSlcIdx = m_basicFeature->m_curNumSlices;

        params.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
        bitSize                          = slcData[currSlcIdx].BitSize;
        offSet                           = slcData[currSlcIdx].SliceOffset;

        if (m_hevcSeqParams->SliceSizeControl)
        {
            params.bLastHeader                = false;
            params.bEmulationByteBitsInsert   = false;
            bitSize                           = m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            params.bResetBitstreamStartingPos = true;
            params.dwPadding                  = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw           = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, NULL, data, byteSize));
            }

            // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
            params.bLastHeader = true;
            bitSize            = bitSize - m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            offSet += ((m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            params.bResetBitstreamStartingPos = true;
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            byteSize = (bitSize + 7) >> 3;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, NULL, data, byteSize));
            }
        }
        else
        {
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, NULL, data, byteSize));
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer)
{

    ENCODE_FUNC_CALL();
    PMHW_BATCH_BUFFER   batchBufferInUse = nullptr;
    PMOS_COMMAND_BUFFER cmdBufferInUse   = nullptr;


    if (m_useBatchBufferForPakSlices)
    {
        batchBufferInUse = &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx];
        ENCODE_CHK_NULL_RETURN(batchBufferInUse);
    }
    else
    {
        cmdBufferInUse = &cmdBuffer;
    }

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_WEIGHTOFFSET_STATE(&cmdBuffer));

    m_basicFeature->m_useDefaultRoundingForHcpSliceState = true;/*d ? true还是falese*/
    SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));

    if (m_useBatchBufferForPakSlices && batchBufferInUse)
    {
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, batchBufferInUse));

        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
        secondLevelBatchBuffer.OsResource   = batchBufferInUse->OsResource;
        secondLevelBatchBuffer.dwOffset     = m_batchBufferForPakSlicesStartOffset;
        secondLevelBatchBuffer.bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, &secondLevelBatchBuffer)));
    }


    // Insert Batch Buffer Start command to send HCP_PAK_OBJ data for LCUs in this slice
    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    secondLevelBatchBuffer.OsResource   = *m_newMbCodeBuffer;

    secondLevelBatchBuffer.dwOffset     = slcData[currSlcIdx].CmdOffset;
    secondLevelBatchBuffer.bSecondLevel = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, &secondLevelBatchBuffer)));
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();


    auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
    forceWakeupParams                           = {};
    forceWakeupParams.bMFXPowerWellControl      = false;
    forceWakeupParams.bMFXPowerWellControlMask  = true;
    forceWakeupParams.bHEVCPowerWellControl     = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktMcts::AddHcpPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();


    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.initialization = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &cmdBuffer);

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcVdencPktMcts)
{
    ENCODE_FUNC_CALL();


     ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::MHW_SETPAR_F(HCP_PIPE_MODE_SELECT)(params));

    params.bVdencEnabled              = false;
    params.bBRCEnabled                = false;
    params.bAdvancedRateControlEnable = false;

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
    params.bStreamOutEnabled = m_basicFeature->m_hevcSeqParams->RateControlMethod != RATECONTROL_CQP;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcVdencPktMcts)
{
    ENCODE_FUNC_CALL();
    

    uint32_t                          currSlcIdx    = m_basicFeature->m_curNumSlices;
    params.intrareffetchdisable         = false;
    params.tailInsertionEnable          = (m_hevcPicParams->bLastPicInSeq || m_hevcPicParams->bLastPicInStream) && ((currSlcIdx == m_basicFeature->m_numSlices - 1));
    params.roundintra                   = m_basicFeature->m_roundingIntra;
    params.roundinter                   = m_basicFeature->m_roundingInter;


    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, HevcVdencPktMcts)
{
    ENCODE_FUNC_CALL();


    params.presMvObjectBuffer      = m_newMbCodeBuffer;
    params.dwMvObjectOffset        = m_newmvOffset;
    params.dwMvObjectSize          = m_newmbCodeSize - m_newmvOffset;
    params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
    params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcVdencPktMcts)
{


    params.sseEnable                    = false;
    params.rhodomainRateControlEnable   = false;
    params.fractionalQpAdjustmentEnable = false;

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
    if (m_basicFeature->m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint32_t *data     = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &(vdenc2ndLevelBatchBuffer->OsResource), &lockFlags);
        ENCODE_CHK_NULL_RETURN(data);

        uint32_t value           = *(data + MINFRAMESIZE_OFFSET_RSVD);
        params.minframesize      = (uint16_t)value;
        params.minframesizeunits = (uint8_t)(value >> 30);

        m_osInterface->pfnUnlockResource(m_osInterface, &(vdenc2ndLevelBatchBuffer->OsResource));
    }

    return MOS_STATUS_SUCCESS;
}


}  // namespace encode
