/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file        mos_utilities_specific.h 
//! \brief       This module defines MOS interface on Linux/Android platform 
//!
#ifndef __MOS_UTILITIES_SPECIFIC_H__
#define __MOS_UTILITIES_SPECIFIC_H__

#include "mos_defs.h"

#define NOT_FOUND            -1

#define USER_FEATURE_KEY_INTERNAL            "UFKEY_INTERNAL\\"
#define USER_FEATURE_KEY_EXTERNAL            "UFKEY_EXTERNAL\\"
#define LINUX_OS_VERSION_FILE                "/proc/version"

//user feature
#if ANDROID_VERSION >= 800
#define USER_FEATURE_FILE                   "/data/igfx_user_feature.txt"
#else
#define USER_FEATURE_FILE                   "/etc/igfx_user_feature.txt"
#define USER_FEATURE_FILE_NEXT              "/etc/igfx_user_feature_next.txt"
#endif
#define UF_KEY_ID                           "[KEY]"
#define UF_VALUE_ID                         "[VALUE]"
#define UF_CAPABILITY                       64
#define MAX_USERFEATURE_LINE_LENGTH         256
#define MAX_UF_LINE_STRING_FORMAT           "%255[^\n]\n"

#define UF_NONE                             ( 0 )   // No value type
#define UF_SZ                               ( 1 )   // Unicode nul terminated string
#define UF_EXPAND_SZ                        ( 2 )   // Unicode nul terminated string
                                                    // (with environment variable references)
#define UF_BINARY                           ( 3 )   // Free form binary
#define UF_DWORD                            ( 4 )   // 32-bit number
#define UF_DWORD_LITTLE_ENDIAN              ( 4 )   // 32-bit number (same as UF_DWORD)
#define UF_DWORD_BIG_ENDIAN                 ( 5 )   // 32-bit number
#define UF_LINK                             ( 6 )   // Symbolic Link (unicode)
#define UF_MULTI_SZ                         ( 7 )   // Multiple Unicode strings
#define UF_RESOURCE_LIST                    ( 8 )   // Resource list in the resource map
#define UF_FULL_RESOURCE_DESCRIPTOR         ( 9 )   // Resource list in the hardware description
#define UF_RESOURCE_REQUIREMENTS_LIST       ( 10 )
#define UF_QWORD                            ( 11 )  // 64-bit number
#define UF_QWORD_LITTLE_ENDIAN              ( 11 )  // 64-bit number (same as UF_QWORD)

#define RRF_RT_UF_NONE                      0x00000001
#define RRF_RT_UF_SZ                        0x00000002
#define RRF_RT_UF_EXPAND_SZ                 0x00000004
#define RRF_RT_UF_BINARY                    0x00000008
#define RRF_RT_UF_DWORD                     0x00000010
#define RRF_RT_UF_MULTI_SZ                  0x00000020
#define RRF_RT_UF_QWORD                     0x00000040

#define KEY_READ                             0
#define KEY_WRITE                            1

#define EVENT_QUERY_STATE                    0x0001
#define EVENT_MODIFY_STATE                   0x0002
#define STANDARD_RIGHTS_ALL                  (0x001F0000L)

#define UFKEY_INTERNAL                       (( uint32_t ) 0x80000001 )
#define UFKEY_EXTERNAL                       (( uint32_t ) 0x80000002 )

#define UFKEY_INTERNAL_NEXT                   ""
#define USER_SETTING_CONFIG_PATH             "[config]"
#define USER_SETTING_REPORT_PATH             "[report]"

//to avoid runing error in Ubuntu.
#define RegCloseKey   RegCloseKey_Linux

#define MAX_UF_PATH                 256
#define DEFAULT_SUBKEY              "LibVa"
#define UFINT_PATH_LINUX            "UFINT"
#define UFEXT_PATH_LINUX            "UFEXT"

#define _aligned_malloc(size, alignment)  aligned_alloc(alignment, size)
#define _aligned_free(ptr)                free(ptr)

#define __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR     "1"
#define __MOS_USER_FEATURE_VALUE_ADAPTIVE_TRANSFORM_DECISION_ENABLE_DEFAULT_VALUE "0"

typedef enum
{
    LINUX_UF_FUNCTYPE_INVALID,
    LINUX_UF_FUNCTYPE_READ,
    LINUX_UF_FUNCTYPE_WRITE,
}LINUX_UF_FUNC_TYPE,*PLINUX_UF_FUNC_TYPE;

typedef struct _MOS_UF_VALUE {
    char              pcValueName[MAX_USERFEATURE_LINE_LENGTH];
    uint32_t          ulValueLen;
    void              *ulValueBuf;
    uint32_t          ulValueType;
} MOS_UF_VALUE;

typedef struct _MOS_UF_KEY {
    void              *UFKey;
    char              pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    uint32_t          ulValueNum;
    MOS_UF_VALUE     *pValueArray;
} MOS_UF_KEY;

typedef struct _MOS_UF_KEY_NODE_T
{
    MOS_UF_KEY*                 pElem;
    struct _MOS_UF_KEY_NODE_T*  pNext;
} MOS_UF_KEYNODE;

typedef MOS_UF_KEYNODE* MOS_PUF_KEYLIST;

typedef void (*MOS_UserFeatureCallback)( void*, bool);

//!
//! Structure UFKEYOPS
//! \brief User Feature Key operation function pointers
//!
typedef struct UFKEYOPS
{
    MOS_STATUS (* pfnUserFeatureOpenKey)(
        void       *UFKey,
        const char *lpSubKey,
        uint32_t   ulOptions,
        uint32_t   samDesired,
        PPVOID     phkResult);
    MOS_STATUS (* pfnUserFeatureGetValue)(
        void       *UFKey,
        const char *lpSubKey,
        const char *lpValue,
        uint32_t   dwFlags,
        uint32_t   *pdwType,
        void       *pvData,
        uint32_t   *pcbData);
    MOS_STATUS (* pfnUserFeatureSetValueEx)(
        void       *UFKey,
        const char *lpValueName,
        uint32_t   Reserved,
        uint32_t   dwType,
        uint8_t    *lpData,
        uint32_t   cbData);
} UFKEYOPS,*PUFKEYOPS;
#endif // __MOS_UTILITIES_SPECIFIC_H__
