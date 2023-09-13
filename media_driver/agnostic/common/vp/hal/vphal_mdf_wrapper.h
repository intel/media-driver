/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     vphal_mdf_wrapper.h
//! \brief    Abstraction for MDF related operations.
//! \details  It is a thin wrapper layer based on MDF APIs.
//!
#ifndef __VPHAL_MDF_WRAPPER_H__
#define __VPHAL_MDF_WRAPPER_H__

#include <fstream>
#include <vector>
#include <type_traits>
#include <string>
#include <unordered_map>
#include "cm_rt_umd.h"
#include "vphal.h"

template <typename CmSurfType>
class VpCmSurfaceHolder;
class CmContext;

class EventListener
{
public:
    virtual void OnEventAvailable(CmEvent *event, const std::string &name) = 0;
};

class EventManager : public EventListener
{
public:
    EventManager(const std::string &owner, CmContext *cmContext) :
        mOwner(owner),
        m_cmContext(cmContext)
    {
    }
    virtual ~EventManager()
    {
        Clear();
    }

    void OnEventAvailable(CmEvent *event, const std::string &name) override;
    CmEvent* GetLastEvent() const;

private:
    void AddEvent(const std::string &name, CmEvent *event);
    void Clear();
    void Profiling() const;

    std::unordered_map<std::string, std::vector<CmEvent *> > mEventMap;
    const std::string     mOwner;
    int                   mEventCount = 0;
    CmEvent              *mLastEvent = nullptr;
    bool                  mReport = false;
    CmContext            *m_cmContext = nullptr;
};

// This is not multi-threads safe. Is it a good idea to use singleton here?
class CmContext
{
public:
    // noncopyable
    CmContext(const CmContext&) = delete;
    CmContext& operator=(const CmContext&) = delete;
    CmContext(PMOS_INTERFACE osInterface);
    virtual ~CmContext();

    void Destroy();

    CmDevice* GetCmDevice() const
    {
        return mCmDevice;
    }

    CmQueue* GetCmQueue() const
    {
        return mCmQueue;
    }

    CmVebox* GetCmVebox() const
    {
        return mCmVebox;
    }

    void ConnectEventListener(EventListener *listener)
    {
        mEventListener = listener;
    }

    CmKernel* CloneKernel(CmKernel *kernel);
    void BatchKernel(CmKernel *kernel, CmThreadSpace *threadSpace, bool bFence);
    void FlushBatchTask(bool waitForFinish);
    void RunSingleKernel(
        CmKernel *kernel,
        CmThreadSpace *threadSpace,
        const std::string &name,
        bool waitForFinish);

    void BeginConditionalExecute(VpCmSurfaceHolder<CmBuffer>  *conditionalBatchBuffer)
    {
        FlushBatchTask(false);
        mConditionalBatchBuffer = conditionalBatchBuffer;
    }

    void EndConditionalExecute()
    {
        FlushBatchTask(false);
        mConditionalBatchBuffer = nullptr;
    }

private:

    void EnqueueTask(CmTask *task, CmThreadSpace *threadSpace, const std::string &name, bool waitForFinish);

    int mRefCount;

    CmDevice  *mCmDevice;
    CmQueue   *mCmQueue;
    CmVebox   *mCmVebox;

    PMOS_INTERFACE                m_osInterface = nullptr;
    CmTask                       *mBatchTask;
    std::vector<CmKernel *>       mAddedKernels;
    std::vector<CmKernel *>       mKernelsToPurge;
    std::vector<CmThreadSpace *>  mThreadSpacesToPurge;
    bool                          mHasBatchedTask;
    VpCmSurfaceHolder<CmBuffer>  *mConditionalBatchBuffer; // CmContext does NOT own this.
    CM_CONDITIONAL_END_PARAM      mCondParam;
    EventListener  *mEventListener; // CmContext does NOT own this.
};

static inline
GMM_RESOURCE_FORMAT ConvertMosFmtToGmmFmt(MOS_FORMAT format)
{
    switch (format)
    {
        case Format_X8R8G8B8:      return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Format_A8R8G8B8:      return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Format_NV12:          return GMM_FORMAT_NV12_TYPE;
        case Format_A8:            return GMM_FORMAT_A8_UNORM_TYPE;
        case Format_YUY2:          return GMM_FORMAT_R8G8_UNORM_TYPE;
        case Format_R8G8UN:        return GMM_FORMAT_R8G8_UNORM_TYPE;
        case Format_R32F:          return GMM_FORMAT_R32_FLOAT_TYPE;
        case Format_AYUV:          return GMM_FORMAT_AYUV_TYPE;
        case Format_Buffer:        return GMM_FORMAT_A8_UNORM_TYPE;
        // Format_A16R16G16B16 and Format_A16B16G16R16 are using the same surface layout.
        case Format_A16R16G16B16:  return GMM_FORMAT_R16G16B16A16_UNORM_TYPE;
        case Format_A16B16G16R16:  return GMM_FORMAT_R16G16B16A16_UNORM_TYPE;
        default:
        {
            VPHAL_RENDER_ASSERTMESSAGE("Unsupported format %d\n", format);
            return GMM_FORMAT_INVALID;
        }
    }
}

