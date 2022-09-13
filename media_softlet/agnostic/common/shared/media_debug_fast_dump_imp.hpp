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
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#define _CHK_STATUS_RETURN(exp)      \
    if ((exp) != MOS_STATUS_SUCCESS) \
    {                                \
        return;                      \
    }

class MediaDebugFastDumpImp : public MediaDebugFastDump
{
public:
    MediaDebugFastDumpImp(
        MOS_INTERFACE      &osItf,
        MediaCopyBaseState &mediaCopyItf) : m_osItf(osItf),
                                            m_mediaCopyItf(mediaCopyItf)
    {
        // configure data write mode
        {
            // todo: decide write2File and write2Trace
            bool write2File  = true;
            bool write2Trace = false;
            if (write2File && !write2Trace)
            {
                m_write = [](const std::string &name, const void *data, size_t size) {
                    std::ofstream ofs(name);
                    ofs.write(static_cast<const char *>(data), size);
                };
            }
            else if (!write2File && write2Trace)
            {
                m_write = [](const std::string &name, const void *data, size_t size) {
                    MOS_TraceDataDump(name.c_str(), 0, data, size);
                };
            }
            else if (write2File && write2Trace)
            {
                m_write = [](const std::string &name, const void *data, size_t size) {
                    auto future = std::async(
                        std::launch::async,
                        [&] {
                            std::ofstream ofs(name);
                            ofs.write(static_cast<const char *>(data), size);
                        });
                    MOS_TraceDataDump(name.c_str(), 0, data, size);
                    future.wait();
                };
            }
            else
            {
                // should not happen
                m_write = [](const std::string &, const void *, size_t) {};
            }
        }

        // launch scheduler thread
        m_scheduler =
#if __cplusplus < 201402L
            std::unique_ptr<std::thread>(new
#else
            std::make_unique<std::thread>(
#endif
            std::thread(
                [this] {
                    ScheduleTasks();
                }));

        Res::SetOsInterface(&osItf);
    }

    ~MediaDebugFastDumpImp()
    {
        if (m_scheduler)
        {
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_stopScheduler = true;
            }

            if (m_scheduler->joinable())
            {
                m_cond.notify_one();
                m_scheduler->join();
            }
        }
    }

    void AddTask(MOS_RESOURCE &res, std::string &&name, size_t dumpSize, size_t offset)
    {
        size_t resSize = 0;

        if (res.pGmmResInfo == nullptr ||
            (resSize = static_cast<size_t>(res.pGmmResInfo->GetSizeMainSurface())) <
                offset + dumpSize)
        {
            return;
        }

        ResInfo resInfo{};

        // fill in resource info
        {
            auto        resType = GetResType(&res);
            MOS_SURFACE details{};

            if (resType != MOS_GFXRES_BUFFER)
            {
                details.Format = Format_Invalid;
            }

            _CHK_STATUS_RETURN(m_osItf.pfnGetResourceInfo(&m_osItf, &res, &details));

            resInfo.Type             = resType;
            resInfo.dwWidth          = details.dwWidth;
            resInfo.dwHeight         = details.dwHeight;
            resInfo.TileType         = MOS_TILE_LINEAR;
            resInfo.Format           = details.Format;
            resInfo.Flags.bCacheable = 1;
            resInfo.dwMemType        = MOS_MEMPOOL_SYSTEMMEMORY;
        }

        // prepare resource pool and resource queue
        {
            decltype(m_resPool)::mapped_type::iterator resIt;
            std::lock_guard<std::mutex>                lk(m_mutex);
            auto                                      &resArray = m_resPool[resInfo];
            for (resIt = resArray.begin(); resIt != resArray.end(); resIt++)
            {
                if ((*resIt)->occupied == false)
                {
                    break;
                }
            }
            if (resIt == resArray.end())
            {
                if (m_currentMemUsage + resSize > m_memCap)
                {
                    return;
                }

                auto tmpRes = std::make_shared<Res>();
                if (m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &tmpRes->res) != MOS_STATUS_SUCCESS)
                {
                    resInfo.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
                    _CHK_STATUS_RETURN(m_osItf.pfnAllocateResource(&m_osItf, &resInfo, &tmpRes->res));
                }
                resArray.emplace_back(tmpRes);
                resIt = resArray.end();
                --resIt;
                m_currentMemUsage += resSize;
            }

            _CHK_STATUS_RETURN(m_mediaCopyItf.SurfaceCopy(&res, &(*resIt)->res, MCPY_METHOD_PERFORMANCE));

            (*resIt)->size     = (dumpSize == 0) ? resSize - offset : dumpSize;
            (*resIt)->offset   = offset;
            (*resIt)->name     = std::move(name);
            (*resIt)->occupied = true;
            m_resQueue.emplace(*resIt);
        }

        m_cond.notify_one();
    }

protected:
    using ResInfo = MOS_ALLOC_GFXRES_PARAMS;

    struct ResInfoCmp
    {
        bool operator()(const ResInfo &a, const ResInfo &b) const
        {
            return a.Type < b.Type ? true : a.dwWidth < b.dwWidth ? true
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
        MOS_RESOURCE res      = {};
        size_t       size     = 0;
        size_t       offset   = 0;
        std::string  name;
    };

protected:
    void ScheduleTasks()
    {
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
                        m_cond.notify_one();
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
    }

    void DoDump(std::shared_ptr<Res> res) const
    {
        MOS_LOCK_PARAMS lockFlags{};
        lockFlags.ReadOnly     = 1;
        lockFlags.TiledAsTiled = 1;

        auto data = static_cast<const uint8_t *>(
            m_osItf.pfnLockResource(&m_osItf, &res->res, &lockFlags));

        if (data)
        {
            m_write(res->name, data + res->offset, res->size);
            m_osItf.pfnUnlockResource(&m_osItf, &res->res);
        }
        else if (m_informWhenLockFails)
        {
            const uint32_t dummy = 0xdeadbeef;
            m_write(res->name + ".lock_failed", &dummy, sizeof(dummy));
        }
    }

protected:
    // global configurations
    bool m_informWhenLockFails = true;

    std::unique_ptr<
        std::thread>
        m_scheduler;

    std::map<
        ResInfo,
        std::vector<std::shared_ptr<Res>>,
        ResInfoCmp>
        m_resPool;  // synchronization needed

    std::queue<
        std::shared_ptr<Res>>
        m_resQueue;  // synchronization needed

    std::function<
        void(const std::string &, const void *, size_t)>
        m_write;

    // threads intercommunication flags, synchronization needed
    bool m_ready4Dump    = true;
    bool m_stopScheduler = false;

    std::mutex              m_mutex;
    std::condition_variable m_cond;

    size_t m_currentMemUsage = 0;
    size_t m_memCap          = 2 << 30;  // 2GB

    MOS_INTERFACE      &m_osItf;
    MediaCopyBaseState &m_mediaCopyItf;

MEDIA_CLASS_DEFINE_END(MediaDebugFastDumpImp)
};

PMOS_INTERFACE MediaDebugFastDumpImp::Res::osItf = nullptr;

#undef _CHK_STATUS_RETURN

#endif  // USE_MEDIA_DEBUG_TOOL
