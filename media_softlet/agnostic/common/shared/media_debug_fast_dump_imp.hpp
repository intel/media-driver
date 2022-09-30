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
//! \file     media_debug_fast_dump_imp.hpp
//!

#pragma once

#include "media_debug_fast_dump.h"

#if USE_MEDIA_DEBUG_TOOL

#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
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

    struct Res
    {
    public:
        static void SetOsInterface(PMOS_INTERFACE itf)
        {
            osItf = itf;
        }

    private:
        static PMOS_INTERFACE osItf;

    public:
        ~Res()
        {
            if (Mos_ResourceIsNull(&res) == false)
            {
                osItf->pfnFreeResource(osItf, &res);
            }
        }

    public:
        bool         occupied = false;
        bool         localMem = false;
        MOS_RESOURCE res      = {};
        size_t       size     = 0;
        size_t       offset   = 0;
        std::string  name;
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

                ofs.open(name);
                ofs.write(static_cast<const char *>(data), size);
            }
        }

    private:
        void Flush(std::ofstream &ofs)
        {
            std::for_each(
                m_files.begin(),
                m_files.end(),
                [this, &ofs](decltype(m_files)::const_reference file) {
                    ofs.open(file.name);
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
        auto resSize = static_cast<size_t>(pGmmResInfo->GetSizeMainSurface());
        auto w       = static_cast<size_t>(pGmmResInfo->GetBaseWidth());
        auto h       = static_cast<size_t>(pGmmResInfo->GetBaseHeight());
        auto p       = static_cast<size_t>(pGmmResInfo->GetRenderPitch());
        auto sizeY   = static_cast<size_t>(p * h * ((pGmmResInfo->GetBitsPerPixel() + 7) >> 3));

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
            resSize = sizeY;  // 400 or buffer
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
        MOS_INTERFACE      &osItf,
        MediaCopyBaseState &mediaCopyItf,
        const Config       *cfg) : m_osItf(osItf),
                             m_mediaCopyItf(mediaCopyItf)
    {
        std::unique_ptr<const Config> cfg1 = nullptr;

        const auto &c = cfg ? *cfg : *(cfg1 = decltype(cfg1)(new Config{}));

        m_allowDataLoss = c.allowDataLoss;

        ConfigureSamplingMode(c);
        ConfigureAllocator(c);
        ConfigureCopyMethod(c);
        ConfigureWriter(c);

        LaunchScheduler();

        Res::SetOsInterface(&osItf);
    }

    ~MediaDebugFastDumpImp()
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_stopScheduler = true;
        }

        if (m_scheduler.joinable())
        {
            m_cond.notify_one();
            m_scheduler.join();
        }
    }

    void operator()(MOS_RESOURCE &res, std::string &&name, size_t dumpSize, size_t offset)
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
            std::unique_lock<std::mutex> lk(m_mutex);

            auto &resArray = m_resPool[resInfo];

            using CR = std::remove_reference<decltype(resArray)>::type::const_reference;

            auto resIt = std::find_if(
                resArray.begin(),
                resArray.end(),
                [](CR r) {
                    return r->occupied == false;
                });

            if (resIt == resArray.end())
            {
                auto tmpRes = std::make_shared<Res>();
                if (m_allocate(resInfo, tmpRes->res) > 0)
                {
                    resArray.emplace_back(std::move(tmpRes));
                    --(resIt = resArray.end());
                }
                else if (!m_allowDataLoss && !resArray.empty())
                {
                    m_cond.wait(
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

            if (m_mediaCopyItf.SurfaceCopy(&res, &(*resIt)->res, m_copyMethod()) !=
                MOS_STATUS_SUCCESS)
            {
                return m_writeError(
                    name,
                    "input_surface_copy_failed");
            }

            (*resIt)->occupied = true;
            (*resIt)->localMem = resInfo.dwMemType != MOS_MEMPOOL_SYSTEMMEMORY;
            (*resIt)->size     = dumpSize;
            (*resIt)->offset   = offset;
            (*resIt)->name     = std::move(name);
            m_resQueue.emplace(*resIt);
        }

        m_cond.notify_one();
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
                return (elapsed % (samplingTime + samplingInterval)) <= samplingTime;
            };
        }
        else  // frame index based sampling
        {
            const auto  samplingTime     = cfg.samplingTime;
            const auto  samplingInterval = cfg.samplingInterval;
            const auto *frameIdx         = cfg.frameIdx;

            m_2CacheTask = [=] {
                return (*frameIdx % (samplingTime + samplingInterval)) <= samplingTime;
            };
        }
    }

    void ConfigureAllocator(const Config &cfg)
    {
        m_memMng1st.policy = MOS_MEMPOOL_SYSTEMMEMORY;
        m_memMng2nd.policy = MOS_MEMPOOL_VIDEOMEMORY;

        auto adapter = MosInterface::GetAdapterInfo(m_osItf.osStreamState);
        if (adapter)
        {
            m_memMng1st.cap = static_cast<size_t>(adapter->SystemSharedMemory) /
                              100 * cfg.maxPercentSharedMem;

            m_memMng2nd.cap = static_cast<size_t>(adapter->DedicatedVideoMemory) /
                              100 * cfg.maxPercentLocalMem;
        }
        m_memMng1st.cap = m_memMng1st.cap ? m_memMng1st.cap : -1;

        if (m_memMng2nd.cap == 0)
        {
            m_allocate = [this](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                if (m_memMng1st.usage >= m_memMng1st.cap)
                {
                    return 0;
                }
                resInfo.dwMemType = m_memMng1st.policy;
                if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &res) == MOS_STATUS_SUCCESS)
                {
                    assert(res.pGmmResInfo != nullptr);
                    auto resSize = static_cast<size_t>(res.pGmmResInfo->GetSizeAllocation());
                    m_memMng1st.usage += resSize;
                    return resSize;
                }
                return 0;
            };
        }
        else
        {
            auto allocator = [this](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                if (m_memMng1st.usage >= m_memMng1st.cap)
                {
                    if (m_memMng2nd.usage >= m_memMng2nd.cap)
                    {
                        return 0;
                    }
                    resInfo.dwMemType = m_memMng2nd.policy;
                }
                else
                {
                    resInfo.dwMemType = m_memMng1st.policy;
                }
                if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &res) == MOS_STATUS_SUCCESS)
                {
                    assert(res.pGmmResInfo != nullptr);
                    auto resSize = static_cast<size_t>(res.pGmmResInfo->GetSizeAllocation());
                    m_memMng1st.usage += (resInfo.dwMemType == m_memMng1st.policy ? resSize : 0);
                    m_memMng2nd.usage += (resInfo.dwMemType == m_memMng2nd.policy ? resSize : 0);
                    return resSize;
                }
                return 0;
            };

            if (cfg.balanceMemUsage)
            {
                m_allocate = [this, allocator](ResInfo &resInfo, MOS_RESOURCE &res) -> size_t {
                    std::random_device           rd;
                    std::mt19937                 gen(rd());
                    std::discrete_distribution<> distribution({
                        static_cast<double>(m_memMng1st.cap - m_memMng1st.usage),
                        static_cast<double>(m_memMng2nd.cap - m_memMng2nd.usage),
                    });
                    if (distribution(gen))
                    {
                        std::swap(m_memMng1st, m_memMng2nd);
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
        std::function<
            void(std::string &&, const void *, size_t)>
            fileWriter;

        if (cfg.write2File)
        {
            if (cfg.bufferSize4Write > 0)
            {
                fileWriter = BufferedWriter(cfg.bufferSize4Write);
            }
            else
            {
                fileWriter = [](std::string &&name, const void *data, size_t size) {
                    std::ofstream ofs(name);
                    ofs.write(static_cast<const char *>(data), size);
                };
            }
        }

        if (cfg.write2File && !cfg.write2Trace)
        {
            m_write = [this, fileWriter](std::string &&name, const void *data, size_t size) {
                fileWriter(std::move(name), data, size);
            };
        }
        else if (!cfg.write2File && cfg.write2Trace)
        {
            m_write = [](std::string &&name, const void *data, size_t size) {
                MOS_TraceDataDump(name.c_str(), 0, data, size);
            };
        }
        else if (cfg.write2File && cfg.write2Trace)
        {
            m_write = [this, fileWriter](std::string &&name, const void *data, size_t size) {
                auto future = std::async(
                    std::launch::async,
                    fileWriter,
                    std::move(name),
                    data,
                    size);
                MOS_TraceDataDump(name.c_str(), 0, data, size);
                future.wait();
            };
        }
        else
        {
            // should not happen
            m_write = [](std::string &&, const void *, size_t) {};
        }

        if (cfg.informOnError && cfg.write2File)
        {
            m_writeError = [this](const std::string &name, const std::string &error) {
                static const char dummy = 0;
                std::thread       w(
                    [&] {
                        std::ofstream ofs(name + "." + error);
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
        m_scheduler = std::thread(
            [this] {
                std::future<void> future;
                while (true)
                {
                    std::unique_lock<std::mutex> lk(m_mutex);
                    m_cond.wait(
                        lk,
                        [this] {
                            return (m_ready4Dump && !m_resQueue.empty()) || m_stopScheduler;
                        });
                    if (m_stopScheduler)
                    {
                        break;
                    }
                    if (m_ready4Dump && !m_resQueue.empty())
                    {
                        auto qf      = m_resQueue.front();
                        m_ready4Dump = false;
                        lk.unlock();
                        future = std::async(
                            std::launch::async,
                            [this, qf] {
                                DoDump(qf);
                                {
                                    std::lock_guard<std::mutex> lk(m_mutex);
                                    m_resQueue.front()->occupied = false;
                                    m_resQueue.pop();
                                    m_ready4Dump = true;
                                }
                                m_cond.notify_all();
                            });
                    }
                }
                if (future.valid())
                {
                    future.wait();
                }
                std::lock_guard<std::mutex> lk(m_mutex);
                while (!m_resQueue.empty())
                {
                    DoDump(m_resQueue.front());
                    m_resQueue.front()->occupied = false;
                    m_resQueue.pop();
                }
            });
    }

    MOS_STATUS GetResInfo(MOS_RESOURCE &res, ResInfo &resInfo) const
    {
        auto        ret     = MOS_STATUS_SUCCESS;
        auto        resType = GetResType(&res);
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

    void DoDump(std::shared_ptr<Res> res) const
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

            if (m_mediaCopyItf.SurfaceCopy(&res->res, &tmpRes, m_copyMethod()) !=
                MOS_STATUS_SUCCESS)
            {
                return m_writeError(
                    res->name,
                    "internal_surface_copy_failed");
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

        auto data = static_cast<const uint8_t *>(
            m_osItf.pfnLockResource(&m_osItf, pRes, &lockFlags));

        if (data)
        {
            m_write(
                std::move(res->name),
                data + res->offset,
                res->size == 0 ? resSize - res->offset : res->size);
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
    // global configurations
    bool   m_allowDataLoss = true;
    MemMng m_memMng1st;
    MemMng m_memMng2nd;

    std::thread m_scheduler;

    std::map<
        ResInfo,
        std::vector<std::shared_ptr<Res>>,
        ResInfoCmp>
        m_resPool;  // synchronization needed

    std::queue<
        std::shared_ptr<Res>>
        m_resQueue;  // synchronization needed

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
        void(std::string &&, const void *, size_t)>
        m_write;

    std::function<
        void(const std::string &, const std::string &)>
        m_writeError;

    // threads intercommunication flags, synchronization needed
    bool m_ready4Dump    = true;
    bool m_stopScheduler = false;

    std::mutex              m_mutex;
    std::condition_variable m_cond;

    MOS_INTERFACE      &m_osItf;
    MediaCopyBaseState &m_mediaCopyItf;

    MEDIA_CLASS_DEFINE_END(MediaDebugFastDumpImp)
};

PMOS_INTERFACE MediaDebugFastDumpImp::Res::osItf = nullptr;

#endif  // USE_MEDIA_DEBUG_TOOL
