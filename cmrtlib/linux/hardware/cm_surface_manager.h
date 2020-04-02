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
#ifndef CMRTLIB_LINUX_HARDWARE_CM_SURFACE_MANAGER_H_
#define CMRTLIB_LINUX_HARDWARE_CM_SURFACE_MANAGER_H_

#include "cm_def_os.h"
#include "cm_include.h"

class CmDevice_RT;
class CmSurface2D;
class CmSurface2DUP;
class CmSurface3D;
class CmBuffer;
class CmBufferUP;
class CmBufferSVM;
class CmBufferStateless;

class CmSurfaceManager
{
public:
    CmSurfaceManager(CmDevice_RT *device);

    ~CmSurfaceManager();

    int32_t CreateSurface2D(uint32_t width,
                        uint32_t height,
                        CM_SURFACE_FORMAT format,
                        CmSurface2D* &surface);

    int32_t CreateSurface2D(VASurfaceID *vaSurface, CmSurface2D *&surface);

    int32_t CreateSurface2D(VASurfaceID *vaSurface,
                        uint32_t firstArraySlice,
                        uint32_t mipSlice,
                        CmSurface2D *&surface);

    int32_t DestroySurface(CmSurface2D* &surface);

    int32_t CreateBuffer(uint32_t size, CmBuffer* &buffer);

    int32_t CreateBufferUP(uint32_t size,
                       void *sysMem,
                       CmBufferUP* &buffer);

    int32_t DestroyBuffer(CmBuffer *&buffer);

    int32_t DestroyBufferUP(CmBufferUP *&buffer);

    int32_t CreateSurface2DUP(uint32_t width,
                          uint32_t height,
                          CM_SURFACE_FORMAT format,
                          void *sysMem,
                          CmSurface2DUP* &surface);

    int32_t DestroySurface2DUP(CmSurface2DUP* &surface);

    int32_t CreateSurface3D(uint32_t width,
                        uint32_t height,
                        uint32_t depth,
                        CM_SURFACE_FORMAT format,
                        CmSurface3D *&surface);

    int32_t DestroySurface3D(CmSurface3D *&surface);

    int32_t CreateBufferSVM(uint32_t size,
                        void* &sysMem,
                        uint32_t accessFlag,
                        CmBufferSVM* &buffer);

    int32_t DestroyBufferSVM(CmBufferSVM* &buffer);

    int32_t CreateVaSurface2D(uint32_t width,
                          uint32_t height,
                          CM_SURFACE_FORMAT format,
                          VASurfaceID &vaSurface,
                          CmSurface2D* &surface);

    int32_t CreateSurface2D(VASurfaceID vaSurface, CmSurface2D* &surface);

    int32_t CreateSurface2D(VASurfaceID *vaSurfaceArray,
                        const uint32_t surfaceCount,
                        CmSurface2D **surfaceArray);

    int32_t CreateBufferStateless(size_t size,
                                  uint32_t option,
                                  void *sysMem,
                                  CmBufferStateless *&buffer);

    int32_t DestroyBufferStateless(CmBufferStateless *&buffer);

protected:
    int32_t CreateSurface2D(VASurfaceID &vaSurface,
                        bool cmCreated,
                        bool createdByLibva,
                        CmSurface2D* &surface);

    int32_t ConvertToLibvaFormat(int32_t format);

    int32_t AllocateSurface2DInUmd(uint32_t width,
                             uint32_t height,
                             CM_SURFACE_FORMAT format,
                             bool cmCreated,
                             bool createdByLibva,
                             VASurfaceID vaSurface,
                             CmSurface2D* &surface);

    int32_t Surface2DSanityCheck(uint32_t width,
                             uint32_t height,
                             CM_SURFACE_FORMAT format);

    int32_t DestroySurface2DInUmd(CmSurface2D* &surface);

private:
    CmDevice_RT *m_device;

private:
    CmSurfaceManager(const CmSurfaceManager &other);
    CmSurfaceManager &operator=(const CmSurfaceManager &other);

};

#endif  // #ifndef CMRTLIB_LINUX_HARDWARE_CM_SURFACE_MANAGER_H_
