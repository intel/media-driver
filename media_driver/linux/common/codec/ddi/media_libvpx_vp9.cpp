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
//! \file     media_libvpx_vp9.cpp
//! \brief    Defines the VP9 structure/function that is from libvpx
//!

/*
 * This file defines some functions related with vp9 enc, and
 * they are ported from libvpx (https://github.com/webmproject/libvpx/).
 * The original copyright and licence statement as below.
 */

/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the media_libvpx.LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file media_libvpx.PATENTS.  All contributing project authors may
 *  be found in the media_libvpx.AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stdint.h>
#include "media_libva_encoder.h"
#include "media_libvpx_vp9.h"

struct vp9_write_bit_buffer {
    uint8_t *bit_buffer;
    int bit_offset;
};

static
void vp9_wb_write_bit(struct vp9_write_bit_buffer *wb, int bit)
{
    const int off = wb->bit_offset;
    const int p = off / 8;
    const int q = 7 - off % 8;

    if (q == 7)
    {
        wb->bit_buffer[p] = bit << q;
    }
    else
    {
        wb->bit_buffer[p] &= ~(1 << q);
        wb->bit_buffer[p] |= bit << q;
    }
    wb->bit_offset = off + 1;
}

static
void vp9_wb_write_literal(struct vp9_write_bit_buffer *wb, int data, int bits)
{
    int bit;
    for (bit = bits - 1; bit >= 0; bit--)
        vp9_wb_write_bit(wb, (data >> bit) & 1);
}

static
void write_bitdepth_colorspace_sampling(uint32_t codecProfile,
                                        struct vp9_write_bit_buffer *wb)
{

    if (codecProfile >= VP9_PROFILE_2)
    {
        /* Profile 2 can support 10/12 bits */
        /* Currently it is 10 bits */
        vp9_wb_write_literal(wb, 0, 1);
    }

    /* Add the default color-space */
    vp9_wb_write_literal(wb, 0, 3);
    vp9_wb_write_bit(wb, 0);  // 0: [16, 235] (i.e. xvYCC), 1: [0, 255]

    if ((codecProfile == VP9_PROFILE_1) ||
        (codecProfile == VP9_PROFILE_3))
    {
        /* sub_sampling_x/y */
        /* Currently the sub_sampling_x = 0, sub_sampling_y = 0 */
        vp9_wb_write_bit(wb, 0);
        vp9_wb_write_bit(wb, 0);
        vp9_wb_write_bit(wb, 0); // unused
    }
}

#define    MAX_TILE_WIDTH_B64    64
#define    MIN_TILE_WIDTH_B64    4

static int get_min_log2_tile_cols(const int sb_cols)
{
    int min_log2 = 0;

    while ((MAX_TILE_WIDTH_B64 << min_log2) < sb_cols)
        ++min_log2;

    return min_log2;
}

static int get_max_log2_tile_cols(const int sb_cols)
{
    int max_log2 = 1;

    while ((sb_cols >> max_log2) >= MIN_TILE_WIDTH_B64)
        ++max_log2;

    return max_log2 - 1;
}

