/*
 * Copyright © 2023 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *      Xu, Zhengguo <zhengguo.xu@intel.com>
 */

#include <fcntl.h>
#include <algorithm>
#include <unistd.h>
#include "dma-buf.h"
#include "xf86drm.h"
#include "xf86atomic.h"
#include "mos_bufmgr_xe.h"
#include "mos_synchronization_xe.h"
#include "mos_utilities.h"
#include "mos_bufmgr_util_debug.h"

/**
 * @flags indicates to create opration:
 * If flags=0(recommended), it will create a syncobj with not signaled.
 * If flags=DRM_SYNCOBJ_CREATE_SIGNALED, it will create a syncobj with signaled state.
 * So first exec with this syncobj will not block by waiting it. If want to wait this syncobj,
 * and it must use DRM_IOCTL_SYNCOBJ_RESET to reset it as not signaled.
 * After the syncobj is signaled by any process, we must use DRM_IOCTL_SYNCOBJ_RESET to reset it
 * as not signaled state, otherwise next exec will ignore this syncobj in exec sync array.
 */
int mos_sync_syncobj_create(int fd, uint32_t flags)
{
    struct drm_syncobj_create create = { 0 };
    int ret = 0;

    create.flags = flags;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_CREATE, &create);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_SYNCOBJ_CREATE, return error(%d)", ret)
    return create.handle;
}

/**
 * Syncobj handle that needs to destroy.
 *
 * Note: kmd will wait the syncobj signaled when umd tries to destroy it.
 * So, there is no need to wait it in umd normally.
 */
int mos_sync_syncobj_destroy(int fd, uint32_t handle)
{
    struct drm_syncobj_destroy destroy;
    memclear(destroy);
    int ret = 0;

    destroy.handle = handle;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_DESTROY, &destroy);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_SYNCOBJ_DESTROY, return error(%d)", ret)
    return ret;
}

/**
 * @handles indicates to the syncobj array that wants to wait signaled.
 * @abs_timeout_nsec indicates to the timeout:
 *     if abs_timeout_nsec = 0, the call will return immediately when any syncobj in the array is busy state.
 *     if abs_timeout_nsec > 0, the call will wait the syncobj in the array to be signaled.
 * @flags indicates to wait operation for the handles:
 *     if flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL, wait all syncobj to be signaled;
 * @first_signaled indicates to first recored syncobj handle that is signaled in the handles array.
 * Note: if return == -ETIME, wait timeout, it means busy state.
 */
int mos_sync_syncobj_wait_err(int fd, uint32_t *handles, uint32_t count,
         int64_t abs_timeout_nsec, uint32_t flags, uint32_t *first_signaled)
{
    if(handles == nullptr || count == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }
    struct drm_syncobj_wait wait;
    int ret = 0;

    wait.handles = (uintptr_t)(handles);
    wait.timeout_nsec = abs_timeout_nsec;
    wait.count_handles = count;
    wait.flags = flags;
    wait.first_signaled = 0;
    wait.pad = 0;

    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_WAIT, &wait);
    if (first_signaled)
        *first_signaled = wait.first_signaled;
    return ret;
}

/**
 * Reset the syncobj to not signaled state.
 *
 * Note: kmd will wait all syncobj in handles array to be signaled before reset them.
 * So, there is no need to wait in umd normally.
 */
int mos_sync_syncobj_reset(int fd, uint32_t *handles, uint32_t count)
{
    if(handles == nullptr || count == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }

    struct drm_syncobj_array array;
    memclear(array);
    int ret = 0;

    array.handles = (uintptr_t)(handles);
    array.count_handles = count;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_RESET, &array);
    return ret;
}

/**
 * Signal the syncobj in handles array.
 */
int mos_sync_syncobj_signal(int fd, uint32_t *handles, uint32_t count)
{
    if(handles == nullptr || count == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }

    struct drm_syncobj_array array;
    memclear(array);
    int ret = 0;

    array.handles = (uintptr_t)(handles);
    array.count_handles = count;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_SIGNAL, &array);
    return ret;
}

/**
 * Signal the syncobjs' timeline value with given points.
 */
int mos_sync_syncobj_timeline_signal(int fd, uint32_t *handles, uint64_t *points, uint32_t count)
{
    if(handles == nullptr || points == nullptr || count == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }

    struct drm_syncobj_timeline_array array;
    memclear(array);
    int ret = 0;

    array.handles = (uintptr_t)(handles);
    array.points = (uintptr_t)(points);
    array.count_handles = count;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL, &array);
    return ret;
}

