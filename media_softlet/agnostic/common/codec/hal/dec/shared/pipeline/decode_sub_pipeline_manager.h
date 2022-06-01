/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_sub_pipeline_manager.h
//! \brief    Defines the common interface for decode sub pipeline manager
//! \details  Defines the common interface for decode sub pipeline manager
//!
#ifndef __DECODE_SUB_PIPELINE_MANAGER_H__
#define __DECODE_SUB_PIPELINE_MANAGER_H__

#include "decode_scalability_defs.h"
#include "media_packet.h"
#include "media_task.h"
#include "media_context.h"
#include "decode_sub_pipeline.h"

namespace decode {

struct DecodePipelineParams;

class DecodeSubPipelineManager
{
public:
    //!
    //! \brief  Decode sub pipeline manager constructor
    //!
    DecodeSubPipelineManager(DecodePipeline& decodePipeline);

    //!
    //! \brief  Decode sub pipeline manager destructor
    //!
    virtual ~DecodeSubPipelineManager();

    //!
    //! \brief  Registe pre-execute sub pipeline
    //! \param  [in] subPipeline
    //!         sub pipeline to be excuted
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Register(DecodeSubPipeline& subPipeline);

    //!
    //! \brief  Init sub pipeline list
    //! \param  [in] settings
    //!         Codechal setting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(CodechalSetting& settings);

    //!
    //! \brief  Prepare sub pipeline
    //! \param  [in] params
    //!         Decode pipeline parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Prepare(DecodePipelineParams& params);

    //!
    //! \brief  Execute sub pipeline
    //! \param  [in] subPipelineList
    //!         Sub pipeline list
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute();

protected:
    //!
    //! \brief  Execute sub pipeline
    //! \param  [in] subPipeline
    //!         sub pipeline to be excuted
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteSubPipeline(DecodeSubPipeline &subPipeline);

    //!
    //! \brief  Destroy sub pipeline list
    //! \param  [in] subPipelineList
    //!         Sub pipeline list
    //!
    void Destroy(std::vector<DecodeSubPipeline *> &subPipelineList);

protected:
    DecodePipeline* m_decodePipeline = nullptr;
    std::vector<DecodeSubPipeline *> m_subPipelineList; //!< sub pipeline list

MEDIA_CLASS_DEFINE_END(decode__DecodeSubPipelineManager)
};

}//decode

#endif // !__DECODE_SUB_PIPELINE_MANAGER_H__
