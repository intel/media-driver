/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     media_debug_fast_dump_imp.hpp
//!

#pragma once

#include "media_debug_fast_dump.h"

#if USE_MEDIA_DEBUG_TOOL

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <thread>
#include <vector>

class MediaDebugFastDumpImp : public MediaDebugFastDump
{
protected:
    using ResInfo = MOS_ALLOC_GFXRES_PARAMS;

    struct ResInfoCmp
    {
        bool operator()(const ResInfo &a, const ResInfo &b) const
        {
            return a.Type < b.Type           ? true
                   : a.dwWidth < b.dwWidth   ? true
                   : a.dwHeight < b.dwHeight ? true
                   : a.Format < b.Format     ? true
                                             : false;
        }
    };

    struct ResBase
    {
    public:
        virtual ~ResBase() = default;

    public:
        bool        occupied = false;
        size_t      size     = 0;
        size_t      offset   = 0;
        std::string name;
        std::function<
            void(std::ostream &, const void *, size_t)>
            serializer;
    };

    struct ResGfx final : public ResBase
    {
    public:
        static void SetOsInterface(PMOS_INTERFACE itf)
        {
            osItf = itf;
        }

    private:
        static PMOS_INTERFACE osItf;

    public:
        ~ResGfx()
        {
            if (Mos_ResourceIsNull(&res) == false)
            {
                osItf->pfnFreeResource(osItf, &res);
            }
        }

    public:
        bool         localMem = false;
        MOS_RESOURCE res      = {};
    };

    struct ResSys final : public ResBase
    {
    public:
        ~ResSys()
        {
            MOS_DeleteArray(res);
        }

    public:
        char *res = nullptr;
    };

    struct MemMng
    {
        Mos_MemPool policy = MOS_MEMPOOL_VIDEOMEMORY;
        size_t      cap    = 0;
        size_t      usage  = 0;
    };

    class BufferedWriter final
    {
    private:
        struct File
        {
            File(std::string &&n, size_t p, size_t s)
            {
                name = std::move(n);
                pos  = p;
                size = s;
            }

            std::string name;
            size_t      pos  = 0;
            size_t      size = 0;
        };

    public:
        BufferedWriter(size_t bufferSizeInMB)
        {
            m_bufSize = bufferSizeInMB << 20;  // byte
        }

        ~BufferedWriter()
        {
            std::ofstream ofs;

            Flush(ofs);
        }

        void operator()(std::string &&name, const void *data, size_t size)
        {
            if (m_buf.size() == 0)
            {
                m_buf.resize(m_bufSize);
                m_buf.shrink_to_fit();
            }

            size_t pos   = 0;
            size_t space = m_buf.size();
            if (!m_files.empty())
            {
                pos = m_files.back().pos + m_files.back().size;
                space -= pos;
            }

            if (size <= space)
            {
                memcpy(m_buf.data() + pos, data, size);
                m_files.emplace_back(std::move(name), pos, size);
            }
            else
            {
                std::ofstream ofs;

                Flush(ofs);

                ofs.open(name, std::ios_base::out | std::ios_base::binary);
                ofs.write(static_cast<const char *>(data), size);
            }
        }

        void operator()(
            std::string &&name,
            const void   *data,
            size_t        size,
            std::function<void(std::ostream &, const void *, size_t)> &&)
        {
            (*this)(std::move(name), data, size);
        }

    private:
        void Flush(std::ofstream &ofs)
        {
            std::for_each(
                m_files.begin(),
                m_files.end(),
                [this, &ofs](decltype(m_files)::const_reference file) {
                    ofs.open(file.name, std::ios_base::out | std::ios_base::binary);
                    ofs.write(m_buf.data() + file.pos, file.size);
                    ofs.close();
                });

            m_files.clear();
        }

