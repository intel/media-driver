/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     ddi_vp_tools.cpp
//! \brief    ddi vp tool functions implementation
//!
#include <sys/time.h>
#include "ddi_vp_tools.h"
#include "media_libva_util_next.h"
#include "media_libva_interface_next.h"

#if (_DEBUG || _RELEASE_INTERNAL)
VAStatus DdiVpTools::InitDumpConfig(PDDI_VP_CONTEXT vpCtx)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    vpCtx->pCurVpDumpDDIParam = nullptr;
    vpCtx->pPreVpDumpDDIParam = nullptr;

    // allocate pCurVpDumpDDIParam
    vpCtx->pCurVpDumpDDIParam = MOS_New(DDI_VP_DUMP_PARAM);
    if (vpCtx->pCurVpDumpDDIParam)
    {
        for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
        {
            vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex] = MOS_New(VAProcPipelineParameterBuffer);
            if (nullptr == vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex])
            {
                MOS_Delete(vpCtx->fpDumpFile);
                vpCtx->fpDumpFile = nullptr;
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
        }
    }
    // allocate pPreVpDumpDDIParam
    vpCtx->pPreVpDumpDDIParam = MOS_New(DDI_VP_DUMP_PARAM);
    if (vpCtx->pPreVpDumpDDIParam)
    {
        for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
        {
            vpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[surfIndex] = MOS_New(VAProcPipelineParameterBuffer);
            if (nullptr == vpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[surfIndex])
            {
                MOS_Delete(vpCtx->fpDumpFile);
                vpCtx->fpDumpFile = nullptr;
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
        }
    }

    vpCtx->fpDumpFile = nullptr;
    return vaStatus;
}

void DdiVpTools::DestoryDumpConfig(PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx", );

    if (vpCtx->pCurVpDumpDDIParam)
    {
        for (uint32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
        {
            if (vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex])
            {
                MOS_Delete(vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex]);
            }
        }
        MOS_Delete(vpCtx->pCurVpDumpDDIParam);
    }

    if (vpCtx->pPreVpDumpDDIParam)
    {
        for (uint32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
        {
            if (vpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[surfIndex])
            {
                MOS_Delete(vpCtx->pPreVpDumpDDIParam->pPipelineParamBuffers[surfIndex]);
            }
        }
        MOS_Delete(vpCtx->pPreVpDumpDDIParam);
    }

    if (vpCtx->fpDumpFile)
    {
        fclose(vpCtx->fpDumpFile);
        MOS_Delete(vpCtx->fpDumpFile);
        vpCtx->fpDumpFile = nullptr;
    }

    return;
}