bool Vp9WriteUncompressHeader(struct _DDI_ENCODE_CONTEXT *ddiEncContext,
                                uint32_t codecProfile,
                                uint8_t  *headerData,
                                uint32_t *headerLen,
                                vp9_header_bitoffset *headerBitoffset)
{
#define    VP9_SYNC_CODE_0    0x49
#define    VP9_SYNC_CODE_1    0x83
#define    VP9_SYNC_CODE_2    0x42

#define    VP9_FRAME_MARKER   0x2

#define    REFS_PER_FRAME     3

#define    REF_FRAMES_LOG2    3
#define    REF_FRAMES         (1 << REF_FRAMES_LOG2)

#define    VP9_KEY_FRAME      0

    if ((ddiEncContext == nullptr) ||
        (headerData == nullptr) ||
        (headerLen == nullptr) ||
        (headerBitoffset == nullptr))
        return false;

    CODEC_VP9_ENCODE_PIC_PARAMS *picParam = (CODEC_VP9_ENCODE_PIC_PARAMS *)ddiEncContext->pPicParams;
    CODEC_VP9_ENCODE_SEGMENT_PARAMS *segParams = (CODEC_VP9_ENCODE_SEGMENT_PARAMS *)ddiEncContext->pVpxSegParams;

    if (picParam == nullptr)
        return false;

    struct vp9_write_bit_buffer *wb, vp9_wb;

    memset(headerBitoffset, 0, sizeof(vp9_header_bitoffset));

    vp9_wb.bit_buffer = (uint8_t *)headerData;
    vp9_wb.bit_offset = 0;
    wb = &vp9_wb;
    vp9_wb_write_literal(wb, VP9_FRAME_MARKER, 2);

    /* Only Profile0/1/2/3 is supported */
    if (codecProfile > VP9_PROFILE_3)
        codecProfile = VP9_PROFILE_0;

    switch(codecProfile)
    {
    case VP9_PROFILE_0:
        //Profile 0
        vp9_wb_write_literal(wb, 0, 2);
        break;
    case VP9_PROFILE_1:
        //Profile 1
        vp9_wb_write_literal(wb, 2, 2);
        break;
    case VP9_PROFILE_2:
        //Profile 2
        vp9_wb_write_literal(wb, 1, 2);
        break;
    case VP9_PROFILE_3:
        vp9_wb_write_literal(wb, 6, 3);
        break;
    default:
        break;
    }

    vp9_wb_write_bit(wb, 0);  // show_existing_frame
    vp9_wb_write_bit(wb, picParam->PicFlags.fields.frame_type);
    vp9_wb_write_bit(wb, picParam->PicFlags.fields.show_frame);
    vp9_wb_write_bit(wb, picParam->PicFlags.fields.error_resilient_mode);

    if (picParam->PicFlags.fields.frame_type == VP9_KEY_FRAME)
    {
        vp9_wb_write_literal(wb, VP9_SYNC_CODE_0, 8);
        vp9_wb_write_literal(wb, VP9_SYNC_CODE_1, 8);
        vp9_wb_write_literal(wb, VP9_SYNC_CODE_2, 8);

        write_bitdepth_colorspace_sampling(codecProfile, wb);

        /* write the encoded frame size */
        vp9_wb_write_literal(wb, picParam->DstFrameWidthMinus1, 16);
        vp9_wb_write_literal(wb, picParam->DstFrameHeightMinus1, 16);
        /* write display size */
        if ((picParam->DstFrameWidthMinus1 != picParam->SrcFrameWidthMinus1) ||
            (picParam->DstFrameHeightMinus1 != picParam->SrcFrameHeightMinus1))
        {
            vp9_wb_write_bit(wb, 1);
            vp9_wb_write_literal(wb, picParam->SrcFrameWidthMinus1, 16);
            vp9_wb_write_literal(wb, picParam->SrcFrameHeightMinus1, 16);
        }
        else
        {
            vp9_wb_write_bit(wb, 0);
        }
    }
    else
    {
        /* for the non-Key frame */
        if (!picParam->PicFlags.fields.show_frame)
        {
            vp9_wb_write_bit(wb, picParam->PicFlags.fields.intra_only);
        }

        if (!picParam->PicFlags.fields.error_resilient_mode)
        {
            vp9_wb_write_literal(wb, picParam->PicFlags.fields.reset_frame_context, 2);
        }

        if (picParam->PicFlags.fields.intra_only)
        {
            vp9_wb_write_literal(wb, VP9_SYNC_CODE_0, 8);
            vp9_wb_write_literal(wb, VP9_SYNC_CODE_1, 8);
            vp9_wb_write_literal(wb, VP9_SYNC_CODE_2, 8);

            /* Add the bit_depth for VP9Profile1/2/3 */
            if (codecProfile)
                write_bitdepth_colorspace_sampling(codecProfile, wb);

            /* write the refreshed_frame_flags */
            vp9_wb_write_literal(wb, picParam->RefFlags.fields.refresh_frame_flags, REF_FRAMES);
            /* write the encoded frame size */
            vp9_wb_write_literal(wb, picParam->DstFrameWidthMinus1, 16);
            vp9_wb_write_literal(wb, picParam->DstFrameHeightMinus1, 16);
            /* write display size */
            if ((picParam->DstFrameWidthMinus1 != picParam->SrcFrameWidthMinus1) ||
                (picParam->DstFrameHeightMinus1 != picParam->SrcFrameHeightMinus1))
            {
                vp9_wb_write_bit(wb, 1);
                vp9_wb_write_literal(wb, picParam->SrcFrameWidthMinus1, 16);
                vp9_wb_write_literal(wb, picParam->SrcFrameHeightMinus1, 16);
            }
            else
            {
                vp9_wb_write_bit(wb, 0);
            }
        }
        else
        {
            /* The refresh_frame_map is  for the next frame so that it can select Last/Godlen/Alt ref_index */
            /*
            if ((picParam->RefFlags.fields.ref_frame_ctrl_l0) & (1 << 0))
                refresh_flags = 1 << picParam->RefFlags.fields.ref_last_idx;
            if ((picParam->RefFlags.fields.ref_frame_ctrl_l0) & (1 << 0))
                refresh_flags = 1 << picParam->RefFlags.fields.ref_last_idx;
            if ((picParam->RefFlags.fields.ref_frame_ctrl_l0) & (1 << 0))
                refresh_flags = 1 << picParam->RefFlags.fields.ref_last_idx;
            */
            vp9_wb_write_literal(wb, picParam->RefFlags.fields.refresh_frame_flags, REF_FRAMES);

            vp9_wb_write_literal(wb, picParam->RefFlags.fields.LastRefIdx, REF_FRAMES_LOG2);
            vp9_wb_write_bit(wb, picParam->RefFlags.fields.LastRefSignBias);
            vp9_wb_write_literal(wb, picParam->RefFlags.fields.GoldenRefIdx, REF_FRAMES_LOG2);
            vp9_wb_write_bit(wb, picParam->RefFlags.fields.GoldenRefSignBias);
            vp9_wb_write_literal(wb, picParam->RefFlags.fields.AltRefIdx, REF_FRAMES_LOG2);
            vp9_wb_write_bit(wb, picParam->RefFlags.fields.AltRefSignBias);

            /* write three bits with zero so that it can parse width/height directly */
            vp9_wb_write_literal(wb, 0, 3);
            vp9_wb_write_literal(wb, picParam->DstFrameWidthMinus1, 16);
            vp9_wb_write_literal(wb, picParam->DstFrameHeightMinus1, 16);

            /* write display size */
            if ((picParam->DstFrameWidthMinus1 != picParam->SrcFrameWidthMinus1) ||
                (picParam->DstFrameHeightMinus1 != picParam->SrcFrameHeightMinus1))
            {
                vp9_wb_write_bit(wb, 1);
                vp9_wb_write_literal(wb, picParam->SrcFrameWidthMinus1, 16);
                vp9_wb_write_literal(wb, picParam->SrcFrameHeightMinus1, 16);
            }
            else
            {
                vp9_wb_write_bit(wb, 0);
            }

            vp9_wb_write_bit(wb, picParam->PicFlags.fields.allow_high_precision_mv);

#define    SWITCHABLE_FILTER    4
#define    FILTER_MASK          3

            if (picParam->PicFlags.fields.mcomp_filter_type == SWITCHABLE_FILTER)
            {
                vp9_wb_write_bit(wb, 1);
            }
            else
            {
                const int filter_to_literal[4] = { 1, 0, 2, 3 };
                uint8_t filter_flag = picParam->PicFlags.fields.mcomp_filter_type;
                filter_flag = filter_flag & FILTER_MASK;
                vp9_wb_write_bit(wb, 0);
                vp9_wb_write_literal(wb, filter_to_literal[filter_flag], 2);
            }
        }
    }

    /* write refresh_frame_context/paralle frame_decoding */
    if (!picParam->PicFlags.fields.error_resilient_mode)
    {
        vp9_wb_write_bit(wb, picParam->PicFlags.fields.refresh_frame_context);
        vp9_wb_write_bit(wb, picParam->PicFlags.fields.frame_parallel_decoding_mode);
    }

    vp9_wb_write_literal(wb, picParam->PicFlags.fields.frame_context_idx, 2);

    /* write loop filter */
    headerBitoffset->bit_offset_lf_level = wb->bit_offset;
    vp9_wb_write_literal(wb, picParam->filter_level, 6);
    vp9_wb_write_literal(wb, picParam->sharpness_level, 3);

    {
        int i, mode_flag;

        vp9_wb_write_bit(wb, 1);
        vp9_wb_write_bit(wb, 1);
        headerBitoffset->bit_offset_ref_lf_delta = wb->bit_offset;
        for (i = 0; i < 4; i++)
        {
            /*
             * This check is skipped to prepare the bit_offset_lf_ref
            if (picParam->LFRefDelta[i] == 0) {
                vp9_wb_write_bit(wb, 0);
                continue;
            }
             */
            vp9_wb_write_bit(wb, 1);
            mode_flag = picParam->LFRefDelta[i];
            if (mode_flag >= 0)
            {
                vp9_wb_write_literal(wb, mode_flag & (0x3F), 6);
                vp9_wb_write_bit(wb, 0);
            }
            else
            {
                mode_flag = -mode_flag;
                vp9_wb_write_literal(wb, mode_flag & (0x3F), 6);
                vp9_wb_write_bit(wb, 1);
            }
        }

        headerBitoffset->bit_offset_mode_lf_delta = wb->bit_offset;
        for (i = 0; i < 2; i++)
        {
            /*
             * This check is skipped to prepare the bit_offset_lf_mode
            if (picParam->LFModeDelta[i] == 0) {
                vp9_wb_write_bit(wb, 0);
                continue;
            }
             */
            vp9_wb_write_bit(wb, 1);
            mode_flag = picParam->LFRefDelta[i];
            if (mode_flag >= 0)
            {
                vp9_wb_write_literal(wb, mode_flag & (0x3F), 6);
                vp9_wb_write_bit(wb, 0);
            }
            else
            {
                mode_flag = -mode_flag;
                vp9_wb_write_literal(wb, mode_flag & (0x3F), 6);
                vp9_wb_write_bit(wb, 1);
            }
        }
    }

    /* write basic quantizer */
    headerBitoffset->bit_offset_qindex = wb->bit_offset;
    vp9_wb_write_literal(wb, picParam->LumaACQIndex, 8);
    if (picParam->LumaDCQIndexDelta)
    {
        int delta_q = picParam->LumaDCQIndexDelta;
        vp9_wb_write_bit(wb, 1);
        vp9_wb_write_literal(wb, abs(delta_q), 4);
        vp9_wb_write_bit(wb, delta_q < 0);
    }
    else
    {
        vp9_wb_write_bit(wb, 0);
    }

    if (picParam->ChromaDCQIndexDelta)
    {
        int delta_q = picParam->ChromaDCQIndexDelta;
        vp9_wb_write_bit(wb, 1);
        vp9_wb_write_literal(wb, abs(delta_q), 4);
        vp9_wb_write_bit(wb, delta_q < 0);
    }
    else
    {
        vp9_wb_write_bit(wb, 0);
    }

    if (picParam->ChromaACQIndexDelta)
    {
        int delta_q = picParam->ChromaACQIndexDelta;
        vp9_wb_write_bit(wb, 1);
        vp9_wb_write_literal(wb, abs(delta_q), 4);
        vp9_wb_write_bit(wb, delta_q < 0);
    }
    else
    {
        vp9_wb_write_bit(wb, 0);
    }

    headerBitoffset->bit_offset_segmentation = wb->bit_offset;
    vp9_wb_write_bit(wb, picParam->PicFlags.fields.segmentation_enabled);
    if (picParam->PicFlags.fields.segmentation_enabled)
    {
        // Segmentation syntax will be filled by HW, need leave dummy here.
        vp9_wb_write_bit(wb, 0);
        vp9_wb_write_bit(wb, 0);
    }
    headerBitoffset->bit_size_segmentation = wb->bit_offset - headerBitoffset->bit_offset_segmentation;

    /* write tile info */
    {
        int sb_cols = (picParam->DstFrameWidthMinus1 + 64) / 64;
        int min_log2_tile_cols, max_log2_tile_cols;
        int col_data;

        /* write tile column info */
        min_log2_tile_cols = get_min_log2_tile_cols(sb_cols);
        max_log2_tile_cols = get_max_log2_tile_cols(sb_cols);

        col_data = picParam->log2_tile_columns - min_log2_tile_cols;
        while (col_data--)
        {
            vp9_wb_write_bit(wb, 1);
        }
        if (picParam->log2_tile_columns < max_log2_tile_cols)
        {
            vp9_wb_write_bit(wb, 0);
        }

        /* write tile row info */
        vp9_wb_write_bit(wb, picParam->log2_tile_rows);
        if (picParam->log2_tile_rows)
        {
            vp9_wb_write_bit(wb, (picParam->log2_tile_rows != 1));
        }
    }

    /* get the bit_offset of the first partition size */
    headerBitoffset->bit_offset_first_partition_size = wb->bit_offset;

    /* reserve the space for writing the first partitions ize */
    vp9_wb_write_literal(wb, 0, 16);

    *headerLen = (wb->bit_offset + 7) / 8;

    return true;
}
