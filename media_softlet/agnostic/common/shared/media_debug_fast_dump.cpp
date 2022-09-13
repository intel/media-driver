/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_debug_fast_dump.cpp
//!

#include "media_debug_fast_dump_imp.hpp"

#if USE_MEDIA_DEBUG_TOOL

std::unique_ptr<MediaDebugFastDump> MediaDebugFastDump::m_instance = nullptr;

void MediaDebugFastDump::CreateInstance(MOS_INTERFACE &osItf, MediaCopyBaseState &mediaCopyItf)
{
    if (m_instance == nullptr)
    {
        m_instance =
#if __cplusplus < 201402L
            std::unique_ptr<MediaDebugFastDumpImp>(
                new MediaDebugFastDumpImp(osItf, mediaCopyItf));
#else
            std::make_unique<MediaDebugFastDumpImp>(osItf, mediaCopyItf);
#endif
    }
}

void MediaDebugFastDump::DestroyInstance()
{
    if (m_instance)
    {
        m_instance.reset();
    }
}

void MediaDebugFastDump::Dump(MOS_RESOURCE &res, std::string &&name, size_t dumpSize, size_t offset)
{
    if (m_instance)
    {
        dynamic_cast<MediaDebugFastDumpImp *>(
            m_instance.get())
            ->AddTask(res, std::move(name), dumpSize, offset);
    }
}

#endif  // USE_MEDIA_DEBUG_TOOL
