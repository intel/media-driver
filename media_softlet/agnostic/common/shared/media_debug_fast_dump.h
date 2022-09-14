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
//! \file     media_debug_fast_dump.h
//!

#pragma once

#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL) && !defined(USE_MEDIA_DEBUG_TOOL)
#define USE_MEDIA_DEBUG_TOOL 1
#endif  // ((_DEBUG || _RELEASE_INTERNAL) && !EMUL) && !defined(USE_MEDIA_DEBUG_TOOL)

#if USE_MEDIA_DEBUG_TOOL

#include <memory>
#include <string>
#include "media_copy.h"

class MediaDebugFastDump
{
public:
    struct Config
    {
    private:
        template <size_t MIN, size_t MAX = 100>
        class RangedValue final
        {
        public:
            RangedValue(size_t v)
            {
                value = v < MIN ? MIN : v > MAX ? MAX
                                                : v;
            }

            operator size_t() const
            {
                return value;
            }

        private:
            size_t value;
        };

    public:
        bool            write2File          = true;   // write dumped data to file
        bool            write2Trace         = false;  // write dumped data to trace
        bool            informOnError       = true;   // dump 1 byte filename.error_info file instead of nothing when error occurs
        bool            allowDataLoss       = true;   // allow dumped data loss to reduce perf impact
        RangedValue<10> maxPercentSharedMem = 75;     // max percentage of shared graphic memory used for fast dump, 10% - 100%
        RangedValue<0>  maxPercentLocalMem  = 0;      // max percentage of local graphic memory used for fast dump, 0% - 100%,
                                                      // very slow to lock, little perf impact on main workload but significantly
                                                      // slows down dump, use with caution (suggest using local memory only when
                                                      // allowDataLoss must be true and shared memory is not enough)
        size_t samplingTime     = 0;                  // sampling time in ms
        size_t samplingInterval = 0;                  // sampling interval in ms
                                                      // when both sampling time and interval are positive, sampling mode is enabled
    };

public:
    static void CreateInstance(MOS_INTERFACE &osItf, MediaCopyBaseState &mediaCopyItf, const Config *cfg = nullptr);

    static void DestroyInstance();

    static void Dump(MOS_RESOURCE &res, std::string &&name, size_t dumpSize = 0, size_t offset = 0);

protected:
    static std::unique_ptr<MediaDebugFastDump> m_instance;

public:
    virtual ~MediaDebugFastDump() = default;

protected:
    MediaDebugFastDump() = default;

    MEDIA_CLASS_DEFINE_END(MediaDebugFastDump)
};

#endif  // USE_MEDIA_DEBUG_TOOL
