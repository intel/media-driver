/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_internal_target.h
//! \brief    Defines the interface for internal dest surface alloacted by driver
//!

#ifndef __DECODE_INTERNAL_TARGET_H__
#define __DECODE_INTERNAL_TARGET_H__

#include "decode_allocator.h"
#include "decode_utils.h"
#include "decode_basic_feature.h"

namespace decode {

class InternalTargets
{
public:
    //!
    //! \brief  InternalTargets constructor
    //!
    InternalTargets() {};

    //!
    //! \brief  InternalTargets deconstructor
    //!
    virtual ~InternalTargets()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            for (auto &surface : m_activeSurfaces)
            {
                m_allocator->Destroy(surface.second);
            }
            m_activeSurfaces.clear();

            for (auto &surface : m_aviableSurfaces)
            {
                m_allocator->Destroy(surface);
            }
        }
        m_aviableSurfaces.clear();
    }

    //!
    //! \brief  Initialize surfaces
    //! \param  [in] allocator
    //!         Reference to decode allocator
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();

        m_allocator    = &allocator;
        DECODE_ASSERT(m_aviableSurfaces.empty());
        DECODE_ASSERT(m_activeSurfaces.empty());

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Figure out surface for current picture, and add it to active surface list
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \param  [in] dstSurface
    //!         The requirement for the current surface
    //! \param  [in] isMmcEnabled
    //!         The MMC flag for currrent surface
    //! \param  [in] resUsageType
    //!         The resource usage for currrent surface
    //! \param  [in] accessReq
    //!         The access requirement for current surface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ActiveCurSurf(uint32_t curFrameIdx, PMOS_SURFACE dstSurface, bool isMmcEnabled,
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem)
    {
        DECODE_FUNC_CALL();

        for (auto iter = m_activeSurfaces.begin(); iter!= m_activeSurfaces.end(); iter++)
        {
            if (curFrameIdx == iter->first)
            {
                return MOS_STATUS_SUCCESS;
            }
        }

        if (m_aviableSurfaces.size() == 0)
        {
            m_currentSurface = m_allocator->AllocateSurface(
                                            dstSurface->dwWidth,
                                            MOS_ALIGN_CEIL(dstSurface->dwHeight, 8),
                                            "Internal target surface",
                                            dstSurface->Format,
                                            isMmcEnabled,
                                            resUsageType,
                                            accessReq,
                                            dstSurface->TileModeGMM);
        }
        else
        {
            auto iter = m_aviableSurfaces.begin();
            m_currentSurface = *iter;
            m_aviableSurfaces.erase(iter);
            DECODE_CHK_STATUS(m_allocator->Resize(m_currentSurface,
                                dstSurface->dwWidth,
                                MOS_ALIGN_CEIL(dstSurface->dwHeight, 8),
                                accessReq,
                                false,
                                "Internal target surface"));
        }

        DECODE_CHK_NULL(m_currentSurface);

        auto ret = m_activeSurfaces.insert(std::make_pair(curFrameIdx, m_currentSurface));
        DECODE_CHK_COND(ret.second == false,
            "Failed to active surface with index %d, maybe it is already existed",
            ret.first->first);

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Update internal surfaces corresponding to reference list
    //! \param  [in] curFrameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \param  [in] fixedFrameIdx
    //!         The frameIdx user wants to keep into the refFrameList
    //!         Default value is 0xff
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRefList(uint32_t curFrameIdx, const std::vector<uint32_t> &refFrameList, uint32_t fixedFrameIdx = 0xff)
    {
        DECODE_FUNC_CALL();

        auto iter = m_activeSurfaces.begin();
        while (iter != m_activeSurfaces.end())
        {
            if (iter->first == fixedFrameIdx)
            {
                ++iter;
                continue;
            }

            if (!IsReference(iter->first, curFrameIdx, refFrameList))
            {
                auto buffer = iter->second;
                iter = m_activeSurfaces.erase(iter);

                m_aviableSurfaces.push_back(buffer);
            }
            else
            {
                ++iter;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Return current internal surface for current frame
    //! \return PMOS_SURFACE*
    //!         Point to surface for current frame, nullptr if fail
    //!
    PMOS_SURFACE GetCurSurf() { return m_currentSurface; }

protected:
    //!
    //! \brief  Check if the index of surface is persent in current reference list
    //! \param  [in] frameIndex
    //!         The frame index corresponding to surface
    //! \param  [in] frameIdx
    //!         The frame index for current picture
    //! \param  [in] refFrameList
    //!         The frame indicies of reference frame list
    //! \return bool
    //!         True if surface is using by current frame, false if not
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

private:
    std::map<uint32_t, PMOS_SURFACE> m_activeSurfaces;           //!< Active surfaces corresponding to current reference frame list
    std::vector<PMOS_SURFACE>        m_aviableSurfaces;          //!< Surfaces in idle
    PMOS_SURFACE                     m_currentSurface = nullptr; //!< Point to surface of current picture
    DecodeAllocator*                 m_allocator = nullptr;

MEDIA_CLASS_DEFINE_END(decode__InternalTargets)
};

}  // namespace decode
#endif // !__DECODE_INTERNAL_TARGET_H__
