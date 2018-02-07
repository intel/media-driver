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

#ifndef _CM_EXTENSION_CREATOR_H_
#define _CM_EXTENSION_CREATOR_H_

#include "mos_utilities.h"

static const int priorityNum = 4;

template <class T>
class CmExtensionCreator
{
    typedef T* (*Creator)();

public:
    template <class C>
    static bool RegisterClass(int priority = 0)
    {
        if (priority >= priorityNum)
        {
            return false;
        }
        Creator *c = GetCreators();
        c[priority] = Create<C>;
        return true;
    }
    
    static T* CreateClass()
    {
        Creator *c = GetCreators();
        for (int i = priorityNum - 1; i >= 0; i--)
        {
            if (c[i] != nullptr)
            {
                return c[i]();
            }
        }
        return nullptr;
    }
    
private:
    static Creator* GetCreators()
    {
        static Creator m_creators[priorityNum];
        return m_creators;
    }
    
    template <class C>
    static T* Create()
    {
        return MOS_New(C);
    }
};
#endif
