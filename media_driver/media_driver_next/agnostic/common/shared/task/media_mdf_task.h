/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_mdf_task.h
//! \brief    Defines the interface for media mdf task
//! \details  The media mdf task is dedicated for mdf kernel submission
//!
#ifndef __MEDIA_MDF_TASK_H__
#define __MEDIA_MDF_TASK_H__
#include "media_task.h"
#include "mos_os.h"
#include "codechal_debug.h"
#include "cm_rt_umd.h"

class MdfTask : public MediaTask
{
public:
    //!
    //! \brief  MdfTask constructor
    //! \param  [in] device
    //!         Pointer to CmDevice
    //!
    MdfTask(CmDevice *device);

    virtual ~MdfTask() { }

    virtual MOS_STATUS Submit(bool immediateSubmit, MediaScalability *scalability, CodechalDebugInterface *debugInterface) override;

protected:
    CmDevice *m_CmDevice = nullptr;        //!< PMOS_INTERFACE
    CmTask *m_cmTask = nullptr;

};



#endif // !__MEDIA_MDF_TASK_H__
