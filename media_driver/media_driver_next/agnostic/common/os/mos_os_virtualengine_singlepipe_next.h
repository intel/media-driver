/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_os_virtualengine_singlepipe_next.h
//! \brief    defines the MOS interface extension cross OS,  supporting virtual engine single pipe mode.
//! \details  defines all types, macros, and functions required by the MOS interface extension cross OS,  supporting virtual engine single pipe mode.
//!

#ifndef __MOS_OS_VIRTUALENGINE_SINGLEPIPE_NEXT_H__
#define __MOS_OS_VIRTUALENGINE_SINGLEPIPE_NEXT_H__

#include "mos_os_virtualengine_next.h"
class MosOsVeSinglePipe : public MosVeInterface
{
public:
    //!
    //! \brief    Construct
    //!
    MosOsVeSinglePipe() {}

    //!
    //! \brief    Deconstruct
    //!
    ~MosOsVeSinglePipe() {}

    //!
    //! \brief    Destroy resources allocated for virtual engine single pipe
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void Destroy();

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    populate debug override parameters for single pipe virtual engine
    //! \details  populate debug override parameters used in single pipe virtual engine for debug override or Linux target submission
    //! \param    [in]  stream
    //!           MOS stream state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PopulateDbgOvrdParams(
        MOS_STREAM_HANDLE stream);
#endif
};
#endif //__MOS_OS_VIRTUALENGINE_SINGLEPIPE_NEXT_H__

