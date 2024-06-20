/*
* Copyright (c) 2019, Intel Corporation
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

#include <map>
#include "mos_defs.h"
#include "media_class_trace.h"

#define LINUX_OS_VERSION_FILE                "/proc/version"

#define NOT_FOUND            -1

#define _aligned_malloc(size, alignment)  aligned_alloc(alignment, size)
#define _aligned_free(ptr)                free(ptr)

typedef void (*MOS_UserFeatureCallback)( void*, bool);

class MosMutex;

#define USER_FEATURE_KEY_INTERNAL            "UFKEY_INTERNAL\\"
#define USER_FEATURE_KEY_EXTERNAL            "UFKEY_EXTERNAL\\"

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

#define UFKEY_INTERNAL                       (( uint32_t ) 0x80000001 )
#define UFKEY_EXTERNAL                       (( uint32_t ) 0x80000002 )

#define UFKEY_INTERNAL_NEXT                   ""
#define USER_SETTING_CONFIG_PATH             "[config]"
#define USER_SETTING_CONFIG_PERF_PATH        "[config]"
#define USER_SETTING_CONFIG_PERMANENT_PATH   "[config]"
#define USER_SETTING_REPORT_PATH             "[report]"

#define MAX_UF_PATH                 256
#define DEFAULT_SUBKEY              "LibVa"
#define UFINT_PATH_LINUX            "UFINT"
#define UFEXT_PATH_LINUX            "UFEXT"

using RegBufferMap = std::map<std::string, std::map<std::string, std::string>>;

#define __MEDIA_USER_FEATURE_SUBKEY_INTERNAL                        "LibVa"
#define __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE                     ""
#define __MEDIA_USER_FEATURE_SUBKEY_PERMANENT                       ""
#define __MEDIA_USER_FEATURE_SUBKEY_REPORT                          "Report"
#define __MOS_USER_FEATURE_KEY_XML_FILEPATH_LOCATION                "LibVa"
#define __MEDIA_USER_FEATURE_SUBKEY_REPORT_EXTERNAL                 ""

#define __MOS_USER_FEATURE_VALUE_ADAPTIVE_TRANSFORM_DECISION_ENABLE_DEFAULT_VALUE "0"

//!
//! \brief User feature keys to define debug message levels and assertions.
//!

#if MOS_MESSAGES_ENABLED
//!
//! \brief Default setting is only critical prints are enabled, so users are not bombarded with irrelevant messages
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE                (0x1)
#endif // MOS_MESSAGES_ENABLED

typedef struct _MOS_UF_VALUE {
    char              pcValueName[MAX_USERFEATURE_LINE_LENGTH];
    uint32_t          ulValueLen;
    void              *ulValueBuf;
    uint32_t          ulValueType;
} MOS_UF_VALUE;

typedef struct _MOS_UF_KEY {
    void              *UFKey;
    char              pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    int32_t           valueNum;
    MOS_UF_VALUE     *pValueArray;
} MOS_UF_KEY;

typedef struct _MOS_UF_KEY_NODE_T
{
    MOS_UF_KEY*                 pElem;
    struct _MOS_UF_KEY_NODE_T*  pNext;
} MOS_UF_KEYNODE;

typedef MOS_UF_KEYNODE* MOS_PUF_KEYLIST;

class MosUtilitiesSpecificNext
{
public:

/*----------------------------------------------------------------------------
| Name      : MosUserFeatureSetValueExFile
| Purpose   : Sets the data and type of a specified value under a user feature key.
| Arguments : UFKey        [in] A handle to an open user feature key.
|             lpValueName  [in] The name of the user feature value.
|             Reserved      in] This parameter is reserved and must be NULL.
|             dwType       [in] The type of data pointed to by the lpData
|                               parameter.
|             lpData       [in] The data to be stored.
|             cbData       [in] The size of the information pointed to by the
|                               lpData parameter, in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS MosUserFeatureSetValueExFile(
    void            *UFKey,
    const char      *lpValueName,
    uint32_t        Reserved,
    uint32_t        dwType,
    uint8_t         *lpData,
    uint32_t        cbData);


/*----------------------------------------------------------------------------
| Name      : MosUserFeatureGetValueFile
| Purpose   : Retrieves the type and data for the specified user feature value.
| Arguments : UFKey     [in]  A handle to an open user feature key.
|             lpSubKey  [in]  The name of the user feature key. This key must be a
|                             subkey of the key specified by the UFKey parameter
|             lpValue   [in]  The name of the user feature value.
|             dwFlags   [in]  Reserved, could be any uint32_t type value
|             pdwType   [out] A pointer to a variable that receives a code
|                             indicating the type of data stored in the
|                             specified value.
|             pvData    [out] A pointer to a buffer that receives the value's
|                             data.
|             pcbData   [out] A pointer to a variable that specifies the size
|                             of the buffer pointed to by the pvData parameter,
|                             in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS MosUserFeatureGetValueFile(
    void       *UFKey,
    const char *lpSubKey,
    const char *lpValue,
    uint32_t   dwFlags,
    uint32_t   *pdwType,
    void       *pvData,
    uint32_t   *pcbData);

/*----------------------------------------------------------------------------
| Name      : MosUserFeatureOpenKeyFile
| Purpose   : Opens the specified user feature key.
| Arguments : ufKey        [in]  A handle to an open user feature key.
|             lpSubKey     [in]  The name of the user feature subkey to be opened.
|             ulOptions    [in]  This parameter is reserved and must be zero.
|             samDesired   [in]  Reserved, could be any REGSAM type value
|             phkResult    [out] A pointer to a variable that receives a handle
|                                to the opened key.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS MosUserFeatureOpenKeyFile(
    void       *ufKey,
    const char *lpSubKey,
    uint32_t   ulOptions,  // reserved
    uint32_t   samDesired,
    void       **phkResult);

/*----------------------------------------------------------------------------
    | Name      : UserFeatureDumpFile
    | Purpose   : This function read the whole User Feature File and dump User Feature File
    |             data to key linked list.
    | Arguments : szFileName         [in]  User Feature File name.
    |             pKeyList           [out] Key Linked list.
    | Returns   : MOS_STATUS_SUCCESS           Operation success.
    |             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
    |             MOS_STATUS_NO_SPACE          no space left for allocate
    |             MOS_STATUS_UNKNOWN           unknown user feature type found in User Feature File
    |             MOS_STATUS_INVALID_PARAMETER unknown items found in User Feature File
    | Comments  :
    \---------------------------------------------------------------------------*/
