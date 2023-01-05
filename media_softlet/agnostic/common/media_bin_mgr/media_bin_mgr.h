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
//! \file     media_bin_mgr.h
//!

#ifndef MEDIA_BIN_MGR_H__
#define MEDIA_BIN_MGR_H__

#if defined(MEDIA_BIN_SUPPORT) && !defined(MEDIA_BIN_DLL)
#define DECLARE_SHARED_ARRAY_UINT8(ARRAY_NAME) extern uint8_t *ARRAY_NAME
#define DECLARE_SHARED_ARRAY_UINT32(ARRAY_NAME) extern unsigned int *ARRAY_NAME
#elif defined(MEDIA_BIN_ULT)
#define DECLARE_SHARED_ARRAY_UINT8(ARRAY_NAME) extern const uint8_t ULT_##ARRAY_NAME[]; extern uint8_t *ARRAY_NAME
#define DEFINE_SHARED_ARRAY_UINT8(ARRAY_NAME) extern const uint8_t ULT_##ARRAY_NAME[]
#define DECLARE_SHARED_ARRAY_UINT32(ARRAY_NAME) extern const unsigned int ULT_##ARRAY_NAME[]; extern unsigned int *ARRAY_NAME
#define DEFINE_SHARED_ARRAY_UINT32(ARRAY_NAME) extern const unsigned int ULT_##ARRAY_NAME[]
#else
#define DECLARE_SHARED_ARRAY_UINT8(ARRAY_NAME) extern const uint8_t ARRAY_NAME[]
#define DEFINE_SHARED_ARRAY_UINT8(ARRAY_NAME) extern const uint8_t ARRAY_NAME[]
#define DECLARE_SHARED_ARRAY_UINT32(ARRAY_NAME) extern const unsigned int ARRAY_NAME[]
#define DEFINE_SHARED_ARRAY_UINT32(ARRAY_NAME) extern const unsigned int ARRAY_NAME[]
#endif

#if defined(MEDIA_BIN_SUPPORT)
#define DECLARE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE) extern unsigned int ARRAY_SIZE
#define DEFINE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE, SIZE) extern unsigned int ARRAY_SIZE = SIZE
#elif defined(MEDIA_BIN_ULT)
#define DECLARE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE) extern const unsigned int ULT_##ARRAY_SIZE; extern unsigned int ARRAY_SIZE
#define DEFINE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE, SIZE) extern const unsigned int ULT_##ARRAY_SIZE = SIZE
#else
#define DECLARE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE) extern const unsigned int ARRAY_SIZE
#define DEFINE_SHARED_ARRAY_SIZE_UINT32(ARRAY_SIZE, SIZE) extern const unsigned int ARRAY_SIZE = SIZE
#endif

#if defined(MEDIA_BIN_SUPPORT)
#include <string>
#include <map>

typedef struct _MEDIA_BIN_INFO
{
    uint32_t* size;
    void** data;
} MEDIA_BIN_INFO;

#if defined(MEDIA_BIN_DLL)

typedef struct _MEDIA_BIN_DATA_INFO
{
    uint32_t    size;
    const void* data;
} MEDIA_BIN_DATA_INFO;

bool RegisterMediaBin(const char* name, uint32_t size, const void* data);
bool GetMediaBinInternal(std::map<std::string, MEDIA_BIN_INFO>& map);

#else
bool LoadMediaBinInternal(const char* name, uint32_t* p_size, void** data);

template <typename T>
bool LoadMediaBin(const char* name, uint32_t* p_size, T** data)
{
    return LoadMediaBinInternal(name, p_size, (void**)data);
}
#endif
#endif  // (MEDIA_BIN_SUPPORT)

#endif  // MEDIA_BIN_MGR_H__
