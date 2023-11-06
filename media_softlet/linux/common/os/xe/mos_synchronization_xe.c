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
 * This is global synchronizarion mode for media umd, umd should alway select
 * a sync mode when init bufmgr
 */
static uint32_t SYNCHRONIZATION_MODE;

void mos_sync_set_synchronization_mode(uint32_t mode)
{
    SYNCHRONIZATION_MODE = mode;
}

uint32_t mos_sync_get_synchronization_mode()
{
    return SYNCHRONIZATION_MODE;
}

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
 * Get a dep from the queue to use as timeline fence out syncobj;
 *
 * @fd indicates to opened device;
 * @queue_busy indicates to busy queue in ctx, each ctx only contains one timeline syncobj in busy queue for each submission;
 *
 * @return indicates to timeline dep found from the queue; caller should alway check whether returned dep is nullptr.
 */
struct mos_xe_dep *mos_sync_get_timeline_dep_from_queue(int fd,
            std::queue<struct mos_xe_dep*> &queue_busy)
{
    struct mos_xe_dep *dep = nullptr;

    if (!queue_busy.empty())
    {
        dep = queue_busy.front();
    }
    else
    {
        //create new one and push it into queue_busy
        dep = (struct mos_xe_dep *)calloc(1, sizeof(struct mos_xe_dep));
        MOS_DRM_CHK_NULL_RETURN_VALUE(dep, nullptr);
        dep->sync.handle = mos_sync_syncobj_create(fd, 0);
        dep->timeline_index = 1;
        queue_busy.push(dep);
    }

    return dep;
}

/**
 * Get a dep from the queue to use as fence out syncobj;
 *
 * @fd indicates to opened device;
 * @queue_free indicates to free queue in ctx;
 * @queue_busy indicates to busy queue in ctx;
 * @need_wait indicates to boolean of whether to wait the dep signaled before adding into exec sync array.
 *     if need_wait==true, caller must reset dep->sync.handle, and then add it into sync array.
 *
 * @return indicates to dep found from the queue; caller should alway check whether returned dep is nullptr.
 */
