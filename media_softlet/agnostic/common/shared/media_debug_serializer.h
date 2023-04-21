/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     media_debug_serializer.h
//!

#pragma once

#if USE_MEDIA_DEBUG_TOOL

#include <cstring>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <stdint.h>
#include "media_class_trace.h"

template <typename T, typename = void>
class MediaDebugSerializer
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        const size_t num = size / sizeof(T);
        size_t       idx = 0;

        if (num > 1)
        {
            os << "Total Num: " << num << std::endl;
            os << std::endl;
        }

        while (idx < num)
        {
            if (num > 1)
            {
                os << "---------"
                   << "Index = " << idx << "---------" << std::endl;
            }

            os << reinterpret_cast<const T *>(data)[idx++] << std::endl;
        }

        size_t numTrailingBytes = size - num * sizeof(T);
        if (numTrailingBytes)
        {
            os << numTrailingBytes << " trailing bytes cannot be reinterpreted as "
               << typeid(T).name() << " whose size is " << sizeof(T) << ":"
               << std::endl;
            for (size_t i = size - numTrailingBytes; i < size; ++i)
            {
                os << std::setfill('0') << std::setw(2) << std::hex << std::nouppercase
                   << static_cast<uint32_t>(reinterpret_cast<const uint8_t *>(data)[i])
                   << std::endl;
            }
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

template <typename T>
class MediaDebugSerializer<
    T,
#if __cplusplus < 201703L
    typename std::enable_if<std::is_fundamental<T>::value && !std::is_unsigned<T>::value>::type>
#else
    std::enable_if_t<std::is_fundamental_v<T> && !std::is_unsigned_v<T> > >
#endif
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        const size_t num = size / sizeof(T);
        size_t       idx = 0;

        while (idx < num)
        {
            os << reinterpret_cast<const T *>(data)[idx++] << std::endl;
        }

        size_t numTrailingBytes = size - num * sizeof(T);
        if (numTrailingBytes)
        {
            os << numTrailingBytes << " trailing bytes cannot be reinterpreted as "
               << typeid(T).name() << " whose size is " << sizeof(T) << ":"
               << std::endl;
            for (size_t i = size - numTrailingBytes; i < size; ++i)
            {
                os << std::setfill('0') << std::setw(2) << std::hex << std::nouppercase
                   << static_cast<uint32_t>(reinterpret_cast<const uint8_t *>(data)[i])
                   << std::endl;
            }
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

template <typename T>
class MediaDebugSerializer<
    T,
#if __cplusplus < 201703L
    typename std::enable_if<std::is_unsigned<T>::value>::type>
#else
    std::enable_if_t<std::is_unsigned_v<T> > >
#endif
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        constexpr size_t LINE_WIDTH = 16 / sizeof(T);
        constexpr size_t ELEM_WIDTH = sizeof(T) << 1;

        const size_t num = size / sizeof(T);
        size_t       idx = 0;
        while (idx < num)
        {
            os << std::setfill('0') << std::setw(ELEM_WIDTH) << std::hex
               << std::nouppercase << reinterpret_cast<const T *>(data)[idx]
               << ((idx + 1) % LINE_WIDTH == 0 ? "\n" : " ");
            ++idx;
        }

        size_t numTrailingBytes = size - num * sizeof(T);
        if (numTrailingBytes)
        {
            T last = 0;
            std::memcpy(
                &last,
                static_cast<const char *>(data) + size - numTrailingBytes,
                numTrailingBytes);

            std::stringstream ss;
            ss << std::setfill('?') << std::setw(ELEM_WIDTH) << std::hex
               << std::nouppercase << last;

            std::string str(ss.str());
            if (str[0] == '?')  // little endian
            {
                for (auto it = str.rbegin(); it != str.rbegin() + numTrailingBytes * 2; ++it)
                {
                    *it = *it == '?' ? '0' : *it;
                }
            }
            else  // big endian
            {
                for (auto it = str.begin(); it != str.begin() + numTrailingBytes * 2; ++it)
                {
                    *it = *it == '?' ? '0' : *it;
                }
            }

            os << str << ((idx + 1) % LINE_WIDTH == 0 ? "\n" : " ");
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

template <>
class MediaDebugSerializer<uint8_t>
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        size_t idx = 0;
        while (idx < size)
        {
            os << std::setfill('0') << std::setw(2) << std::hex << std::nouppercase
               << static_cast<uint32_t>(reinterpret_cast<const uint8_t *>(data)[idx])
               << ((idx + 1) % 32 == 0 ? "\n" : " ");
            ++idx;
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

template <>
class MediaDebugSerializer<int8_t>
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        size_t idx = 0;
        while (idx < size)
        {
            os << static_cast<int32_t>(reinterpret_cast<const int8_t *>(data)[idx++])
               << std::endl;
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

template <>
class MediaDebugSerializer<void>
{
public:
    void operator()(std::ostream &os, const void *data, size_t size)
    {
        size_t idx = 0;
        while (idx < size)
        {
            os << std::setfill('0') << std::setw(2) << std::hex << std::nouppercase
               << static_cast<uint32_t>(reinterpret_cast<const uint8_t *>(data)[idx])
               << ((idx + 1) % 32 == 0 ? "\n" : " ");
            ++idx;
        }
    }

    MEDIA_CLASS_DEFINE_END(MediaDebugSerializer)
};

#endif  // USE_MEDIA_DEBUG_TOOL