    private:
        size_t            m_bufSize = 0;
        std::vector<char> m_buf;
        std::vector<File> m_files;
    };

protected:
    static size_t GetResSizeAndFixName(PGMM_RESOURCE_INFO pGmmResInfo, std::string &name)
    {
        auto resSize       = static_cast<size_t>(pGmmResInfo->GetSizeMainSurface());
        auto w             = static_cast<size_t>(pGmmResInfo->GetBaseWidth());
        auto h             = static_cast<size_t>(pGmmResInfo->GetBaseHeight());
        auto bytesPerPixel = static_cast<size_t>((pGmmResInfo->GetBitsPerPixel() + 7) >> 3);
        auto p             = static_cast<size_t>(pGmmResInfo->GetRenderPitch()) / bytesPerPixel;
        auto sizeY         = p * h * bytesPerPixel;

        // a lazy method to get real resource size without checking resource format
        if (sizeY * 3 <= resSize)
        {
            resSize = sizeY * 3;  // 444
        }
        else if (sizeY * 2 <= resSize)
        {
            resSize = sizeY * 2;  // 422
        }
        else if (sizeY * 3 / 2 <= resSize)
        {
            resSize = sizeY * 3 / 2;  // 420
        }
        else
        {
            resSize = sizeY;  // 400, buffer or packed YUV formats like Y210
        }

        name = std::regex_replace(
            name,
            std::regex("w\\[[0-9]+\\]_h\\[[0-9]+\\]_p\\[[0-9]+\\]"),
            "w[" + std::to_string(w) +
                "]_h[" + std::to_string(h) +
                "]_p[" + std::to_string(p) +
                "]");

        return resSize;
    }

public:
    MediaDebugFastDumpImp(
        MOS_INTERFACE    &osItf,
        MediaCopyWrapper &mediaCopyWrapper,
        const Config     *cfg) : m_osItf(osItf),
                             m_mediaCopyWrapper(mediaCopyWrapper)
    {
        std::unique_ptr<const Config> cfg1 = nullptr;

        const auto &c = cfg ? *cfg : *(cfg1 = decltype(cfg1)(new Config{}));

        m_allowDataLoss = c.allowDataLoss;

        ConfigureSamplingMode(c);
        ConfigureAllocator(c);
        ConfigureCopyMethod(c);
        ConfigureWriter(c);

        LaunchScheduler();

        ResGfx::SetOsInterface(&osItf);
    }

    ~MediaDebugFastDumpImp()
    {
        {
            std::lock_guard<std::mutex> lk(m_gfxMemAsyncData.mutex);
            m_gfxMemAsyncData.stopScheduler = true;
        }
        if (m_gfxMemAsyncData.scheduler.joinable())
        {
            m_gfxMemAsyncData.cond.notify_one();
            m_gfxMemAsyncData.scheduler.join();
        }

        {
            std::lock_guard<std::mutex> lk(m_sysMemAsyncData.mutex);
            m_sysMemAsyncData.stopScheduler = true;
        }
        if (m_sysMemAsyncData.scheduler.joinable())
        {
            m_sysMemAsyncData.cond.notify_one();
            m_sysMemAsyncData.scheduler.join();
        }
    }

