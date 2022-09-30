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

#include <string>
#include "media_copy.h"

class MediaDebugFastDump
{
public:
    struct Config
    {
    private:
        template <size_t MIN = 0, size_t MAX = 100>
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
        bool allowDataLoss = true;  // allow dumped data loss to reduce perf impact

        // sampling mode configurations
        const size_t *frameIdx         = nullptr;  // pointer to the frame index managed by user
        size_t        samplingTime     = 0;        // sampling time in ms or frame index
        size_t        samplingInterval = 0;        // sampling interval in ms or frame index
                                                   // when sampling time and sampling interval are not both 0, sampling mode is enabled,
                                                   // if frameIdx is null, sampling is based on ms, otherwise on frame index, e.g.,
                                                   // if ElapsedTime%(samplingTime+samplingInterval) <= samplingTime, current dump
                                                   // task will be queued, otherwise discarded, ElapsedTime is equal to CurrentFrameIndex
                                                   // for frame index based sampling and CurrentTime-StartTime otherwise

        // graphic memory usage configurations
        bool            balanceMemUsage     = true;  // balance the usage of shared/local graphic memory, use shared memory first if disabled
        RangedValue<10> maxPercentSharedMem = 75;    // max percentage of shared graphic memory can be used for fast dump, 10% - 100%
        RangedValue<>   maxPercentLocalMem  = 75;    // max percentage of local graphic memory can be used for fast dump, 0% - 100%

        // media copy configurations
        RangedValue<> weightRenderCopy = 100;  // weight for render copy, 0 - 100
        RangedValue<> weightVECopy     = 80;   // weight for VE copy, 0 - 100
        RangedValue<> weightBLTCopy    = 20;   // weight for BLT copy, 0 - 100
                                               // when weightRenderCopy, weightVECopy and weightBLTCopy are all 0, use default copy method,
                                               // otherwise randomly select 1 of the 3 methods based on their weights, e.g., the chance of
                                               // selecting render copy is weightRenderCopy/(weightRenderCopy+weightVECopy+weightBLTCopy)

        // file writing configurations
        size_t bufferSize4Write = 0;      // buffer size in MB for buffered file writer, write is not buffered when size is 0
        bool   write2File       = true;   // write dumped data to file
        bool   write2Trace      = false;  // write dumped data to trace
        bool   informOnError    = true;   // dump 1 byte filename.error_info file instead of nothing when error occurs
    };

public:
    static void CreateInstance(
        MOS_INTERFACE      &osItf,
        MediaCopyBaseState &mediaCopyItf,
        const Config       *cfg = nullptr);

    static void DestroyInstance();

    // if file name contains "w[0]_h[0]_p[0]", it will be replaced to "w[RealWidth]_h[RealHeight]_p[RealPitch]" by fast dump
    static void Dump(
        MOS_RESOURCE &res,
        std::string &&name,
        size_t        dumpSize = 0,
        size_t        offset   = 0);

public:
    virtual ~MediaDebugFastDump() = default;

protected:
    MediaDebugFastDump() = default;

    MEDIA_CLASS_DEFINE_END(MediaDebugFastDump)
};

#endif  // USE_MEDIA_DEBUG_TOOL