bool DdiVpTools::VpCmpDDIDumpParam(
    PDDI_VP_DUMP_PARAM curParams,
    PDDI_VP_DUMP_PARAM preParams)
{
    bool ret = true;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(curParams, "nullptr curParams.", false);
    DDI_VP_CHK_NULL(preParams, "nullptr preParams.", false);

    if (memcmp(curParams->SrcFormat, preParams->SrcFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_SOURCES) != 0)
    {
        ret = false;
    }

    if (memcmp(curParams->TargetFormat, preParams->TargetFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_TARGETS) != 0)
    {
        ret = false;
    }

    for (uint32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
    {
        if (memcmp(curParams->pPipelineParamBuffers[surfIndex], preParams->pPipelineParamBuffers[surfIndex], sizeof(VAProcPipelineParameterBuffer)) != 0)
        {
            ret = false;
        }
    }

    return ret;
}

int DdiVpTools::VpParseLogConfig(
    const char *env,
    char       *envValue)
{
    char *token = nullptr, *value = nullptr, *saveptr = nullptr;
    char oneline[1024] = {};
    FILE *fp           = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(env, "nullptr env.", 1);

    fp = fopen("/etc/vpddi.conf", "r");
    while (fp && (fgets(oneline, 1024, fp) != nullptr))
    {
        if (strlen(oneline) == 1)
        {
            continue;
        }

        token = strtok_r(oneline, "=\n", &saveptr);
        value = strtok_r(nullptr, "=\n", &saveptr);

        if (nullptr == token || nullptr == value)
        {
            continue;
        }

        if (strcmp(token, env) == 0)
        {
            if (envValue)
            {
                strncpy(envValue, value, 1024);
            }

            fclose(fp);
            fp = nullptr;
            return 0;
        }
    }
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    // no setting in config file, use env setting
    value = getenv(env);
    if (value)
    {
        if (envValue)
        {
            strncpy(envValue, value, 1024);
        }

        return 0;
    }

    return 1;
}

void DdiVpTools::VpDumpDeinterlacingParameterBuffer(
    FILE                                      *fpLog,
    VAProcFilterParameterBufferDeinterlacing  *deint)
{
    DDI_VP_FUNC_ENTER;
    if (deint && fpLog)
    {
        fprintf(fpLog, "\t    type = %d\n",      deint->type);
        fprintf(fpLog, "\t    algorithm = %d\n", deint->algorithm);
        fprintf(fpLog, "\t    flags = %d\n",     deint->flags);
    }
    return;
}

void DdiVpTools::VpDumpColorBalanceParameterBuffer(
    FILE                                     *fpLog,
    VAProcFilterParameterBufferColorBalance  *colorbalance,
    uint32_t                                 elementNum)
{
    DDI_VP_FUNC_ENTER;
    for (uint32_t i = 0; i < elementNum; i++)
    {
        if (fpLog && colorbalance)
        {
            fprintf(fpLog, "\t    type = %d\n",   colorbalance[i].type);
            fprintf(fpLog, "\t    attrib = %d\n", colorbalance[i].attrib);
            fprintf(fpLog, "\t    value = %f\n",  colorbalance[i].value);
        }
    }
    return;
}

void DdiVpTools::VpDumpTotalColorCorrectionParameterBuffer(
    FILE                                             *fpLog,
    VAProcFilterParameterBufferTotalColorCorrection  *filterParam,
    uint32_t                                         elementNum)
{
    DDI_VP_FUNC_ENTER;
    for (uint32_t i = 0; i < elementNum; i++)
    {
        if (fpLog && filterParam)
        {
            fprintf(fpLog, "\t    type = %d\n",   filterParam[i].type);
            fprintf(fpLog, "\t    attrib = %d\n", filterParam[i].attrib);
            fprintf(fpLog, "\t    value = %f\n",  filterParam[i].value);
        }
    }
    return;
}

void DdiVpTools::VpDumpFilterParameterBuffer(
    FILE                            *fpLog,
    VAProcFilterParameterBuffer     *buffer)
{
    DDI_VP_FUNC_ENTER;
    if (fpLog && buffer)
    {
        fprintf(fpLog, "\t    type = %d\n",  buffer->type);
        fprintf(fpLog, "\t    value = %f\n", buffer->value);
    }
    return;
}

void DdiVpTools::VpDumpProcFiltersParameterBufferSurface(
    FILE                          *fpLog,
    VAProcPipelineParameterBuffer *pipelineParameterBuffer,
    MOS_FORMAT                    srcFormat)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(fpLog, "nullptr fpLog", );
    DDI_VP_CHK_NULL(pipelineParameterBuffer, "nullptr pipelineParameterBuffer", );

    fprintf(fpLog, "\t--VAProcPipelineParameterBuffer\n");
    fprintf(fpLog, "\t  surface = 0x%08x\n", pipelineParameterBuffer->surface);
    fprintf(fpLog, "\t    Format = %d\n", (int32_t)(srcFormat));

    if (pipelineParameterBuffer->surface_region)
    {
        fprintf(fpLog, "\t  surface_region\n");
        fprintf(fpLog, "\t    x = %d\n", pipelineParameterBuffer->surface_region->x);
        fprintf(fpLog, "\t    y = %d\n", pipelineParameterBuffer->surface_region->y);
        fprintf(fpLog, "\t    width = %d\n", pipelineParameterBuffer->surface_region->width);
        fprintf(fpLog, "\t    height = %d\n", pipelineParameterBuffer->surface_region->height);
    }

    fprintf(fpLog, "\t  surface_color_standard = %d\n", pipelineParameterBuffer->surface_color_standard);

    if (pipelineParameterBuffer->output_region)
    {
        fprintf(fpLog, "\t  output_region\n");
        fprintf(fpLog, "\t    x = %d\n", pipelineParameterBuffer->output_region->x);
        fprintf(fpLog, "\t    y = %d\n", pipelineParameterBuffer->output_region->y);
        fprintf(fpLog, "\t    width = %d\n", pipelineParameterBuffer->output_region->width);
        fprintf(fpLog, "\t    height = %d\n", pipelineParameterBuffer->output_region->height);
    }

    fprintf(fpLog, "\t  output_background_color = 0x%08x\n", pipelineParameterBuffer->output_background_color);
    fprintf(fpLog, "\t  output_color_standard = %d\n", pipelineParameterBuffer->output_color_standard);
    fprintf(fpLog, "\t  pipeline_flags = 0x%08x\n", pipelineParameterBuffer->pipeline_flags);
    fprintf(fpLog, "\t  filter_flags = 0x%08x\n", pipelineParameterBuffer->filter_flags);
    return;
}