static MOS_STATUS UserFeatureDumpFile(const char *const szFileName, MOS_PUF_KEYLIST *pKeyList);

/*----------------------------------------------------------------------------
| Name      : UserFeatureDumpDataToFile
| Purpose   : This function dump key linked list data to File.
| Arguments : szFileName             [in] A handle to the File.
|             pKeyList               [in] Reserved, any LPDWORD type value.
| Returns   : MOS_STATUS_SUCCESS                        Operation success.
|             MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED  File can't be written.
| Comments  :
\---------------------------------------------------------------------------*/

static MOS_STATUS UserFeatureDumpDataToFile(const char *szFileName, MOS_PUF_KEYLIST pKeyList);

/*----------------------------------------------------------------------------
| Name      : UserFeatureFreeKeyList
| Purpose   : Free key list
| Arguments : pKeyList           [in] key list to be free.
| Returns   : None
| Comments  :
\---------------------------------------------------------------------------*/
static void UserFeatureFreeKeyList(MOS_PUF_KEYLIST pKeyList);

private:

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureFindKey
    | Purpose   : This function finds a key in keys linked list according to key
    |             name.
    | Arguments : pKeyList   [in] Key Linked list.
    |             pcKeyName  [in] Name to the key to find.
    | Returns   : Matched uf_key data. otherwise return NULL.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_UF_KEY*UserFeatureFindKey(MOS_PUF_KEYLIST pKeyList, char * const pcKeyName);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureFindValue
    | Purpose   : Find a value in values array of a key. Return position in values
    |             array
    | Arguments : UFKey        [in] Searched Key node.
    |             pcValueName  [in] Value name.
    | Returns   : Matched value No. if it can be found, otherwise, return
    |             NOT_FOUND(-1);
    | Comments  :
    \---------------------------------------------------------------------------*/
    static int32_t UserFeatureFindValue(MOS_UF_KEY UFKey, char * const pcValueName);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureAdd
    | Purpose   : Add new key to keys' linked list.
    | Arguments : pKeyList       [in] Key linked list.
    |             NewKey         [in] Added new key.
    | Returns   : MOS_STATUS_SUCCESS            success
    |             MOS_STATUS_INVALID_PARAMETER  invalid NewKey
    |             MOS_STATUS_NO_SPACE           no space left for allocate
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureAdd(MOS_PUF_KEYLIST *pKeyList, MOS_UF_KEY *NewKey);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureSet
    | Purpose   : This function set a key to the key list.
    | Arguments : pKeyList          [in] Key linked list.
    |             NewKey            [in] Set key content.
    | Returns   : MOS_STATUS_SUCCESS      Operation success.
    |             MOS_STATUS_UNKNOWN      Can't find key in User Feature File.
    |             MOS_STATUS_NO_SPACE     no space left for allocate
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureSet(MOS_PUF_KEYLIST *pKeyList, MOS_UF_KEY NewKey);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureQuery
    | Purpose   : This function query a key's value and return matched key node
    |             content just with matched value content.
    | Arguments : pKeyList      [in] Key linked list.
    |             NewKey        [in] New key content with matched value.
    | Returns   : MOS_STATUS_SUCCESS         Operation success.
    |             MOS_STATUS_UNKNOWN         Can't find key or value in User Feature File.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureQuery(MOS_PUF_KEYLIST pKeyList, MOS_UF_KEY *NewKey);

    static MOS_STATUS UserFeatureReadNextTokenFromFile(FILE *pFile, const char *szFormat, char  *szToken);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureSetValue
    | Purpose   : Modify or add a value of the specified user feature key.
    | Arguments : strKey         [in] Pointer to user feature key name.
    |             pcValueName    [in] Pointer to a string containing the name of
    |                                 the value to set. If a value with this name
    |                                 is not already present in the key, the
    |                                 function adds it to the key.
    |             uiValueType    [in] Type of information to be stored.
    |             szValueData    [in] Pointer to a null-terminated string
    |                                 containing the data to set for the default
    |                                 value of the specified key
    |             uiValueDataLen [in] Size of the string pointed to by the
    |                                 szValueData parameter, not including the
    |                                 terminating null character, in bytes
    | Returns   : MOS_STATUS_SUCCESS           function success
    |             MOS_STATUS_INVALID_PARAMETER invalid paramater
    |             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
    |             MOS_STATUS_NO_SPACE          no space left for allocate
    |             MOS_STATUS_UNKNOWN           unknown user feature type found in User Feature File
    |             MOS_STATUS_INVALID_PARAMETER unknown items found in User Feature File
    |             MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED  User Feature File can't be written.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureSetValue(
        char * const        strKey,
        const char * const  pcValueName,
        uint32_t            uiValueType,
        void                *pData,
        int32_t             nDataSize);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureQueryValue
    | Purpose   : The QueryValue function retrieves the type and data for a
    |             specified value name associated with a special user feature key.
    | Arguments : strKey         [in]  Pointer to user feature key name.
    |             pcValueName    [in]  Pointer to a string containing the name
    |                                  of the value to query.
    |             uiValueType    [out] Output Value's type
    |             pData          [out] Output value's content
    |             nDataSize      [out] Output the size of value's content.
    | Returns   : MOS_STATUS_SUCCESS           function success
    |             MOS_STATUS_INVALID_PARAMETER invalid paramater
    |             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
    |             MOS_STATUS_NO_SPACE          no space left for allocate
    |             MOS_STATUS_UNKNOWN           Can't find key or value in User Feature File.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureQueryValue(
        char * const        strKey,
        const char * const  pcValueName,
        uint32_t            *uiValueType,
        void                *pData,
        int32_t             *nDataSize);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureGetKeyIdbyName
    | Purpose   : Get ID of the user feature key bu its name
    | Arguments : pcKeyName      [in]  Pointer to user feature key name.
    |             pUFKey         [out] A UFKEY pointer to store returned UFKey
    | Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
    |             If the function fails, the return value is a error code defined
    |             in mos_utilities.h.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureGetKeyIdbyName(const char  *pcKeyName, void **pUFKey);

    /*----------------------------------------------------------------------------
    | Name      : UserFeatureGetKeyNamebyId
    | Purpose   : Get name of the user feature key bu its ID
    | Arguments : UFKey      [in]  ID of the user feature key
    |             pcKeyName  [out] To store user feature key name.
    | Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
    |             If the function fails, the return value is a error code defined
    |             in mos_utilities.h.
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS UserFeatureGetKeyNamebyId(void  *UFKey, char  *pcKeyName);

public:
    static const char*          m_szUserFeatureFile;
    static MOS_PUF_KEYLIST      m_ufKeyList;
    static int32_t              m_mosTraceFd;
    static uint64_t             m_filterEnv;
    static uint32_t             m_levelEnv;
    static const char* const    m_mosTracePath;
    static std::map<std::string, std::map<std::string, std::string>> m_regBuffer;

private:
    static MosMutex             m_userSettingMutex;
MEDIA_CLASS_DEFINE_END(MosUtilitiesSpecificNext)
};
#endif // __MOS_UTILITIES_SPECIFIC_H__
