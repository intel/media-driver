/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_user_setting_value.h
//! \brief    Implementation for user setting value
//!

#ifndef __MEDIA_USER_SETTING_VALUE_H__
#define __MEDIA_USER_SETTING_VALUE_H__

#include "mos_utilities_common.h"
#include <string>
#include <sstream>

namespace MediaUserSetting {

class Value
{
public:
    Value();
    virtual ~Value();
    Value(const Value &value);
    Value(const int32_t &value);
    Value(const int64_t &value);
    Value(const uint32_t &value);
    Value(const uint64_t &value);
    Value(const bool &value);
    Value(const float &value);
    Value(const std::string &value);
    Value(const char* value);
    Value(char* value);

    Value& operator=(const Value &value);
    Value& operator=(const int32_t &value);
    Value& operator=(const int64_t &value);
    Value& operator=(const uint32_t &value);
    Value& operator=(const uint64_t &value);
    Value& operator=(const bool &value);
    Value& operator=(const float &value);
    Value& operator=(const std::string &value);
    Value& operator=(const char* value);
    Value& operator=(char* value);

    template <class T>
    T Get() const
    {
        return m_sValue;
    }

    const std::string &ConstString() const { return m_sValue; }

    const std::size_t &Size() const { return m_size; }
    const MOS_USER_FEATURE_VALUE_TYPE &ValueType() const { return m_type; }

private:
    std::size_t m_size = 0;
    std::string m_sValue{};
    union NUMERIC_VALUE
    {
        NUMERIC_VALUE();
        NUMERIC_VALUE(const bool value);
        NUMERIC_VALUE(const uint32_t value);
        NUMERIC_VALUE(const uint64_t value);
        NUMERIC_VALUE(const int32_t value);
        NUMERIC_VALUE(const int64_t value);
        NUMERIC_VALUE(const float value);
        NUMERIC_VALUE &operator=(const int32_t &value);
        NUMERIC_VALUE &operator=(const int64_t &value);
        NUMERIC_VALUE &operator=(const uint32_t &value);
        NUMERIC_VALUE &operator=(const uint64_t &value);
        NUMERIC_VALUE &operator=(const bool &value);
        NUMERIC_VALUE &operator=(const float &value);

        bool        m_bData;
        uint32_t    m_u32Data;
        uint64_t    m_u64Data;
        int32_t     m_i32Data;
        int64_t     m_i64Data;
        float       m_fData;
    };
    NUMERIC_VALUE               m_numericValue = {};
    MOS_USER_FEATURE_VALUE_TYPE m_type         = MOS_USER_FEATURE_VALUE_TYPE_INVALID;
};

template <> bool            Value::Get() const;
template <> uint8_t         Value::Get() const;
template <> uint32_t        Value::Get() const;
template <> int32_t         Value::Get() const;
template <> unsigned long   Value::Get() const;
template <> int64_t         Value::Get() const;
template <> float           Value::Get() const;
template <> unsigned long long  Value::Get() const;
}

#endif