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
     * Indicate to the handle for timeline syncobj.
     */
    uint32_t syncobj_handle;
    /**
     * Indicate to latest avaiable timeline value(index) for fence out point.
     */
    uint64_t timeline_index;
};

struct mos_xe_bo_dep
{
    /**
     * Indicate to reusable dep in the ctx queue.
     */
    struct mos_xe_dep *dep;

    /**
     * Indicate to the timeline point that bo execution on certain exec queue.
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
struct mos_xe_dep *mos_sync_create_timeline_dep(int fd);
void mos_sync_update_exec_syncs_from_timeline_dep(int fd,
            struct mos_xe_dep *dep,
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
void mos_sync_destroy_timeline_dep(int fd, struct mos_xe_dep *dep);


#if defined(__cplusplus)
}
#endif

#endif //__MOS_SYNCHRONIZATION_XE_
