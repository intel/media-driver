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
//!
//! \file     codechal_encode_tracked_buffer.h
//!
//! \brief    Encoder needs to track and store certain surface/resource/buffers, to be used as reference for future frames
//!           Given the refFrame list of current frame, a slot is dynamically allocated to index/track these resources
//!           Whenever a RefFrame is no longer used, its corresponding slot will be re-used to minimize memory allocation
//!           "Tracked buffer" so far includes MbCode/MvData/MvTemporal buffer, and Csc/Ds/DsRecon surface
//!

#ifndef __CODECHAL_ENCODE_TRACKED_BUFFER_H__
#define __CODECHAL_ENCODE_TRACKED_BUFFER_H__

#include "codechal.h"
#include "codechal_encode_allocator.h"
#include "codec_def_common_encode.h"

//!
//! Tracked buffer 
//!
class CodechalEncodeTrackedBuffer
{
public:
    //!
    //! \brief  Get the buffer index allocated for current frame
    //!
    //! \return the buffer index allocated for current frame
    //!
    inline uint8_t GetCurrIndex()
    {
        return m_trackedBufCurrIdx;
    }

    //!
    //! \brief  Get the buffer index allocated for MbCode
    //!
    //! \return the buffer index allocated for MbCode
    //!
    inline uint8_t GetCurrIndexMbCode()
    {
        return m_mbCodeCurrIdx;
    }

    //!
    //! \brief  Get the current MbCode buffer
    //!
    //! \return the current MbCode buffer
    //!
    inline MOS_RESOURCE* GetCurrMbCodeBuffer()
    {
        return m_trackedBufCurrMbCode;
    }

    //!
    //! \brief  Get the current MvData buffer
    //!
    //! \return the current MvData buffer
    //!
    inline MOS_RESOURCE* GetCurrMvDataBuffer()
    {
        return m_trackedBufCurrMvData;
    }

