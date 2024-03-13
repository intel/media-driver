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

#ifndef __MOS_SYNCHRONIZATION_XE_
#define __MOS_SYNCHRONIZATION_XE_


#include <stdint.h>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include "xe_drm.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct mos_xe_dep {
    /**
     * Maximun count of dependency;
     */
#define MAX_DEPS_SIZE 32

    struct drm_xe_sync sync;
    /**
     * Indicate dep is free status
     */
#define STATUS_DEP_FREE    1
    /**
     * Indicate dep is busy status
     */
#define STATUS_DEP_BUSY   2
    /**
     * Save dep status;
     * If the dep is getting from the queue and waiting to add into exec syncs array, it should be set as STATUS_DEP_BUSY
     * If the dep is signaled and moved to free queue, it should be set as STATUS_DEP_FREE
     */
    uint8_t status;

    /**
     * Indicate to the timeline index in the busy queue.
     */
    uint64_t timeline_index;

    /**
     * Sync obj needs to be used in DRM_IOCTL_XE_EXEC and DRM_IOCTL_SYNCOBJ_WAIT.
     * If ref_count != 0, it means this sync obj is still being used in DRM_IOCTL_XE_EXEC or DRM_IOCTL_SYNCOBJ_WAIT.
     * Should protect this sync obj by sync_obj_rw_lock when resetting or destroying this sync obj which is being used.
     * If ref_count == 0, we could reset or destroy this sync obj without sync_obj_rw_lock protection.
     */
    atomic_t ref_count;
};

struct mos_xe_bo_dep
{
    /**
     * Indicate to reusable dep in the ctx queue
     */
    struct mos_xe_dep *dep;

    /**
     * Save timeline_index of this bo
     * If bo's timeline_index does't equal to busy dep's timeline_index,
     * It means this busy dep is finished for this bo and reused by other bo.
     */
    uint64_t exec_timeline_index;
};

int mos_sync_syncobj_create(int fd, uint32_t flags);
int mos_sync_syncobj_destroy(int fd, uint32_t handle);
int mos_sync_syncobj_reset(int fd, uint32_t *handles, uint32_t count);
int mos_sync_syncobj_wait_err(int fd, uint32_t *handles, uint32_t count,
         int64_t abs_timeout_nsec, uint32_t flags, uint32_t *first_signaled);
int mos_sync_syncobj_timeline_wait(int fd, uint32_t *handles, uint64_t *points,
            unsigned num_handles,
            int64_t timeout_nsec, unsigned flags,
            uint32_t *first_signaled);

int mos_sync_syncobj_handle_to_syncfile_fd(int fd, int syncobj_handle);
int mos_sync_import_syncfile_to_external_bo(int fd, int prime_fd, int syncfile_fd);
int mos_sync_syncobj_timeline_to_binary(int fd, uint32_t binary_handle,
        uint32_t timeline_handle,
        uint64_t point,
        uint32_t flags);
void mos_sync_update_timeline_dep(struct mos_xe_dep *dep);
int mos_sync_update_exec_syncs_from_timeline_deps(uint32_t curr_engine,
            uint32_t lst_write_engine, uint32_t flags,
            std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::vector<drm_xe_sync> &syncs);
int mos_sync_update_exec_syncs_from_handle(int fd,
            uint32_t bo_handle, uint32_t flags,
            std::vector<struct drm_xe_sync> &syncs,
            int &out_prime_fd);
struct mos_xe_dep *mos_sync_update_exec_syncs_from_timeline_queue(int fd,
            std::queue<struct mos_xe_dep*> &queue_busy,
            std::vector<struct drm_xe_sync> &syncs);
int mos_sync_update_bo_deps(uint32_t curr_engine,
            uint32_t flags, mos_xe_dep *dep,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps);
void mos_sync_get_bo_wait_timeline_deps(std::set<uint32_t> &engine_ids,
            std::map<uint32_t, struct mos_xe_bo_dep> &read_deps,
            std::map<uint32_t, struct mos_xe_bo_dep> &write_deps,
            std::map<uint32_t, uint64_t> &max_timeline_data,
            uint32_t lst_write_engine,
            uint32_t rw_flags);
void mos_sync_clear_dep_queue(int fd, std::queue<struct mos_xe_dep*> &queue);
void mos_sync_clear_dep_list(int fd, std::list<struct mos_xe_dep*> &temp_list);


#if defined(__cplusplus)
}
#endif

#endif //__MOS_SYNCHRONIZATION_XE_
