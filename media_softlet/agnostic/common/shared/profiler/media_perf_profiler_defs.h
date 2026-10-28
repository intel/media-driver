/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     media_perf_profiler_defs.h
//! \brief    Definitions of the media performance profiler output file format that are shared
//!           between MediaPerfProfiler and MediaPerfProfilerNext.
//! \details  Both profilers write the same binary layout, so anything the parsing tools rely on
//!           has to be described in exactly one place.

#ifndef __MEDIA_PERF_PROFILER_DEFS_H__
#define __MEDIA_PERF_PROFILER_DEFS_H__

#include <stdint.h>

#define MEDIA_PERF_TS_CORR_VERSION      1        //!< Layout version of MediaPerfTsCorrelation
#define MEDIA_PERF_TS_CORR_FLAG_VALID   (1 << 0) //!< The correlation point was successfully queried
#define MEDIA_PERF_TS_CORR_FLAG_MEDIA   (1 << 1) //!< Ticks are in the media CS domain, otherwise in the GT OA domain

//!
//! \struct  MediaPerfTsCorrelation
//! \brief   One CPU/GPU timestamp correlation point, as returned by the
//!          GTDI_FNC_GET_GPU_CPU_TIMESTAMPS / GTDI_FNC_GET_MEDIA_GPU_CPU_TIMESTAMPS escapes.
//! \details This is a binary format definition: it is written verbatim into the tsCorrBegin and
//!          tsCorrEnd arrays of MediaPerfBinHeader, so its size must stay exactly 32 bytes and
//!          current fields must never be reordered. The perf entries do not carry a copy. Be sure
//!          to notify tool owners when changing it, and bump MEDIA_PERF_TS_CORR_VERSION.
//!
struct MediaPerfTsCorrelation
{
    uint64_t cpuPerfTicks;  //!< CPU (QPC) ticks at the correlation point
    uint64_t gpuPerfTicks;  //!< GPU ticks at the same point, in the domain given by flags
    uint32_t cpuPerfFreq;   //!< CPU tick frequency, in Hz
    uint32_t gpuPerfFreq;   //!< GPU tick frequency, in Hz
    uint32_t flags;         //!< MEDIA_PERF_TS_CORR_FLAG_*
    uint32_t version;       //!< MEDIA_PERF_TS_CORR_VERSION
};

//! \brief Values of the binVersion field of the node header dword at offset 0 of the output file.
#define MEDIA_PERF_BIN_VERSION_LEGACY   0 //!< No file header, first entry at offset sizeof(node header dword)
#define MEDIA_PERF_BIN_VERSION_TS_CORR  1 //!< MediaPerfBinHeader at offset 0, written when "Perf Profiler
                                          //!< Timestamp Correlation" is set

#define MEDIA_PERF_BIN_HEADER_MAGIC     0x4652504D //!< 'M','P','R','F' in file byte order
#define MEDIA_PERF_BIN_HEADER_SIZE      256        //!< sizeof(MediaPerfBinHeader)
#define MEDIA_PERF_BIN_HEADER_RESERVED  24         //!< Spare dwords at the end of MediaPerfBinHeader

//! \brief Index of a clock domain in MediaPerfBinHeader::tsCorrBegin / tsCorrEnd.
#define MEDIA_PERF_TS_CORR_DOMAIN_GT     0 //!< GT (OA) domain
#define MEDIA_PERF_TS_CORR_DOMAIN_MEDIA  1 //!< Media (command streamer) domain
#define MEDIA_PERF_TS_CORR_DOMAIN_COUNT  2

//!
//! \struct  MediaPerfBinHeader
//! \brief   File header of a MEDIA_PERF_BIN_VERSION_TS_CORR profiler output file.
//! \details This is a binary format definition, written at offset 0 of the output file (or of every
//!          blob of a multi process single binary container), immediately followed by the perf
//!          entries. Its size must stay a multiple of 8 and current fields must never be
//!          reordered; growing it only means writing a larger headerSize, because parsers take the
//!          offset of the first entry from that field rather than from their own sizeof. Be sure to
//!          notify tool owners when changing it.
//!
//!          Two correlation points per domain are stored, one sampled when the profiler is
//!          initialized and one when the data is written out, so a parser can recover the actual
//!          ratio of the two clocks instead of trusting the nominal frequencies. A single point
//!          leaves a rate error of a few tens of ppm, which is hundreds of microseconds over a ten
//!          second capture.
//!
struct MediaPerfBinHeader
{
    uint32_t nodeHeader;    //!< The node header dword, unchanged from MEDIA_PERF_BIN_VERSION_LEGACY
    uint32_t magic;         //!< MEDIA_PERF_BIN_HEADER_MAGIC
    uint32_t headerSize;    //!< Size of this header, i.e. the offset of the first perf entry
    uint32_t entrySize;     //!< Size of one perf entry, i.e. the stride of the entry array
    uint32_t entryCount;    //!< Number of entries that follow, 0 when unknown, see below
    uint32_t processId;     //!< Process that produced the entries
    uint32_t reserved0[2];  //!< Keeps tsCorrBegin 8 byte aligned
    //! Correlation sampled when the profiler was initialized, indexed by MEDIA_PERF_TS_CORR_DOMAIN_*
    MediaPerfTsCorrelation tsCorrBegin[MEDIA_PERF_TS_CORR_DOMAIN_COUNT];
    //! Correlation sampled when the entries were written out, indexed by MEDIA_PERF_TS_CORR_DOMAIN_*
    MediaPerfTsCorrelation tsCorrEnd[MEDIA_PERF_TS_CORR_DOMAIN_COUNT];
    uint32_t reserved[MEDIA_PERF_BIN_HEADER_RESERVED];
};

static_assert(sizeof(MediaPerfBinHeader) == MEDIA_PERF_BIN_HEADER_SIZE,
    "MediaPerfBinHeader is a binary format, its size is written into every output file.");

#endif // __MEDIA_PERF_PROFILER_DEFS_H__
