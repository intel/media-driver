/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_reference_associated_buffer.h
//! \brief    Defines the interface for reference associated buffer
//! \details  Each reference has associated buffers for most of the codecs,
//!           this file implements the basic management for such buffers.
//!

#ifndef __DECODE_REFRENCE_ASSOCIATED_BUFFER_H__
#define __DECODE_REFRENCE_ASSOCIATED_BUFFER_H__

#include "decode_allocator.h"
#include "decode_utils.h"
#include "codec_hw_next.h"

namespace decode {

template<typename BufferType, typename BasicFeature>
class BufferOpInf
{
public:
    virtual ~BufferOpInf() {}
    virtual MOS_STATUS Init(void *hwInterface, DecodeAllocator &allocator, BasicFeature &basicFeature)
    {
        m_hwInterface  = hwInterface;
        m_allocator    = &allocator;
        m_basicFeature = &basicFeature;
        return MOS_STATUS_SUCCESS;
    }

    virtual BufferType *Allocate() = 0;
    virtual MOS_STATUS Resize(BufferType* &buffer) = 0;
    virtual MOS_STATUS Deactive(BufferType* &buffer) { return MOS_STATUS_SUCCESS; }
    virtual bool IsAvailable(BufferType* &buffer) { return true; }
    virtual void Destroy(BufferType* &buffer) = 0;

    void*                m_hwInterface  = nullptr;
    DecodeAllocator*     m_allocator    = nullptr;
    BasicFeature*        m_basicFeature = nullptr;

MEDIA_CLASS_DEFINE_END(decode__BufferOpInf)
};

template<typename BufferType, typename BufferOp, typename BasicFeature>
class RefrenceAssociatedBuffer
{
public:
    //!
    //! \brief  RefrenceAssociatedBuffer constructor
    //!
    RefrenceAssociatedBuffer() 
    {};

    //!
    //! \brief  RefrenceAssociatedBuffer deconstructor
    //!
    virtual ~RefrenceAssociatedBuffer()
    {
        DECODE_FUNC_CALL();

        for (auto& buf : m_activeBuffers)
        {
            m_bufferOp.Destroy(buf.second);
        }
        m_activeBuffers.clear();

        for (auto& buf : m_availableBuffers)
        {
            m_bufferOp.Destroy(buf);
        }
        m_availableBuffers.clear();
    }

    //!
    //! \brief  Initialize buffers
    //! \param  [in] hwInterface
    //!         Point to hardware interface
    //! \param  [in] allocator
    //!         Reference to decode allocator
    //! \param  [in] basicFeature
    //!         Basic feature
    //! \param  [in] initialAllocNum
    //!         The number of buffers allocated when initialize
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void* hwInterface, DecodeAllocator& allocator, BasicFeature& basicFeature,
                    uint32_t initialAllocNum)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(m_bufferOp.Init(hwInterface, allocator, basicFeature));

        DECODE_ASSERT(m_availableBuffers.empty());
        DECODE_ASSERT(m_activeBuffers.empty());

        for (uint32_t i = 0; i < initialAllocNum; i++)
        {
            BufferType *buffer = m_bufferOp.Allocate();
            DECODE_CHK_NULL(buffer);
            m_availableBuffers.push_back(buffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Update buffers for current picture
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \param  [in] fixedFrameIdx
    //!         The frameIdx user wants to keep into the refFrameList
    //!         Default value is 0xff
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList, uint32_t fixedFrameIdx = 0xff)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateRefList(curFrameIdx, refFrameList, fixedFrameIdx));
        DECODE_CHK_STATUS(ActiveCurBuffer(curFrameIdx));

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Reactive buffers for current picture
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReActiveCurBuffer(uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DeActiveCurBuffer(curFrameIdx, refFrameList));
        DECODE_CHK_STATUS(ActiveCurBuffer(curFrameIdx));

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Return buffer for current frame
    //! \return  BufferType*
    //!         Point to buffer for current frame, nullptr if fail
    //!
    BufferType* GetCurBuffer() { return m_currentBuffer; }

    //!
    //! \brief  Return valid buffer for reference error concealment
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \return  BufferType*
    //!         Point of valid buffer, nullptr if fail
    //!
    BufferType* GetValidBufferForReference(const std::vector<uint32_t> &refFrameList)
    {
        DECODE_FUNC_CALL();

        for(auto frameIdx : refFrameList)
        {
            BufferType* buffer = GetBufferByFrameIndex(frameIdx);
            if (buffer != nullptr)
            {
                return buffer;
            }
        }

        return GetCurBuffer();
    }