/**
 * Same as @mos_sync_syncobj_wait_err;
 * Difference is that each syncobj needs to wait the timeline_value in points array.
 */
int mos_sync_syncobj_timeline_wait(int fd, uint32_t *handles, uint64_t *points,
            unsigned num_handles,
            int64_t timeout_nsec, unsigned flags,
            uint32_t *first_signaled)
{
    if(handles == nullptr || points == nullptr || num_handles == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }

    struct drm_syncobj_timeline_wait args;
    int ret;

    args.handles = (uintptr_t)(handles);
    args.points = (uintptr_t)(points);
    args.timeout_nsec = timeout_nsec;
    args.count_handles = num_handles;
    args.flags = flags;
    args.first_signaled = 0;
    args.pad = 0;

    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT, &args);

    if (first_signaled)
        *first_signaled = args.first_signaled;

    return ret;
}

/*
 * @handles: a set of syncobj handle
 * @points: output value, a set of sync points queried
 * */
int mos_sync_syncobj_timeline_query(int fd, uint32_t *handles, uint64_t *points,
             uint32_t handle_count)
{
    if(handles == nullptr || points == nullptr || handle_count == 0)
    {
        MOS_DRM_ASSERTMESSAGE("Invalid inputs");
        return -EINVAL;
    }

    struct drm_syncobj_timeline_array args;
    int ret;

    args.handles = (uintptr_t)(handles);
    args.points = (uintptr_t)(points);
    args.count_handles = handle_count;
    args.flags = 0;

    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_QUERY, &args);

    return ret;
}

/**
 * Export the syncfile fd from given prime fd.
 *
 * @prime_fd indicates to the prime fd of the bo handle;
 * @flags indicates to the operation flags of read or write;
 *
 * @return value indicates to sync file fd got from dma buffer;
 *
 * Note: Caller must close the syncfile fd after using to avoid leak.
 */
int mos_sync_dmabuf_export_syncfile(int prime_fd, uint32_t flags)
{
    int ret = 0;
    struct dma_buf_export_sync_file export_sync_file;
    memclear(export_sync_file);
    if(flags & EXEC_OBJECT_READ_XE)
    {
        export_sync_file.flags |= DMA_BUF_SYNC_READ;
    }
    if(flags & EXEC_OBJECT_WRITE_XE)
    {
        export_sync_file.flags |= DMA_BUF_SYNC_WRITE;
    }

    export_sync_file.fd = -1;
    ret = drmIoctl(prime_fd, DMA_BUF_IOCTL_EXPORT_SYNC_FILE,
                   &export_sync_file);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DMA_BUF_IOCTL_EXPORT_SYNC_FILE, return error(%d)", ret);
    return export_sync_file.fd;
}

/**
 * Sync file fd to syncobj handle.
 *
 * @fd indicates to the opened device;
 * @syncfile_fd indicates to exported syncfile fd by the prime fd;
 *
 * @return value indicates to syncobj handle imported into the syncfile fd;
 *
 * Note: Caller must close the syncobj handle after using to avoid leak.
 */
int mos_sync_syncfile_fd_to_syncobj_handle(int fd, int syncfile_fd)
{
    int ret = 0;
    struct drm_syncobj_handle syncobj_import;
    memclear(syncobj_import);
    syncobj_import.handle = mos_sync_syncobj_create(fd, 0);
    syncobj_import.flags = DRM_SYNCOBJ_FD_TO_HANDLE_FLAGS_IMPORT_SYNC_FILE;
    syncobj_import.fd = syncfile_fd;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE,
                   &syncobj_import);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE, return error(%d)", ret);
    return syncobj_import.handle;
}

/**
 * Syncobj handle to sync file fd.
 *
 * @fd indicates to the opened device;
 * @syncobj_handle indicates to the syncobj handle;
 *
 * @return value indicates to sync file fd got from the syncobj handle;
 *
 * Note: Caller must close the sync file fd after using to avoid leak.
 */
int mos_sync_syncobj_handle_to_syncfile_fd(int fd, int syncobj_handle)
{
    int ret = 0;
    struct drm_syncobj_handle syncobj_import;
    memclear(syncobj_import);
    syncobj_import.handle = syncobj_handle;
    syncobj_import.flags = DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE;
    syncobj_import.fd = -1;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD,
                   &syncobj_import);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD, return error(%d)", ret);
    return syncobj_import.fd;
}

