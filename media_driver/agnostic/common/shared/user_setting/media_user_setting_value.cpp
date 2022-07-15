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
//! \file     media_user_setting_value.cpp
//! \brief    Implementation for user setting value
//!
#include "media_user_setting_value.h"

namespace MediaUserSetting {

Value::Value()
{
}

Value::~Value()
{
}

template <class T>
inline std::string Value::ToString(const T &data)
{
    std::stringstream convert;
    convert << data;

    return convert.str();
}

Value::Value(const Value &value)
{
    m_value = value.m_value;
    m_size = value.m_size;
    m_type  = value.m_type;
}

Value::Value(const int32_t &value)
{
    m_value = ToString<int32_t>(value);
    m_size = sizeof(int32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_INT32;
}

Value::Value(const int64_t &value)
{
    m_value = ToString<int64_t>(value);
    m_size = sizeof(int64_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_INT64;
}
Value::Value(const uint32_t &value)
{
    m_value = ToString<uint32_t>(value);
    m_size = sizeof(uint32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_UINT32;
}

Value::Value(const uint64_t &value)
{
    m_value = ToString<uint64_t>(value);
    m_size = sizeof(uint64_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_UINT64;
}

Value::Value(const bool &value)
{
    m_value = value ? "1" : "0";
    m_size = sizeof(uint32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_BOOL;
}

Value::Value(const float &value)
{
    m_value = ToString<float>(value);
    m_size = sizeof(float);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_FLOAT;
}

Value::Value(const std::string &value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value::Value(const char* value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value::Value(char* value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value& Value::operator=(const Value &value)
{
    if (this != &value)
    {
        m_value = value.m_value;
        m_size = value.m_size;
        m_type  = value.m_type;
    }
    return *this;
}

Value& Value::operator=(const int32_t &value)
{
    m_value = ToString<int32_t>(value);
    m_size = sizeof(int32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_INT32;
    return *this;
}

Value& Value::operator=(const int64_t &value)
{
    m_value = ToString<int64_t>(value);
    m_size = sizeof(int64_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_INT64;
    return *this;
}
Value& Value::operator=(const uint32_t &value)
{
    m_value = ToString<uint32_t>(value);
    m_size = sizeof(uint32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_UINT32;
    return *this;
}
Value& Value::operator=(const uint64_t &value)
{
    m_value = ToString<uint64_t>(value);
    m_size = sizeof(uint64_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_UINT64;
    return *this;
}
Value& Value::operator=(const bool &value)
{
    m_value = value ? "1" : "0";
    m_size = sizeof(uint32_t);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_BOOL;
    return *this;
}
Value& Value::operator=(const float &value)
{
    m_value = ToString<float>(value);
    m_size = sizeof(float);
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_FLOAT;
    return *this;
}

Value& Value::operator=(const std::string &value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    return *this;
}

Value& Value::operator=(const char* value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    return *this;
}

Value& Value::operator=(char* value)
{
    m_value = value;
    m_size = m_value.length();
    m_type  = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    return *this;
}
}