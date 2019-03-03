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
//! \file      cm_rt_api_os.h
//! \brief     Contains Linux Specific APIs and Definitions for CM
//!

#ifndef __CM_RT_API_OS_H__
#define __CM_RT_API_OS_H__

class CmSurface2D
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 

    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT ReadSurfaceStride( unsigned char* pSysMem, CmEvent* pEvent, const UINT stride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurfaceStride( const unsigned char* pSysMem, CmEvent* pEvent, const UINT stride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;

    CM_RT_API virtual INT GetVaSurfaceID( VASurfaceID  &iVASurface) = 0;

    CM_RT_API virtual INT ReadSurfaceHybridStrides( unsigned char* pSysMem, CmEvent* pEvent, const UINT iWidthStride, const UINT iHeightStride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL, UINT uiOption = 0 ) = 0;
    CM_RT_API virtual INT WriteSurfaceHybridStrides( const unsigned char* pSysMem, CmEvent* pEvent, const UINT iWidthStride, const UINT iHeightStride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL, UINT uiOption = 0 ) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;
    CM_RT_API virtual INT SetProperty(CM_FRAME_TYPE frameType) = 0; 
    CM_RT_API virtual INT SetSurfaceStateParam( SurfaceIndex *pSurfIndex, const CM_SURFACE2D_STATE_PARAM *pSSParam ) = 0;
protected:
    ~CmSurface2D(){}
};

class CmDevice
{
public:

    CM_RT_API virtual INT CreateBuffer(UINT size, CmBuffer* & pSurface )=0;
    CM_RT_API virtual INT CreateSurface2D(UINT width, UINT height, CM_SURFACE_FORMAT format, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface3D(UINT width, UINT height, UINT depth, CM_SURFACE_FORMAT format, CmSurface3D* & pSurface ) = 0;

    CM_RT_API virtual INT CreateSurface2D( VASurfaceID iVASurface, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface2D( VASurfaceID* iVASurface, const UINT surfaceCount, CmSurface2D** pSurface ) = 0;

    CM_RT_API virtual INT DestroySurface( CmBuffer* & pSurface) = 0;
    CM_RT_API virtual INT DestroySurface( CmSurface2D* & pSurface) = 0;
    CM_RT_API virtual INT DestroySurface( CmSurface3D* & pSurface) = 0;
  
    CM_RT_API virtual INT CreateQueue( CmQueue* & pQueue) = 0; 
    CM_RT_API virtual INT LoadProgram( void* pCommonISACode, const UINT size, CmProgram*& pProgram, const char* options = nullptr ) = 0;
    CM_RT_API virtual INT CreateKernel( CmProgram* pProgram, const char* kernelName, CmKernel* & pKernel, const char* options = nullptr) = 0;
    CM_RT_API virtual INT CreateKernel( CmProgram* pProgram, const char* kernelName, const void * fncPnt, CmKernel* & pKernel, const char* options = nullptr) = 0;
    CM_RT_API virtual INT CreateSampler( const CM_SAMPLER_STATE & sampleState, CmSampler* & pSampler ) = 0;

    CM_RT_API virtual INT DestroyKernel( CmKernel*& pKernel) = 0;
    CM_RT_API virtual INT DestroySampler( CmSampler*& pSampler ) = 0;
    CM_RT_API virtual INT DestroyProgram( CmProgram*& pProgram ) = 0;
    CM_RT_API virtual INT DestroyThreadSpace( CmThreadSpace* & pTS ) = 0; 

    CM_RT_API virtual INT CreateTask(CmTask *& pTask)=0;
    CM_RT_API virtual INT DestroyTask(CmTask*& pTask)=0;

    CM_RT_API virtual INT GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* pCapValue ) = 0;

    CM_RT_API virtual INT CreateThreadSpace( UINT width, UINT height, CmThreadSpace* & pTS ) = 0;

    CM_RT_API virtual INT CreateBufferUP(UINT size, void* pSystMem, CmBufferUP* & pSurface )=0;
    CM_RT_API virtual INT DestroyBufferUP( CmBufferUP* & pSurface) = 0;

    CM_RT_API virtual INT GetSurface2DInfo( UINT width, UINT height, CM_SURFACE_FORMAT format, UINT & pitch, UINT & physicalSize)= 0;
    CM_RT_API virtual INT CreateSurface2DUP( UINT width, UINT height, CM_SURFACE_FORMAT format, void* pSysMem, CmSurface2DUP* & pSurface )= 0;
    CM_RT_API virtual INT DestroySurface2DUP( CmSurface2DUP* & pSurface) = 0;

    CM_RT_API virtual INT CreateVmeSurfaceG7_5 ( CmSurface2D* pCurSurface, CmSurface2D** pForwardSurface, CmSurface2D** pBackwardSurface, const UINT surfaceCountForward, const UINT surfaceCountBackward, SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT DestroyVmeSurfaceG7_5( SurfaceIndex* & pVmeIndex ) = 0;
    CM_RT_API virtual INT CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  & smplDescr, CmSampler8x8*& psmplrState)=0;
    CM_RT_API virtual INT DestroySampler8x8( CmSampler8x8*& pSampler )=0;
    CM_RT_API virtual INT CreateSampler8x8Surface(CmSurface2D* p2DSurface, SurfaceIndex* & pDIIndex, CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE, CM_SURFACE_ADDRESS_CONTROL_MODE address_mode = CM_SURFACE_CLAMP )=0;
    CM_RT_API virtual INT DestroySampler8x8Surface(SurfaceIndex* & pDIIndex)=0;

    CM_RT_API virtual INT CreateThreadGroupSpace( UINT thrdSpaceWidth, UINT thrdSpaceHeight, UINT grpSpaceWidth, UINT grpSpaceHeight, CmThreadGroupSpace*& pTGS ) = 0;
    CM_RT_API virtual INT DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS) = 0;
    CM_RT_API virtual INT SetL3Config(const L3ConfigRegisterValues *l3_c) = 0;
    CM_RT_API virtual INT SetSuggestedL3Config( L3_SUGGEST_CONFIG l3_s_c) = 0;

    CM_RT_API virtual INT SetCaps(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* pCapValue) = 0;

    CM_RT_API virtual INT CreateSamplerSurface2D(CmSurface2D* p2DSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;
    CM_RT_API virtual INT CreateSamplerSurface3D(CmSurface3D* p3DSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;
    CM_RT_API virtual INT DestroySamplerSurface(SurfaceIndex* & pSamplerSurfaceIndex) = 0;

    CM_RT_API virtual INT InitPrintBuffer(size_t printbufsize = 1048576) = 0; 
    CM_RT_API virtual INT FlushPrintBuffer() = 0;

    CM_RT_API virtual INT CreateVebox( CmVebox* & pVebox ) = 0;
    CM_RT_API virtual INT DestroyVebox( CmVebox* & pVebox ) = 0;

    CM_RT_API virtual INT GetVaDpy(VADisplay* & pva_dpy) = 0;
    CM_RT_API virtual INT CreateVaSurface2D( UINT width, UINT height, CM_SURFACE_FORMAT format, VASurfaceID & iVASurface, CmSurface2D* & pSurface) = 0;

    CM_RT_API virtual INT CreateBufferSVM(UINT size, void* & pSystMem, uint32_t accessFlag, CmBufferSVM* & pSurface ) = 0;
    CM_RT_API virtual INT DestroyBufferSVM( CmBufferSVM* & pSurface) = 0;
    CM_RT_API virtual INT CreateSamplerSurface2DUP(CmSurface2DUP* p2DUPSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;

    CM_RT_API virtual INT CloneKernel( CmKernel * &pKernelDest, CmKernel * pKernelSrc ) = 0;

    CM_RT_API virtual INT CreateSurface2DAlias(CmSurface2D* p2DSurface, SurfaceIndex* &aliasSurfaceIndex) = 0;

    CM_RT_API virtual INT CreateHevcVmeSurfaceG10 ( CmSurface2D* pCurSurface, CmSurface2D** pForwardSurface, CmSurface2D** pBackwardSurface, const UINT surfaceCountForward, const UINT surfaceCountBackward, SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT DestroyHevcVmeSurfaceG10( SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT CreateSamplerEx( const CM_SAMPLER_STATE_EX & sampleState, CmSampler* & pSampler ) = 0;
    CM_RT_API virtual INT FlushPrintBufferIntoFile(const char *filename) = 0;
    CM_RT_API virtual INT CreateThreadGroupSpaceEx(UINT thrdSpaceWidth, UINT thrdSpaceHeight, UINT thrdSpaceDepth, UINT grpSpaceWidth, UINT grpSpaceHeight, UINT grpSpaceDepth, CmThreadGroupSpace*& pTGS) = 0;

    CM_RT_API virtual INT CreateSampler8x8SurfaceEx(CmSurface2D* p2DSurface, SurfaceIndex* & pDIIndex, CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE, CM_SURFACE_ADDRESS_CONTROL_MODE address_mode = CM_SURFACE_CLAMP, CM_FLAG* pFlag = nullptr) = 0;
    CM_RT_API virtual INT CreateSamplerSurface2DEx(CmSurface2D* p2DSurface, SurfaceIndex* & pSamplerSurfaceIndex, CM_FLAG* pFlag = nullptr) = 0;
    CM_RT_API virtual INT CreateBufferAlias(CmBuffer *pBuffer, SurfaceIndex* &pAliasIndex) = 0;

    CM_RT_API virtual INT SetVmeSurfaceStateParam(SurfaceIndex* pVmeIndex, CM_VME_SURFACE_STATE_PARAM *pSSParam) = 0;

    CM_RT_API virtual int32_t GetVISAVersion(uint32_t& majorVersion, uint32_t& minorVersion) = 0;
    CM_RT_API virtual int32_t CreateQueueEx(CmQueue *&pQueue, CM_QUEUE_CREATE_OPTION QueueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION) = 0;

    //adding new functions in the bottom is a must 
protected:
    ~CmDevice(){}
};

class SurfaceIndex
{
public:
    CM_NOINLINE SurfaceIndex() { index = 0; };
    CM_NOINLINE SurfaceIndex(const SurfaceIndex& _src) { index = _src.index; };
    CM_NOINLINE SurfaceIndex(const unsigned int& _n) { index = _n; };
    CM_NOINLINE SurfaceIndex& operator = (const unsigned int& _n) { this->index = _n; return *this; };
    CM_NOINLINE SurfaceIndex& operator + (const unsigned int& _n) { this->index += _n; return *this; };
    virtual unsigned int get_data(void) { return index; };

    virtual ~SurfaceIndex(){};
private:
    unsigned int index;
    unsigned char extra_byte; // an extra byte to align the object size among OSes
};

class SamplerIndex
{
public:
    CM_NOINLINE SamplerIndex() { index = 0; };
    CM_NOINLINE SamplerIndex(SamplerIndex& _src) { index = _src.get_data(); };
    CM_NOINLINE SamplerIndex(const unsigned int& _n) { index = _n; };
    CM_NOINLINE SamplerIndex& operator = (const unsigned int& _n) { this->index = _n; return *this; };
    virtual unsigned int get_data(void) { return index; };

    virtual ~SamplerIndex(){};
private:
    unsigned int index;
    unsigned char extra_byte; // an extra byte to align the object size among OSes
};

EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &device, UINT& version, VADisplay vaDisplay = nullptr);
EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &device, UINT& version, VADisplay vaDisplay, UINT DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);

#endif //__CM_RT_API_OS_H__