    void operator()(
        MOS_RESOURCE &res,
        std::string &&name,
        size_t        dumpSize,
        size_t        offset,
        std::function<
            void(std::ostream &, const void *, size_t)>
            &&serializer)
    {
        if (m_2CacheTask() == false)
        {
            return;
        }

        ResInfo resInfo{};
        if (GetResInfo(res, resInfo) != MOS_STATUS_SUCCESS)
        {
            return m_writeError(
                name,
                "get_input_resource_info_failed");
        }

        // prepare resource pool and resource queue
        {
            std::unique_lock<std::mutex> lk(m_gfxMemAsyncData.mutex);

            auto &resArray = m_gfxMemAsyncData.resPool[resInfo];

            using CR = std::remove_reference<decltype(resArray)>::type::const_reference;

            auto resIt = std::find_if(
                resArray.begin(),
                resArray.end(),
                [](CR r) {
                    return r->occupied == false;
                });

            if (resIt == resArray.end())
            {
                auto tmpRes = std::make_shared<ResGfx>();
                if (m_allocate(resInfo, tmpRes->res) > 0)
                {
                    resArray.emplace_back(std::move(tmpRes));
                    --(resIt = resArray.end());
                }
                else if (!m_allowDataLoss && !resArray.empty())
                {
                    m_gfxMemAsyncData.cond.wait(
                        lk,
                        [&] {
                            resIt = std::find_if(
                                resArray.begin(),
                                resArray.end(),
                                [](CR r) {
                                    return r->occupied == false;
                                });
                            return resIt != resArray.end();
                        });
                }
                else
                {
                    return m_writeError(
                        name,
                        "discarded");
                }
            }

            if (m_mediaCopyWrapper.MediaCopy(&res, &(*resIt)->res, m_copyMethod()) !=
                MOS_STATUS_SUCCESS)
            {
                m_mediaCopyIsGood = false;
                return m_writeError(
                    name,
                    "input_surface_copy_failed");
            }
            else
            {
                m_mediaCopyIsGood = true;
            }

            (*resIt)->occupied   = true;
            (*resIt)->localMem   = resInfo.dwMemType != MOS_MEMPOOL_SYSTEMMEMORY;
            (*resIt)->size       = dumpSize;
            (*resIt)->offset     = offset;
            (*resIt)->name       = std::move(name);
            (*resIt)->serializer = std::move(serializer);
            m_gfxMemAsyncData.resQueue.emplace(*resIt);
        }

        m_gfxMemAsyncData.cond.notify_one();
    }

    void operator()(
        const void   *res,
        std::string &&name,
        size_t        dumpSize,
        size_t        offset,
        std::function<
            void(std::ostream &, const void *, size_t)>
            &&serializer)
    {
        if (m_2CacheTask() == false)
        {
            return;
        }

        if (res == nullptr)
        {
            return m_writeError(
                name,
                "resource_is_null");
        }

        if (dumpSize == 0)
        {
            return m_writeError(
                name,
                "dump_size_is_0");
        }

        // prepare resource pool and resource queue
        {
            std::unique_lock<std::mutex> lk(m_sysMemAsyncData.mutex);

            auto &resArray = m_sysMemAsyncData.resPool[dumpSize];

            using CR = std::remove_reference<decltype(resArray)>::type::const_reference;

            auto resIt = std::find_if(
                resArray.begin(),
                resArray.end(),
                [](CR r) {
                    return r->occupied == false;
                });

            if (resIt == resArray.end())
            {
                auto tmpRes = std::make_shared<ResSys>();
                if ((tmpRes->res = MOS_NewArray(char, dumpSize)) != nullptr)
                {
                    resArray.emplace_back(std::move(tmpRes));
                    --(resIt = resArray.end());
                }
                else if (!m_allowDataLoss && !resArray.empty())
                {
                    m_sysMemAsyncData.cond.wait(
                        lk,
                        [&] {
                            resIt = std::find_if(
                                resArray.begin(),
                                resArray.end(),
                                [](CR r) {
                                    return r->occupied == false;
                                });
                            return resIt != resArray.end();
                        });
                }
                else
                {
                    return m_writeError(
                        name,
                        "discarded");
                }
            }

            MOS_SecureMemcpy(
                (*resIt)->res,
                dumpSize,
                static_cast<const char *>(res) + offset,
                dumpSize);

            (*resIt)->occupied   = true;
            (*resIt)->size       = dumpSize;
            (*resIt)->offset     = 0;
            (*resIt)->name       = std::move(name);
            (*resIt)->serializer = std::move(serializer);
            m_sysMemAsyncData.resQueue.emplace(*resIt);
        }

        m_sysMemAsyncData.cond.notify_one();
    }

