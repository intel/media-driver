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

#ifndef __CM_NOTIFIER_H__
#define __CM_NOTIFIER_H__

#include <vector>
#include <stdio.h>
#include "mos_utilities.h"

struct CM_KERNEL_INFO;

namespace CMRT_UMD
{
class CmDevice;
class CmKernel;
class CmTaskInternal;

const uint32_t NOTIFIER_NULL_ID = 0;

class CmNotifier
{
public:
    CmNotifier() {};
    virtual ~CmNotifier() {};
    virtual bool Valid() {return false;}
    virtual unsigned int ID() {return NOTIFIER_NULL_ID;}
    virtual int NotifyDeviceCreated(CmDevice *device)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }
    virtual int NotifyDeviceDestroyed(CmDevice *device)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }

    virtual int NotifyKernelCreated(CmKernel *kernel)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }

    virtual int NotifyTaskFlushed(CmDevice* device, CmTaskInternal *task)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }

    virtual int NotifyTaskCompleted(CmTaskInternal *task)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }

    virtual int NotifyCallingJitter(void **extraInfo)
    {
        // if the derived class doesn't implement this, just return 0
        return 0;
    }
};

class CmNotifierGroup
{
public:
    typedef CmNotifier* (*Creator)();
    typedef std::vector<Creator>  Creators;
    typedef typename Creators::iterator Iterator;
 
    CmNotifierGroup() : m_ids(0)
    {
        Creators &creators = GetCreators();
        for (Iterator iter = creators.begin(); iter != creators.end(); iter ++)
        {
            CmNotifier *notifier = (*iter)();
            if (notifier != nullptr && notifier->Valid() && !IsAdded(notifier->ID()))
            {
                m_notifiers.push_back(notifier);
                m_ids = m_ids | (1 << notifier->ID());
            }
            else if (notifier != nullptr)
            {
                MOS_Delete(notifier);
            }
        }
    }
    
    ~CmNotifierGroup()
    {
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            MOS_Delete(m_notifiers[i]);
        }
    }
    
    int NotifyDeviceCreated(CmDevice *device)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyDeviceCreated(device) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }

    int NotifyDeviceDestroyed(CmDevice *device)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyDeviceDestroyed(device) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }

    int NotifyKernelCreated(CmKernel *kernel)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyKernelCreated(kernel) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }

    int NotifyTaskFlushed(CmDevice* device, CmTaskInternal *task)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyTaskFlushed(device, task) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }

    int NotifyTaskCompleted(CmTaskInternal *task)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyTaskCompleted(task) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }

    int NotifyCallingJitter(void **extraInfo)
    {
        int ret = 0;
        for (unsigned int i = 0; i < m_notifiers.size(); i++)
        {
            if (m_notifiers[i]->NotifyCallingJitter(extraInfo) != 0)
            {
                ret = -1;
            }
        }
        return ret;
    }
    
    template <class T>
    static bool RegisterNotifier()
    {
        Creators &creators = GetCreators();
        creators.push_back(Create<T>);
        return true;
    }
    
protected:
    std::vector<CmNotifier *> m_notifiers;
    uint32_t m_ids;
    
    static Creators& GetCreators()
    {
        static Creators creators;
        return creators;
    }
    
    template <class T>
    static CmNotifier* Create()
    {
        return MOS_New(T);
    }

    bool IsAdded(uint32_t id)
    {
        return m_ids & (1<<id);
    }
  
};
};
#endif // __CM_NOTIFIER_H__
