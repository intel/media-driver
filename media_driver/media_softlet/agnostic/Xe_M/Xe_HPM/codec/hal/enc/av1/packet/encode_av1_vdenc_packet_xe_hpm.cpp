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
//! \file     encode_av1_vdenc_packet_xe_hpm.cpp
//! \brief    Defines the interface for av1 encode vdenc packet of Xe_HPM
//!
#include "encode_av1_vdenc_packet_xe_hpm.h"
#include "mhw_vdbox_avp_hwcmd_xe_hpm.h"

namespace encode
{
    Av1VdencPktXe_Hpm::~Av1VdencPktXe_Hpm()
    {
#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugInterface* debugInterface = m_pipeline->GetDebugInterface();
    if (debugInterface && debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar)) {
        DumpSeqParFile();
        MOS_Delete(m_av1Par);
    }
#endif
    }

#if USE_CODECHAL_DEBUG_TOOL
    static inline uint32_t *FindCmd(uint32_t *cmdBuf, uint32_t size, uint32_t dw0)
    {
        uint32_t offset = 0;

        while (offset < size)
        {
            if (cmdBuf[offset] == dw0)
            {
                return &cmdBuf[offset];
            }
            offset++;
        }

        return nullptr;
    }

    MOS_STATUS Av1VdencPktXe_Hpm::DumpSeqParFile()
    {
        ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::DumpSeqParFile());
        return MOS_STATUS_SUCCESS;
    }
#endif  // USE_CODECHAL_DEBUG_TOOL
}
