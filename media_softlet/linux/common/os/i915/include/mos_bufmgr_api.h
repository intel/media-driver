/*
* Copyright (c) 2023, Intel Corporation
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
//! \file    mos_bufmgr_api.h
//! \brief   Public definitions of Intel-specific bufmgr api functions.
//!

#ifndef MOS_BUFMGR_API_H
#define MOS_BUFMGR_API_H

#include <stdio.h>
#include "drm.h"
#include "libdrm_macros.h"
#include "media_skuwa_specific.h"

#define S_SUCCESS 0

#define TILING_NONE  0
#define TILING_X     1
#define TILING_Y     2

#define DRM_ENGINE_CLASS_RENDER         0
#define DRM_ENGINE_CLASS_COPY           1
#define DRM_ENGINE_CLASS_VIDEO_DECODE   2
#define DRM_ENGINE_CLASS_VIDEO_ENHANCE  3
#define DRM_ENGINE_CLASS_COMPUTE        4

#define DRM_EXEC_DEFAULT                0
#define DRM_EXEC_RENDER                 1
#define DRM_EXEC_BSD                    2
#define DRM_EXEC_BLT                    3
#define DRM_EXEC_VEBOX                  4
#define DRM_EXEC_COMPUTE                5
#define DRM_EXEC_VCS2                   6

#define DRM_CONTEXT_PARAM_PRIORITY      0x6
#define DRM_CONTEXT_PARAM_RECOVERABLE   0x8

#define mos_safe_free(p)        \
        if(p) free(p);          \

#define CHK_CONDITION(condition, _str, _ret)    \
    if (condition) {                            \
        fprintf(stderr, _str);                  \
        return _ret;                            \
    }

#define INVALID_VM -1

#ifndef MOS_UNUSED
#define MOS_UNUSED(param) (void)(param)
#endif

/**
 * Supported data type:
 * MOS_USER_FEATURE_VALUE_TYPE_BOOL
 * MOS_USER_FEATURE_VALUE_TYPE_INT32
 * MOS_USER_FEATURE_VALUE_TYPE_INT64
 * MOS_USER_FEATURE_VALUE_TYPE_UINT32
 * MOS_USER_FEATURE_VALUE_TYPE_UINT64
 * MOS_USER_FEATURE_VALUE_TYPE_FLOAT
*/
#define MOS_READ_ENV_VARIABLE(env_key, data_type, out_value)                               \
{                                                                                          \
    MediaUserSetting::Value v(out_value);                                                  \
    int ret = MosUtilities::MosReadEnvVariable(ENV_VARIABLE_TABLE[env_key], data_type, v); \
    if(ret == 0)                                                                           \
    {                                                                                      \
        if(data_type == MOS_USER_FEATURE_VALUE_TYPE_INT64)                                 \
            out_value = v.Get<int64_t>();                                                  \
        else if(data_type == MOS_USER_FEATURE_VALUE_TYPE_UINT64)                           \
            out_value = v.Get<uint64_t>();                                                 \
        else if(data_type == MOS_USER_FEATURE_VALUE_TYPE_INT32)                            \
            out_value = v.Get<int32_t>();                                                  \
        else if(data_type == MOS_USER_FEATURE_VALUE_TYPE_UINT32)                           \
            out_value = v.Get<uint32_t>();                                                 \
        else if(data_type == MOS_USER_FEATURE_VALUE_TYPE_FLOAT)                            \
            out_value = v.Get<float>();                                                    \
        else if(data_type == MOS_USER_FEATURE_VALUE_TYPE_BOOL)                             \
            out_value = v.Get<bool>();                                                     \
    }                                                                                      \
}


struct mos_linux_bo;
struct mos_linux_context;
struct mos_bufmgr;
struct MOS_OCA_EXEC_LIST_INFO;

typedef struct mos_linux_bo MOS_LINUX_BO;
typedef struct mos_linux_context MOS_LINUX_CONTEXT;
typedef struct mos_bufmgr MOS_BUFMGR;
typedef struct MOS_OCA_EXEC_LIST_INFO mos_oca_exec_list_info;

struct _MOS_OS_CONTEXT;

enum mos_memory_zone {
   MEMZONE_SYS,
   MEMZONE_DEVICE,
   MEMZONE_PRIME, //for imported PRIME buffers
   MEMZONE_COUNT,
};