    bool IsGood() const
    {
        return m_mediaCopyIsGood;
    }

protected:
    void ConfigureSamplingMode(const Config &cfg)
    {
        if (cfg.samplingTime + cfg.samplingInterval == 0)  // sampling disabled
        {
            m_2CacheTask = [] { return true; };
        }
        else if (cfg.frameIdx == nullptr)  // time based sampling
        {
            using Clock = std::chrono::system_clock;
            using MS    = std::chrono::duration<size_t, std::milli>;

            const auto samplingTime     = MS(cfg.samplingTime);
            const auto samplingInterval = MS(cfg.samplingInterval);
            const auto startTime        = Clock::now();

            m_2CacheTask = [=] {
                auto elapsed = std::chrono::duration_cast<MS>(Clock::now() - startTime);
                return (elapsed % (samplingTime + samplingInterval)) < samplingTime;
            };
        }
        else  // frame index based sampling
        {
            const auto  samplingTime     = cfg.samplingTime;
            const auto  samplingInterval = cfg.samplingInterval;
            const auto *frameIdx         = cfg.frameIdx;

            m_2CacheTask = [=] {
                return (*frameIdx % (samplingTime + samplingInterval)) < samplingTime;
            };
        }
    }

    void ConfigureAllocator(const Config &cfg)
    {
#define TMP_ASSIGN(shared, local, dst)                              \
    m_memMng[0].dst = cfg.memUsagePolicy != 2 ? (shared) : (local); \
    m_memMng[1].dst = cfg.memUsagePolicy == 2 ? (shared) : (local)

        TMP_ASSIGN(
            MOS_MEMPOOL_SYSTEMMEMORY,
            MOS_MEMPOOL_VIDEOMEMORY,
            policy);

        auto adapter = m_osItf.pfnGetAdapterInfo(m_osItf.osStreamState);
        if (adapter)
        {
            TMP_ASSIGN(
                static_cast<size_t>(adapter->SystemSharedMemory),
                static_cast<size_t>(adapter->DedicatedVideoMemory),
                cap);
            m_memMng[0].cap = m_memMng[0].cap / 100 * cfg.maxPrioritizedMem;
            m_memMng[1].cap = m_memMng[1].cap / 100 * cfg.maxDeprioritizedMem;
        }
        m_memMng[0].cap = m_memMng[0].cap ? m_memMng[0].cap : -1;
        m_memMng[1].cap = m_memMng[1].cap || cfg.maxDeprioritizedMem == 0 ? m_memMng[1].cap : -1;

#undef TMP_ASSIGN

        if (m_memMng[1].cap == 0)
        {
            m_allocate = [this](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                if (m_memMng[0].usage >= m_memMng[0].cap)
                {
                    return 0;
                }
                resInfo.dwMemType = m_memMng[0].policy;
                if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &res) == MOS_STATUS_SUCCESS)
                {
                    assert(res.pGmmResInfo != nullptr);
                    auto resSize = static_cast<size_t>(res.pGmmResInfo->GetSizeAllocation());
                    m_memMng[0].usage += resSize;
                    return resSize;
                }
                return 0;
            };
        }
        else
        {
            auto allocator = [this](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                if (m_memMng[0].usage >= m_memMng[0].cap)
                {
                    if (m_memMng[1].usage >= m_memMng[1].cap)
                    {
                        return 0;
                    }
                    resInfo.dwMemType = m_memMng[1].policy;
                }
                else
                {
                    resInfo.dwMemType = m_memMng[0].policy;
                }
                if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &res) == MOS_STATUS_SUCCESS)
                {
                    assert(res.pGmmResInfo != nullptr);
                    auto resSize = static_cast<size_t>(res.pGmmResInfo->GetSizeAllocation());
                    m_memMng[0].usage += (resInfo.dwMemType == m_memMng[0].policy ? resSize : 0);
                    m_memMng[1].usage += (resInfo.dwMemType == m_memMng[1].policy ? resSize : 0);
                    return resSize;
                }
                return 0;
            };

            if (cfg.memUsagePolicy == 0)
            {
                m_allocate = [this, allocator](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                    std::random_device           rd;
                    std::mt19937                 gen(rd());
                    std::discrete_distribution<> distribution({
                        static_cast<double>(m_memMng[0].cap - m_memMng[0].usage),
                        static_cast<double>(m_memMng[1].cap - m_memMng[1].usage),
                    });
                    if (distribution(gen))
                    {
                        std::swap(m_memMng[0], m_memMng[1]);
                    }
                    return allocator(resInfo, res);
                };
            }
            else
            {
                m_allocate = std::move(allocator);
            }
        }
    }

    void ConfigureCopyMethod(const Config &cfg)
    {
        if (cfg.weightRenderCopy + cfg.weightVECopy + cfg.weightBLTCopy == 0)
        {
            m_copyMethod = [] {
                return MCPY_METHOD_DEFAULT;
            };
        }
        else
        {
            const auto weightRenderCopy = static_cast<double>(cfg.weightRenderCopy);
            const auto weightVECopy     = static_cast<double>(cfg.weightVECopy);
            const auto weightBLTCopy    = static_cast<double>(cfg.weightBLTCopy);

            m_copyMethod = [=] {
                const MCPY_METHOD methods[] = {
                    MCPY_METHOD_PERFORMANCE,
                    MCPY_METHOD_BALANCE,
                    MCPY_METHOD_POWERSAVING,
                };
                std::random_device           rd;
                std::mt19937                 gen(rd());
                std::discrete_distribution<> distribution({
                    weightRenderCopy,
                    weightVECopy,
                    weightBLTCopy,
                });
                return methods[distribution(gen)];
            };
        }
    }

    void ConfigureWriter(const Config &cfg)
    {
        switch (cfg.writeDst)
        {
        case 0: {
            if (cfg.writeMode == 0 && cfg.bufferSize > 0)
            {
                m_write = BufferedWriter(cfg.bufferSize);
            }
            else if (cfg.writeMode == 0)
            {
                m_write = [=](
                              std::string &&name,
                              const void   *data,
                              size_t        size,
                              std::function<void(std::ostream &, const void *, size_t)> &&) {
                    std::ofstream ofs(name, std::ios_base::out | std::ios_base::binary);
                    ofs.write(static_cast<const char *>(data), size);
                };
            }
            else if (cfg.writeMode == 1)
            {
                m_write = [](std::string &&name,
                              const void  *data,
                              size_t       size,
                              std::function<void(std::ostream &, const void *, size_t)>
                                  &&serializer) {
                    std::ofstream ofs(name, std::ios_base::out);
                    serializer(ofs, data, size);
                };
            }
            else
            {
                m_write = [](std::string &&name,
                              const void  *data,
                              size_t       size,
                              std::function<void(std::ostream &, const void *, size_t)>
                                  &&serializer) {
                    if (serializer.target_type() ==
                        std::function<void(std::ostream &, const void *, size_t)>(
                            DefaultSerializer())
                            .target_type())
                    {
                        std::ofstream ofs(name, std::ios_base::out | std::ios_base::binary);
                        ofs.write(static_cast<const char *>(data), size);
                    }
                    else
                    {
                        std::ofstream ofs(name, std::ios_base::out);
                        serializer(ofs, data, size);
                    }
                };
            }
            break;
        }
        case 1: {
            m_write = [](
                          std::string &&name,
                          const void   *data,
                          size_t        size,
                          std::function<void(std::ostream &, const void *, size_t)> &&) {
                MOS_TraceDataDump(name.c_str(), 0, data, size);
            };
            break;
        }
        case 2:
        default: {
            m_write = [](
                          std::string &&,
                          const void *,
                          size_t,
                          std::function<void(std::ostream &, const void *, size_t)> &&) {};
            break;
        }
        }

        if (cfg.informOnError && cfg.writeDst == 0)
        {
            m_writeError = [this](const std::string &name, const std::string &error) {
                static const char dummy = 0;
                std::thread       w(
                    [=] {
                        std::ofstream ofs(
                            name + "." + error,
                            std::ios_base::out | std::ios_base::binary);
                        ofs.write(&dummy, sizeof(dummy));
                    });
                w.detach();
            };
        }
        else
        {
            m_writeError = [](const std::string &, const std::string &) {};
        }
    }

    void LaunchScheduler()
    {
        m_gfxMemAsyncData.scheduler = std::thread(
            [this] {
                std::future<void> future;
                while (true)
                {
                    std::unique_lock<std::mutex> lk(m_gfxMemAsyncData.mutex);
                    m_gfxMemAsyncData.cond.wait(
                        lk,
                        [this] {
                            return (m_gfxMemAsyncData.ready4Dump && !m_gfxMemAsyncData.resQueue.empty()) || m_gfxMemAsyncData.stopScheduler;
                        });
                    if (m_gfxMemAsyncData.stopScheduler)
                    {
                        break;
                    }
                    auto qf                      = m_gfxMemAsyncData.resQueue.front();
                    m_gfxMemAsyncData.ready4Dump = false;
                    lk.unlock();
                    future = std::async(
                        std::launch::async,
                        [this, qf] {
                            DoDump(qf);
                            {
                                std::lock_guard<std::mutex> lk(m_gfxMemAsyncData.mutex);
                                m_gfxMemAsyncData.resQueue.front()->occupied = false;
                                m_gfxMemAsyncData.resQueue.pop();
                                m_gfxMemAsyncData.ready4Dump = true;
                            }
                            m_gfxMemAsyncData.cond.notify_all();
                        });
                }
                if (future.valid())
                {
                    future.wait();
                }
                std::lock_guard<std::mutex> lk(m_gfxMemAsyncData.mutex);
                while (!m_gfxMemAsyncData.resQueue.empty())
                {
                    DoDump(m_gfxMemAsyncData.resQueue.front());
                    m_gfxMemAsyncData.resQueue.front()->occupied = false;
                    m_gfxMemAsyncData.resQueue.pop();
                }
            });

        m_sysMemAsyncData.scheduler = std::thread(
            [this] {
                std::future<void> future;
                while (true)
                {
                    std::unique_lock<std::mutex> lk(m_sysMemAsyncData.mutex);
                    m_sysMemAsyncData.cond.wait(
                        lk,
                        [this] {
                            return (m_sysMemAsyncData.ready4Dump && !m_sysMemAsyncData.resQueue.empty()) || m_sysMemAsyncData.stopScheduler;
                        });
                    if (m_sysMemAsyncData.stopScheduler)
                    {
                        break;
                    }
                    auto qf                      = m_sysMemAsyncData.resQueue.front();
                    m_sysMemAsyncData.ready4Dump = false;
                    lk.unlock();
                    future = std::async(
                        std::launch::async,
                        [this, qf] {
                            m_write(
                                std::move(qf->name),
                                qf->res + qf->offset,
                                qf->size,
                                std::move(qf->serializer));
                            {
                                std::lock_guard<std::mutex> lk(m_sysMemAsyncData.mutex);
                                m_sysMemAsyncData.resQueue.front()->occupied = false;
                                m_sysMemAsyncData.resQueue.pop();
                                m_sysMemAsyncData.ready4Dump = true;
                            }
                            m_sysMemAsyncData.cond.notify_all();
                        });
                }
                if (future.valid())
                {
                    future.wait();
                }
                std::lock_guard<std::mutex> lk(m_sysMemAsyncData.mutex);
                while (!m_sysMemAsyncData.resQueue.empty())
                {
                    auto qf = m_sysMemAsyncData.resQueue.front();
                    m_write(
                        std::move(qf->name),
                        qf->res + qf->offset,
                        qf->size,
                        std::move(qf->serializer));
                    qf->occupied = false;
                    m_sysMemAsyncData.resQueue.pop();
                }
            });
    }

    MOS_STATUS GetResInfo(MOS_RESOURCE &res, ResInfo &resInfo) const
    {
        auto        ret     = MOS_STATUS_SUCCESS;
        auto        resType = m_osItf.pfnGetResType(&res);
        MOS_SURFACE details = {};

        if (resType != MOS_GFXRES_BUFFER)
        {
            details.Format = Format_Invalid;
        }

        ret = m_osItf.pfnGetResourceInfo(&m_osItf, &res, &details);

        resInfo.Type             = resType;
        resInfo.dwWidth          = details.dwWidth;
        resInfo.dwHeight         = details.dwHeight;
        resInfo.TileType         = MOS_TILE_LINEAR;
        resInfo.Format           = details.Format;
        resInfo.Flags.bCacheable = 1;

        return ret;
    }

    void DoDump(std::shared_ptr<ResGfx> res) const
    {
        MOS_LOCK_PARAMS lockFlags{};
        lockFlags.ReadOnly     = 1;
        lockFlags.TiledAsTiled = 1;

        auto         pRes   = &res->res;
        MOS_RESOURCE tmpRes = {};

        if (res->localMem)
        {
            // locking/reading resource from local graphic memory is extremely inefficient, so
            // copy resource to a temporary buffer allocated in shared memory before lock

            ResInfo resInfo{};
            if (GetResInfo(res->res, resInfo) != MOS_STATUS_SUCCESS)
            {
                return m_writeError(
                    res->name,
                    "get_internal_resource_info_failed");
            }
            resInfo.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;

            if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &tmpRes) != MOS_STATUS_SUCCESS)
            {
                return m_writeError(
                    res->name,
                    "allocate_tmp_resource_failed");
            }

            if (m_mediaCopyWrapper.MediaCopy(&res->res, &tmpRes, m_copyMethod()) !=
                MOS_STATUS_SUCCESS)
            {
                m_mediaCopyIsGood = false;
                return m_writeError(
                    res->name,
                    "internal_surface_copy_failed");
            }
            else
            {
                m_mediaCopyIsGood = true;
            }

            pRes = &tmpRes;
        }

        auto resSize = GetResSizeAndFixName(pRes->pGmmResInfo, res->name);
        if (resSize < res->offset + res->size)
        {
            return m_writeError(
                res->name,
                "incorrect_size_offset");
        }

        auto data = static_cast<const char *>(
            m_osItf.pfnLockResource(&m_osItf, pRes, &lockFlags));

        if (data)
        {
            m_write(
                std::move(res->name),
                data + res->offset,
                res->size == 0 ? resSize - res->offset : res->size,
                std::move(res->serializer));
            m_osItf.pfnUnlockResource(&m_osItf, pRes);
        }
        else
        {
            m_writeError(
                res->name,
                "lock_failed");
        }

        if (Mos_ResourceIsNull(&tmpRes) == false)
        {
            m_osItf.pfnFreeResource(&m_osItf, &tmpRes);
        }
    }