static inline
MOS_FORMAT ConvertGmmFmtToMosFmt(GMM_RESOURCE_FORMAT format)
{
    switch (format)
    {
        case GMM_FORMAT_B8G8R8X8_UNORM_TYPE :      return Format_X8R8G8B8;
        case GMM_FORMAT_B8G8R8A8_UNORM_TYPE :      return Format_A8R8G8B8;
        case GMM_FORMAT_NV12_TYPE:                 return Format_NV12;
        case GMM_FORMAT_A8_UNORM_TYPE :            return Format_A8;
        case GMM_FORMAT_R8G8_UNORM_TYPE :          return Format_R8G8UN;
        case GMM_FORMAT_R32_FLOAT_TYPE :           return Format_R32F;
        case GMM_FORMAT_AYUV_TYPE :                return Format_AYUV;
        // WA for GMM and MDF issue. Will revisit it after fixing the issue.
        case GMM_FORMAT_R16G16B16A16_UNORM_TYPE:   return Format_A16B16G16R16;
        default:
        {
            VPHAL_RENDER_ASSERTMESSAGE("Unsupported format %d\n", format);
            return Format_Invalid;
        }
    }
}

static inline
int GetBitsPerPixel(GMM_RESOURCE_FORMAT format)
{
    switch (format)
    {
        case GMM_FORMAT_B8G8R8X8_UNORM_TYPE:     return 32;
        case GMM_FORMAT_B8G8R8A8_UNORM_TYPE:     return 32;
        case GMM_FORMAT_NV12_TYPE:               return 12;
        case GMM_FORMAT_A8_UNORM_TYPE:           return 8;
        case GMM_FORMAT_R8G8_UNORM_TYPE:         return 16;
        case GMM_FORMAT_R32_FLOAT_TYPE:          return 32;
        case GMM_FORMAT_AYUV_TYPE:               return 32;
        case GMM_FORMAT_R16G16B16A16_UNORM_TYPE: return 64;
        case GMM_FORMAT_R16G16B16X16_UNORM_TYPE: return 64;
        default:
        {
            VPHAL_RENDER_ASSERTMESSAGE("Unsupported format %d\n", format);
            return 0;
        }
    }
}

template <typename CmSurfType>
class VpCmSurfaceHolder
{
public:
    static_assert(
        std::is_same<CmSurfType, CmBuffer   >::value ||
        std::is_same<CmSurfType, CmSurface2D>::value ||
        std::is_same<CmSurfType, CmSurface3D>::value,
        "CmSurfType need to be one of CmBuffer, CmSurface2D or CmSurface3D.");