enum device_type {
    DEVICE_TYPE_I915,
    DEVICE_TYPE_XE,
    DEVICE_TYPE_COUNT,
};

#define MEMZONE_SYS_START     (1ull << 16)
#define MEMZONE_DEVICE_START  (1ull << 40)
#define MEMZONE_SYS_SIZE      (MEMZONE_DEVICE_START - MEMZONE_SYS_START)
#define MEMZONE_DEVICE_SIZE   (1ull << 40)
#define MEMZONE_PRIME_START   (MEMZONE_DEVICE_START + MEMZONE_DEVICE_SIZE)
#define MEMZONE_PRIME_SIZE    (1ull << 40)
#define MEMZONE_TOTAL         (1ull << 48)
#define PAGE_SIZE_4K          (1ull << 12)
#define PAGE_SIZE_64K         (1ull << 16)
#define PAGE_SIZE_1M          (1ull << 20)
#define PAGE_SIZE_2M          (1ull << 21)
#define PAGE_SIZE_4G          (1ull << 32)
#define ARRAY_INIT_SIZE       5

struct mos_linux_context {
    unsigned int ctx_id;
    struct mos_bufmgr *bufmgr;
    struct _MOS_OS_CONTEXT    *pOsContext;
    __u32 vm_id;
};

struct mos_linux_bo {
    /**
     * Size in bytes of the buffer object.
     *
     * The size may be larger than the size originally requested for the
     * allocation, such as being aligned to page size.
     */
    unsigned long size;

    /**
     * Alignment requirement for object
     *
     * Used for GTT mapping & pinning the object.
     */
    unsigned long align;

    /**
     * Deprecated field containing (possibly the low 32-bits of) the last
     * seen virtual card address.  Use offset64 instead.
     */
    unsigned long offset;

    /**
     * Virtual address for accessing the buffer data.  Only valid while
     * mapped.
     */
#ifdef __cplusplus
    void *virt;
#else
    void *virtual;
#endif

    /** Buffer manager context associated with this buffer object */
    struct mos_bufmgr *bufmgr;

    /**
     * MM-specific handle for accessing object
     */
    int handle;

    /**
     * Last seen card virtual address (offset from the beginning of the
     * aperture) for the object.  This should be used to fill relocation
     * entries when calling drm_intel_bo_emit_reloc()
     */
    uint64_t offset64;

    /**
     * indicate if the bo mapped into aux table
     */
    bool aux_mapped;

    __u32 vm_id;
};

#define BO_ALLOC_FOR_RENDER (1<<0)

#define PAT_INDEX_INVALID ((uint16_t)-1)
struct mos_drm_bo_alloc_ext{
    unsigned long flags = 0;
    uint32_t tiling_mode = TILING_NONE;
    int mem_type = 0;
    uint16_t pat_index = PAT_INDEX_INVALID;
    bool     cpu_cacheable = true;
    bool     scanout_surf  = false;
};

struct mos_drm_bo_alloc {
    const char *name = "\0";
    unsigned long size = 0;
    unsigned int alignment = 0;
    unsigned long stride = 0;

    struct mos_drm_bo_alloc_ext ext;
};

struct mos_drm_bo_alloc_userptr {
    const char *name = "\0";
    void *addr = 0;
    uint32_t tiling_mode = 0;
    uint32_t stride = 0;
    unsigned long size = 0;
    unsigned long flags = 0;
    uint16_t pat_index = 0;
};

struct mos_drm_bo_alloc_tiled {
    const char *name = "\0";
    int x = 0;
    int y = 0;
    int cpp = 0;
    unsigned long pitch = 0;
    unsigned int alignment = 0;

    struct mos_drm_bo_alloc_ext ext;
};

struct mos_drm_bo_alloc_prime {
    const char *name = "\0";
    int      prime_fd;
    int      size;
    uint16_t pat_index;
};

struct mos_drm_uc_version {
#define UC_TYPE_GUC_SUBMISSION 0
#define UC_TYPE_HUC            1
#define UC_TYPE_MAX            2
#define UC_TYPE_INVALID        (uint16_t)-1
    uint16_t uc_type;
    uint32_t major_version;
    uint32_t minor_version;
};

struct mos_linux_bo *mos_bo_alloc(struct mos_bufmgr *bufmgr,
                                struct mos_drm_bo_alloc *alloc);
