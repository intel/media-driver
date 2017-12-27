/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_memdecomp_g9.h
//! \brief    Defines data structures and interfaces for Gen9 media memory decompression.
//! \details  
//!

#ifndef __CODECHAL_MEDIAMEMCOMP_G9_H__
#define __CODECHAL_MEDIAMEMCOMP_G9_H__

#include "codechal_memdecomp.h"

//!
//! \class DecompKernerlState
//! \brief Media memory compression state. This class defines the member fields
//!        functions etc used by memory compression.
//!
class MediaMemDecompStateG9:public MediaMemDecompState
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaMemDecompStateG9();

    //!
    //! \brief    Destructor
    //!
    ~MediaMemDecompStateG9(){};
protected:
    
    //!
    //! \brief    Initialize kernel state
    //! \details  Initialize kernel state
    //! \param    uint32_t kernelStateIdx
    //!           [in] Kernel state index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!  
    MOS_STATUS InitKernelState(uint32_t kernelStateIdx); 
};

#endif  //__CODECHAL_MEDIAMEMCOMP_G9_H__