    VpCmSurfaceHolder(PVPHAL_SURFACE vpSurf, CmContext *cmContext):
        mCmSurface(nullptr),
        mSurfaceIndex(nullptr),
        mSamplerSurfaceIndex(nullptr),
        mSampler8x8SurfaceIndex(nullptr),
        mWidth(vpSurf->dwWidth),
        mHeight(vpSurf->dwHeight),
        mDepth(vpSurf->dwDepth),
        mFormat(ConvertMosFmtToGmmFmt(vpSurf->Format)),
        m_cmContext(cmContext)
    {
        int result = CreateCmSurfaceSpecialized(vpSurf, mCmSurface);
        if ((result != CM_SUCCESS) || (!mCmSurface))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to create VpCmSurfaceHolder from VP Surface!\n");
            return;
        }
        result = mCmSurface->GetIndex(mSurfaceIndex);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to Get Surface Index");
        }
    }

    VpCmSurfaceHolder(int width, int height, int depth, GMM_RESOURCE_FORMAT format, CmContext *cmContext) :
        mCmSurface(nullptr),
        mSurfaceIndex(nullptr),
        mSamplerSurfaceIndex(nullptr),
        mSampler8x8SurfaceIndex(nullptr),
        mWidth(width),
        mHeight(height),
        mDepth(depth),
        mFormat(format),
        m_cmContext(cmContext)
    {
        int result = CreateCmSurfaceSpecialized(width, height, depth, format, mCmSurface);
        if ((result != CM_SUCCESS) || (!mCmSurface))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to create VpCmSurfaceHolder!\n");
            return;
        }
        mCmSurface->GetIndex(mSurfaceIndex);
    }

    virtual ~VpCmSurfaceHolder()
    {
        VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_cmContext);
        CmDevice *dev = m_cmContext->GetCmDevice();
        int result = CM_SUCCESS;

        if (mSampler8x8SurfaceIndex)
        {
            result = dev->DestroySampler8x8Surface(mSampler8x8SurfaceIndex);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to destroy mSampler8x8SurfaceIndex!");
            }
        }

        if (mSamplerSurfaceIndex)
        {
            result = dev->DestroySamplerSurface(mSamplerSurfaceIndex);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to destroy mSamplerSurfaceIndex!");
            }
        }

        if (mCmSurface)
        {
            result = dev->DestroySurface(mCmSurface);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to destroy mCmSurface!");
            }
        }
    }

    CmSurfType* GetCmSurface() const
    {
        return mCmSurface;
    }

    SurfaceIndex* GetCmSurfaceIndex()
    {
        if (!mSurfaceIndex)
        {
            int result = mCmSurface->GetIndex(mSurfaceIndex);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to GetCmSurfaceIndex!");
            }
        }
        return mSurfaceIndex;
    }

    SurfaceIndex* GetCmSamplerSurfaceIndex()
    {
        if (!mSamplerSurfaceIndex)
        {
            if (!m_cmContext)
            {
                return mSamplerSurfaceIndex;
            }
            int result = m_cmContext->GetCmDevice()->CreateSamplerSurface2D(mCmSurface, mSamplerSurfaceIndex);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed in CreateSamplerSurface2D!\n");
            }

        }
        return mSamplerSurfaceIndex;
    }

    SurfaceIndex* GetCmSampler8x8SurfaceIndex()
    {
        if (!mSampler8x8SurfaceIndex)
        {
            if (!m_cmContext)
            {
                return mSampler8x8SurfaceIndex;
            }
            int result = m_cmContext->GetCmDevice()->CreateSampler8x8Surface(mCmSurface, mSampler8x8SurfaceIndex, CM_AVS_SURFACE, CM_SURFACE_CLAMP);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed in CreateSampler8x8Surface!\n");
            }
        }
        return mSampler8x8SurfaceIndex;
    }

    void GetSurfaceDimentions(int &width, int &height, int &depth)
    {
        width  = mWidth;
        height = mHeight;
        depth  = mDepth;
    }

    int GetSurfaceSize() const
    {
        return (mWidth * mHeight * mDepth * GetBitsPerPixel(mFormat)) >> 3;
    }

    GMM_RESOURCE_FORMAT GetFormat() const
    {
        return mFormat;
    }

    void InitSurfaceFromFile(const std::string &fileName)
    {
        std::ifstream blob(fileName, std::ifstream::ate | std::ifstream::binary);
        if (!blob.is_open()) 
        {
            VPHAL_RENDER_ASSERTMESSAGE("Error in opening raw data file: %s.\n", fileName.c_str());
            return;
        }
        const int fileSize = static_cast<int>(blob.tellg());
        if (fileSize == 0)
        {
            VPHAL_RENDER_ASSERTMESSAGE("file size is 0.\n");
            return;
        }
        blob.seekg(0, blob.beg);
        std::vector<char> temp(GetSurfaceSize());
        blob.read(temp.data(), MOS_MIN(fileSize, GetSurfaceSize()));
        mCmSurface->WriteSurface((unsigned char *)temp.data(), nullptr);
    }

    void DumpSurfaceToFile(const std::string &fileName)
    {
        std::ofstream blob(fileName, std::ofstream::out | std::ifstream::binary);

        if (!blob.is_open()) 
        {
            VPHAL_RENDER_ASSERTMESSAGE("Error in opening raw data file: %s.\n", fileName.c_str());
            return;
        }

        const int size = GetSurfaceSize();
        std::vector<char> temp(size);
        mCmSurface->ReadSurface((unsigned char*)temp.data(), nullptr, size);
        blob.write(temp.data(), size);
    }

private:
    VpCmSurfaceHolder(const VpCmSurfaceHolder &) = delete;
    VpCmSurfaceHolder& operator=(const VpCmSurfaceHolder &) = delete;

    inline int CreateCmSurfaceSpecialized(PVPHAL_SURFACE vpSurf, CmSurfType* &surf)
    {
        return 0;
    }

    inline int CreateCmSurfaceSpecialized(int width, int height, int depth, GMM_RESOURCE_FORMAT format, CmSurfType* &surf)
    {
        return 0;
    }

    PVPHAL_SURFACE    mVphalSurface            = nullptr;
    CmSurfType       *mCmSurface               = nullptr;
    SurfaceIndex     *mSurfaceIndex            = nullptr;
    SurfaceIndex     *mSamplerSurfaceIndex     = nullptr;
    SurfaceIndex     *mSampler8x8SurfaceIndex  = nullptr;

    const int                   mWidth      = 0;
    const int                   mHeight     = 0;
    const int                   mDepth      = 0;
    const GMM_RESOURCE_FORMAT   mFormat;
    CmContext                  *m_cmContext = nullptr;
};

