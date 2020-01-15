/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     cp_utils.cpp
//! \brief    Implement of CPLibUtils to interact with CPLib
//!
#include "cplib_utils.h"
#include "mos_utilities.h"

std::unordered_map<const char*, void*> CPLibUtils::m_symbols;
std::mutex                             CPLibUtils::m_referenceMutex;
int                                    CPLibUtils::m_referenceCount = 0;

void *      CPLibUtils::m_phandle                    = nullptr;
const char *CPLibUtils::CPLIB_PATH                   = "cplib.so";
const char *CPLibUtils::FUNC_INIT_CPLIB              = "Init_CPLib";
const char *CPLibUtils::FUNC_RELEASE_CPLIB           = "Release_CPLib";
const char *CPLibUtils::FUNC_GET_CPLIB_MAJOR_VERSION = "Get_CPLib_Major_Version";
const char *CPLibUtils::FUNC_CREATE_DDICP            = "Create_DdiCp";
const char *CPLibUtils::FUNC_DELETE_DDICP            = "Delete_DdiCp";
const char *CPLibUtils::FUNC_CREATE_MHWCP            = "Create_MhwCp";
const char *CPLibUtils::FUNC_DELETE_MHWCP            = "Delete_MhwCp";
const char *CPLibUtils::FUNC_CREATE_MOSCP            = "Create_MosCp";
const char *CPLibUtils::FUNC_DELETE_MOSCP            = "Delete_MosCp";
const char *CPLibUtils::FUNC_CREATE_MEDIALIBVACAPSCP = "Create_MediaLibvaCapsCp";
const char *CPLibUtils::FUNC_DELETE_MEDIALIBVACAPSCP = "Delete_MediaLibvaCapsCp";
const char *CPLibUtils::FUNC_CREATE_SECUREDECODE     = "Create_SecureDecode";
const char *CPLibUtils::FUNC_DELETE_SECUREDECODE     = "Delete_SecureDecode";
const char *CPLibUtils::FUNC_CREATE_DECODECP         = "Create_DecodeCp";
const char *CPLibUtils::FUNC_DELETE_DECODECP         = "Delete_DecodeCp";
const char *CPLibUtils::FUNC_CREATE_CPSTREAMOUT      = "Create_CpStreamOut";
const char *CPLibUtils::FUNC_DELETE_CPSTREAMOUT      = "Delete_CpStreamOut";

