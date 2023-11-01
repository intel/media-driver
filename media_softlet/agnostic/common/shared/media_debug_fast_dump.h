/*
* Copyright (c) 2022-2023, Intel Corporation
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

#if ((_DEBUG || _RELEASE_INTERNAL)) && !defined(USE_MEDIA_DEBUG_TOOL)
#define USE_MEDIA_DEBUG_TOOL 1
#endif  // ((_DEBUG || _RELEASE_INTERNAL) && !EMUL) && !defined(USE_MEDIA_DEBUG_TOOL)

#if USE_MEDIA_DEBUG_TOOL

#include <functional>
#include <string>
#include "media_debug_serializer.h"
#include "media_copy_wrapper.h"

class MediaDebugFastDump
{
public:
    struct Config
    {
    private:
        template <typename T, T MIN, T MAX>
        class RangedValue final
        {
        public:
            RangedValue(T v)
            {
                value = v < MIN ? MIN : v > MAX ? MAX
                                                : v;
            }

            operator T() const
            {
                return value;
            }

        private:
            T value;
        };

        template <uint8_t MIN = 0, uint8_t MAX = 100>
        using RangedUint8 = RangedValue<uint8_t, MIN, MAX>;

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
        RangedUint8<0, 2> memUsagePolicy      = 0;   // 0: balance shared/local memory usage; 1: prioritize shared; 2: prioritize local
        RangedUint8<10>   maxPrioritizedMem   = 75;  // max percentage of prioritized memory can be used for fast dump, 10% - 100%
        RangedUint8<>     maxDeprioritizedMem = 75;  // max percentage of deprioritized memory can be used for fast dump, 0% - 100%

        // media copy configurations
        RangedUint8<> weightRenderCopy = 100;  // weight for render copy, 0 - 100
        RangedUint8<> weightVECopy     = 80;   // weight for VE copy, 0 - 100
        RangedUint8<> weightBLTCopy    = 20;   // weight for BLT copy, 0 - 100
                                               // when weightRenderCopy, weightVECopy and weightBLTCopy are all 0, use default copy method,
                                               // otherwise randomly select 1 of the 3 methods based on their weights, e.g., the chance of
                                               // selecting render copy is weightRenderCopy/(weightRenderCopy+weightVECopy+weightBLTCopy)

        // file/trace writing configurations
        RangedUint8<0, 2> writeDst      = 0;     // 0: file; 1: trace; 2: no write, for debug purpose
        RangedUint8<0, 2> writeMode     = 2;     // 0: binary; 1: text, valid when writeDst is 0; 2: adaptive
        size_t            bufferSize    = 0;     // buffer size in MB for buffered writing, valid when writeDst and writeMode are both 0
        bool              informOnError = true;  // dump 1 byte filename.error_info file instead of nothing when error occurs, valid when
                                                 // writeDst is 0
    };

    using DefaultSerializer = MediaDebugSerializer<void>;

public:
    static void CreateInstance(
        MOS_INTERFACE    &osItf,
        MediaCopyWrapper &mediaCopyWrapper,
        const Config     *cfg = nullptr);

    static void DestroyInstance();

    static bool IsGood();

    // if file name contains "w[0]_h[0]_p[0]", it will be replaced to "w[RealWidth]_h[RealHeight]_p[RealPitch]" by fast dump
    static void Dump(
        MOS_RESOURCE &res,
        std::string &&name,
        size_t        dumpSize = 0,
        size_t        offset   = 0,
        std::function<
            void(std::ostream &, const void *, size_t)>
            &&serializer = DefaultSerializer());

    static void Dump(
        const void   *res,
        std::string &&name,
        size_t        dumpSize = 0,
        size_t        offset   = 0,
        std::function<
            void(std::ostream &, const void *, size_t)>
            &&serializer = DefaultSerializer());

public:
    virtual ~MediaDebugFastDump() = default;

protected:
    MediaDebugFastDump() = default;

    MEDIA_CLASS_DEFINE_END(MediaDebugFastDump)
};

#endif  // USE_MEDIA_DEBUG_TOOL
