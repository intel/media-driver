/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_scalability_defs.h
//! \brief    Defines the structures for decode scalability

#ifndef __DECODE_SCALABILITY_DEFS_H__
#define __DECODE_SCALABILITY_DEFS_H__
#include "mos_os.h"
#include "mhw_vdbox.h"
#include "media_scalability_defs.h"

namespace decode
{
enum ScalabilityMode
{
    scalabilitySingleMode,       //!< Legacy decode mode with single pipe
    scalabilityVirtualTileMode,  //!< virtual tile decode mode with multiple pipes
    scalabilityRealTileMode,     //!< Real tile decode mode with multiple pipes
};

struct DecodeScalabilityPars : public ScalabilityPars
{
    bool    disableScalability = false;
    bool    disableRealTile = false;
    bool    disableVirtualTile = false;

    bool    usingSfc = false;
    bool    usingHcp = false;
    bool    usingSlimVdbox = false;

    MOS_FORMAT surfaceFormat = Format_NV12;

    uint8_t maxTileColumn = 0;
    uint8_t maxTileRow = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t modeSwithThreshold1 = 0;
    uint32_t modeSwithThreshold2 = 0;
    uint8_t  userPipeNum = 0;
#endif
};
}
#endif  // !__DECODE_SCALABILITY_DEFS_H__