template <>
inline int VpCmSurfaceHolder<CmBuffer>::CreateCmSurfaceSpecialized(PVPHAL_SURFACE vpSurf, CmBuffer* &surf)
{
    if (!m_cmContext)
    {
        return CM_NULL_POINTER;
    }
    return m_cmContext->GetCmDevice()->CreateBuffer(&vpSurf->OsResource, surf);
}

template <>
inline int VpCmSurfaceHolder<CmSurface2D>::CreateCmSurfaceSpecialized(PVPHAL_SURFACE vpSurf, CmSurface2D* &surf)
{
    if (!m_cmContext)
    {
        return CM_NULL_POINTER;
    }
    return m_cmContext->GetCmDevice()->CreateSurface2D(&vpSurf->OsResource, surf);
}

template <>
inline int VpCmSurfaceHolder<CmBuffer>::CreateCmSurfaceSpecialized(int width, int height, int depth, GMM_RESOURCE_FORMAT format, CmBuffer* &surf)
{
    if (!m_cmContext)
    {
        return CM_NULL_POINTER;
    }
    return m_cmContext->GetCmDevice()->CreateBuffer(width, surf);
}

template <>
inline int VpCmSurfaceHolder<CmSurface2D>::CreateCmSurfaceSpecialized(int width, int height, int depth, GMM_RESOURCE_FORMAT format, CmSurface2D* &surf)
{
    if (!m_cmContext)
    {
        return CM_NULL_POINTER;
    }
    return m_cmContext->GetCmDevice()->CreateSurface2D(width, height, ConvertGmmFmtToMosFmt(format), surf);
}

template <>
inline int VpCmSurfaceHolder<CmSurface3D>::CreateCmSurfaceSpecialized(int width, int height, int depth, GMM_RESOURCE_FORMAT format, CmSurface3D* &surf)
{
    if (!m_cmContext)
    {
        return CM_NULL_POINTER;
    }
    return m_cmContext->GetCmDevice()->CreateSurface3D(width, height, depth, ConvertGmmFmtToMosFmt(format), surf);
}

class VPRender
{
public:
    virtual ~VPRender()
    {
    }

    virtual void Render(void *payload) = 0;
};

// A simple CM kernel render wrapper.
// All methods derived classes need to implement are private.
// This means the calling order of such methods is fixed, and they will
// get hooked to the Render() entry method in this base class.
class VPCmRenderer: public VPRender
{
public:
    VPCmRenderer(const std::string &name, CmContext *cmContext);
    virtual ~VPCmRenderer();

    void Render(void *payload) override;

    void SetmBlockingMode(bool enable)
    {
        mBlockingMode = enable;
    }

    void EnableDump(bool enable)
    {
        mEnableDump = enable;
    }

    void EnableBatchDispatch(bool enable)
    {
        mBatchDispatch = enable;
    }

protected:
    CmProgram* LoadProgram(const void *binary, int size);
    CmProgram* LoadProgram(const std::string& binaryFileName);

    const std::string  mName;
    CmContext          *m_cmContext = nullptr;

private:
    virtual void AttachPayload(void *) = 0;
    virtual CmKernel* GetKernelToRun(std::string &name) = 0;
    virtual void GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor) = 0;
    virtual void PrepareKernel(CmKernel *kernel) = 0;

    // Derived class can override this method if special setup needs be performed on thread space,
    // like walking pattern, dependency pattern(scoreboard), etc.
    virtual void SetupThreadSpace(CmThreadSpace *threadSpace, int /*tsWidth*/, int /*tsHeight*/, int /*tsColor*/)
    {
        int32_t iStatus = threadSpace->SelectMediaWalkingPattern(CM_WALK_VERTICAL);
        if (iStatus != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("SelectMediaWalkingPattern Returns %d", iStatus);
        }
    }

    // This method will take effect only if this renderer works in batch dispatching mode(multi-kernels in one task).
    // Be careful if derived class want to override it. Return true is always safe and workable.
    // Return false if its kernel has no dependency against previous ones,
    // thus will enable kernel parallel execution, which can improve performance in some cases.
    virtual bool NeedAddSync()
    {
        return true;
    }

    virtual bool CannotAssociateThreadSpace()
    {
        return true;
    }

    virtual void Dump()
    {
    }

    bool  mBatchDispatch;
    bool  mBlockingMode;
    bool  mEnableDump;
};

#endif // __VPHAL_MDF_WRAPPER_H__