    //!
    //! \brief  Get the current MV temporal buffer
    //!
    //! \return the current MV temporal buffer
    //!
    MOS_RESOURCE* GetMvTemporalBuffer(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrMvTemporal;
        }
        else
        {
            return  (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mvTemporalBuffer, bufIndex);
        }
    }

    //!
    //! \brief  Get the current CSC surface
    //!
    //! \return the current CSC surface
    //!
    MOS_SURFACE* GetCscSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrCsc;
        }
        else
        {
            return (MOS_SURFACE*)m_allocator->GetResource(m_standard, cscSurface, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 4x downscaled surface
    //!
    //! \return the current 4x downscaled surface
    //!
    MOS_SURFACE* Get4xDsSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrDs4x;
        }
        else
        {
            return  (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xSurface, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 2x downscaled surface
    //!
    //! \return the current 2x downscaled surface
    //!
    MOS_SURFACE* Get2xDsSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrDs2x;
        }
        else
        {
            return  (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds2xSurface, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 16x downscaled surface
    //!
    //! \return the current 16x downscaled surface
    //!
    MOS_SURFACE* Get16xDsSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrDs16x;
        }
        else
        {
            return  (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds16xSurface, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 32x downscaled surface
    //!
    //! \return the current 32x downscaled surface
    //!
    MOS_SURFACE* Get32xDsSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurrDs32x;
        }
        else
        {
            return  (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds32xSurface, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 4x DsRecon surface
    //!
    //! \return the current 4x DsRecon buffer
    //!
    inline MOS_SURFACE* Get4xDsReconSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurr4xDsRecon;
        }
        else
        {
            return (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds4xRecon, bufIndex);
        }
    }

    //!
    //! \brief  Get the current 8x DsRecon surface
    //!
    //! \return the current 8x DsRecon buffer
    //!
    inline MOS_SURFACE* Get8xDsReconSurface(uint8_t bufIndex)
    {
        if (bufIndex == CODEC_CURR_TRACKED_BUFFER)
        {
            return m_trackedBufCurr8xDsRecon;
        }
        else
        {
            return (MOS_SURFACE*)m_allocator->GetResource(m_standard, ds8xRecon, bufIndex);
        }
    }

    //!
    //! \brief  Get the wait flag for tracked buffer
    //!
    inline bool GetWait()
    {
        return m_waitTrackedBuffer;
    }

    //!
    //! \brief  Get the wait flag for CSC surface
    //!
    inline bool GetWaitCsc()
    {
        return m_waitCscSurface;
    }

    //!
    //! \brief  Get the allocation flag
    //!
    inline bool IsMbCodeAllocationNeeded()
    {
        return m_allocateMbCode;
    }

    //!
    //! \brief  Set allocation flag
    //!
    inline void SetAllocationFlag(bool flag)
    {
        m_allocateMbCode = flag;
    }

    //!
    //! \brief  Allocate resource for current frame
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateForCurrFrame();

    //!
    //! \brief  Resize the Tracked Buffer upon resolution change
    //!
    //! \return void
    //!
    void Resize();

    //!
    //! \brief  Release the existing CSC surfaces upon resolution change
    //!
    //! \return void
    //!
    void ResizeCsc();

    //!
    //! \brief    Allocate for Preenc, so far only set the buffer index for DS surface
    //! \param    [in] bufIndex
    //!           buffer index for DS surface
    //!
    //! \return   void
    //!
    void AllocateForCurrFramePreenc(uint8_t bufIndex);

    //!
    //! \brief  Reset used for current frame flag
    //!
    void ResetUsedForCurrFrame();

    //!
    //! \brief    Encoder pre enc look up buffer index
    //! \param    [in] frameIdx
    //!           frame Index
    //! \param    [in] inCache
    //!           Indicate if it's in cache
    //! \return   uint8_t
    //!           emptyEntry
    //!
    uint8_t PreencLookUpBufIndex(
        uint8_t         frameIdx,
        bool            *inCache);

    //!
    //! \brief  Allocate Mb Code Resources
    //!
    //! \param  [in] bufIndex
    //!         buffer index used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMbCodeResources(uint8_t bufIndex);

    //!
    //! \brief  Allocate Mv Data Resources
    //!
    //! \param  [in] bufIndex
    //!         buffer index used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMvDataResources(uint8_t bufIndex);

    //!
    //! \brief    Allocate CSC surface or pick an existing one from the pool
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSurfaceCsc();

    MOS_STATUS ResizeSurfaceDS();

    MOS_STATUS ResizeDsReconSurfacesVdenc();
    //!
    //! \brief    Allocate DS surface or pick an existing one from the pool
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSurfaceDS();

    //!
    //! \brief    Allocate 2xDS surface or pick an existing one from the pool
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSurface2xDS();

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeTrackedBuffer(CodechalEncoderState* encoder);

    virtual ~CodechalEncodeTrackedBuffer();

protected:
    //!
    //! \brief  Get slot for current frame
    //!
    //! \param  [in] refList
    //!         Reference frame list
    //! \param  [in] numRefFrame
    //!         Number of reference frame
    //! \param  [in] usedAsRef
    //!         Current frame is used as reference or not
    //!
    //! \return uint8_t
    //!         Index found
    //!
    uint8_t LookUpBufIndex(
        PCODEC_PICTURE refList,
        uint8_t        numRefFrame,
        bool           usedAsRef);

    //!
    //! \brief  Get slot for current frame's CSC surface
    //!
    //! \return uint8_t
    //!         Index found
    //!
    uint8_t LookUpBufIndexCsc();

    //!
    //! \brief  Release MbCode buffer
    //!
    //! \param  [in] bufIndex
    //!         buffer index to be released
    //!
    //! \return void
    //!
    void ReleaseMbCode(uint8_t bufIndex);

    //!
    //! \brief  Release MvData buffer
    //!
    //! \param  [in] bufIndex
    //!         buffer index to be released
    //!
    //! \return void
    //!
    void ReleaseMvData(uint8_t bufIndex);

    //!
    //! \brief    Release CSC surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void ReleaseSurfaceCsc(uint8_t index);

    //!
    //! \brief  Release DS surface
    //!
    //! \param  [in] index
    //!         buffer index to be released
    //!
    //! \return void
    //!
    void ReleaseSurfaceDS(uint8_t index);

    //!
    //! \brief  Release DsRecon buffer
    //!
    //! \param  [in] bufIndex
    //!         buffer index to be released
    //!
    //! \return void
    //!
    void ReleaseDsRecon(uint8_t bufIndex);

    //!
    //! \brief  Defer-deallocate buffer used before resolution reset
    //!
    virtual void DeferredDeallocateOnResChange();

    CodechalEncoderState*           m_encoder = nullptr;                        //!< Pointer to ENCODER base class
    CodechalEncodeAllocator*        m_allocator = nullptr;                      //!< Pointer to resource allocator

    MOS_RESOURCE*                   m_trackedBufCurrMbCode = nullptr;           //!< Pointer to current MbCode buffer
    MOS_RESOURCE*                   m_trackedBufCurrMvData = nullptr;           //!< Pointer to current MvData buffer
    MOS_RESOURCE*                   m_trackedBufCurrMvTemporal = nullptr;       //!< Pointer to current MV temporal buffer
    MOS_SURFACE*                    m_trackedBufCurrCsc = nullptr;              //!< Pointer to current CSC surface
    MOS_SURFACE*                    m_trackedBufCurrDs4x = nullptr;             //!< Pointer to current 4x downscaled surface
    MOS_SURFACE*                    m_trackedBufCurrDs2x = nullptr;             //!< Pointer to current 2x downscaled surface
    MOS_SURFACE*                    m_trackedBufCurrDs16x = nullptr;            //!< Pointer to current 16x downscaled surface
    MOS_SURFACE*                    m_trackedBufCurrDs32x = nullptr;            //!< Pointer to current 32x downscaled surface
    MOS_SURFACE*                    m_trackedBufCurr4xDsRecon = nullptr;        //!< Pointer to current 4x DsRecon buffer
    MOS_SURFACE*                    m_trackedBufCurr8xDsRecon = nullptr;        //!< Pointer to current 8x DsRecon buffer

    uint32_t                        m_standard;                                 //!< The encode state's standard
    uint8_t                         m_trackedBufCurrIdx = 0;                    //!< current tracked buffer index
    uint8_t                         m_mbCodeCurrIdx = 0;                        //!< current MbCode buffer index
    bool                            m_allocateMbCode = false;                   //!< need to allocate MbCode buffer for current frame
    bool                            m_mbCodeIsTracked = true;                   //!< tracked buffer algorithm used to manage MbCode buffer

private:
    CodechalEncodeTrackedBuffer(const CodechalEncodeTrackedBuffer&) = delete;
    CodechalEncodeTrackedBuffer& operator=(const CodechalEncodeTrackedBuffer&) = delete;

    virtual void LookUpBufIndexMbCode()
    {
        m_mbCodeCurrIdx = m_trackedBufCurrIdx;
    }

    //!
    //! \brief  Allocate Mv temporal buffer
    //!
    //! \param  [in] bufIndex
    //!         buffer index used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateMvTemporalBuffer() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief  Allocate Ds Recon Surfaces Vdenc
    //!
    //! \param  [in] bufIndex
    //!         buffer index used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateDsReconSurfacesVdenc();

    MOS_INTERFACE*          m_osInterface = nullptr;                            //!< OS interface

    uint8_t                 m_trackedBufNonRefIdx = 0;                          //!< current tracked buffer index when frame won't be used as ref
    uint8_t                 m_trackedBufCountNonRef = 0;                        //!< counting number of tracked buffer when ring buffer is used
    uint8_t                 m_trackedBufCountResize = 0;                        //!< 3 buffers to be delay-destructed during res change
    uint8_t                 m_trackedBufPenuIdx = 0;                            //!< 2nd-to-last tracked buffer index
    uint8_t                 m_trackedBufAnteIdx = 0;                            //!< 3rd-to-last tracked buffer index
    bool                    m_waitTrackedBuffer = false;                        //!< wait to re-use tracked buffer

    uint8_t                 m_cscBufNonRefIdx = 0;                              //!< current CSC buffer index when ring buffer is used
    uint8_t                 m_cscBufCountNonRef = 0;                            //!< counting number of CSC surface when ring buffer is used
    uint8_t                 m_cscBufCurrIdx = 0;                                //!< curr copy buffer index
    uint8_t                 m_cscBufPenuIdx = 0;                                //!< 2nd-to-last CSC buffer index
    uint8_t                 m_cscBufAnteIdx = 0;                                //!< 3rd-to-last CSC buffer index
    bool                    m_waitCscSurface = false;                           //!< wait to re-use CSC surface

    struct tracker
    {
        uint8_t             ucSurfIndex7bits;                                   //!< 0xFF means the entry can be re-used
        bool                bUsedforCurFrame;                                   //!< Used for FEI Preenc to mark whether this enty can be reused in multi-call case
    };
    tracker                 m_tracker[CODEC_NUM_TRACKED_BUFFERS];
};

#endif  // __CODECHAL_ENCODE_TRACKED_BUFFER_H__