/*
// Copyright (C) 2020 Intel Corporation
//
// Licensed under the Apache License,Version 2.0(the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
//!
//! \file     codechal_debug_xe_xpm_plus_ext.h
//! \brief    For internal definitions.
//! \details  Contains definitions that are used only for internal branches
//!
#ifndef __CODECHAL_DEBUG_XE_XPM_PLUS_EXT_H__
#define __CODECHAL_DEBUG_XE_XPM_PLUS_EXT_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include "codechal_debug.h"

#if USE_CODECHAL_DEBUG_TOOL

class CodechalDebugInterfaceXe_Xpm_Plus : public CodechalDebugInterfaceG12
{
public:
    CodechalDebugInterfaceXe_Xpm_Plus();
    ~CodechalDebugInterfaceXe_Xpm_Plus();

    MOS_STATUS DumpBltOutput(
        PMOS_SURFACE              surface,
        const char *              attrName) override;
};

#endif  // USE_CODECHAL_DEBUG_TOOL
#endif  // (_DEBUG || _RELEASE_INTERNAL)
#endif /* __CODECHAL_DEBUG_XE_XPM_PLUS_EXT_H__ */
