/*
* Copyright (c) 2018-2021, Intel Corporation
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
#ifndef __REF_CAPS_TABLE_SKL_H__
#define __REF_CAPS_TABLE_SKL_H__

#include "driver_loader.h"

std::vector<FeatureID> refFeatureIDTable_SKL = {
    { VAProfileH264Main               , VAEntrypointVLD                 },
    { VAProfileH264Main               , VAEntrypointEncSlice            },
    { VAProfileH264Main               , VAEntrypointFEI                 },
    { VAProfileH264Main               , VAEntrypointEncSliceLP          },
    { VAProfileH264High               , VAEntrypointVLD                 },
    { VAProfileH264High               , VAEntrypointEncSlice            },
    { VAProfileH264High               , VAEntrypointFEI                 },
    { VAProfileH264High               , VAEntrypointEncSliceLP          },
    { VAProfileH264ConstrainedBaseline, VAEntrypointVLD                 },
    { VAProfileH264ConstrainedBaseline, VAEntrypointEncSlice            },
    { VAProfileH264ConstrainedBaseline, VAEntrypointFEI                 },
    { VAProfileH264ConstrainedBaseline, VAEntrypointEncSliceLP          },
    { VAProfileMPEG2Simple            , VAEntrypointVLD                 },
    { VAProfileMPEG2Simple            , VAEntrypointEncSlice            },
    { VAProfileMPEG2Main              , VAEntrypointVLD                 },
    { VAProfileMPEG2Main              , VAEntrypointEncSlice            },
    { VAProfileVC1Advanced            , VAEntrypointVLD                 },
    { VAProfileVC1Main                , VAEntrypointVLD                 },
    { VAProfileVC1Simple              , VAEntrypointVLD                 },
    { VAProfileJPEGBaseline           , VAEntrypointVLD                 },
    { VAProfileJPEGBaseline           , VAEntrypointEncPicture          },
    { VAProfileVP8Version0_3          , VAEntrypointVLD                 },
    { VAProfileVP8Version0_3          , VAEntrypointEncSlice            },
    { VAProfileHEVCMain               , VAEntrypointVLD                 },
    { VAProfileHEVCMain               , VAEntrypointEncSlice            },
    { VAProfileHEVCMain               , VAEntrypointFEI                 },
    { VAProfileNone                   , VAEntrypointVideoProc           },
    { VAProfileNone                   , VAEntrypointStats               },
#if defined(_CP_INCLUDED) && VA_CHECK_VERSION(1,11,0)
    { VAProfileProtected              , VAEntrypointProtectedTEEComm    },
    { VAProfileProtected              , VAEntrypointProtectedContent    },
#endif
};

#endif // __REF_CAPS_TABLE_SKL_H__