void DdiVpTools::VpDumpProcFiltersParameterBuffer(
    FILE             *fpLog,
    VADriverContextP vaDrvCtx,
    VABufferID       *filters,
    unsigned int     filtersNum)
{
    VABufferType       type         = VAPictureParameterBufferType;
    unsigned int       size         = 0;
    unsigned int       elementsNums = 0;
    PDDI_MEDIA_CONTEXT mediaCtx     = nullptr;
    void               *data        = nullptr;
    PDDI_MEDIA_BUFFER  filterBuf    = nullptr;
    VAProcFilterParameterBufferBase *filterParam = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(fpLog, "nullptr fpLog", );
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx", );

    fprintf(fpLog, "\t  filtersNum = %d\n", filtersNum);
    fprintf(fpLog, "\t  filters = %p\n", filters);
    DDI_VP_CHK_NULL(filters, "nullptr filters", );

    if (filtersNum == 0)
    {
        return;
    }

    mediaCtx = GetMediaContext(vaDrvCtx);
    // get buffer type information
    for (int i = 0; i < filtersNum; i++)
    {
        VABufferID filter = filters[i];
        filterBuf         = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, filter);

        // Map Buffer data to virtual addres space
        MediaLibvaInterfaceNext::MapBuffer(vaDrvCtx, filter, &data);

        filterParam = (VAProcFilterParameterBufferBase *)data;
        if (nullptr != filterBuf)
        {
            fprintf(fpLog, "\t  num_elements = %d\n", filterBuf->uiNumElements);
        }
        DDI_VP_CHK_NULL(filterParam, "nullptr filterParam", );

        switch (filterParam->type)
        {
            case VAProcFilterDeinterlacing:
                VpDumpDeinterlacingParameterBuffer(fpLog, (VAProcFilterParameterBufferDeinterlacing *)data);
                break;

            case VAProcFilterColorBalance:
                if (nullptr != filterBuf)
                {
                    VpDumpColorBalanceParameterBuffer(fpLog, (VAProcFilterParameterBufferColorBalance *)data, filterBuf->uiNumElements);
                }
                break;

            case VAProcFilterTotalColorCorrection:
                if (nullptr != filterBuf)
                {
                    VpDumpTotalColorCorrectionParameterBuffer(fpLog, (VAProcFilterParameterBufferTotalColorCorrection *)data, filterBuf->uiNumElements);
                }
                break;

            case VAProcFilterNoiseReduction:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)data);
                break;

            case VAProcFilterSharpening:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)data);
                break;

            case VAProcFilterSkinToneEnhancement:
                VpDumpFilterParameterBuffer(fpLog, (VAProcFilterParameterBuffer *)data);
                break;

            default:
                fprintf(fpLog, "\t    type = %d\n", filterParam->type);
                break;
        }
    }
    return;
}