protected:
    template <typename RES>
    struct CommonAsyncData
    {
        std::thread             scheduler;
        std::mutex              mutex;
        std::condition_variable cond;
        std::queue<
            std::shared_ptr<RES>>
             resQueue;
        bool ready4Dump    = true;
        bool stopScheduler = false;
    };

    struct GfxMemAsyncData : CommonAsyncData<ResGfx>
    {
        std::map<
            ResInfo,
            std::vector<std::shared_ptr<ResGfx>>,
            ResInfoCmp>
            resPool;
    };

    struct SysMemAsyncData : CommonAsyncData<ResSys>
    {
        std::map<
            size_t,
            std::vector<std::shared_ptr<ResSys>>>
            resPool;
    };

protected:
    bool m_allowDataLoss = true;

    std::array<
        MemMng,
        2>
        m_memMng;

    std::function<
        bool()>
        m_2CacheTask;

    std::function<
        size_t(ResInfo &, MOS_RESOURCE &)>
        m_allocate;

    std::function<
        MCPY_METHOD()>
        m_copyMethod;

    std::function<
        void(
            std::string &&,
            const void *,
            size_t,
            std::function<void(std::ostream &, const void *, size_t)> &&)>
        m_write;

    std::function<
        void(const std::string &, const std::string &)>
        m_writeError;

    mutable std::atomic_bool m_mediaCopyIsGood{true};

    MOS_INTERFACE    &m_osItf;
    MediaCopyWrapper &m_mediaCopyWrapper;

    GfxMemAsyncData m_gfxMemAsyncData;
    SysMemAsyncData m_sysMemAsyncData;

    MEDIA_CLASS_DEFINE_END(MediaDebugFastDumpImp)
};

PMOS_INTERFACE MediaDebugFastDumpImp::ResGfx::osItf = nullptr;

#endif  // USE_MEDIA_DEBUG_TOOL
