/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_hevc_phase_real_tile.cpp
//! \brief    Defines the interface for Hevc decode real tile phase.
//!

#include "decode_hevc_phase_real_tile.h"
#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"

namespace decode
{

MOS_STATUS HevcPhaseRealTile::Initialize(uint8_t pass, uint8_t pipe, uint8_t activePipeNum)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcPhase::Initialize(pass, pipe, activePipeNum));

    auto featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    auto basicFeature = dynamic_cast<HevcBasicFeature*>(
        featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    PCODEC_HEVC_PIC_PARAMS picParams = basicFeature->m_hevcPicParams;
    DECODE_CHK_NULL(picParams);
    m_numTileColumns = picParams->num_tile_columns_minus1 + 1;

    return MOS_STATUS_SUCCESS;
}

uint32_t HevcPhaseRealTile::GetCmdBufIndex()
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_scalabOption.GetNumPipe() > 1);
    if (!m_pipeline->IsPhasedSubmission() || m_pipeline->IsParallelSubmission())
    {
        return m_secondaryCmdBufIdxBase + GetPipe();
    }
    else
    {
        /*  3 tiles 2 pipe for example:
                cur pass               cur pip
                0                       0, 1                2 cmd buffer needed
                1                       0                   1 cmd buffer needed
                all of 3 tiles cmd ready, submit 3 cmd togather
         */
        return m_secondaryCmdBufIdxBase + GetPipe() + (GetPass() * m_scalabOption.GetNumPipe());
    }
}

uint32_t HevcPhaseRealTile::GetSubmissionType()
{
    DECODE_FUNC_CALL();
    if (IsFirstPipe())
    {
        return SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }
    else if (IsLastPipeOfPass())
    {
        return SUBMISSION_TYPE_MULTI_PIPE_SLAVE |
               SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE |
               ((GetPipe() - 1) << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
    }
    else
    {
        return SUBMISSION_TYPE_MULTI_PIPE_SLAVE |
               ((GetPipe() - 1) << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
    }
}

MOS_STATUS HevcPhaseRealTile::GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode)
{
    DECODE_FUNC_CALL();
    pipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_REAL_TILE;
    if (IsFirstPipe())
    {
        if (IsLastPipeOfPic())
        {
            // if current pass is last pass and last pass only has one pipe, then it will work on leagcy mode
            multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        }
        else
        {
            multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
    }
    else if (IsLastPipeOfPass())
    {
        multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
    }
    else
    {
        multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
    }
    return MOS_STATUS_SUCCESS;
}

uint32_t HevcPhaseRealTile::GetPktId()
{
    DECODE_FUNC_CALL();
    return DecodePacketId(m_pipeline, hevcRealTilePacketId);
}

bool HevcPhaseRealTile::ImmediateSubmit()
{
    DECODE_FUNC_CALL();
    return (IsLastPipeOfPic());
}

bool HevcPhaseRealTile::RequiresContextSwitch()
{
    DECODE_FUNC_CALL();
    if (m_pipeline->IsShortFormat())
    {
        return false; // Don't need switch since same context as s2l phase
    }
    else
    {
        return (IsFirstPipe() && IsFirstPass()); // Switch at first pipe of first pass
    }
}

}
