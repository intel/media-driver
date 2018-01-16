/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_wrapper_os.cpp
//! \brief     Contains implementations of Linux-dependent functions for executing
//!            commands from cmrtlib.
//!

#include "cm_wrapper.h"

#include "cm_device_rt.h"
#include "cm_surface_2d_rt.h"
#include "media_libva_util.h"

#if USE_EXTENSION_CODE
#include "cm_ult.h"
#endif

#include "mos_os_specific.h"

extern MOS_FORMAT Mos_Specific_FmtOsToMos(
    MOS_OS_FORMAT     format);

extern MOS_OS_FORMAT Mos_Specific_FmtMosToOs(
    MOS_FORMAT     format);


using CMRT_UMD::CmDeviceRT;
//!
//! \brief    Create Cm Device from VA Driver Context.
//! \details  Create a CmCtx and a associated MOS_CONTEXT. Put the CmCtx into
//!           the heap of VA Context.
//! \param    pVaDrvCtx
//!           [in] pointer to va drv conetext.
//! \param    pCmDev
//!           [in,out] reference to cm device pointer.
//! \param    DevOption
//!           [in] cm device creation option.
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason.
//!
int32_t CreateCmDeviceFromVA(VADriverContextP pVaDrvCtx,
                             CmDevice* &pCmDev,
                             uint32_t DevOption)
{
    int32_t                           hRes = CM_SUCCESS;
    PDDI_MEDIA_CONTEXT                pMediaCtx;
    PCM_CONTEXT                       pCmCtx;
    uint32_t                          uiCtxIndex;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pVaCtxHeapElmt;
    VAContextID                       VaContextID;

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);

    // allocate pCmCtx
    pCmCtx = (PCM_CONTEXT)MOS_AllocAndZeroMemory(sizeof(CM_CONTEXT));
    CM_DDI_CHK_NULL(pCmCtx, "Null pCmCtx!", CM_OUT_OF_HOST_MEMORY);

    // init pCmCtx
    pCmCtx->mosCtx.bufmgr          = pMediaCtx->pDrmBufMgr;
    pCmCtx->mosCtx.m_gpuContextMgr = pMediaCtx->m_gpuContextMgr;
    pCmCtx->mosCtx.m_cmdBufMgr     = pMediaCtx->m_cmdBufMgr;
    pCmCtx->mosCtx.fd              = pMediaCtx->fd;
    pCmCtx->mosCtx.wRevision       = 0;
    pCmCtx->mosCtx.iDeviceId       = pMediaCtx->iDeviceId;
    pCmCtx->mosCtx.SkuTable        = pMediaCtx->SkuTable;
    pCmCtx->mosCtx.WaTable         = pMediaCtx->WaTable;
    pCmCtx->mosCtx.gtSystemInfo    = *(pMediaCtx->pGtSystemInfo);
    pCmCtx->mosCtx.platform        = pMediaCtx->platform;

    // Create Cm Device
    hRes = CreateCmDevice(&(pCmCtx->mosCtx), pCmDev, DevOption);
    if(hRes != CM_SUCCESS)
    {
        MOS_FreeMemAndSetNull(pCmCtx); // free cm ctx
        CM_DDI_ASSERTMESSAGE("Failed to call CmDevice::Create Error %d \n",hRes);
        return hRes;
    }
    CmDeviceRT* pCmDevRT = static_cast<CmDeviceRT*>(pCmDev);
    DdiMediaUtil_LockMutex(&pMediaCtx->CmMutex);

    // get Free Cm context index
    pVaCtxHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaCtx->pCmCtxHeap);
    if (nullptr == pVaCtxHeapElmt)
    {
        CmDeviceRT::Destroy(pCmDevRT); // destroy cm device
        pCmDev = nullptr;
        MOS_FreeMemAndSetNull(pCmCtx); // free cm ctx
        DdiMediaUtil_UnLockMutex(&pMediaCtx->CmMutex);
        CM_DDI_ASSERTMESSAGE("CM Context number exceeds maximum.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // store pCmCtx in pMedia
    pVaCtxHeapElmt->pVaContext    = (void *)pCmCtx;
    VaContextID = (VAContextID)(pVaCtxHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_CM);

    //Set VaCtx ID to Cm device
    pCmDevRT->SetVaCtxID(VaContextID);

    // increate CM context number
    pMediaCtx->uiNumCMs++;

    DdiMediaUtil_UnLockMutex(&pMediaCtx->CmMutex);

    return hRes;
}

//!
//! \brief    Destroy Cm Device and free heap in VA context.
//! \param    pVaDrvCtx
//!           [in] pointer to va drv conetext.
//! \param    pCmDev
//!           [in] pointer to cm device to release.
//! \return   int32_t
//!           CM_SUCCESS if success, else fail reason.
//!
int32_t DestroyCmDeviceFromVA(VADriverContextP pVaDrvCtx, CmDevice *pCmDev)
{
    int32_t               hr          = CM_SUCCESS;
    uint32_t              index;
    PDDI_MEDIA_CONTEXT    pMediaCtx;
    PCM_CONTEXT           pCmCtx;
    VAContextID           VaContextID;
    uint32_t              ctxType;
    VAStatus              vaStatus;

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    CM_DDI_CHK_NULL(pVaDrvCtx,
                "Null pVaDrvCtx.",
                VA_STATUS_ERROR_INVALID_CONTEXT);

    CM_DDI_CHK_NULL(pCmDev,
            "Null pCmDev.",
            VA_STATUS_ERROR_INVALID_CONTEXT);
    CmDeviceRT* pCmDevRT = static_cast<CmDeviceRT*>(pCmDev);
    //Get VaCtx ID in MediaCtx
    pCmDevRT->GetVaCtxID(VaContextID);

    // Get Cm context index
    index = VaContextID & DDI_MEDIA_MASK_VACONTEXTID;

    //Get Cm Context
    pCmCtx    = (PCM_CONTEXT)DdiMedia_GetContextFromContextID(pVaDrvCtx, VaContextID, &ctxType);
    CM_DDI_CHK_NULL(pCmCtx, "Null pCmCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    CHK_HR(DestroyCmDevice(&(pCmCtx->mosCtx)));

    // remove from context array
    DdiMediaUtil_LockMutex(&pMediaCtx->CmMutex);

    // destroy Cm context
    MOS_FreeMemAndSetNull(pCmCtx);

    DdiMediaUtil_ReleasePVAContextFromHeap(pMediaCtx->pCmCtxHeap, index);

    pMediaCtx->uiNumCMs--;

    DdiMediaUtil_UnLockMutex(&pMediaCtx->CmMutex);

finish:
    return hr;
}


extern MOS_FORMAT   VpGetFormatFromMediaFormat(DDI_MEDIA_FORMAT mf);
//*-----------------------------------------------------------------------------
//| Purpose:    Get resource information from LibVA-created surface and fill into OsResource
//              iVASurfaceID is the index to VA's surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmFillMosResource( VASurfaceID        iVASurfaceID,
                       VADriverContext*   pUMDCtx,
                       PMOS_RESOURCE      pOsResource)
{
    PDDI_MEDIA_CONTEXT    pMediaCtx;
    DDI_MEDIA_SURFACE     *pSurface;
    CmDevice              *pCmDevice;

    CM_DDI_CHK_NULL(pUMDCtx, "Null pUMDCtx", CM_INVALID_UMD_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(pUMDCtx);
    CM_DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", CM_INVALID_UMD_CONTEXT);

    CM_DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", CM_INVALID_UMD_CONTEXT);
    CM_CHK_LESS((uint32_t)iVASurfaceID, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", CM_INVALID_LIBVA_SURFACE);

    pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, iVASurfaceID);
    CM_DDI_CHK_NULL(pSurface, "Null pSurface", CM_INVALID_LIBVA_SURFACE);
    CM_ASSERT(pSurface->iPitch == GFX_ULONG_CAST(pSurface->pGmmResourceInfo->GetRenderPitch()));
    CM_DDI_CHK_NULL(pSurface->bo, "Null BO", CM_INVALID_LIBVA_SURFACE);
    CM_DDI_CHK_NULL(pSurface->pGmmResourceInfo, "Null GMMResInfo", CM_INVALID_LIBVA_SURFACE);

    // Resets the Resource
    Mos_ResetResource(pOsResource);

    pOsResource->iWidth   = pSurface->iWidth;
    pOsResource->iHeight  = pSurface->iHeight;
    pOsResource->iDepth   = 1;
    pOsResource->iPitch   = pSurface->iPitch;

    pOsResource->iCount   = 0;
    pOsResource->bufname  = (char *)"Libva2DSurface";

    pOsResource->Format   = VpGetFormatFromMediaFormat(pSurface->format);
    pOsResource->bo       = pSurface->bo;

    pOsResource->TileType = LinuxToMosTileType(pSurface->TileType);
    pOsResource->isTiled  = pSurface->isTiled;

    pOsResource->bMapped  = pSurface->bMapped;
    pOsResource->pData    = (uint8_t*) pSurface->bo->virt;

    pOsResource->pGmmResInfo = pSurface->pGmmResourceInfo;

    // for wrapper to new MOS MODS interface
    pOsResource->bConvertedFromDDIResource = true;

    return CM_SUCCESS;
}


MOS_FORMAT CmOSFmtToMosFmt(CM_OSAL_SURFACE_FORMAT format)
{
    //CM_SURFACE_FORMAT_R8U/R16U are not valid va surface format.
    //These two formats are MDF specific, so we add the mapping here.
    switch(format)
    {
      case CM_SURFACE_FORMAT_R8U:  return Format_R8U;
      case CM_SURFACE_FORMAT_R16U: return Format_R16U;
      default:
        return Mos_Specific_FmtOsToMos(format);
    }
}

CM_OSAL_SURFACE_FORMAT  CmMosFmtToOSFmt(MOS_FORMAT format)
{
    //CM_SURFACE_FORMAT_R8U/R16U are not valid va surface format.
    //These two formats are MDF specific, so we add the mapping here.
    switch(format)
    {
        case Format_R8U:   return CM_SURFACE_FORMAT_R8U;
        case Format_R16U:  return CM_SURFACE_FORMAT_R16U;
        default:
           return Mos_Specific_FmtMosToOs(format);
    }
}

using CMRT_UMD::CmSurface2DRT;
//*-----------------------------------------------------------------------------
//| Purpose:    CMRT thin layer library supported function execution
//| Return:     CM_SUCCESS if successful
//*-----------------------------------------------------------------------------
int32_t CmThinExecute(VADriverContextP pVaDrvCtx,
                      void *pCmDeviceHandle,
                      uint32_t inputFunctionId,
                      void *inputData,
                      uint32_t inputDataLen)
{
    CmDevice             *pDevice           = nullptr;
    CmDeviceRT           *pDeviceRT         = nullptr;
    VADriverContextP     hUMDevice          = nullptr;
    void                 *pCmPrivateInputData = nullptr;
    uint32_t             CmPrivateInputDataSize = 0 ;
    CMRT_UMD::CmSurface2D *pCmSurface2d       = nullptr;
    SurfaceIndex         *pSurfaceIndex      = nullptr;
    CM_FUNCTION_ID       CmFunctionID;
    int32_t              hr                  = CM_SUCCESS;
    int32_t              cmRet               = CM_INVALID_PRIVATE_DATA;

    hUMDevice               = pVaDrvCtx;
    pCmPrivateInputData     = inputData;
    CmPrivateInputDataSize  = inputDataLen;
    CmFunctionID            = (CM_FUNCTION_ID)inputFunctionId;
    pDevice                 = (CmDevice *)pCmDeviceHandle;
    pDeviceRT               = static_cast<CmDeviceRT*>(pDevice);
    switch(CmFunctionID)
    {
        case CM_FN_CREATECMDEVICE:
            PCM_CREATECMDEVICE_PARAM pCmDeviceParam;
            pCmDeviceParam = (PCM_CREATECMDEVICE_PARAM)(pCmPrivateInputData);
            //Create Cm Device
            cmRet = CreateCmDeviceFromVA(pVaDrvCtx, pDevice, pCmDeviceParam->DevCreateOption);
            if ( cmRet == CM_SUCCESS)
            {
                CM_DDI_CHK_NULL(pDevice, "Null pDevice.", VA_STATUS_ERROR_INVALID_CONTEXT);
                pDeviceRT = static_cast<CmDeviceRT*>(pDevice);
                pDeviceRT->RegisterCallBack(pCmDeviceParam->pCallBackReleaseVaSurf);
                pCmDeviceParam->iDriverStoreEnabled = pDeviceRT->GetDriverStoreFlag();
            }
            //Fill the output message
            pCmDeviceParam->pCmDeviceHandle = pDevice;
            pCmDeviceParam->iReturnValue    = cmRet;
            pCmDeviceParam->iVersion        = CM_VERSION;
            break;

        case CM_FN_DESTROYCMDEVICE:
            PCM_DESTROYCMDEVICE_PARAM pCmDevDestroyParam;
            pCmDevDestroyParam = (PCM_DESTROYCMDEVICE_PARAM)(pCmPrivateInputData);
            pDevice            = (CmDevice *)(pCmDevDestroyParam->pCmDeviceHandle);
            cmRet = DestroyCmDeviceFromVA(pVaDrvCtx,pDevice);
            //Fill the output message
            pCmDevDestroyParam->pCmDeviceHandle = nullptr;
            pCmDevDestroyParam->iReturnValue    = cmRet;
            break;

        case CM_FN_CMDEVICE_CREATESURFACE2D:
            PCM_CREATESURFACE2D_PARAM   pCmCreate2DParam;
            MOS_RESOURCE                MosResource ;
            MOS_ZeroMemory(&MosResource, sizeof(MOS_RESOURCE));
            pCmCreate2DParam    = (PCM_CREATESURFACE2D_PARAM)(pCmPrivateInputData);
            if ( pCmCreate2DParam->bIsLibvaCreated )
            {
                //LibVA-created Surface2D
                cmRet = CmFillMosResource(pCmCreate2DParam->uiVASurfaceID,
                                       pVaDrvCtx,
                                       &MosResource);

                if( cmRet != CM_SUCCESS)
                {
                    CM_ASSERTMESSAGE("Error: Failed to fill MOS resource.");
                    pCmCreate2DParam->iReturnValue          = cmRet;
                    return cmRet;
                }

                cmRet = pDeviceRT->CreateSurface2D(&MosResource, pCmCreate2DParam->bIsCmCreated ,pCmSurface2d);
                if( cmRet != CM_SUCCESS)
                {
                    CM_ASSERTMESSAGE("Error: Failed to create surface 2D from MOS resource.");
                    pCmCreate2DParam->iReturnValue          = cmRet;
                    return cmRet;
                }

                CmSurface2DRT *pCmSurface2dRT = static_cast<CmSurface2DRT *>(pCmSurface2d);
                pCmSurface2dRT->SetVaSurfaceID(pCmCreate2DParam->uiVASurfaceID, pCmCreate2DParam->pVaDpy);
            }
            else
            {
                // CM Created Surface2D
                cmRet = pDevice->CreateSurface2D(
                        pCmCreate2DParam->iWidth,
                        pCmCreate2DParam->iHeight,
                        CmOSFmtToMosFmt(pCmCreate2DParam->Format),
                        pCmSurface2d);
            }
            //Create Surface Index
            if( cmRet == CM_SUCCESS)
            {
                pCmCreate2DParam->pCmSurface2DHandle    = pCmSurface2d;
            }

            //Fill output message
            pCmCreate2DParam->iReturnValue          = cmRet;
            break;

#if USE_EXTENSION_CODE
#if CM_ULT
        case CM_FN_RT_ULT:
        {
            PDDI_MEDIA_CONTEXT                pMediaCtx;
            MOS_CONTEXT                       mosCtx;

            pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);

            MOS_ZeroMemory(&mosCtx, sizeof(MOS_CONTEXT));

            // init pCmCtx
            mosCtx.bufmgr          = pMediaCtx->pDrmBufMgr;
            mosCtx.m_gpuContextMgr = pMediaCtx->m_gpuContextMgr;
            mosCtx.m_cmdBufMgr     = pMediaCtx->m_cmdBufMgr;
            mosCtx.fd              = pMediaCtx->fd;
            mosCtx.wRevision       = 0;
            mosCtx.iDeviceId       = pMediaCtx->iDeviceId;
            mosCtx.SkuTable        = pMediaCtx->SkuTable;
            mosCtx.WaTable         = pMediaCtx->WaTable;
            mosCtx.gtSystemInfo    = *(pMediaCtx->pGtSystemInfo);
            mosCtx.platform        = pMediaCtx->platform;
            hr                     = CmThinExecuteUlt(&mosCtx, pCmPrivateInputData);
            break;
        }
#endif
#endif

        default:
            hr = CmThinExecuteEx(pDevice, CmFunctionID, pCmPrivateInputData, CmPrivateInputDataSize);
    }

    return hr;
}