    //!
    //! \brief  Return buffer by frame index
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \return  BufferType*
    //!         Point of buffer, nullptr if fail
    //!
    BufferType* GetBufferByFrameIndex(uint32_t frameIndex)
    {
        DECODE_FUNC_CALL();

        auto iter = m_activeBuffers.find(frameIndex);
        if (iter != m_activeBuffers.end())
        {
            DECODE_ASSERT(iter->second != nullptr);
            return iter->second;
        }

        return nullptr;
    }

    //!
    //! \brief  Return one available buffer
    //! \return BufferType*
    //!         Point of buffer, nullptr if fail
    //!
    BufferType *GetAvailableBuffer()
    {
        DECODE_FUNC_CALL();

        BufferType *buffer = nullptr;
        for (auto &availableBuffer : m_availableBuffers)
        {
            if (m_bufferOp.IsAvailable(availableBuffer))
            {
                buffer = availableBuffer;
            }
        }

        if (buffer == nullptr)
        {
            buffer = m_bufferOp.Allocate();
            if (buffer != nullptr)
            {
                m_availableBuffers.push_back(buffer);
            }
        }

        return buffer;
    }

    //!
    //! \brief  Figure out buffer for current picture, and add it to active buffer list
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ActiveCurBuffer(uint32_t curFrameIdx)
    {
        DECODE_FUNC_CALL();

        m_currentBuffer = nullptr;

        for (auto iter = m_activeBuffers.begin(); iter != m_activeBuffers.end(); iter++)
        {
            if (curFrameIdx == iter->first)
            {
                m_currentBuffer = iter->second;
                return MOS_STATUS_SUCCESS;
            }
        }

        // The function UpdateRefList always attach the retired buffers to end of
        // available buffer list, reusing those buffers could improve the health with
        // error stream, so pick up the last element of list for current frame as possible.
        for (auto iter = m_availableBuffers.rbegin(); iter != m_availableBuffers.rend(); iter++)
        {
            if (m_bufferOp.IsAvailable(*iter))
            {
                m_currentBuffer = *iter;
                m_availableBuffers.erase((++iter).base());
                break;
            }
        }

        if (m_currentBuffer == nullptr)
        {
            m_currentBuffer = m_bufferOp.Allocate();
            DECODE_CHK_NULL(m_currentBuffer);
        }
        m_bufferOp.Resize(m_currentBuffer);

        auto ret = m_activeBuffers.insert(std::make_pair(curFrameIdx, m_currentBuffer));
        DECODE_CHK_COND(ret.second == false,
            "Failed to acitve reference associated buffer with index %d, maybe it is already existed",
            ret.first->first);

        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief  Update buffers corresponding to reference list
    //! \param  [in] curFrameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \param  [in] fixedFrameIdx
    //!         The frameIdx user wants to keep into the refFrameList
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRefList(uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList, uint32_t fixedFrameIdx)
    {
        DECODE_FUNC_CALL();

        auto iter = m_activeBuffers.begin();
        while (iter != m_activeBuffers.end())
        {
            if (iter->first == fixedFrameIdx)
            {
                ++iter;
                continue;
            }

            if (!IsReference(iter->first, curFrameIdx, refFrameList))
            {
                auto buffer = iter->second;
                iter = m_activeBuffers.erase(iter);

                m_availableBuffers.push_back(buffer);
                DECODE_CHK_STATUS(m_bufferOp.Deactive(buffer));
            }
            else
            {
                ++iter;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Deactive buffers corresponding to reference list
    //! \param  [in] curFrameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DeActiveCurBuffer(uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList)
    {
        DECODE_FUNC_CALL();

        auto iter = m_activeBuffers.begin();
        while (iter != m_activeBuffers.end())
        {
            if (iter->first == curFrameIdx)
            {
                auto buffer = iter->second;
                iter        = m_activeBuffers.erase(iter);

                m_availableBuffers.push_back(buffer);
            }
            else
            {
                ++iter;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Check if the index of buffer is persent in current reference list
    //! \param  [in] frameIndex
    //!         The frame index corresponding to buffer
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \return  bool
    //!         True if buffer is using by current frame, false if not
    //!
    bool IsReference(uint32_t frameIdx, uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList)
    {
        DECODE_FUNC_CALL();

        if (frameIdx == curFrameIdx)
        {
            return false;
        }

        for(auto iter : refFrameList)
        {
            if (frameIdx == iter)
            {
                return true;
            }
        }

        return false;
    }

    BufferOp                        m_bufferOp;                //!< Buffer operation
    std::map<uint32_t, BufferType*> m_activeBuffers;           //!< Active buffers corresponding to current reference frame list
    std::vector<BufferType*>        m_availableBuffers;        //!< Buffers in idle
    BufferType*                     m_currentBuffer = nullptr; //!< Point to buffer of current picture

MEDIA_CLASS_DEFINE_END(decode__RefrenceAssociatedBuffer)
};

}
#endif // !__DECODE_REFRENCE_ASSOCIATED_BUFFER_H__