struct mos_linux_bo *mos_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
                                struct mos_drm_bo_alloc_userptr *alloc_uptr);
struct mos_linux_bo *mos_bo_alloc_tiled(struct mos_bufmgr *bufmgr,
                                struct mos_drm_bo_alloc_tiled *alloc_tiled);
void mos_bo_reference(struct mos_linux_bo *bo);
void mos_bo_unreference(struct mos_linux_bo *bo);
int mos_bo_map(struct mos_linux_bo *bo, int write_enable);
int mos_bo_unmap(struct mos_linux_bo *bo);

void mos_bo_wait_rendering(struct mos_linux_bo *bo);

void mos_bufmgr_set_debug(struct mos_bufmgr *bufmgr, int enable_debug);
void mos_bufmgr_destroy(struct mos_bufmgr *bufmgr);
int mos_bo_exec(struct mos_linux_bo *bo, int used,
              struct drm_clip_rect *cliprects, int num_cliprects, int DR4);
int mos_bo_mrb_exec(struct mos_linux_bo *bo, int used,
            struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
            unsigned int flags);
int mos_bufmgr_check_aperture_space(struct mos_linux_bo ** bo_array, int count);

int mos_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
         struct mos_linux_bo *target_bo, uint32_t target_offset,
         uint32_t read_domains, uint32_t write_domain,
         uint64_t presumed_offset);
int mos_bo_set_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t stride);
int mos_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode);

int mos_bo_flink(struct mos_linux_bo *bo, uint32_t * name);
int mos_bo_busy(struct mos_linux_bo *bo);
int mos_bo_madvise(struct mos_linux_bo *bo, int madv);
int mos_bo_use_48b_address_range(struct mos_linux_bo *bo, uint32_t enable);
void mos_bo_set_object_async(struct mos_linux_bo *bo);
void mos_bo_set_exec_object_async(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo);
void mos_bo_set_object_capture(struct mos_linux_bo *bo);
int mos_bo_set_softpin(struct mos_linux_bo *bo);
int mos_bo_add_softpin_target(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo, bool write_flag);
mos_oca_exec_list_info* mos_bo_get_softpin_targets_info(struct mos_linux_bo *bo, int *count);

int mos_bo_disable_reuse(struct mos_linux_bo *bo);
int mos_bo_is_reusable(struct mos_linux_bo *bo);
int mos_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo);
int mos_bo_pad_to_size(struct mos_linux_bo *bo, uint64_t pad_to_size);

/* drm_intel_bufmgr_gem.c */
struct mos_bufmgr *mos_bufmgr_gem_init(int fd, int batch_size, int *device_type = nullptr);
struct mos_linux_bo *mos_bo_create_from_name(struct mos_bufmgr *bufmgr,
                        const char *name,
                        unsigned int handle);
void mos_bufmgr_enable_reuse(struct mos_bufmgr *bufmgr);
void mos_bufmgr_enable_softpin(struct mos_bufmgr *bufmgr, bool va1m_align);
void mos_bufmgr_enable_vmbind(struct mos_bufmgr *bufmgr);
void mos_bufmgr_disable_object_capture(struct mos_bufmgr *bufmgr);
int mos_bufmgr_get_memory_info(struct mos_bufmgr *bufmgr, char *info, uint32_t length);
int mos_bufmgr_get_devid(struct mos_bufmgr *bufmgr);
void mos_bufmgr_realloc_cache(struct mos_bufmgr *bufmgr, uint8_t alloc_mode);

int mos_bo_map_unsynchronized(struct mos_linux_bo *bo);
int mos_bo_map_gtt(struct mos_linux_bo *bo);
int mos_bo_unmap_gtt(struct mos_linux_bo *bo);
int mos_bo_unmap_wc(struct mos_linux_bo *bo);

void mos_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable);

struct mos_linux_context *mos_context_create(struct mos_bufmgr *bufmgr);
struct mos_linux_context *mos_context_create_ext(
                            struct mos_bufmgr *bufmgr,
                            __u32 flags,
                            bool bContextProtected);
struct mos_linux_context *mos_context_create_shared(
                            struct mos_bufmgr *bufmgr,
                            mos_linux_context* ctx,
                            __u32 flags,
                            bool bContextProtected,
                            void *engine_map,
                            uint8_t ctx_width,
                            uint8_t num_placements,
                            uint32_t ctx_type);