/**
 * Convert external bo handle to a syncobj handle to use as fence in syncobj in umd.
 *
 * @fd indicates to opened device;
 * @bo_handle indicates to external bo handle;
 * @flags indicates to operation flags of read and write;
 * @out_prime_fd indicates to the prime_fd export from the handle;
 *     Umd must clost the prime_fd immedicately after using to avoid leak.
 *
 * @return value indicates to syncobj handle exported.
 *
 * Note: Caller must close the syncobj handle immedicately after using to avoid leak.
 *     And if umd wants to sync with external process, umd should always export all external
 *     bo's syncobj from their dma sync buffer and add them into exec syncs array.
 */
int mos_sync_export_external_bo_sync(int fd, int bo_handle, uint32_t flags, int &out_prime_fd)
{
    int prime_fd = -1;
    int syncfile_fd = -1;
    int syncobj_handle = -1;
    int ret = 0;
    ret = drmPrimeHandleToFD(fd, bo_handle, DRM_CLOEXEC | DRM_RDWR, &prime_fd);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "drmPrimeHandleToFD faled, return error(%d)", ret);
    syncfile_fd = mos_sync_dmabuf_export_syncfile(prime_fd, flags);
    if(syncfile_fd < 0)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to get external bo syncobj");
        close(prime_fd);
        return INVALID_HANDLE;
    }
    syncobj_handle = mos_sync_syncfile_fd_to_syncobj_handle(fd, syncfile_fd);
    if(syncobj_handle < 0)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to get external bo syncobj");
        close(prime_fd);
        close(syncfile_fd);
        return INVALID_HANDLE;
    }
    out_prime_fd = prime_fd;
    close(syncfile_fd);
    return syncobj_handle;
}

/**
 * Import the sync file fd into the DMA buffer of external bo.
 *
 * @fd indicates to opened device;
 * @prime_fd indicates to the prime fd of the external bo;
 * @syncfile_fd indicates to syncfile fd got from fence out syncobj handle umd.
 *
 * Note: if umd wants to export its fence out for external process to sync, umd
 *     should always import its batch syncobj into all external bo dma sync buffer.
 *
 */
int mos_sync_import_syncfile_to_external_bo(int fd, int prime_fd, int syncfile_fd)
{
    int ret = 0;

    struct dma_buf_import_sync_file import_sync_file;
    memclear(import_sync_file);
    import_sync_file.flags = DMA_BUF_SYNC_WRITE;
    import_sync_file.fd = syncfile_fd;
    ret = drmIoctl(prime_fd, DMA_BUF_IOCTL_IMPORT_SYNC_FILE, &import_sync_file);
    return ret;
}

/**
 * Transfer fence in a given syncobj or point to dst syncobj or its point.
 *
 * @fd indicates to opened device
 * @handle_dst indicates to destination syncobj handle
 * @point_dst indicates to destination timeline point
 * @handle_src indicates to source syncobj handle
 * @point_src indicates to source timeline point
 * @flags indicates to transfer flags
 */
int __mos_sync_syncobj_transfer(int fd,
            uint32_t handle_dst, uint64_t point_dst,
            uint32_t handle_src, uint64_t point_src,
            uint32_t flags)
{
    struct drm_syncobj_transfer args;
    memclear(args);
    int ret;

    args.src_handle = handle_src;
    args.dst_handle = handle_dst;
    args.src_point = point_src;
    args.dst_point = point_dst;
    args.flags = flags;
    args.pad = 0;
    ret = drmIoctl(fd, DRM_IOCTL_SYNCOBJ_TRANSFER, &args);
    return ret;
}

int mos_sync_syncobj_timeline_to_binary(int fd, uint32_t binary_handle,
            uint32_t timeline_handle,
            uint64_t point,
            uint32_t flags)
{
    return __mos_sync_syncobj_transfer(fd,
                binary_handle, 0,
                timeline_handle, point,
                flags);
}

/**
 * Initial a new timeline dep object.
 *
 * @fd indicates to opened device;
 *
 * @return indicates to a new timeline deo object
 */
struct mos_xe_dep *mos_sync_create_timeline_dep(int fd)
{
    struct mos_xe_dep *dep = nullptr;
    //create new one
    dep = (struct mos_xe_dep *)calloc(1, sizeof(struct mos_xe_dep));
    MOS_DRM_CHK_NULL_RETURN_VALUE(dep, nullptr);
    int handle = mos_sync_syncobj_create(fd, 0);

