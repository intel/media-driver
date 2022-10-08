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
//! \file     vp_oca_defs.h
//! \brief    Definition of structures and functions for debugging VPHAL
//! \details  This file contains the definition of structures and functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#ifndef __VPHAL_DEBUG_DEFS_H__
#define __VPHAL_DEBUG_DEFS_H__

struct VPHAL_OCA_LOG_HEADER
{
    uint32_t size;                          //!< Size of valid data occupied, which is used when filling OCA buffer.
    uint32_t allocSize;                     //!< Size of allocation. Only used for buffer reusing. This value in OCA
                                            //!< buffer is invalid.
};

struct VPHAL_OCA_SURFACE_INFO
{
    MOS_FORMAT              Format;         //!< Surface format
    VPHAL_SURFACE_TYPE      SurfType;       //!< Surface type (context)
    VPHAL_SAMPLE_TYPE       SampleType;     //!< Interlaced/Progressive sample type
    VPHAL_CSPACE            ColorSpace;     //!< Color Space
    VPHAL_SCALING_MODE      ScalingMode;    //!< Scaling Mode
    MOS_TILE_TYPE           TileType;       //!< Tile Type
    uint32_t                dwWidth;        //!< Surface width
    uint32_t                dwHeight;       //!< Surface height
    uint32_t                dwPitch;        //!< Surface pitch
    RECT                    rcSrc;          //!< Source rectangle
    RECT                    rcDst;          //!< Destination rectangle
};

struct VPHAL_OCA_TARGET_INFO
{
    VPHAL_OCA_SURFACE_INFO  surfInfo;

    struct
    {
        bool                bValid;
        VPHAL_HDR_PARAMS    params;
    } HDRParams;
};

struct VPHAL_OCA_SOURCE_INFO
{
    VPHAL_OCA_SURFACE_INFO  surfInfo;
    VPHAL_ROTATION          Rotation;       //!< 0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degree
    int32_t                 iPalette;       //!< Palette Allocation
    VPHAL_PALETTE           PaletteParams;

    struct
    {
        bool                    bValid;
        VPHAL_BLENDING_PARAMS   params;
    } BlendingParams;

    struct
    {
        bool                    bValid;
        VPHAL_LUMAKEY_PARAMS    params;
    } LumaKeyParams;

    struct
    {
        bool                    bValid;
        VPHAL_PROCAMP_PARAMS    params;
    } ProcampParams;

    struct
    {
        bool                    bValid;
        float                   fIEFFactor;
        VPHAL_IEF_PARAMS        params;
    } IEFParams;

    struct
    {
        bool                    bValid;
        VPHAL_DI_PARAMS         params;
    } DIParams;

    struct
    {
        bool                    bValid;
        VPHAL_DENOISE_PARAMS    params;
    } DNParams;

    struct
    {
        bool                    bValid;
        VPHAL_COLORPIPE_PARAMS  params;
    } ColorPipeParams;

    struct
    {
        bool                    bValid;
        uint32_t                uBwdRefCount;
    } BwdRefInfo;

    struct
    {
        bool                    bValid;
        uint32_t                uFwdRefCount;
    } FwdRefInfo;

    struct
    {
        bool                    bValid;
        VPHAL_HDR_PARAMS        params;
    } HDRParams;
};

struct VPHAL_OCA_RENDER_PARAM
{
    VPHAL_OCA_LOG_HEADER  Header;
    MOS_COMPONENT               Component;  //!< DDI component
    int32_t                     FrameID;
    int32_t                     Pid;
    struct
    {
        bool                    bValid;
        VPHAL_COLORFILL_PARAMS  params;
    } ColorFillParams;

    uint32_t uSrcCount;             //!< Number of sources
    uint32_t uSrcCountDumped;       //!< Number of source info to be dumped into OCA buffer.
                                    //!< This value is used to avoid OCA buffer log section overflow.
    uint32_t uDstCount;             //!< Number of targets
    uint32_t uDstCountDumped;       //!< Number of target info to be dumped into OCA buffer.
                                    //!< This value is used to avoid OCA buffer log section overflow.
    // Followed by VPHAL_OCA_SOURCE_INFO list and VPHAL_OCA_TARGET_INFO list.
};
#endif // #ifndef __VPHAL_DEBUG_DEFS_H__