/*
* Copyright (c) 2019, Intel Corporation
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

#include <cm/cm.h>

#define CURBEDATACTR_SIZE 10

const uint Stat_offsets[16]      = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static const ushort masktab[]    = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
static const uint   offset_idx[] = {0,1,2,3,4,5,6,7};

#define NV12 0
#define P010 1
#define P210 2
#define YUY2 3
#define Y210 4
#define ARGB 5
#define NV12_Linear 6

#define         HEVC_HISTORY_BUFFER_ENTRY_SIZE     32
#define HEVC_MT_TASKDATA_BUFFER_ENTRY_SIZE 96

////////////////////CURBE and VME//////////////////////////////////////

extern "C" _GENX_MAIN_ void DS_Convert(
    vector<uint, CURBEDATACTR_SIZE> CURBEData,
    SurfaceIndex PakSurfIndex,
    SurfaceIndex EncSurfIndex,
    SurfaceIndex DS4XSurf,
    SurfaceIndex MBStats,
    SurfaceIndex DS2XSurf,
    SurfaceIndex Hevc_History_Surface_index,
    SurfaceIndex Hevc_History_Sum_Surface_index,
    SurfaceIndex Scratch_Surface_index
    )
{
    ushort h_pos = get_thread_origin_x();
    ushort v_pos = get_thread_origin_y();

    vector_ref<uchar,  4 * CURBEDATACTR_SIZE> CURBEData_U8  = CURBEData.format<uchar>();
    vector_ref<ushort, 2 * CURBEDATACTR_SIZE> CURBEData_U16 = CURBEData.format<ushort>();

    uchar ENCChromaDepth     = CURBEData_U8(2);
    uchar ENCLumaDepth       = CURBEData_U8(3) & 0x7F;
    uchar colorformat        = CURBEData_U8(4);
    vector<ushort, 1>uWidth  = CURBEData_U8.format<ushort>().select<1, 1>(4);
    vector<ushort, 1>uHeight = CURBEData_U8.format<ushort>().select<1, 1>(5);

    uint  height, width;

    uchar bflagConvert = CURBEData_U8(5) & 0x01;
    uchar stage        = (CURBEData_U8(5) >> 1) & 0x7;
    uchar MBStatflag   = (CURBEData_U8(5) >> 4) & 0x1;

    if(stage == 3)
    {
        height = (((uHeight(0) >> 2) + 31) / 32) * 32;
        width  = (((uWidth(0)  >> 2) + 31) / 32) * 32;
    }
    else
    {
        height = uHeight(0);
        width  = uWidth(0);
    }

    uint Flatness_Threshold = CURBEData(3);

    // CSC Coefficients
    ushort CSC0  = CURBEData_U16(8);
    ushort CSC1  = CURBEData_U16(9);
    ushort CSC2  = CURBEData_U16(10);
    ushort CSC3  = CURBEData_U16(11);
    ushort CSC4  = CURBEData_U16(12);
    ushort CSC5  = CURBEData_U16(13);
    ushort CSC6  = CURBEData_U16(14);
    ushort CSC7  = CURBEData_U16(15);
    ushort CSC8  = CURBEData_U16(16);
    ushort CSC9  = CURBEData_U16(17);
    ushort CSC10 = CURBEData_U16(18);
    ushort CSC11 = CURBEData_U16(19);

    // Chroma Siting Location
    uchar chroma_siting_location = CURBEData_U8(6);

    vector<uint,       1> numLCUs;
    matrix<uchar, 32, 32> YPix;

    vector<ushort, 1> replicate;
    replicate[0] = 0;
    ushort h_pos_write = h_pos << 3;
    ushort v_pos_write = v_pos << 3;

    ushort h_pos_read                  = h_pos_write;
    vector<ushort, 1> v_pos_read       = v_pos_write;
    vector<ushort, 1> max_dst_row      = v_pos_write + 8;
    vector<ushort, 1> max_real_dst_row = (height >> 2);

    if (stage == 3)
    {
        max_real_dst_row = uHeight(0) >> 4;
    }

    replicate.merge((max_dst_row - max_real_dst_row), (max_dst_row > max_real_dst_row));

    v_pos_read.merge(((max_real_dst_row << 2) - 4) >> 2, v_pos_write >= max_real_dst_row);
    replicate.merge(7, v_pos_write >= max_real_dst_row);

    int final_h_pos         = h_pos_read << 2;
    int final_h_pos_plus_32 = 2 * final_h_pos + 32;

    int final_v_pos   = v_pos_read(0) << 2;
    int final_v_pos_c = final_v_pos   >> 1 ;

    vector<ushort, 1> replicate2;
    replicate2[0] = 0;

    ushort h_pos_write2 = h_pos << 4;
    ushort v_pos_write2 = v_pos << 4;
    ushort h_pos_read2  = h_pos_write2;

    vector<ushort, 1> v_pos_read2  = v_pos_write2;
    vector<ushort, 1> max_dst_row2 = v_pos_write2 + 16;

    vector<ushort, 1> max_real_dst_row2 = (height >> 1);

    if(stage==3)
    {
        max_real_dst_row2 = uHeight(0) >> 5;
    }

           replicate2.merge((max_dst_row2 - max_real_dst_row2), (max_dst_row2 > max_real_dst_row2));

           v_pos_read2.merge(max_real_dst_row2 - 1, v_pos_write2 >= max_real_dst_row2);
           replicate2.merge(15, v_pos_write2 >= max_real_dst_row2);

    v_pos_read2[0] = v_pos_read2[0] << 1;

    uint  final_h_pos_argb = 0;
    uchar quot = 0, first_index_val = 0, first_index = 0;

    if(bflagConvert)
    {
        final_h_pos_argb = final_h_pos;
        if(final_h_pos == width)
        {
            final_h_pos_argb = final_h_pos - 32;
        }

        quot            = (final_h_pos + 31) / width;
        first_index_val = (final_h_pos + 31) % width;
        first_index     = first_index_val;

        if((first_index_val > 32) && (quot > 0))
        {
            final_h_pos_argb = final_h_pos     - (first_index_val / 32) * 32;
            first_index      = first_index_val - (first_index_val / 32) * 32;

            if(final_h_pos_argb == width)
            {
                final_h_pos_argb = final_h_pos_argb - 32;
            }
        }

        if(colorformat == P010)
        {
            matrix<ushort, 32, 32> YPix_16;
            vector<ushort, 1> max_col      = h_pos_write;
            vector<ushort, 1> max_real_col = width >> 2;
            vector<ushort, 1> replicate_col(0);

            replicate_col.merge(1, (max_col >= max_real_col));
            if(replicate_col[0])
            {
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, 2 * width - 2, final_v_pos, YPix_16.select<32, 1, 4, 1>(0, 0));
                YPix_16.select<32, 1, 4,  1>(0,  4) = YPix_16.select<32, 1, 4, 1>(0, 0);
                YPix_16.select<32, 1, 8,  1>(0,  8) = YPix_16.select<32, 1, 8, 1>(0, 0);
                YPix_16.select<32, 1, 16, 1>(0, 16) = YPix_16.select<32, 1, 16,1>(0, 0);
            }
            else
            {
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, 2 * final_h_pos,     final_v_pos,      YPix_16.select<8, 1, 16, 1>(0,  0));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos_plus_32, final_v_pos,      YPix_16.select<8, 1, 16, 1>(0,  16));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, 2 * final_h_pos,     final_v_pos + 8,  YPix_16.select<8, 1, 16, 1>(8,  0));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos_plus_32, final_v_pos + 8,  YPix_16.select<8, 1, 16, 1>(8,  16));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, 2 * final_h_pos,     final_v_pos + 16, YPix_16.select<8, 1, 16, 1>(16, 0));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos_plus_32, final_v_pos + 16, YPix_16.select<8, 1, 16, 1>(16, 16));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, 2 * final_h_pos,     final_v_pos + 24, YPix_16.select<8, 1, 16, 1>(24, 0));
                read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos_plus_32, final_v_pos + 24, YPix_16.select<8, 1, 16, 1>(24, 16));
            }

            uint shiftval = 16 - ENCLumaDepth;
            uint tempval  = 1 << (shiftval - 1);

            vector<ushort, 32> roundval;
            vector<uchar,  32> roundingenable = 1;

            roundval.merge(tempval, 0, roundingenable);

            vector<ushort, 32> temptemp;
            int cnt;
            for(cnt=0; cnt < 32; cnt++)
            {
                temptemp = YPix_16.row(cnt);
                temptemp.merge(temptemp, 0xFF00, temptemp < 0xFF00);
                YPix.row(cnt) = cm_shr<uchar>(temptemp + tempval, shiftval);
            }

            matrix_ref<ushort, 16, 32> UVPix_16 = YPix_16.select<16, 1, 32, 1>(0, 0);
            matrix<uchar,      16, 32> UVPix;

            read_plane(PakSurfIndex, GENX_SURFACE_UV_PLANE, 2 * final_h_pos,     final_v_pos_c,     UVPix_16.select<8, 1, 16, 1>(0, 0));
            read_plane(PakSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos_plus_32, final_v_pos_c,     UVPix_16.select<8, 1, 16, 1>(0, 16));
            read_plane(PakSurfIndex, GENX_SURFACE_UV_PLANE, 2 * final_h_pos,     final_v_pos_c + 8, UVPix_16.select<8, 1, 16, 1>(8, 0));
            read_plane(PakSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos_plus_32, final_v_pos_c + 8, UVPix_16.select<8, 1, 16, 1>(8, 16));

            shiftval = 16 - ENCChromaDepth;
            tempval  = 1 << (shiftval - 1);
            roundval.merge(tempval, 0, roundingenable);

            for(cnt = 0; cnt < 16; cnt++)
            {
                temptemp = UVPix_16.row(cnt);
                temptemp.merge(temptemp, 0xFF00, temptemp < 0xFF00);
                UVPix.row(cnt) = cm_shr<uchar>(temptemp + tempval, shiftval);
            }

            write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, final_v_pos_c,     UVPix.select<8, 1, 32, 1>(0, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, final_v_pos_c + 8, UVPix.select<8, 1, 32, 1>(8, 0));

            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos,      YPix.select<8, 1, 32, 1>(0, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 8,  YPix.select<8, 1, 32, 1>(8, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 16, YPix.select<8, 1, 32, 1>(16,0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 24, YPix.select<8, 1, 32, 1>(24,0));

            if (!stage)
            {
                cm_fence();
                return;
            }
        }
        else if (colorformat == Y210)
        {
            matrix<ushort, 8, 32> YUV_Data16_10bit;
            matrix<uchar,  8, 32> UVPix_422;

            read (PakSurfIndex, 4 * final_h_pos_argb,      final_v_pos, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 32, final_v_pos, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);

            YPix.select<8, 1, 16, 1>(0, 0)      = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8, 1, 16, 1>(0, 0) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            read (PakSurfIndex, 4 * final_h_pos_argb+64, final_v_pos, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb+96, final_v_pos, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(0, 16)  = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8,1,16,1>(0,16) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            if((final_v_pos + 8) <= height)
            {
                write(EncSurfIndex, final_h_pos, final_v_pos, YPix.select<8, 1, 32, 1>(0, 0));
            }
            write(EncSurfIndex, final_h_pos, height + final_v_pos, UVPix_422 );

            read (PakSurfIndex, 4 * final_h_pos_argb,      final_v_pos + 8, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 32, final_v_pos + 8, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(8, 0)   = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8,1,16,1>(0, 0) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            read (PakSurfIndex, 4 * final_h_pos_argb + 64, final_v_pos + 8, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 96, final_v_pos + 8, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(8, 16)   = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8,1,16,1>(0, 16) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            if( (final_v_pos + 16) <= height)
            {
                write(EncSurfIndex, final_h_pos, final_v_pos + 8, YPix.select<8, 1, 32, 1>(8, 0) );
            }
            write(EncSurfIndex, final_h_pos, height + final_v_pos + 8, UVPix_422 );

            read (PakSurfIndex, 4 * final_h_pos_argb,      final_v_pos + 16, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 32, final_v_pos + 16, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(16, 0)     = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8, 1, 16, 1>(0, 0) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            read (PakSurfIndex, 4 * final_h_pos_argb + 64, final_v_pos+16, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 96, final_v_pos+16, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(16, 16)  = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8,1,16,1>(0, 16) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            if( (final_v_pos + 24) <= height)
            {
                write(EncSurfIndex, final_h_pos, final_v_pos+16, YPix.select<8, 1, 32, 1>(16, 0) );
            }
            write(EncSurfIndex, final_h_pos, height + final_v_pos + 16, UVPix_422 );

            read (PakSurfIndex, 4 * final_h_pos_argb,      final_v_pos + 24, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 32, final_v_pos + 24, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(24, 0)     = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8, 1, 16, 1>(0, 0) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            read (PakSurfIndex, 4 * final_h_pos_argb + 64, final_v_pos + 24, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 0));
            read (PakSurfIndex, 4 * final_h_pos_argb + 96, final_v_pos + 24, YUV_Data16_10bit.select<8, 1, 16, 1>(0, 16));

            YUV_Data16_10bit.merge(YUV_Data16_10bit, 0xFF00, YUV_Data16_10bit < 0xFF00);
            YPix.select<8, 1, 16, 1>(24, 16)     = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 0) + 0x80, 8);
            UVPix_422.select<8, 1, 16, 1>(0, 16) = cm_shr<uchar>(YUV_Data16_10bit.select<8, 1, 16, 2>(0, 1) + 0x80, 8);

            if( (final_v_pos + 32) <= height)
            {
                write(EncSurfIndex, final_h_pos, final_v_pos + 24, YPix.select<8, 1, 32, 1>(24, 0) );
            }
            write(EncSurfIndex, final_h_pos, height + final_v_pos+24, UVPix_422 );
        }
        else if(colorformat == ARGB)
        {
            matrix<uchar, 16,    32> ARGB_Data;
            matrix_ref<uchar, 8, 32> c1 = ARGB_Data.select<8,1,32,1>(0, 0);
            matrix<uchar, 16, 32> r;
            matrix<uchar, 16, 32> g;
            matrix<uchar, 16, 32> b;

            for(uint i = 0; i < 2; i++)
            {
                #pragma unroll
                for( int j = 0; j < 4;j++)
                {
                    read(PakSurfIndex, 4 * final_h_pos_argb + 32 * j, final_v_pos + i * 16, c1);
                    r.select<8, 1, 8, 1>(0, 8 * j) = c1.select<8, 1, 8, 4>(0, 2);
                    g.select<8, 1, 8, 1>(0, 8 * j) = c1.select<8, 1, 8, 4>(0, 1);
                    b.select<8, 1, 8, 1>(0, 8 * j) = c1.select<8, 1, 8, 4>(0, 0);
                }

                #pragma unroll
                for( int j = 0; j < 4;j++)
                {
                    read(PakSurfIndex, 4 * final_h_pos_argb + 32 * j, final_v_pos + i * 16 + 8,         c1);
                    r.select<8,1,8,1>(8,8*j) = c1.select<8, 1, 8, 4>(0, 2);
                    g.select<8,1,8,1>(8,8*j) = c1.select<8, 1, 8, 4>(0, 1);
                    b.select<8,1,8,1>(8,8*j) = c1.select<8, 1, 8, 4>(0, 0);
                }

                if(quot > 0)
                {
                    vector<uchar,  32> replicants;
                    vector<ushort, 32> masktable = 1;

                    vector<ushort, 32> final_h_pos_mask(masktab);
                    final_h_pos_mask = final_h_pos_mask + final_h_pos;

                    masktable.merge(masktable, 0, final_h_pos_mask < width);

                    for(uint mcount=0;mcount<16;mcount++)
                    {
                        replicants = r(mcount,first_index);
                        r.row(mcount).merge(r.row(mcount), replicants, masktable);

                        replicants = g(mcount,first_index);
                        g.row(mcount).merge(g.row(mcount), replicants, masktable);

                        replicants = b(mcount,first_index);
                        b.row(mcount).merge(b.row(mcount), replicants, masktable);
                    }
                 }

                vector<uint, 32> temp;
                matrix_ref<uchar, 16, 32> v = YPix.select<16, 1, 32, 1>(16, 0).format<uchar, 16, 32>();

                #pragma unroll
                for(uint k = 0; k < 16; k++)
                {
                    temp = g.row(k) * CSC8;
                    temp = temp + r.row(k) * CSC10;

                    temp = (temp + b.row(k) * CSC9) >> 7;
                    ARGB_Data.row(k) = temp.select<32,1>(0) + CSC11;
                }

                #pragma unroll
                for(uint k=0; k<16; k++)
                {
                    temp     = r.row(k) * CSC2;
                    temp     = temp  + g.row(k) * CSC0;
                    temp     = (temp + b.row(k) * CSC1) >> 7;
                    v.row(k) = temp.select<32,1>(0) + CSC3;
                }

                matrix<ushort, 4, 16> first_sum;
                if (chroma_siting_location == 1)
                {

                    #pragma unroll
                    for(uint k = 0; k < 2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0, 0)      = ARGB_Data.select<4, 2, 16, 2>(8 * k, 0)  + ARGB_Data.select<4, 2, 16, 2>(8 * k, 1);
                        first_sum.select<4, 1, 16, 1>(0, 0)     += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 0)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }

                    #pragma unroll
                    for(uint k=0; k<2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0, 0)      = v.select<4, 2, 16, 2>(8 * k, 0)  + v.select<4, 2, 16, 2>(8 * k, 1);
                        first_sum.select<4, 1, 16, 1>(0, 0)     += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 1)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }
                }
                else if (chroma_siting_location == 2)
                {
                    #pragma unroll
                    for(uint k = 0; k < 2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0, 0)      = ARGB_Data.select<4, 2, 16, 2>(8 * k + 1, 0)  + ARGB_Data.select<4, 2, 16, 2>(8 * k + 1, 1);
                        first_sum.select<4, 1, 16, 1>(0, 0)     += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 0)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }

                    #pragma unroll
                    for(uint k=0; k<2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0, 0)      = v.select<4, 2, 16, 2>(8 * k + 1, 0)  + v.select<4, 2, 16, 2>(8 * k + 1, 1);
                        first_sum.select<4, 1, 16, 1>(0, 0)     += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 1)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }
                }
                else if (chroma_siting_location == 3)
                {
                    ARGB_Data.select<8, 1, 16, 2>(0, 0)  = ARGB_Data.select<8, 2, 16, 2>(0, 0);
                    ARGB_Data.select<8, 1, 16, 2>(0, 1)  = v.select<8, 2, 16, 2>(0, 0);
                }
                else if (chroma_siting_location == 4)
                {
                    #pragma unroll
                    for(uint k=0; k<2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0,     0)  = ARGB_Data.select<4, 2, 16, 2>(8 * k, 0) + ARGB_Data.select<4, 2, 16, 2>(8 * k + 1, 0);
                        first_sum.select<4, 1, 16, 1>(0,     0) += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 0)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }

                    #pragma unroll
                    for(uint k=0; k<2; k++)
                    {
                        first_sum.select<4, 1, 16, 1>(0, 0)      = v.select<4, 2, 16, 2>(8 * k, 0) + v.select<4, 2, 16, 2>(8 * k + 1, 0);
                        first_sum.select<4, 1, 16, 1>(0, 0)     += 1;
                        ARGB_Data.select<4, 1, 16, 2>(4 * k, 1)  = first_sum.select<4, 1, 16, 1>(0, 0) >> 1;
                    }
                }
                else if (chroma_siting_location == 5)
                {
                    ARGB_Data.select<8, 1, 16, 2>(0, 0)  = ARGB_Data.select<8, 2, 16, 2>(1, 0);
                    ARGB_Data.select<8, 1, 16, 2>(0, 1)  = v.select<8, 2, 16, 2>(1, 0);
                }
                else
                {

                    #pragma unroll
                    for(uint k = 0; k < 4; k++)
                    {
                        first_sum = ARGB_Data.select<4, 1, 16, 2>(4 * k, 0) + ARGB_Data.select<4, 1, 16, 2>(4 * k, 1);

                        first_sum.select<2, 1, 16, 1>(0, 0)    = first_sum.select<2, 2, 16, 1>(0, 0) + first_sum.select<2, 2, 16, 1>(1, 0);
                        first_sum.select<2, 1, 16, 1>(0, 0)   += 2;
                        ARGB_Data.select<2, 1, 16, 2>(2*k, 0)  = first_sum.select<2, 1, 16, 1>(0, 0) >> 2;
                    }

                    #pragma unroll
                    for(uint k=0; k<4; k++)
                    {
                        first_sum = v.select<4, 1, 16, 2>(4 * k, 0)  + v.select<4, 1, 16, 2>(4 * k, 1);

                        first_sum.select<2, 1, 16, 1>(0, 0)     = first_sum.select<2, 2, 16, 1>(0, 0) + first_sum.select<2, 2, 16, 1>(1, 0);
                        first_sum.select<2, 1, 16, 1>(0, 0)    += 2;
                        ARGB_Data.select<2, 1, 16, 2>(2 * k, 1) = first_sum.select<2, 1, 16, 1>(0, 0) >> 2;
                    }
                }

                write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, (final_v_pos >> 1) + i * 8, ARGB_Data.select<8, 1, 32, 1>(0, 0));

                #pragma unroll
                for(uint k=0; k < 16; k++)
                {
                    temp   = r.row(k) * CSC6;
                    temp   = temp  + g.row(k) * CSC4;
                    temp   = (temp + b.row(k) * CSC5) >> 7;
                    ARGB_Data.row(k) = temp.select<32,1>(0) + CSC7;
                }

                write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + i*16,     ARGB_Data.select<8, 1, 32, 1>(0, 0));
                write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + i*16 + 8, ARGB_Data.select<8, 1, 32, 1>(8, 0));
                YPix.select<16,1,32,1>(0+i*16,0) = ARGB_Data.select<16,1,32,1>(0,0);
            }

            if (!stage)
            {
                cm_fence();
                return;
            }
        }
        else if (colorformat == YUY2)
        {
            matrix<uchar,  8, 64> YUV_Data8_8bit;
            matrix<ushort, 4, 32>       temp = 0;

            uchar YUY2convertflag = (CURBEData_U8(5) >> 5) & 0x1;

            for(int i=0; i<2; i++)
            {
                read (PakSurfIndex, 2 * final_h_pos_argb,      final_v_pos + i * 16, YUV_Data8_8bit.select<8, 1, 32, 1>(0, 0));
                read (PakSurfIndex, 2 * final_h_pos_argb + 32, final_v_pos + i * 16, YUV_Data8_8bit.select<8, 1, 32, 1>(0, 32));

                YPix.select<8, 1, 32, 1>(i * 16, 0) = YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0);

                if(YUY2convertflag)
                {
                    matrix_ref<uchar, 8, 32> uv_channel = YUV_Data8_8bit.select<8, 1, 32, 2>(0, 1);

                    temp.select<4, 1, 32, 1>(0, 0)  = uv_channel.select<4, 2, 32, 1>(0, 0) + uv_channel.select<4, 2, 32, 1>(1, 0);
                    temp.select<4, 1, 32, 1>(0, 0) += 1;
                    temp.select<4, 1, 32, 1>(0, 0)  = temp.select<4, 1, 32, 1>(0, 0) >> 1;

                    matrix_ref <uchar, 4, 64> uv_temp = temp.format<uchar, 4, 64>();
                    matrix_ref <uchar, 4, 32>      uv = uv_temp.select<4, 1, 32, 2>(0, 0);

                    if( (final_v_pos+8 + i * 16) <= height)
                    {
                        write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0));
                    }

                    write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, (final_v_pos >> 1) + i * 8, uv.select<4, 1, 32, 1>(0, 0));
                }
                else
                {
                    if( (final_v_pos + 8 + i * 16) <= height)
                    {
                        write(EncSurfIndex, final_h_pos, final_v_pos + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0));
                    }
                    write(EncSurfIndex, final_h_pos, height + final_v_pos + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 1));
                }


                read (PakSurfIndex, 2 * final_h_pos_argb,      final_v_pos + 8 + i * 16, YUV_Data8_8bit.select<8, 1, 32, 1>(0, 0));
                read (PakSurfIndex, 2 * final_h_pos_argb + 32, final_v_pos + 8 + i * 16, YUV_Data8_8bit.select<8, 1, 32, 1>(0, 32));
                YPix.select<8, 1, 32, 1>(8 + i * 16, 0) = YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0);

                if(YUY2convertflag)
                {
                    matrix_ref<uchar, 8, 32> uv_channel = YUV_Data8_8bit.select<8, 1, 32, 2>(0, 1);
                    temp.select<4, 1, 32, 1>(0, 0)  = uv_channel.select<4, 2, 32, 1>(0, 0) + uv_channel.select<4, 2, 32, 1>(1, 0);
                    temp.select<4, 1, 32, 1>(0, 0) += 1;
                    temp.select<4, 1, 32, 1>(0, 0)  = temp.select<4, 1, 32, 1>(0, 0) >> 1;

                    matrix_ref<uchar, 4, 64> uv_temp = temp.format<uchar, 4, 64>();
                    matrix_ref<uchar, 4, 32> uv      = uv_temp.select<4, 1, 32, 2>(0, 0);

                    if ((final_v_pos+16 + i * 16) <= height)
                    {
                        write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 8 + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0) );
                    }
                    write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, (final_v_pos >> 1) + 4 + i * 8, uv.select<4, 1, 32, 1>(0, 0));
                }
                else
                {
                    if ((final_v_pos + 16 + i * 16) <= height)
                    {
                        write(EncSurfIndex, final_h_pos, final_v_pos + 8 + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 0));
                    }
                    write(EncSurfIndex, final_h_pos, height + final_v_pos + 8 + i * 16, YUV_Data8_8bit.select<8, 1, 32, 2>(0, 1));
                }
            }

            if(!stage)
            {
                cm_fence();
                return;
            }
        }
        else if (colorformat == NV12_Linear)
        {
            read(PakSurfIndex, final_h_pos, height + (final_v_pos >> 1),     YPix.select<8,1,32,1>(0, 0));
            read(PakSurfIndex, final_h_pos, height + (final_v_pos >> 1) + 8, YPix.select<8,1,32,1>(8, 0));

            write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, final_v_pos  >> 1,      YPix.select<8,1,32,1>(0, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_UV_PLANE, final_h_pos, (final_v_pos >> 1) + 8, YPix.select<8,1,32,1>(8, 0));

            read(PakSurfIndex, final_h_pos, final_v_pos,      YPix.select<8, 1, 32, 1>(0, 0));
            read(PakSurfIndex, final_h_pos, final_v_pos + 8,  YPix.select<8, 1, 32, 1>(8, 0));
            read(PakSurfIndex, final_h_pos, final_v_pos + 16, YPix.select<8, 1, 32, 1>(16,0));
            read(PakSurfIndex, final_h_pos, final_v_pos + 24, YPix.select<8, 1, 32, 1>(24,0));

            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos,      YPix.select<8, 1, 32, 1>(0, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 8,  YPix.select<8, 1, 32, 1>(8, 0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 16, YPix.select<8, 1, 32, 1>(16,0));
            write_plane(EncSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 24, YPix.select<8, 1, 32, 1>(24,0));
        }

        if( (colorformat == YUY2) || (colorformat == Y210) )
        {
            if(quot > 0)
            {
                vector<ushort, 32> maskgb = masktab;
                first_index = first_index >> 1;

                vector<ushort, 16> replicants = 0;
                vector<ushort, 16> masktable  = 0;

                vector<ushort, 16> final_h_pos_mask  = 0;
                final_h_pos_mask                     = final_h_pos + 2 * maskgb.select<16, 1>();

                matrix_ref<ushort, 32, 16>YPix_2byte = YPix.format<ushort, 32, 16>();

                masktable = 1;
                masktable.merge(masktable, 0, final_h_pos_mask < width);

                for(uint mcount = 0; mcount < 32; mcount++)
                {
                    replicants = YPix_2byte(mcount, first_index);
                    YPix_2byte.row(mcount).merge(YPix_2byte.row(mcount), replicants, masktable);

                    mcount++;
                    replicants = YPix_2byte(mcount, first_index);
                    YPix_2byte.row(mcount).merge(YPix_2byte.row(mcount), replicants, masktable);

                    mcount++;
                    replicants = YPix_2byte(mcount, first_index);
                    YPix_2byte.row(mcount).merge(YPix_2byte.row(mcount), replicants, masktable);

                    mcount++;
                    replicants = YPix_2byte(mcount,first_index);
                    YPix_2byte.row(mcount).merge(YPix_2byte.row(mcount), replicants, masktable);
                }
            }
        }
    }
    else
    {
        read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos,      YPix.select<8, 1, 32, 1>(0, 0));
        read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 8,  YPix.select<8, 1, 32, 1>(8, 0));
        read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 16, YPix.select<8, 1, 32, 1>(16,0));
        read_plane(PakSurfIndex, GENX_SURFACE_Y_PLANE, final_h_pos, final_v_pos + 24, YPix.select<8, 1, 32, 1>(24,0));
    }

    matrix<uchar, 16, 16> YDown2 = 0;
    matrix<ushort, 8, 16> temp1  = 0;
    matrix<ushort, 8, 8>  temp2  = 0;
    matrix<ushort, 4, 4>  temp3  = 0;
    matrix<uchar,  8, 8>  out    = 0;

    #pragma unroll
    for(uint i = 0; i < 2; i++) {
        #pragma unroll
        for(uint j = 0; j< 2; j++)
        {
            temp1.select<8, 1, 16, 1>(0, 0) = YPix.select<8,  2, 16, 1>(i << 4, j << 4) + YPix.select<8, 2, 16, 1>((i << 4) + 1, j << 4 );
            temp2.select<8, 1,  8, 1>(0, 0) = temp1.select<8, 1,  8, 2>(0, 0)           + temp1.select<8, 1, 8, 2>(0, 1);

            temp2.select<8, 1, 8, 1>(0, 0) += 2;
            temp2.select<8, 1, 8, 1>(0, 0)  = temp2.select<8, 1, 8, 1>(0, 0) >> 2;

            if((stage == 1) || (stage == 4))
            {
                YDown2.select<8, 1, 8, 1>(i << 3, j << 3) = temp2.select<8, 1, 8, 1>(0, 0);
            }

            temp2.select<4, 2, 8, 1>(0, 0)  = temp2.select<4, 2, 8, 1>(0, 0) + temp2.select<4, 2, 8, 1>(1, 0);
            temp3.select<4, 1, 4, 1>(0, 0)  = temp2.select<4, 2, 4, 2>(0, 0) + temp2.select<4, 2, 4, 2>(0, 1);

            temp3.select<4, 1, 4, 1>(0, 0) += 2;
            temp3.select<4, 1, 4, 1>(0, 0)  = temp3.select<4, 1, 4, 1>(0, 0) >> 2;

            out.select<4, 1, 4, 1>(i << 2, j << 2) = temp3.select<4, 1, 4, 1>(0, 0);
        }
    }

    if ((!replicate[0]) || (!replicate2[0]))
    {
        if(stage>1)
        {
            write(DS4XSurf, h_pos_write, v_pos_write, out);
        }

        if(stage ==1 || stage == 4)
        {
            write(DS2XSurf, h_pos_write2, v_pos_write2, YDown2);
        }
    }
    else
    {
        matrix<uchar, 1, 8>last_line = out.select<1, 1, 8, 1>(7 - replicate[0], 0);
        vector<int,     16>mask2;
        vector_ref<int,  8>mask = mask2.select<8, 1>();

        #pragma unroll
        for(uint i = 1; i < 8; i++)
        {
            mask = i > 7 - replicate[0];
            out.select<1, 1, 8, 1>(i, 0).merge(last_line, mask);
        }

        if ((stage == 4) || (stage == 1))
        {
            if(replicate2[0] >= 15)
            {
                matrix<uchar, 1, 16> lastline2 = YDown2.row(1);

                for(uint i=0; i<16; i++)
                {
                    YDown2.row(i) = lastline2;
                }
            }
            else
            {
                matrix<uchar, 1, 16> last_line2 = YDown2.select<1, 1, 16, 1>(15 - replicate2[0], 0);

                #pragma unroll
                for(uint i = 1; i < 16; i++)
                {
                    mask2 = i > 15 - replicate2[0];
                    YDown2.select<1, 1, 16, 1>(i, 0).merge(last_line2, mask2 );
                }
            }
            write(DS2XSurf, h_pos_write2, v_pos_write2, YDown2);
        }

        if(stage>1)
            write(DS4XSurf, h_pos_write, v_pos_write, out);
    }

    if(MBStatflag)
    {
        width  = uWidth(0);
        height = uHeight(0);

        matrix<ushort, 32, 32> squared;
        matrix<uint,    2,  2> sum;
        matrix<uint,    2,  2> pixAvg;
        matrix<uint,    2,  2> SumOfSquared;
        matrix_ref<uint,2,  2> VarianceLuma   = SumOfSquared;
        matrix_ref<uint,2,  2> flatness_check = sum;

        squared = YPix * YPix;

        sum[0][0]          = cm_sum<uint>(YPix.select<16, 1, 16, 1>(0, 0), SAT);
        SumOfSquared[0][0] = cm_sum<uint>(squared.select<16, 1, 16, 1>(0, 0), SAT);

        sum[0][1]          = cm_sum<uint>(YPix.select<16, 1, 16, 1>(0, 16), SAT);
        SumOfSquared[0][1] = cm_sum<uint>(squared.select<16, 1, 16, 1>(0, 16), SAT);

        sum[1][0]          = cm_sum<uint>(YPix.select<16, 1, 16, 1>(16, 0), SAT);
        SumOfSquared[1][0] = cm_sum<uint>(squared.select<16, 1, 16, 1>(16, 0), SAT);

        sum[1][1]          = cm_sum<uint>(YPix.select<16, 1, 16, 1>(16, 16), SAT);
        SumOfSquared[1][1] = cm_sum<uint>(squared.select<16, 1, 16, 1>(16, 16), SAT);

        pixAvg = sum >> 8;
        sum   *= sum;
        sum  >>= 8;

        VarianceLuma   = SumOfSquared - sum;
        VarianceLuma >>= 8;

        flatness_check = (VarianceLuma < Flatness_Threshold);

        unsigned short NumMBperRow = width / 16;
        NumMBperRow               += ((width % 16)> 0);

        unsigned int offset_vp  = (2 * 16 * 4 * (v_pos * NumMBperRow + h_pos));
        unsigned int offset2_vp = offset_vp + NumMBperRow * 64;
        vector<uint, 64> writeVProc(0);
        vector<uint, 16> Element_Offset(Stat_offsets);

        offset_vp  >>= 2;
        offset2_vp >>= 2;

        writeVProc.select<2,16>(5)  = flatness_check.row(0);
        writeVProc.select<2,16>(37) = flatness_check.row(1);

        writeVProc.select<2,16>(6)  = VarianceLuma.row(0);
        writeVProc.select<2,16>(38) = VarianceLuma.row(1);

        writeVProc.select<2,16>(11) = pixAvg.row(0);
        writeVProc.select<2,16>(43) = pixAvg.row(1);

        if((h_pos * 32 < width) && (v_pos * 32 < height))
        {
            if((h_pos * 32)+ 16 >= width)
            {
                write(MBStats, offset_vp,      Element_Offset, writeVProc.select<16,1>(0));
            }
            else
            {
                write(MBStats, offset_vp,      Element_Offset, writeVProc.select<16,1>(0));
                write(MBStats, offset_vp + 16, Element_Offset, writeVProc.select<16,1>(16));
            }

            if((v_pos * 32)+16 < height)
            {
                if((h_pos * 32)+ 16 >= width)
                {
                    write(MBStats, offset2_vp, Element_Offset, writeVProc.select<16,1>(32));
                }
                else
                {
                    write(MBStats, offset2_vp,      Element_Offset, writeVProc.select<16,1>(32));
                    write(MBStats, offset2_vp + 16, Element_Offset, writeVProc.select<16,1>(48));
                }
             }
        }
    }

    uchar hevc_enc_history = (CURBEData_U8(5) >> 6) & 0x1;

    if(hevc_enc_history)
    {
        uchar LCUtype = (CURBEData_U8(5) >> 7) & 0x1;

        vector<ushort,     32> clear_buffer;
        vector_ref<ushort, 16> clear_buffer1 = clear_buffer.select<16, 1>(0);
        vector<ushort,     16> History_perLCU;

        uint  offset = 0;
        uchar shift_factor = LCUtype == 0 ? 6 : 5;

        vector_ref<ushort, 1> WidthLCU  = History_perLCU.select<1, 1>(0);
        vector_ref<ushort, 1> HeightLCU = History_perLCU.select<1, 1>(1);

        WidthLCU(0)  = (uWidth(0)  + ((1 << shift_factor) - 1)) >> shift_factor;
        HeightLCU(0) = (uHeight(0) + ((1 << shift_factor) - 1)) >> shift_factor;

        if(h_pos == 0 &&  v_pos == 0)
        {
            clear_buffer = 0;
            numLCUs      = WidthLCU * HeightLCU;

            for(uint i = 0; i < numLCUs(0); i++)
            {
                read(Hevc_History_Surface_index, offset, History_perLCU);
                clear_buffer.select<16, 1>(0) += History_perLCU;
                offset                        += HEVC_HISTORY_BUFFER_ENTRY_SIZE;
            }

            clear_buffer.select<4,1>(4) = History_perLCU.select<4,1>(4);
            clear_buffer.select<8,1>(8) = History_perLCU.select<8,1>(8);

            write(Hevc_History_Sum_Surface_index, 0, clear_buffer);
        }

        if((h_pos == 1) && (v_pos == 0))
        {
            offset        = 0;
            clear_buffer1 = 0;
            numLCUs       = WidthLCU * HeightLCU;

            for(uint i = 0; i < numLCUs(0); i++)
            {
                write(Scratch_Surface_index, offset,      clear_buffer1);
                write(Scratch_Surface_index, offset + 32, clear_buffer1);
                write(Scratch_Surface_index, offset + 64, clear_buffer1);

                offset += 96;
            }
        }
    }

    cm_fence();
}