    if (handle > 0)
    {
        dep->syncobj_handle = handle;
        dep->timeline_index = 1;
    }
    else
    {
        MOS_XE_SAFE_FREE(dep)
        return nullptr;
    }

    return dep;
}

/**
 * Update the timeline dep to the queue by timeline index++.
 *
 * @deps indicates to the timeline dep from ctx busy queue.
 */
void mos_sync_update_timeline_dep(struct mos_xe_dep *dep)
{
    if(dep)
    {
        dep->timeline_index++;
    }
}

/**
 * Destroy the syncobj and timeline dep object
 *
 * @fd indicates to opened device
 * @dep indicates to the timeline dep object in its context
 *
 */
void mos_sync_destroy_timeline_dep(int fd, struct mos_xe_dep* dep)
{
    if (dep)
    {
        mos_sync_syncobj_destroy(fd, dep->syncobj_handle);
        MOS_XE_SAFE_FREE(dep)
    }
}

/**
 * Add the timeline dep from read and write dep map into exec syncs array.
 *
 * @curr_engine indicates to current exec engine id;
 * @lst_write_engine indicates to last exec engine id for writing;
 * @flags indicates to operation flags(read or write) for current exec;
 * @engine_ids indicates to valid engine IDs;
 * @read_deps indicates to read deps on previous exec;
 * @write_deps indicates to write deps on previous exec;
 * @syncs indicates to exec syncs array for current exec.
 *
 * Note: all deps from bo dep map are used as fence in, in this case,
 *     we should never set DRM_XE_SYNC_FLAG_SIGNAL for sync, otherwise kmd will
 *     not wait this sync.
 *
 * @return indicates to update status.
 */
int mos_sync_update_exec_syncs_from_timeline_deps(uint32_t curr_engine,
            uint32_t lst_write_engine, uint32_t flags,
            std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::vector<drm_xe_sync> &syncs)
{
    if (lst_write_engine != curr_engine)
    {
        if (write_deps.count(lst_write_engine) > 0
                && engine_ids.count(lst_write_engine) > 0)
        {
            if (write_deps[lst_write_engine].dep)
            {
                drm_xe_sync sync;
                memclear(sync);
                sync.handle = write_deps[lst_write_engine].dep->syncobj_handle;
                sync.type = DRM_XE_SYNC_TYPE_TIMELINE_SYNCOBJ;
                sync.timeline_value = write_deps[lst_write_engine].exec_timeline_index;
                syncs.push_back(sync);
            }
        }
    }

    //For flags & write, we need to add all sync in read_deps into syncs.
    if (flags & EXEC_OBJECT_WRITE_XE)
    {
        auto it = read_deps.begin();
        while (it != read_deps.end())
        {
            uint32_t engine_id = it->first;
            if (engine_id != curr_engine
                    && engine_ids.count(engine_id) > 0)
            {
                if (it->second.dep)
                {
                    drm_xe_sync sync;
                    memclear(sync);
                    sync.handle = it->second.dep->syncobj_handle;
                    sync.type = DRM_XE_SYNC_TYPE_TIMELINE_SYNCOBJ;
                    sync.timeline_value = it->second.exec_timeline_index;
                    syncs.push_back(sync);
                }
            }
            it++;
        }
    }

    return MOS_XE_SUCCESS;
}

/**
 * Add the dep from dma buffer of external bo into exec syncs array.
 *
 * @fd indicates to opened device;
 * @bo_handle indicates to external bo handle;
 * @flags indicates to operation flags(read or write) for current exec;
 * @syncs indicates to exec syncs array for current exec. Exported syncobj handle from external bo is used as fence in.
 * @out_prime_fd indicates to the prime_fd export from external bo handle; umd must close the prime fd after using to avoid leak.
 *
 * Note: Caller must close this exported syncobj handle after using to avoid leak.
 */
int mos_sync_update_exec_syncs_from_handle(int fd,
            uint32_t bo_handle, uint32_t flags,
            std::vector<struct drm_xe_sync> &syncs,
            int &out_prime_fd)
{
    int syncobj_handle = mos_sync_export_external_bo_sync(fd, bo_handle, flags, out_prime_fd);
    if(syncobj_handle < 0)
    {
        MOS_DRM_ASSERTMESSAGE("failed to add syncobj for external bo with invalid syncobj handle: %d",
                    syncobj_handle);
        return -1;
    }

    struct drm_xe_sync sync;
    memclear(sync);
    sync.type = DRM_XE_SYNC_TYPE_SYNCOBJ;
    sync.handle = syncobj_handle;
    syncs.push_back(sync);

    return MOS_XE_SUCCESS;
}

