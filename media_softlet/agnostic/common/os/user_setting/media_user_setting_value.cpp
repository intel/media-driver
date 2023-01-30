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
Value::NUMERIC_VALUE::NUMERIC_VALUE() : m_u64Data(0)
{
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const bool value) : m_u64Data(0)
{
    m_bData = value;
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const uint32_t value) : m_u64Data(0)
{
    m_u32Data = value;
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const uint64_t value) : m_u64Data(0)
{
    m_u64Data = value;
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const int32_t value) : m_u64Data(0)
{
    m_i32Data = value;
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const int64_t value) : m_u64Data(0)
{
    m_i64Data = value;
}
Value::NUMERIC_VALUE::NUMERIC_VALUE(const float value) : m_u64Data(0)
{
    m_fData = value;
}

Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator=(const int32_t &value)
{
    m_i32Data = value;
    return *this;
}
Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator = (const int64_t &value)
{
    m_i64Data = value;
    return *this;
}
Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator = (const uint32_t &value)
{
    m_u32Data = value;
    return *this;
}
Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator = (const uint64_t &value)
{
    m_u64Data = value;
    return *this;
}
Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator=(const bool &value)
{
    m_bData = value;
    return *this;
}
Value::NUMERIC_VALUE &Value::NUMERIC_VALUE::operator=(const float &value)
{
    m_fData = value;
    return *this;
}

Value::Value()
{
}

Value::~Value()
{
}

Value::Value(const Value &value)
{
    m_sValue        = value.m_sValue;
    m_size          = value.m_size;
    m_type          = value.m_type;
    m_numericValue  = value.m_numericValue;
}

Value::Value(const int32_t &value) : m_numericValue(value)
{
    m_sValue        = std::to_string(value);
    m_size          = sizeof(int32_t);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_INT32;
}

Value::Value(const int64_t &value): m_numericValue(value)
{
    m_sValue        = std::to_string(value);
    m_size          = sizeof(int64_t);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_INT64;
}
Value::Value(const uint32_t &value) : m_numericValue(value)
{
    m_sValue        = std::to_string(value);
    m_size          = sizeof(uint32_t);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_UINT32;
}

Value::Value(const uint64_t &value) : m_numericValue(value)
{
    m_sValue        = std::to_string(value);
    m_size          = sizeof(uint64_t);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_UINT64;
}

Value::Value(const bool &value) : m_numericValue(value)
{
    m_sValue        = value ? "1" : "0";
    m_size          = sizeof(uint32_t);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_BOOL;
}

Value::Value(const float &value) : m_numericValue(value)
{
    m_sValue        = std::to_string(value);
    m_size          = sizeof(float);
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_FLOAT;
}

Value::Value(const std::string &value) : m_numericValue(0)
{
    m_sValue        = value;
    m_size          = m_sValue.length();
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value::Value(const char *value) : m_numericValue(0)
{
    m_sValue        = value;
    m_size          = m_sValue.length();
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value::Value(char *value) : m_numericValue(0)
{
    m_sValue        = value;
    m_size          = m_sValue.length();
    m_type          = MOS_USER_FEATURE_VALUE_TYPE_STRING;
}

Value& Value::operator=(const Value &value)
{
    if (this != &value)
    {
        m_sValue                = value.m_sValue;
        m_size                  = value.m_size;
        m_type                  = value.m_type;
        m_numericValue          = value.m_numericValue;
    }
    return *this;
}

Value& Value::operator=(const int32_t &value)
{
    m_sValue                    = std::to_string(value);
    m_size                      = sizeof(int32_t);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_INT32;
    m_numericValue.m_i32Data    = value;
    return *this;
}

Value& Value::operator=(const int64_t &value)
{
    m_sValue                    = std::to_string(value);
    m_size                      = sizeof(int64_t);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_INT64;
    m_numericValue.m_i64Data    = value;
    return *this;
}
Value& Value::operator=(const uint32_t &value)
{
    m_sValue                    = std::to_string(value);
    m_size                      = sizeof(uint32_t);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_UINT32;
    m_numericValue.m_u32Data    = value;
    return *this;
}
Value& Value::operator=(const uint64_t &value)
{
    m_sValue                    = std::to_string(value);
    m_size                      = sizeof(uint64_t);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_UINT64;
    m_numericValue.m_u64Data    = value;
    return *this;
}
Value& Value::operator=(const bool &value)
{
    m_sValue                    = value ? "1" : "0";
    m_size                      = sizeof(uint32_t);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_BOOL;
    m_numericValue.m_bData      = value;
    return *this;
}
Value& Value::operator=(const float &value)
{
    m_sValue                    = std::to_string(value);
    m_size                      = sizeof(float);
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_FLOAT;
    m_numericValue.m_fData      = value;
    return *this;
}

Value& Value::operator=(const std::string &value)
{
    m_sValue                    = value;
    m_size                      = m_sValue.length();
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    m_numericValue.m_u64Data    = 0;
    return *this;
}

Value& Value::operator=(const char* value)
{
    m_sValue                    = value;
    m_size                      = m_sValue.length();
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    m_numericValue.m_u64Data    = 0;
    return *this;
}

Value& Value::operator=(char* value)
{
    m_sValue                    = value;
    m_size                      = m_sValue.length();
    m_type                      = MOS_USER_FEATURE_VALUE_TYPE_STRING;
    m_numericValue.m_u64Data    = 0;
    return *this;
}

template <>
bool Value::Get() const
{
    return m_numericValue.m_bData;
}

template <>
uint8_t Value::Get()const
{
    return (uint8_t)m_numericValue.m_u32Data;
}

template <>
uint32_t Value::Get()const
{
    return m_numericValue.m_u32Data;
}

template <>
int32_t Value::Get()const
{
    return m_numericValue.m_i32Data;
}

template <>
int64_t Value::Get()const
{
    return m_numericValue.m_i64Data;
}

template <>
unsigned long Value::Get() const
{
    return (unsigned long) m_numericValue.m_u64Data;
}

template <>
unsigned long long Value::Get()const
{
    return m_numericValue.m_u64Data;
}

template <>
float Value::Get() const
{
    return m_numericValue.m_fData;
}
}