void DdiVpTools::VpDumpProcFiltersParameterBufferFWDReference(
    FILE        *fpLog,
    VASurfaceID *forwardReferences,
    uint32_t    forwardReferencesNum)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(fpLog, "nullptr fpLog", );

    fprintf(fpLog, "\t  num_forward_references = 0x%08x\n", forwardReferencesNum);

    if (forwardReferencesNum)
    {
        fprintf(fpLog, "\t  forward_references\n");

        if (forwardReferences)
        {
            /* only dump the first 5 forward references */
            for (int32_t i = 0; i < forwardReferencesNum && i < 5; i++)
            {
                fprintf(fpLog, "\t    forward_references[%d] = 0x%08x\n", i, forwardReferences[i]);
            }
        }
        else
        {
            for (int32_t i = 0; i < forwardReferencesNum && i < 5; i++)
            {
                fprintf(fpLog, "\t    forward_references[%d] = (nullptr)\n", i);
            }
        }
    }
    return;
}

void DdiVpTools::VpDumpProcFiltersParameterBufferBKWReference(
    FILE        *fpLog,
    VASurfaceID *backwardReferences,
    uint32_t    backwardReferencesNum)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(fpLog, "nullptr fpLog", );

    fprintf(fpLog, "\t  num_backward_references = 0x%08x\n", backwardReferencesNum);

    if (backwardReferencesNum)
    {
        fprintf(fpLog, "\t  backward_references\n");

        if (backwardReferences)
        {
            /* only dump the first 5 backward references */
            for (int32_t i = 0; i < backwardReferencesNum && i < 5; i++)
            {
                fprintf(fpLog, "\t    backward_references[%d] = 0x%08x\n", i, backwardReferences[i]);
            }
        }
        else
        {
            for (int32_t i = 0; i < backwardReferencesNum && i < 5; i++)
            {
                fprintf(fpLog, "\t    backward_references[%d] = (nullptr)\n", i);
            }
        }
    }
    return;
}

