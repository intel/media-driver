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
//! \file     decode_predication.cpp
//! \brief    Defines the common interface for decode predication
//! \details  The decode predication interface implements the decode predication feature.
//!
#include "decode_predication.h"
#include "decode_utils.h"

namespace decode {

DecodePredication::DecodePredication(DecodeAllocator& allocator) :
    m_allocator(&allocator)
{

}

DecodePredication::~DecodePredication()
{
    if (m_allocator)
    {
        m_allocator->Destroy(m_predicationBuffer);
    }
    MOS_Delete(m_resPredication);
}

MOS_STATUS DecodePredication::Update(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);

    CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
    m_predicationEnabled = decodeParams->m_predicationEnabled;
    if (!m_predicationEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_predicationNotEqualZero = decodeParams->m_predicationNotEqualZero;
    m_predicationResOffset    = decodeParams->m_predicationResOffset;

    if (m_resPredication == nullptr)
    {
        m_resPredication = MOS_New(MOS_BUFFER);
    }

    DECODE_CHK_NULL(m_resPredication);
    if (decodeParams->m_presPredication != nullptr)
    {
        m_resPredication->OsResource = *decodeParams->m_presPredication;
    }
    else
    {
        MOS_ZeroMemory(m_resPredication, sizeof(MOS_BUFFER));
    }

    if (m_predicationBuffer == nullptr)
    {
        m_predicationBuffer = m_allocator->AllocateBuffer(sizeof(uint32_t), "PredicationBuffer");
        DECODE_CHK_NULL(m_predicationBuffer);
    }

    *(decodeParams->m_tempPredicationBuffer) = &m_predicationBuffer->OsResource;

    return MOS_STATUS_SUCCESS;
}

}