bool CPLibUtils::LoadCPLib(VADriverContextP ctx)
{
    m_referenceMutex.lock();

    m_phandle = dlopen(CPLIB_PATH, RTLD_NOW | RTLD_LOCAL);
    if(nullptr == m_phandle)
    {
        CPLIB_ASSERTMESSAGE("Failed to open %s %s", CPLIB_PATH, dlerror());
        m_referenceMutex.unlock();
        return false;
    }
    else
    {
        ++m_referenceCount;
        if(1 == m_referenceCount)
        {
            m_symbols.clear();

            m_symbols[FUNC_GET_CPLIB_MAJOR_VERSION] = dlsym(m_phandle, FUNC_GET_CPLIB_MAJOR_VERSION);
            m_symbols[FUNC_INIT_CPLIB]              = dlsym(m_phandle, FUNC_INIT_CPLIB);
            m_symbols[FUNC_RELEASE_CPLIB]           = dlsym(m_phandle, FUNC_RELEASE_CPLIB);
            m_symbols[FUNC_CREATE_DDICP]            = dlsym(m_phandle, FUNC_CREATE_DDICP);
            m_symbols[FUNC_DELETE_DDICP]            = dlsym(m_phandle, FUNC_DELETE_DDICP);
            m_symbols[FUNC_CREATE_MHWCP]            = dlsym(m_phandle, FUNC_CREATE_MHWCP);
            m_symbols[FUNC_DELETE_MHWCP]            = dlsym(m_phandle, FUNC_DELETE_MHWCP);
            m_symbols[FUNC_CREATE_MOSCP]            = dlsym(m_phandle, FUNC_CREATE_MOSCP);
            m_symbols[FUNC_DELETE_MOSCP]            = dlsym(m_phandle, FUNC_DELETE_MOSCP);
            m_symbols[FUNC_CREATE_MEDIALIBVACAPSCP] = dlsym(m_phandle, FUNC_CREATE_MEDIALIBVACAPSCP);
            m_symbols[FUNC_DELETE_MEDIALIBVACAPSCP] = dlsym(m_phandle, FUNC_DELETE_MEDIALIBVACAPSCP);
            m_symbols[FUNC_CREATE_SECUREDECODE]     = dlsym(m_phandle, FUNC_CREATE_SECUREDECODE);
            m_symbols[FUNC_DELETE_SECUREDECODE]     = dlsym(m_phandle, FUNC_DELETE_SECUREDECODE);
            m_symbols[FUNC_CREATE_DECODECP]         = dlsym(m_phandle, FUNC_CREATE_DECODECP);
            m_symbols[FUNC_DELETE_DECODECP]         = dlsym(m_phandle, FUNC_DELETE_DECODECP);
            m_symbols[FUNC_CREATE_CPSTREAMOUT]      = dlsym(m_phandle, FUNC_CREATE_CPSTREAMOUT);
            m_symbols[FUNC_DELETE_CPSTREAMOUT]      = dlsym(m_phandle, FUNC_DELETE_CPSTREAMOUT);

            // confirm if all symbols exported from CPLIB are exist
            for(auto item : m_symbols)
            {
                if(nullptr == item.second)
                {
                    CPLIB_ASSERTMESSAGE("Symbol: %s not found in CPLIB", item.first);
                    m_referenceMutex.unlock();
                    UnloadCPLib(ctx);
                    return false;
                }
            }

            using FuncTypeGetVersion = uint32_t (*)();
            FuncTypeGetVersion func = reinterpret_cast<FuncTypeGetVersion>(m_symbols[FUNC_GET_CPLIB_MAJOR_VERSION]);

            if(REQUIRED_CPLIB_MAJOR_VERSION != func())
            {
                CPLIB_ASSERTMESSAGE("The CPLIB version does not meet the require");
                m_referenceMutex.unlock();
                UnloadCPLib(ctx);
                return false;
            }
        }

        using FuncTypeInitCplib = bool (*)(VADriverContextP ctx);
        bool initialized = false;
        InvokeCpFunc<FuncTypeInitCplib>(initialized, FUNC_INIT_CPLIB, ctx);
        if (!initialized)
        {
            CPLIB_ASSERTMESSAGE("Failed to initialized CPLIB");
            m_referenceMutex.unlock();
            UnloadCPLib(ctx);
            return false;
         }
    }

    m_referenceMutex.unlock();
    CPLIB_NORMALMESSAGE("CPLIB Loaded Successfully");
    return true;
}

void CPLibUtils::UnloadCPLib(VADriverContextP ctx)
{
    m_referenceMutex.lock();
    --m_referenceCount;

    if(m_referenceCount < 0)
    {
        CPLIB_ASSERTMESSAGE("Invalid m_referenceCount");
        m_referenceMutex.unlock();
        return;
    }
    if(m_symbols.empty())
    {
        CPLIB_ASSERTMESSAGE("m_symbols is empty");
        m_referenceMutex.unlock();
        return;
    }

    using FuncType = void (*)(VADriverContextP ctx);
    InvokeCpFunc<FuncType>(FUNC_RELEASE_CPLIB, ctx);

    if((0 == m_referenceCount) && (nullptr != m_phandle))
    {
        m_symbols.clear();
        if(0 != dlclose(m_phandle)) // dlclose will return 0 if execution sucecceed
            CPLIB_ASSERTMESSAGE("Failed to close CPLIB %s", dlerror());
    }
    m_referenceMutex.unlock();
}
