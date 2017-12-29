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
    inline MOS_RESOURCE* GetCurrMvTemporalBuffer()
    {
        return m_trackedBufCurrMvTemporal;
    }

    //!
    //! \brief  Get the current 4x DsRecon surface
    //!
    //! \return the current 4x DsRecon buffer
    //!
    inline MOS_SURFACE* GetCurr4xDsReconSurface()
    {
        return m_trackedBufCurr4xDsRecon;
    }

    //!
    //! \brief  Get the current 8x DsRecon surface
    //!
    //! \return the current 8x DsRecon buffer
    //!
    inline MOS_SURFACE* GetCurr8xDsReconSurface()
    {
        return m_trackedBufCurr8xDsRecon;
    }

    //!
    //! \brief  Get the wait flag
    //!
    inline bool GetWait()
    {
        return m_waitForTrackedBuffer;
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
    //! \brief  Reset used for current frame flag
    //!
    void ResetUsedForCurrFrame();

    //!
    //! \brief    Encoder pre enc look up buffer index
    //! \param    [in] frameIdx
    //!           ucFrame Index
    //!           [in] inCache
    //!           Indicate if it's in cache
    //! \return   uint8_t
    //!           uiEmptyEntry
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
    //! \brief  Allocate Ds Recon Surfaces Vdenc
    //!
    //! \param  [in] bufIndex
    //!         buffer index used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateDsReconSurfacesVdenc(uint8_t bufIndex);

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
    //! \brief  Release DsRecon buffer
    //!
    //! \param  [in] bufIndex
    //!         buffer index to be released
    //!
    //! \return void
    //!
    void ReleaseDsRecon(uint8_t bufIndex);

    CodechalEncoderState*           m_encoder = nullptr;                        //!< Pointer to ENCODER base class
    CodechalEncodeAllocator*        m_allocator = nullptr;                      //!< Pointer to resource allocator
    CODEC_TRACKED_BUFFER*           m_trackedBuffer;                            //!< Pointer to tracked buffer struct

    MOS_RESOURCE*                   m_trackedBufCurrMbCode = nullptr;           //!< Pointer to current MbCode buffer
    MOS_RESOURCE*                   m_trackedBufCurrMvData = nullptr;           //!< Pointer to current MvData buffer
    MOS_RESOURCE*                   m_trackedBufCurrMvTemporal = nullptr;       //!< Pointer to current MV temporal buffer
    MOS_SURFACE*                    m_trackedBufCurr4xDsRecon = nullptr;        //!< Pointer to current 4x DsRecon buffer
    MOS_SURFACE*                    m_trackedBufCurr8xDsRecon = nullptr;        //!< Pointer to current 8x DsRecon buffer

    uint32_t                        m_standard;                                 //!< The encode state's standard
    uint8_t                         m_trackedBufCurrIdx = 0;                    //!< current tracked buffer index
    uint8_t                         m_trackedBufPenuIdx = 0;                    //!< 2nd-to-last tracked buffer index
    uint8_t                         m_trackedBufAnteIdx = 0;                    //!< 3rd-to-last tracked buffer index
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
    virtual MOS_STATUS AllocateMvTemporalBuffer(uint8_t bufIndex) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief  Release buffer used before resolution reset
    //!
    virtual void ReleaseBufferOnResChange();

    MOS_INTERFACE*                  m_osInterface = nullptr;                    //!< OS interface

    uint8_t                         m_trackedBufNonRefIdx = 0;                  //!< current tracked buffer index when frame won't be used as ref
    uint8_t                         m_trackedBufCountNonRef = 0;                //!< counting number of tracked buffer when ring buffer is used
    uint8_t                         m_trackedBufCountResize = 0;                //!< 3 buffers to be delay-destructed during res change
    bool                            m_waitForTrackedBuffer = false;             //!< wait to re-use tracked buffer
};

#endif  // __CODECHAL_ENCODE_TRACKED_BUFFER_H__