/**
 * Add the timeline dep from its context into exec syncs array for fence out.
 *
 * @fd indicates to opened device;
 * @dep indicates to a timeline dep in its context for fence out on current submission.
 * @syncs indicates to exec syncs array for current exec; timeline dep from queue is used as fence out.
 *
 */
void mos_sync_update_exec_syncs_from_timeline_dep(int fd,
            struct mos_xe_dep *dep,
            std::vector<struct drm_xe_sync> &syncs)
{
    if (dep)
    {
        drm_xe_sync sync;
        memclear(sync);
        //must set DRM_XE_SYNC_FLAG_SIGNAL for timeline fence out syncobj.
        sync.handle = dep->syncobj_handle;
        sync.timeline_value = dep->timeline_index;
        sync.flags = DRM_XE_SYNC_FLAG_SIGNAL;
        sync.type = DRM_XE_SYNC_TYPE_TIMELINE_SYNCOBJ;
        syncs.push_back(sync);
    }
}

/**
 * Update read and write deps of bo with given dep.
 *
 * @curr_engine indicates to current exec dummy engine id;
 * @flags indicates to operation flags(read or write) for current exec;
 * @dep indicates to the fence out dep that needs to update into the deps map;
 * @read_deps indicates to read deps on previous exec;
 * @write_deps indicates to write deps on previous exec;
 */
int mos_sync_update_bo_deps(uint32_t curr_engine,
            uint32_t flags, mos_xe_dep *dep,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(dep, -EINVAL)
    mos_xe_bo_dep bo_dep;
    bo_dep.dep = dep;
    bo_dep.exec_timeline_index = dep->timeline_index;
    if(flags & EXEC_OBJECT_READ_XE)
    {
        read_deps[curr_engine] = bo_dep;
    }

    if(flags & EXEC_OBJECT_WRITE_XE)
    {
        write_deps[curr_engine] = bo_dep;
    }

    return MOS_XE_SUCCESS;
}

/**
 * Get busy timeline deps from read and write deps map for bo wait.
 *
 * @engine_ids indicates to valid engine IDs;
 * @read_deps indicates to read deps on previous exec;
 * @write_deps indicates to write deps on previous exec;
 * @max_timeline_data max exec timeline value on each context for this bo resource;
 * @lst_write_engine indicates to last exec engine id for writing;
 * @rw_flags indicates to read/write operation:
 *     if rw_flags & EXEC_OBJECT_WRITE_XE, means bo write. Otherwise it means bo read.
 */
void mos_sync_get_bo_wait_timeline_deps(std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::map<uint32_t, uint64_t> &max_timeline_data,
            uint32_t lst_write_engine,
            uint32_t rw_flags)
{
    max_timeline_data.clear();

    //case1: get all timeline dep from read dep on all engines
    if(rw_flags & EXEC_OBJECT_WRITE_XE)
    {
        for(auto it = read_deps.begin(); it != read_deps.end(); it++)
        {
            uint32_t engine_id = it->first;
            uint64_t bo_exec_timeline = it->second.exec_timeline_index;
            // Get the valid busy dep in this read dep map for this bo.
            if (it->second.dep && engine_ids.count(engine_id) > 0)
            {
                // Save the max timeline data
                max_timeline_data[it->second.dep->syncobj_handle] = bo_exec_timeline;
            }
        }
    }

    //case2: get timeline dep from write dep on last write engine.
    if (engine_ids.count(lst_write_engine) > 0
        && write_deps.count(lst_write_engine) > 0
       && write_deps[lst_write_engine].dep)
    {
        uint32_t syncobj_handle = write_deps[lst_write_engine].dep->syncobj_handle;
        uint64_t bo_exec_timeline = write_deps[lst_write_engine].exec_timeline_index;
        if (max_timeline_data.count(syncobj_handle) == 0
                || max_timeline_data[syncobj_handle] < bo_exec_timeline)
        {
            // Save the max timeline data
            max_timeline_data[syncobj_handle] = bo_exec_timeline;
        }
    }
}
