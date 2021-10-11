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
//! \file     caps_register_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPS_REGISTER_SPECIFIC_H__
#define __CAPS_REGISTER_SPECIFIC_H__

#include "capstable_data_m12_0_r0_specific.h"

template<> typename MediaCapsTable<CapsData>::OsCapsTable MediaCapsTable<CapsData>::m_pltCaps = {};

static bool capsTableM12_0Registered = MediaCapsTable<CapsData>::RegisterCaps(plt_M12_0_r0, capsDataM12_0_r0);

#endif //__CAPS_REGISTER_SPECIFIC_H__