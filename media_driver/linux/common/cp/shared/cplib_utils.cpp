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

bool CPLibUtils::LoadCPLib(VADriverContextP ctx)
{
    m_phandle = dlopen(CPLIB_PATH, RTLD_NOW | RTLD_LOCAL);

    if(nullptr == m_phandle)
    {
        CPLIB_NORMALMESSAGE("Failed to open %s %s", CPLIB_PATH, dlerror());
        return false;
    }
    else
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

        // confirm if all symbols exported from CPLIB are exist
        for(auto item : m_symbols)
        {
            if(nullptr == item.second)
            {
                CPLIB_NORMALMESSAGE("Symbol: %s not found in CPLIB", item.first);
                UnloadCPLib(ctx);
                return false;
            }
        }

        using FuncType = uint32_t (*)();
        FuncType func = reinterpret_cast<FuncType>(m_symbols[FUNC_GET_CPLIB_MAJOR_VERSION]);

        if(REQUIRED_CPLIB_MAJOR_VERSION != func()) 
        {
            CPLIB_NORMALMESSAGE("The CPLIB version does not meet the require");
            UnloadCPLib(ctx);
            return false;
        }
    }

    using FuncType   = bool (*)(VADriverContextP ctx);
    bool initialized = false;
    InvokeCpFunc<FuncType>(initialized, FUNC_INIT_CPLIB, ctx);
    if (!initialized)
    {
        CPLIB_NORMALMESSAGE("Failed to initialized CPLIB");
        UnloadCPLib(ctx);
        return false;
    }

    CPLIB_NORMALMESSAGE("CPLIB Loaded Successfully");
    return true;
}

void CPLibUtils::UnloadCPLib(VADriverContextP ctx)
{
    using FuncType = void (*)(VADriverContextP ctx);
    InvokeCpFunc<FuncType>(FUNC_RELEASE_CPLIB, ctx);

    if(nullptr != m_phandle)
    {
        m_symbols.clear();
        if(0 != dlclose(m_phandle)) // dlclose will return 0 if execution sucecceed
            CPLIB_ASSERTMESSAGE("Failed to close CPLIB %s", dlerror());
    }
}