__u32 mos_vm_create(struct mos_bufmgr *bufmgr);
void mos_vm_destroy(struct mos_bufmgr *bufmgr, __u32 vm_id);

#define MAX_ENGINE_INSTANCE_NUM 8
#define MAX_PARALLEN_CMD_BO_NUM MAX_ENGINE_INSTANCE_NUM

int mos_query_engines_count(struct mos_bufmgr *bufmgr,
                      unsigned int *nengine);

int mos_query_engines(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *ci);

size_t mos_get_engine_class_size(struct mos_bufmgr *bufmgr);

void mos_select_fixed_engine(struct mos_bufmgr *bufmgr,
            void *engine_map,
            uint32_t *nengine,
            uint32_t fixed_instance_mask);

void mos_context_destroy(struct mos_linux_context *ctx);

int
mos_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence);

int
mos_bo_context_exec3(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence);

int mos_bo_export_to_prime(struct mos_linux_bo *bo, int *prime_fd);
struct mos_linux_bo *mos_bo_create_from_prime(struct mos_bufmgr *bufmgr,
                        struct mos_drm_bo_alloc_prime *alloc_prime);

int mos_reg_read(struct mos_bufmgr *bufmgr,
               uint32_t offset,
               uint64_t *result);

int mos_get_reset_stats(struct mos_linux_context *ctx,
                  uint32_t *reset_count,
                  uint32_t *active,
                  uint32_t *pending);

int mos_get_context_param(struct mos_linux_context *ctx,
                           uint32_t size,
                           uint64_t param,
                           uint64_t *value);

int mos_set_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t value);

uint8_t mos_switch_off_n_bits(struct mos_linux_context *ctx, uint8_t in_mask, int n);
unsigned int mos_hweight8(struct mos_linux_context *ctx, uint8_t w);

int mos_query_sys_engines(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info);
int mos_query_device_blob(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info);
int mos_query_hw_ip_version(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info);
int mos_get_param(int fd, int32_t param, uint32_t *param_value);

struct LinuxDriverInfo;
int mos_get_driver_info(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo);

int mos_get_device_id(int fd, uint32_t *deviceId);

#if defined(__cplusplus)
extern "C" {
#endif
extern drm_export int drmIoctl(int fd, unsigned long request, void *arg);

static inline int mos_query_device_type(int fd)
{
    int device_type = DEVICE_TYPE_COUNT;
    drm_version_t version;
    char name[5] = "";

    memset(&version, 0, sizeof(version));
    version.name_len = 4;
    version.name = name;

    if (drmIoctl(fd, DRM_IOCTL_VERSION, &version))
    {
        fprintf(stderr, "DRM_IOCTL_VERSION failed: %s\n", strerror(errno));
        return device_type;
    }

    if(strcmp("i915", name) == 0)
    {
        device_type = DEVICE_TYPE_I915;
    }
    else if(strcmp("xe", name) == 0)
    {
        device_type = DEVICE_TYPE_XE;
    }
    else
    {
        fprintf(stderr, "DRM_IOCTL_VERSION, unsupported drm device by media driver: %s\n", name);
    }
    return device_type;
}

drm_export int mos_bo_map_wc(struct mos_linux_bo *bo);
drm_export void mos_bo_clear_relocs(struct mos_linux_bo *bo, int start);
drm_export int mos_bo_wait(struct mos_linux_bo *bo, int64_t timeout_ns);

drm_export bool mos_bo_is_softpin(struct mos_linux_bo *bo);
drm_export bool mos_bo_is_exec_object_async(struct mos_linux_bo *bo);
#if defined(__cplusplus)
}
#endif

#define PLATFORM_INFORMATION_IS_SERVER     0x1

uint64_t mos_get_platform_information(struct mos_bufmgr *bufmgr);
void mos_set_platform_information(struct mos_bufmgr *bufmgr, uint64_t p);
bool mos_has_bsd2(struct mos_bufmgr *bufmgr);
int mos_get_ts_frequency(struct mos_bufmgr *bufmgr, uint32_t *ts_freq);
void mos_enable_turbo_boost(struct mos_bufmgr *bufmgr);

#endif /* MOS_BUFMGR_API_H */
