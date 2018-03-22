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

#include "cm_test.h"

using CMRT_UMD::CmQueue;
class QueueTest: public CmTest
{
public:
    QueueTest(): m_queue(nullptr) {}

    ~QueueTest() {}

    int32_t CreateTwice()
    {
        int32_t result = m_mockDevice->CreateQueue(m_queue);
        EXPECT_EQ(CM_SUCCESS, result);
        CmQueue *another_queue = nullptr;
        result = m_mockDevice->CreateQueue(another_queue);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_EQ(m_queue, another_queue);
        return CM_SUCCESS;
    }//===================

    int32_t EnqueueWithoutTask()
    {
        int32_t result = m_mockDevice->CreateQueue(m_queue);
        EXPECT_EQ(CM_SUCCESS, result);
        CMRT_UMD::CmEvent *event = nullptr;
        result = m_queue->Enqueue(nullptr, event, nullptr);
        EXPECT_EQ(CM_INVALID_ARG_VALUE, result);
        result = m_queue->DestroyEvent(event);
        EXPECT_NE(CM_SUCCESS, result);
        return CM_SUCCESS;
    }//===================

private:
    CmQueue *m_queue;
};//=================

TEST_F(QueueTest, CreateTwice)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateTwice(); });
    return;
}//========

TEST_F(QueueTest, EnqueueWithoutTask)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return EnqueueWithoutTask(); });
    return;
}//========
