// SPDX-License-Identifier: MIT
/*
 * Copyright © 2020 Intel Corporation
 */

#ifndef _INTEL_HWCONFIG_TYPES_H_
#define _INTEL_HWCONFIG_TYPES_H_

/**
 * enum intel_hwconfig - Global definition of hwconfig blob attributes
 *
 * Intel devices provide a KLV (Key/Length/Value) blob containing
 * the static hardware configuration for that giving platform.
 * This header defines the current attribute keys for this KLV.
 */
enum intel_hwconfig {
    INTEL_HWCONFIG_MAX_SLICES_SUPPORTED = 1,
    INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED,        /* 2 */
    INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS,          /* 3 */
    INTEL_HWCONFIG_NUM_PIXEL_PIPES,             /* 4 */
    INTEL_HWCONFIG_MAX_NUM_GEOMETRY_PIPES,          /* 5 */
    INTEL_HWCONFIG_L3_CACHE_SIZE_IN_KB,         /* 6 */
    INTEL_HWCONFIG_L3_BANK_COUNT,               /* 7 */
    INTEL_HWCONFIG_L3_CACHE_WAYS_SIZE_IN_BYTES,     /* 8 */
    INTEL_HWCONFIG_L3_CACHE_WAYS_PER_SECTOR,        /* 9 */
    INTEL_HWCONFIG_MAX_MEMORY_CHANNELS,         /* 10 */
    INTEL_HWCONFIG_MEMORY_TYPE,             /* 11 */
    INTEL_HWCONFIG_CACHE_TYPES,                             /* 12 */
    /*
     * Local Memory page sizes supported lists all possible supported sizes
     * For example, 4KB and 64KB will be listed as (SZ_4K | SZ_64K)
     */
    INTEL_HWCONFIG_LOCAL_MEMORY_PAGE_SIZES_SUPPORTED,   /* 13 */
    INTEL_HWCONFIG_SLM_SIZE_IN_KB,              /* 14 */
    INTEL_HWCONFIG_NUM_THREADS_PER_EU,          /* 15 */
    INTEL_HWCONFIG_TOTAL_VS_THREADS,            /* 16 */
    INTEL_HWCONFIG_TOTAL_GS_THREADS,            /* 17 */
    INTEL_HWCONFIG_TOTAL_HS_THREADS,            /* 18 */
    INTEL_HWCONFIG_TOTAL_DS_THREADS,            /* 19 */
    INTEL_HWCONFIG_TOTAL_VS_THREADS_POCS,           /* 20 */
    INTEL_HWCONFIG_TOTAL_PS_THREADS,            /* 21 */
    INTEL_HWCONFIG_MAX_FILL_RATE,               /* 22 */
    INTEL_HWCONFIG_MAX_RCS,                 /* 23 */
    INTEL_HWCONFIG_MAX_CCS,                 /* 24 */
    INTEL_HWCONFIG_MAX_VCS,                 /* 25 */
    INTEL_HWCONFIG_MAX_VECS,                /* 26 */
    INTEL_HWCONFIG_MAX_COPY_CS,             /* 27 */
    /* URB Size might be configurable by UMD in certain platforms */
    INTEL_HWCONFIG_URB_SIZE_IN_KB,              /* 28 */
    INTEL_HWCONFIG_MIN_VS_URB_ENTRIES,          /* 29 */
    INTEL_HWCONFIG_MAX_VS_URB_ENTRIES,          /* 30 */
    INTEL_HWCONFIG_MIN_PCS_URB_ENTRIES,         /* 31 */
    INTEL_HWCONFIG_MAX_PCS_URB_ENTRIES,         /* 32 */
    INTEL_HWCONFIG_MIN_HS_URB_ENTRIES,          /* 33 */
    INTEL_HWCONFIG_MAX_HS_URB_ENTRIES,          /* 34 */
    INTEL_HWCONFIG_MIN_GS_URB_ENTRIES,          /* 35 */
    INTEL_HWCONFIG_MAX_GS_URB_ENTRIES,          /* 36 */
    INTEL_HWCONFIG_MIN_DS_URB_ENTRIES,          /* 37 */
    INTEL_HWCONFIG_MAX_DS_URB_ENTRIES,          /* 38 */
    INTEL_HWCONFIG_PUSH_CONSTANT_URB_RESERVED_SIZE,     /* 39 */
    INTEL_HWCONFIG_POCS_PUSH_CONSTANT_URB_RESERVED_SIZE,    /* 40 */
    INTEL_HWCONFIG_URB_REGION_ALIGNMENT_SIZE_IN_BYTES,  /* 41 */
    INTEL_HWCONFIG_URB_ALLOCATION_SIZE_UNITS_IN_BYTES,  /* 42 */
    INTEL_HWCONFIG_MAX_URB_SIZE_CCS_IN_BYTES,       /* 43 */
    INTEL_HWCONFIG_VS_MIN_DEREF_BLOCK_SIZE_HANDLE_COUNT,    /* 44 */
    INTEL_HWCONFIG_DS_MIN_DEREF_BLOCK_SIZE_HANDLE_COUNT,    /* 45 */
    INTEL_HWCONFIG_NUM_RT_STACKS_PER_DSS,           /* 46 */
    __INTEL_HWCONFIG_MAX
};

#define INTEL_HWCONFIG_MAX (__INTEL_HWCONFIG_MAX - 1)

enum {
    INTEL_HWCONFIG_MEMORY_TYPE_LPDDR4 = 0,
    INTEL_HWCONFIG_MEMORY_TYPE_LPDDR5,
    INTEL_HWCONFIG_MEMORY_TYPE_HBM2,
    INTEL_HWCONFIG_MEMORY_TYPE_HBM2e,
    INTEL_HWCONFIG_MEMORY_TYPE_GDDR6,
};

#define INTEL_HWCONFIG_CACHE_TYPE_L3    BIT(0)
#define INTEL_HWCONFIG_CACHE_TYPE_LLC   BIT(1)
#define INTEL_HWCONFIG_CACHE_TYPE_EDRAM BIT(2)

#endif /* _INTEL_HWCONFIG_TYPES_H_ */