VAStatus DdiVpTools::VpDumpProcPipelineParams(
    VADriverContextP vaDrvCtx,
    PDDI_VP_CONTEXT  vpCtx)
{
    PDDI_MEDIA_CONTEXT    mediaCtx        = nullptr;
    FILE                  *fpLog          = nullptr;
    long                  suffix          = 0;
    struct timeval        time            = {};
    VPHAL_RENDER_PARAMS   *renderParams   = nullptr;
    PDDI_MEDIA_SURFACE    mediaSrcSurf    = nullptr;
    bool                  dumpToFile      = false;
    PDDI_VP_DUMP_PARAM    curDDIParam     = nullptr;
    PDDI_VP_DUMP_PARAM    preDDIParam     = nullptr;
    char                  env_value[1024] = {};
    VAProcPipelineParameterBuffer *pipelineParameterBuffer = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx->pVpHalRenderParams, "nullptr Render Params.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpCtx->pCurVpDumpDDIParam, "nullptr Dump Params.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpCtx->pPreVpDumpDDIParam, "nullptr Dump Params.", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (uint32_t index = 0; index < vpCtx->pVpHalRenderParams->uDstCount; index++)
    {
        vpCtx->pCurVpDumpDDIParam->TargetFormat[index] = vpCtx->pVpHalRenderParams->pTarget[index]->Format;
    }

    curDDIParam = vpCtx->pCurVpDumpDDIParam;
    preDDIParam = vpCtx->pPreVpDumpDDIParam;

    // If current ddi param is same with the previous, don't dump into file.
    dumpToFile = VpCmpDDIDumpParam(curDDIParam, preDDIParam);
    if (dumpToFile)
    {
        return VA_STATUS_SUCCESS;
    }

    mediaCtx     = GetMediaContext(vaDrvCtx);
    renderParams = vpCtx->pVpHalRenderParams;

    gettimeofday(&time, nullptr);
    suffix = ((long)time.tv_sec) * 1000 + (long)time.tv_usec / 1000;

    if (vpCtx->fpDumpFile == nullptr)
    {
        const char *traceName = "VPDDI_TRACE";
        if (VpParseLogConfig(traceName, &env_value[0]) == 0)
        {
            int32_t tmp  = strnlen(env_value, sizeof(env_value));
            int32_t left = sizeof(env_value) - tmp;
            snprintf(env_value + tmp, left, "%ld", suffix);
            MosUtilities::MosSecureFileOpen(&vpCtx->fpDumpFile, env_value, "w");
        }
    }
    fpLog = vpCtx->fpDumpFile;
    DDI_VP_CHK_NULL(fpLog, "nullptr fpLog", VA_STATUS_SUCCESS);

    fprintf(fpLog, "\t-------------------------------\n");
    fprintf(fpLog, "\tTargetFormat Count = %d\n", renderParams->uDstCount);
    for (uint32_t dstIndex = 0; dstIndex < renderParams->uDstCount; dstIndex++)
    {
        fprintf(fpLog, "\tTargetFormat = %d\n", (int32_t)(curDDIParam->TargetFormat[dstIndex]));
    }
    for (uint32_t surfIndex = 0; surfIndex < renderParams->uSrcCount; surfIndex++)
    {
        pipelineParameterBuffer = curDDIParam->pPipelineParamBuffers[surfIndex];
        if (pipelineParameterBuffer)
        {
            VpDumpProcFiltersParameterBufferSurface(fpLog, pipelineParameterBuffer, curDDIParam->SrcFormat[surfIndex]);

            VpDumpProcFiltersParameterBuffer(fpLog, vaDrvCtx, pipelineParameterBuffer->filters, pipelineParameterBuffer->num_filters);

            VpDumpProcFiltersParameterBufferFWDReference(fpLog, pipelineParameterBuffer->forward_references, pipelineParameterBuffer->num_forward_references);

            VpDumpProcFiltersParameterBufferBKWReference(fpLog, pipelineParameterBuffer->backward_references, pipelineParameterBuffer->num_backward_references);

            fprintf(fpLog, "\t  rotation_state = %d\n", pipelineParameterBuffer->rotation_state);
            if (pipelineParameterBuffer->blend_state)
            {
                fprintf(fpLog, "\t  blend_state\n");
                fprintf(fpLog, "\t    flags = %d\n", pipelineParameterBuffer->blend_state->flags);
                fprintf(fpLog, "\t    global_alpha = %f\n", pipelineParameterBuffer->blend_state->global_alpha);
                fprintf(fpLog, "\t    min_luma = %f\n", pipelineParameterBuffer->blend_state->min_luma);
                fprintf(fpLog, "\t    max_luma = %f\n", pipelineParameterBuffer->blend_state->max_luma);
            }

            fprintf(fpLog, "\t  mirror_state = %d\n", pipelineParameterBuffer->mirror_state);
            //fprintf(fpLog, "\t  additional_outputs = 0x%08x\n", (unsigned int32_t)(*(p->additional_outputs)));
            fprintf(fpLog, "\t  num_additional_outputs = %d\n", pipelineParameterBuffer->num_additional_outputs);
#if (VA_MAJOR_VERSION < 1)
            fprintf(fpLog, "\t  chroma_siting_flag = %d\n", pipelineParameterBuffer->input_surface_flag & 0x3);
#else
            fprintf(fpLog, "\t  chroma_siting_flag = %d\n", pipelineParameterBuffer->input_color_properties.chroma_sample_location & 0x3);
#endif
        }
    }

    MOS_SecureMemcpy(preDDIParam->SrcFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_SOURCES, curDDIParam->SrcFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_SOURCES);
    MOS_SecureMemcpy(preDDIParam->TargetFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_TARGETS, curDDIParam->TargetFormat, sizeof(MOS_FORMAT) * VPHAL_MAX_TARGETS);
    for (uint32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
    {
        MOS_SecureMemcpy(preDDIParam->pPipelineParamBuffers[surfIndex], sizeof(VAProcPipelineParameterBuffer), curDDIParam->pPipelineParamBuffers[surfIndex], sizeof(VAProcPipelineParameterBuffer));
    }

    return VA_STATUS_SUCCESS;
}
#endif  //(_DEBUG || _RELEASE_INTERNAL)