struct mos_xe_dep *mos_sync_get_dep_from_queue(int fd,
            std::queue<struct mos_xe_dep*> &queue_free,
            std::queue<struct mos_xe_dep*> &queue_busy,
            std::list<struct mos_xe_dep*> &free_dep_tmp_list,
            bool &need_wait)
{
    struct mos_xe_dep *dep = nullptr;
    need_wait = false;

    if (!queue_free.empty())
    {
        //case1 - free queue not empty: pop() it for sync
        dep = queue_free.front();
        queue_free.pop();
    }
    else
    {
        //free queue is empty:
        if (queue_busy.size() + free_dep_tmp_list.size() >= MAX_DEPS_SIZE)
        {
            //case2: busy queue + free_dep_tmp_list is full
            need_wait = true;
            if (free_dep_tmp_list.size() > 0)
            {
                dep = free_dep_tmp_list.front();
                free_dep_tmp_list.pop_front();
            }
            else
            {
                dep = queue_busy.front();
                queue_busy.pop();
            }
        }
        else
        {
            //case 3: free queue is empty but busy queue size not achieve MAX_DEPS_SIZE
            //create new one and push it into queue_busy
            dep = (struct mos_xe_dep *)calloc(1, sizeof(struct mos_xe_dep));
            MOS_DRM_CHK_NULL_RETURN_VALUE(dep, nullptr);
            dep->sync.handle = mos_sync_syncobj_create(fd, 0);
        }
    }
    if(dep)
    {
        //must set DRM_XE_SYNC_SIGNAL for fence out syncobj.
        dep->sync.flags = DRM_XE_SYNC_SYNCOBJ | DRM_XE_SYNC_SIGNAL;
        dep->status = STATUS_DEP_BUSY;
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
 * Return back the dep to the queue.
 *
 * @queue indicates to queue in ctx; Nomamally it is busy queue.
 * @deps indicates to the dep got from queue that needs to return back to the queue after using.
 * @cur_timeline_index indicates to mos_xe_context.cur_timeline_index.
 *
 * Note: if umd get a dep from queue, it must return back it to the busy queue after using.
 */
void mos_sync_return_back_dep(std::queue<struct mos_xe_dep*> &queue,
            struct mos_xe_dep *dep, uint64_t &cur_timeline_index)
{
    if(dep)
    {
        dep->timeline_index = cur_timeline_index;
        queue.push(dep);
        cur_timeline_index++;
    }
}

/**
 * Move all deps whose timeline_index <= dep->timeline_index in busy queue to free queue.
 *
 * @fd indicates to opened device;
 * @queue_free indicates to free queue in ctx;
 * @queue_busy indicates to busy queue in ctx;
 * @bo_exec_timeline bo exec timeline index for this dep engine.
 *
 * Note: When a dep is confirmed as siganled by any caller, it could call this function to
 *     update busy queue and free queue on specific ctx.
 */
void mos_sync_update_dep_queue(int fd, std::queue<struct mos_xe_dep*> &queue_free,
            std::queue<struct mos_xe_dep*> &queue_busy,
            std::list<struct mos_xe_dep*> &free_dep_tmp_list,
            uint64_t bo_exec_timeline)
{
    if (mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        return;
    }

    if(!queue_busy.empty())
    {
        uint8_t count = queue_busy.size();
        uint32_t handles[count];
        uint8_t index = 0;
        while(!queue_busy.empty())
        {
            struct mos_xe_dep *temp = queue_busy.front();
            if(temp && temp->timeline_index <= bo_exec_timeline)
            {
                queue_busy.pop();
                temp->status = STATUS_DEP_FREE;
                if (atomic_read(&temp->ref_count) > 0)
                {
                    free_dep_tmp_list.push_back(temp);
                }
                else
                {
                    handles[index++] = temp->sync.handle;
                    queue_free.push(temp);
                }
            }
            else
            {
                break;
            }
        }
        count = index;
        if(count > 0)
        {
            mos_sync_syncobj_reset(fd, handles, count);
        }
    }
}

/**
 * Reset ref_count==0 dep, and move them from free_dep_tmp_list to free queue.
 *
 * @fd indicates to opened device;
 * @free_dep_tmp_list indicates to free dep which is not reseted;
 * @queue_free indicates to free queue in ctx;
 *
 */
void mos_sync_update_free_dep_tmp_list(int fd, std::list<struct mos_xe_dep*> &free_dep_tmp_list,
            std::queue<struct mos_xe_dep*> &queue_free)
{
    if (mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        return;
    }

    if (!free_dep_tmp_list.empty())
    {
        std::vector<uint32_t> handles;

        for (auto it = free_dep_tmp_list.begin(); it != free_dep_tmp_list.end();)
        {
            struct mos_xe_dep *tmp_ptr = *it;
            
            if (tmp_ptr == nullptr || atomic_read(&tmp_ptr->ref_count) > 0)
            {
                ++it;
            }
            else
            {
                handles.push_back(tmp_ptr->sync.handle);
                queue_free.push(tmp_ptr);
                it = free_dep_tmp_list.erase(it);
            }
        }
        if (!handles.empty())
        {
            mos_sync_syncobj_reset(fd, handles.data(), handles.size());
        }
    }
}

/**
 * Decrease ref_count in sync objs When finish DRM_IOCTL_XE_EXEC or DRM_IOCTL_SYNCOBJ_WAIT
 *
 * @deps indicates to sync objs
 *
 */
void mos_sync_dec_reference_count_in_deps(std::vector<struct mos_xe_dep*> &deps)
{
    for (auto dep : deps)
    {
        if(dep)
        {
            atomic_dec(&dep->ref_count, 1);
        }
    }
}

/**
 * Destroy the syncobj and clear the queue.
 *
 * @fd indicates to opened device
 * @queue indicates to the queue that needs to clear
 *
 */
void mos_sync_clear_dep_queue(int fd, std::queue<struct mos_xe_dep*> &queue)
{
    while(!queue.empty())
    {
        struct mos_xe_dep *dep = queue.front();
        queue.pop();
        if(dep)
        {
            mos_sync_syncobj_destroy(fd, dep->sync.handle);
            MOS_XE_SAFE_FREE(dep)
        }
    }
}

/**
 * Clear free tmp dep list
 *
 * @fd indicates to opened device
 * @temp_list indicates to sync objs
 *
 */
void mos_sync_clear_dep_list(int fd, std::list<struct mos_xe_dep*> &temp_list)
{
    for (auto dep : temp_list)
    {
        if(dep)
        {
            mos_sync_syncobj_destroy(fd, dep->sync.handle);
            MOS_XE_SAFE_FREE(dep)
        }
    }
    temp_list.clear();
}

/**
 * Is valid busy dep in the bo.
 *
 * @dep indicates to bo dep;
 * @engine_id bo dep dummy engine id;
 * @engine_ids all valid UMD dummy ctx engine IDs;
 * @bo_exec_timeline exec time line index in the bo;
 * 
 * @return if true, the dep is valid busy in the bo;
 */
bool mos_sync_is_valid_busy_dep(struct mos_xe_dep *dep,
            uint32_t engine_id,
            std::set<uint32_t> &engine_ids,
            uint64_t bo_exec_timeline)
{
    /* If ctx engine_ids does not contain this engine_id,
     * it means this engine ctx was destroyed and its dep was also freed.
     * Thus, this dep in the bo is a dangling pointer.
     * If the timeline of dep does not equal to bo_exec_timeline,
     * it means dep is used by other bo and it is not busy for this bo.
     * So valid busy dep should has valid engine_id, busy status and
     * timeline equals to bo_exec_timeline.
     */
    if (engine_ids.count(engine_id) > 0 &&
        dep &&
        dep->status == STATUS_DEP_BUSY &&
        dep->timeline_index == bo_exec_timeline)
    {
        return true;
    }
    return false;
}

/**
 * Add the dep into exec syncs array.
 *
 * @dep indicates to bo dep;
 * @engine_id bo dep dummy engine id;
 * @engine_ids all valid UMD dummy ctx engine IDs;
 * @bo_exec_timeline bo dep exec time;
 * @syncs indicates to exec syncs array for current exec;
 * @is_busy_dep is busy dep in this bo;
 * 
 * Note: all deps from bo dep map are used as fence in, in this case,
 *     we should never set DRM_XE_SYNC_SIGNAL for sync, otherwise kmd will
 *     not wait this sync.
 */
void mos_sync_update_exec_syncs_by_dep(struct mos_xe_dep *dep,
            uint32_t engine_id,
            std::set<uint32_t> &engine_ids,
            uint64_t bo_exec_timeline,
            std::vector<drm_xe_sync> &syncs,
            std::vector<mos_xe_dep*> &used_deps,
            bool &is_busy_dep)
{
    if (mos_sync_is_valid_busy_dep(dep, engine_id, engine_ids, bo_exec_timeline))
    {
        is_busy_dep = true;
        dep->sync.flags = DRM_XE_SYNC_SYNCOBJ;
        syncs.push_back(dep->sync);
        atomic_inc(&dep->ref_count);
        used_deps.push_back(dep);
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
 *     we should never set DRM_XE_SYNC_SIGNAL for sync, otherwise kmd will
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
            if(write_deps[lst_write_engine].dep)
            {
                drm_xe_sync sync;
                memclear(sync);
                sync.handle = write_deps[lst_write_engine].dep->sync.handle;
                sync.flags = DRM_XE_SYNC_TIMELINE_SYNCOBJ;
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
                if(it->second.dep)
                {
                    drm_xe_sync sync;
                    memclear(sync);
                    sync.handle = it->second.dep->sync.handle;
                    sync.flags = DRM_XE_SYNC_TIMELINE_SYNCOBJ;
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
 * Add the dep from read and write dep map into exec syncs array.
 *
 * @curr_engine indicates to current exec engine id;
 * @lst_write_engine indicates to last exec engine id for writing;
 * @flags indicates to operation flags(read or write) for current exec;
 * @engine_ids indicates to valid UMD dummy engine IDs;
 * @read_deps indicates to read deps on previous exec;
 * @write_deps indicates to write deps on previous exec;
 * @syncs indicates to exec syncs array for current exec.
 *
 * Note: all deps from bo dep map are used as fence in, in this case,
 *     we should never set DRM_XE_SYNC_SIGNAL for sync, otherwise kmd will
 *     not wait this sync.
 *
 * @return indicates to update status.
 */
int mos_sync_update_exec_syncs_from_deps(uint32_t curr_engine,
            uint32_t lst_write_engine, uint32_t flags,
            std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::vector<drm_xe_sync> &syncs,
            std::vector<mos_xe_dep*> &used_deps)
{
    if (mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        return mos_sync_update_exec_syncs_from_timeline_deps(
                    curr_engine,
                    lst_write_engine,
                    flags, engine_ids,
                    read_deps,
                    write_deps,
                    syncs);
    }

    if (lst_write_engine != curr_engine)
    {
        bool is_busy_dep = false;
        if (write_deps.count(lst_write_engine) > 0)
        {
            mos_sync_update_exec_syncs_by_dep(
                write_deps[lst_write_engine].dep,
                lst_write_engine,
                engine_ids,
                write_deps[lst_write_engine].exec_timeline_index,
                syncs,
                used_deps,
                is_busy_dep);
        }
        if (!is_busy_dep)
        {
            write_deps.clear();
        }
    }

    //For flags & write, we need to add all sync in read_deps into syncs.
    if (flags & EXEC_OBJECT_WRITE_XE)
    {
        auto it = read_deps.begin();
        while (it != read_deps.end())
        {
            uint32_t engine_id = it->first;
            if (engine_id != curr_engine)
            {
                bool is_busy_dep = false;
                mos_sync_update_exec_syncs_by_dep(
                    it->second.dep,
                    engine_id,
                    engine_ids,
                    it->second.exec_timeline_index,
                    syncs,
                    used_deps,
                    is_busy_dep);
                if (!is_busy_dep)
                {
                    it = read_deps.erase(it);
                    continue;
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
    sync.flags = DRM_XE_SYNC_SYNCOBJ;
    sync.handle = syncobj_handle;
    syncs.push_back(sync);

    return MOS_XE_SUCCESS;
}

/**
 * Add the timeline dep from ctx busy queue into exec syncs array.
 *
 * @fd indicates to opened device;
 * @queue_busy indicates to ctx timeline fence queue.
 * @syncs indicates to exec syncs array for current exec; timeline dep from queue is used as fence out.
 * @return indicates to the timeline dep got from queue, caller must update this dep->timeline_index into read_deps or write_deps after exec.
 *
 */
struct mos_xe_dep *mos_sync_update_exec_syncs_from_timeline_queue(int fd,
            std::queue<struct mos_xe_dep*> &queue_busy,
            std::vector<struct drm_xe_sync> &syncs)
{
    struct mos_xe_dep *dep =
        mos_sync_get_timeline_dep_from_queue(fd, queue_busy);

    MOS_DRM_CHK_NULL_RETURN_VALUE(dep, nullptr)

    if (dep)
    {
        drm_xe_sync sync;
        memclear(sync);
        //must set DRM_XE_SYNC_SIGNAL for timeline fence out syncobj.
        sync.handle = dep->sync.handle;
        sync.timeline_value = dep->timeline_index;
        sync.flags = DRM_XE_SYNC_TIMELINE_SYNCOBJ | DRM_XE_SYNC_SIGNAL;
        syncs.push_back(sync);
    }

    return dep;
}

/**
 * Add the dep from ctx dep queue into exec syncs array.
 *
 * @fd indicates to opened device;
 * @queue_free indicates to queue with deps of free status;
 * @queue_busy indicates to queue with deps of busy status;
 * @syncs indicates to exec syncs array for current exec; dep from queue is used as fence out.
 * @return indicates to the dep got from queue, caller must update this dep into read_deps or write_deps after exec.
 *
 * Note: if need_wait == true, caller must reset the dep before adding it into exec syncs array since it comes from busy queue.
 */
struct mos_xe_dep *mos_sync_update_exec_syncs_from_queue(int fd,
            std::queue<struct mos_xe_dep*> &queue_free,
            std::queue<struct mos_xe_dep*> &queue_busy,
            std::list<struct mos_xe_dep*> &free_dep_tmp_list,
            std::vector<struct drm_xe_sync> &syncs,
            bool &need_wait)
{

    if (mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        need_wait = false;
        return mos_sync_update_exec_syncs_from_timeline_queue(
                    fd,
                    queue_busy,
                    syncs);
    }

    struct mos_xe_dep *dep =
        mos_sync_get_dep_from_queue(fd, queue_free, queue_busy, free_dep_tmp_list, need_wait);

    MOS_DRM_CHK_NULL_RETURN_VALUE(dep, nullptr)

    syncs.push_back(dep->sync);

    return dep;
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
                max_timeline_data[it->second.dep->sync.handle] = bo_exec_timeline;
            }
        }
    }

    //case2: get timeline dep from write dep on last write engine.
    if (engine_ids.count(lst_write_engine) > 0
        && write_deps.count(lst_write_engine) > 0
       && write_deps[lst_write_engine].dep)
    {
        uint32_t syncobj_handle = write_deps[lst_write_engine].dep->sync.handle;
        uint64_t bo_exec_timeline = write_deps[lst_write_engine].exec_timeline_index;
        if (max_timeline_data.count(syncobj_handle) == 0
                || max_timeline_data[syncobj_handle] < bo_exec_timeline)
        {
            // Save the max timeline data
            max_timeline_data[syncobj_handle] = bo_exec_timeline;
        }
    }
}

/**
 * Get busy deps from read and write deps map for bo wait.
 *
 * @engine_ids indicates to valid UMD dummy engine IDs;
 * @read_deps indicates to read deps on previous exec;
 * @write_deps indicates to write deps on previous exec;
 * @max_timeline_data max timeline data in every engine context for this bo resource;
 * @lst_write_engine indicates to last exec engine id for writing;
 * @handles indicates to syncobj handles for waiting;
 * @rw_flags indicates to read/write operation:
 *     if rw_flags & EXEC_OBJECT_WRITE_XE, means bo write. Otherwise it means bo read.
 */
void mos_sync_get_bo_wait_deps(std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::map<uint32_t, uint64_t> &max_timeline_data,
            uint32_t lst_write_engine,
            std::vector<uint32_t> &handles,
            std::vector<struct mos_xe_dep*> &used_deps,
            uint32_t rw_flags)
{
    max_timeline_data.clear();

    //case1: get all dep from read dep on all engines
    if(rw_flags & EXEC_OBJECT_WRITE_XE)
    {
        for(auto it = read_deps.begin(); it != read_deps.end(); it++)
        {
            uint32_t engine_id = it->first;
            uint64_t bo_exec_timeline = it->second.exec_timeline_index;
            // Get the valid busy dep in this read dep map for this bo.
            if (mos_sync_is_valid_busy_dep(it->second.dep, engine_id, engine_ids, bo_exec_timeline))
            {
                // Save the max timeline data
                max_timeline_data[engine_id] = bo_exec_timeline;
                handles.push_back(it->second.dep->sync.handle);
                atomic_inc(&it->second.dep->ref_count);
                used_deps.push_back(it->second.dep);
            }
        }
    }

    //case2: get dep from write dep on last write engine.
    for (auto it = write_deps.begin(); it != write_deps.end(); it++)
    {
        uint32_t engine_id = it->first;
        uint64_t bo_exec_timeline = it->second.exec_timeline_index;
        // Get the valid busy dep in this write dep map for this bo.
        if (mos_sync_is_valid_busy_dep(it->second.dep, engine_id, engine_ids, bo_exec_timeline) &&
            (max_timeline_data.count(engine_id) == 0 || max_timeline_data[engine_id] < bo_exec_timeline))
        {
            // Save the max timeline data
            max_timeline_data[engine_id] = bo_exec_timeline;
            if (engine_id == lst_write_engine)
            {
                // Only add the handle from the write dep on last write engine
                handles.push_back(it->second.dep->sync.handle);
                atomic_inc(&it->second.dep->ref_count);
                used_deps.push_back(it->second.dep);
            }
        }
    }
}
