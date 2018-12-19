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

#define NUM_OF_DWORDS_PER_MB (16)
#define NUM_OF_BYTES_PER_MB  (64)

const uint Stat_offsets[8]         = {5,6,7,8,9,10,11,12};
const uint Stat_offsets_[8]       = {8,9,10,11,12,13,14,15};
const uint Stat_offsets_2[16] = {5,6,7,8,9,10,11,12,13,14,15,21,22,23,24,25};
const uint Stat_offsets_2_[8] = {24,25,26,27,28,29,30,31};

_GENX_ void
inline check_flatness(uint MBFlatnessThreshold,
                      matrix_ref<uchar,16,32> field_in,
                      matrix_ref<uint,  1, 2> field_flatness,
                      matrix_ref<uint,  1, 2> VarianceLuma,
                      matrix_ref <uint, 1, 2> PixAvg,
                      matrix_ref<uint,  2, 4> VarianceLuma_8x8,
                      matrix_ref <uint, 2, 4> PixAvg_8x8,
                      unsigned short Enable_8x8)
{
    matrix <ushort, 16, 32> squared;
    vector <uint, 32>       temp_sum;
    vector <uint, 32>       temp_SumOfSquared;
    matrix <uint, 1, 2>     sum;
    matrix <uint, 1, 2>     SumOfSquared;
    matrix <uint, 2, 4>     sum_8x8;
    matrix <uint, 2, 4>     SumOfSquared_8x8;

    squared = field_in * field_in;

    if (!Enable_8x8)
    {
        sum(0,0)          = cm_sum<uint>(field_in.select<16,1,16,1>(0,0),  SAT);
        SumOfSquared(0,0) = cm_sum<uint>(squared.select<16,1,16,1>(0,0),   SAT);

        sum(0,1)          = cm_sum<uint>(field_in.select<16,1,16,1>(0,16), SAT);
        SumOfSquared(0,1) = cm_sum<uint>(squared.select<16,1,16,1>(0,16),  SAT);

        PixAvg = sum >> 8;
        sum   *= sum;
        sum  >>= 8;

        VarianceLuma = SumOfSquared - sum;
        VarianceLuma >>= 8;

        field_flatness = (VarianceLuma < MBFlatnessThreshold);
    }
    else
    {
        sum_8x8(0, 0)          = cm_sum<uint>(field_in.select<8, 1, 8, 1>(0, 0) , SAT);
        SumOfSquared_8x8(0, 0) = cm_sum<uint>(squared.select<8,  1, 8, 1>(0, 0) , SAT);
        sum_8x8(0, 1)          = cm_sum<uint>(field_in.select<8, 1, 8, 1>(0, 8) , SAT);
        SumOfSquared_8x8(0, 1) = cm_sum<uint>(squared.select<8,  1, 8, 1>(0, 8) , SAT);
        sum_8x8(0, 2)          = cm_sum<uint>(field_in.select<8, 1, 8, 1>(8, 0) , SAT);
        SumOfSquared_8x8(0, 2) = cm_sum<uint>(squared.select<8,  1, 8, 1>(8, 0) , SAT);
        sum_8x8(0, 3)          = cm_sum<uint>(field_in.select<8, 1, 8, 1>(8, 8) , SAT);
        SumOfSquared_8x8(0, 3) = cm_sum<uint>(squared.select<8,  1, 8, 1>(8, 8) , SAT);

        sum(0, 0)             = cm_sum<uint>(sum_8x8.select<1, 1, 4, 1>(0, 0), SAT);
        SumOfSquared(0, 0)    = cm_sum<uint>(SumOfSquared_8x8.select<1, 1, 4, 1>(0, 0), SAT);
        sum_8x8(1, 0)         = cm_sum<uint>(field_in.select<8, 1, 8, 1>(0, 16), SAT);
        SumOfSquared_8x8(1, 0)= cm_sum<uint>(squared.select<8,  1, 8, 1>(0, 16), SAT);
        sum_8x8(1, 1)         = cm_sum<uint>(field_in.select<8, 1, 8, 1>(0, 24), SAT);
        SumOfSquared_8x8(1, 1)= cm_sum<uint>(squared.select<8,  1, 8, 1>(0, 24), SAT);
        sum_8x8(1, 2)         = cm_sum<uint>(field_in.select<8, 1, 8, 1>(8, 16), SAT);
        SumOfSquared_8x8(1, 2)= cm_sum<uint>(squared.select<8,  1, 8, 1>(8, 16), SAT);
        sum_8x8(1, 3)         = cm_sum<uint>(field_in.select<8, 1, 8, 1>(8, 24), SAT);
        SumOfSquared_8x8(1, 3)= cm_sum<uint>(squared.select<8,  1, 8, 1>(8, 24), SAT);

        sum(0, 1) = cm_sum<uint>(sum_8x8.select<1, 1, 4, 1>(1, 0), SAT);
        SumOfSquared(0, 1) = cm_sum<uint>(SumOfSquared_8x8.select<1, 1, 4, 1>(1, 0), SAT);

        PixAvg = sum >> 8;
        sum   *= sum;
        sum  >>= 8;

        VarianceLuma   = SumOfSquared - sum;
        VarianceLuma >>= 8;
        field_flatness = (VarianceLuma < MBFlatnessThreshold);

        PixAvg_8x8 = sum_8x8 >> 6;
        sum_8x8   *= sum_8x8;
        sum_8x8  >>= 6;

        VarianceLuma_8x8   = SumOfSquared_8x8 - sum_8x8;
        VarianceLuma_8x8 >>= 6;
    }
}

extern "C" _GENX_MAIN_ void hme_frame_downscale(
    unsigned short  width,
    unsigned short  height,
    SurfaceIndex    InputBuf,
    SurfaceIndex    DownscaleBuf,
    vector<uint, 2> reserved1,
    unsigned int    MBFlatnessThreshold,
    unsigned int    Enablers,
    unsigned int    reserved2,
    SurfaceIndex    MB_VProc_Stats)
{
    matrix<uchar, 32, 32> in;
    matrix<uchar,  8,  8> downscale_out;
    matrix<uint,   2,  2> flatness_out;

    matrix<ushort, 16,16> temp1;
    matrix<ushort,  8, 8> temp2;
    matrix<ushort,  4, 4> temp3;
    vector<int,       8>  mask;
    vector<ushort,    1>  replicate;

    matrix<ushort,32,32> squared;
    vector<uint,     32> temp_sum;
    vector<uint,     32> temp_SumOfSquared;
    matrix<uint,   2, 2> sum;
    matrix<uint,   2, 2> pixAvg;
    matrix<uint,   2, 2> SumOfSquared;
    matrix_ref<uint,2, 2>VarianceLuma = SumOfSquared;

    matrix<uint, 4, 4> sum_8x8;
    matrix<uint, 4, 4> pixAvg_8x8;
    matrix<uint, 4, 4> SumOfSquared_8x8;
    matrix_ref<uint, 4, 4>VarianceLuma_8x8 = SumOfSquared_8x8;

    replicate[0] = 0;

    ushort h_pos = get_thread_origin_x();
    ushort v_pos = get_thread_origin_y();

    ushort h_pos_write = h_pos << 3;
    ushort v_pos_write = v_pos << 3;

    ushort h_pos_read            = h_pos_write;
    vector<ushort, 1> v_pos_read = v_pos_write;

    vector<ushort, 1> max_dst_row      = v_pos_write + 8;
    vector<ushort, 1> max_real_dst_row = (height >> 2);

    replicate.merge((max_dst_row - max_real_dst_row), (max_dst_row > max_real_dst_row));

    v_pos_read.merge((max_real_dst_row -1), v_pos_write >= max_real_dst_row);
    replicate.merge(7, v_pos_write >= max_real_dst_row);

    v_pos_read[0] =  v_pos_read[0] <<2;

    read(InputBuf, h_pos_read << 2, v_pos_read[0],      in.select<8,1,32,1>(0, 0));
    read(InputBuf, h_pos_read << 2, v_pos_read[0] + 8,  in.select<8,1,32,1>(8, 0));
    read(InputBuf, h_pos_read << 2, v_pos_read[0] + 16, in.select<8,1,32,1>(16,0));
    read(InputBuf, h_pos_read << 2, v_pos_read[0] + 24, in.select<8,1,32,1>(24,0));

    #pragma unroll
    for(uint i=0; i<2; i++) {
    #pragma unroll
        for(uint j=0; j<2; j++)
        {
            temp1.select<8,2,16,1>(0,0) = in.select<8,2,16,1>(i<<4,j<<4) + in.select<8,2,16,1>( (i<<4)+1, j<<4 );
            temp2.select<8,1, 8,1>(0,0) = temp1.select<8,2,8,2>(0,0)     + temp1.select<8,2,8,2>(0,1);

            temp2.select<8,1,8,1>(0,0) += 2;
            temp2.select<8,1,8,1>(0,0)  = temp2.select<8,1,8,1>(0,0) >> 2;

            temp2.select<4,2,8,1>(0,0)  = temp2.select<4,2,8,1>(0,0) + temp2.select<4,2,8,1>(1,0);
            temp3.select<4,1,4,1>(0,0)  = temp2.select<4,2,4,2>(0,0) + temp2.select<4,2,4,2>(0,1);

            temp3.select<4,1,4,1>(0,0) += 2;
            temp3.select<4,1,4,1>(0,0)  = temp3.select<4,1,4,1>(0,0) >> 2;

            downscale_out.select<4,1,4,1>(i<<2,j<<2) = temp3.select<4,1,4,1>(0,0);
        }
    }

    if ( ! replicate[0] )
    {
        write(DownscaleBuf, h_pos_write, v_pos_write, downscale_out);
    }
    else
    {
        matrix<uchar, 1, 8> last_line = downscale_out.select<1,1,8,1>(7 - replicate[0], 0);

        #pragma unroll
        for(uint i=1; i<8; i++)
        {
            mask = i > 7 - replicate[0];
            downscale_out.select<1,1,8,1>(i, 0).merge(last_line, mask);
        }

        write(DownscaleBuf, h_pos_write, v_pos_write, downscale_out);
    }

    unsigned short EnableMBFlatnessCheck = Enablers & 0x1;
    unsigned short EnableVarianceOut     = Enablers & 0x2;
    unsigned short EnablePixAvgOut       = Enablers & 0x4;
    unsigned short Enable_8x8_Stats      = Enablers & 0x8;

    if (Enablers != 0)
    {
        squared = in * in;

        if (!Enable_8x8_Stats)
        {
            sum(0,0)          = cm_sum<uint>(in.select<16,1,16,1>(0,0), SAT);
            SumOfSquared(0,0) = cm_sum<uint>(squared.select<16,1,16,1>(0,0), SAT);

            sum(0,1)          = cm_sum<uint>(in.select<16, 1, 16, 1>(0, 16), SAT);
            SumOfSquared(0,1) = cm_sum<uint>(squared.select<16, 1, 16, 1>(0, 16), SAT);

            sum(1,0)          = cm_sum<uint>(in.select<16,1,16,1>(16,0), SAT);
            SumOfSquared(1,0) = cm_sum<uint>(squared.select<16,1,16,1>(16,0), SAT);

            sum(1,1)          = cm_sum<uint>(in.select<16,1,16,1>(16,16), SAT);
            SumOfSquared(1,1) = cm_sum<uint>(squared.select<16,1,16,1>(16,16), SAT);

            pixAvg = sum >> 8;
            sum   *= sum;
            sum  >>= 8;

            VarianceLuma   = SumOfSquared - sum;
            VarianceLuma >>= 8;

            flatness_out   = (VarianceLuma < MBFlatnessThreshold);
        }
        else
        {
            sum_8x8(0,0)          = cm_sum<uint>(in.select<8,1,8,1>(0,0) , SAT);
            SumOfSquared_8x8(0,0) = cm_sum<uint>(squared.select<8,1,8,1>(0,0) , SAT);
            sum_8x8(0,1)          = cm_sum<uint>(in.select<8,1,8,1>(0,8) , SAT);
            SumOfSquared_8x8(0,1) = cm_sum<uint>(squared.select<8,1,8,1>(0,8) , SAT);
            sum_8x8(0,2)          = cm_sum<uint>(in.select<8,1,8,1>(8,0) , SAT);
            SumOfSquared_8x8(0,2) = cm_sum<uint>(squared.select<8,1,8,1>(8,0) , SAT);
            sum_8x8(0,3)          = cm_sum<uint>(in.select<8,1,8,1>(8,8) , SAT);
            SumOfSquared_8x8(0,3) = cm_sum<uint>(squared.select<8,1,8,1>(8,8) , SAT);

            sum(0,0)              = cm_sum<uint>(sum_8x8.select<1,1,4,1>(0,0), SAT);
            SumOfSquared(0,0)     = cm_sum<uint>(SumOfSquared_8x8.select<1,1,4,1>(0,0), SAT);

            sum_8x8(1,0)          = cm_sum<uint>(in.select<8,1,8,1>(0,16) , SAT);
            SumOfSquared_8x8(1,0) = cm_sum<uint>(squared.select<8,1,8,1>(0,16) , SAT);
            sum_8x8(1,1)          = cm_sum<uint>(in.select<8,1,8,1>(0,24) , SAT);
            SumOfSquared_8x8(1,1) = cm_sum<uint>(squared.select<8,1,8,1>(0,24) , SAT);
            sum_8x8(1,2)          = cm_sum<uint>(in.select<8,1,8,1>(8,16) , SAT);
            SumOfSquared_8x8(1,2) = cm_sum<uint>(squared.select<8,1,8,1>(8,16) , SAT);
            sum_8x8(1,3)          = cm_sum<uint>(in.select<8,1,8,1>(8,24) , SAT);
            SumOfSquared_8x8(1,3) = cm_sum<uint>(squared.select<8,1,8,1>(8,24) , SAT);

            sum(0,1)              = cm_sum<uint>(sum_8x8.select<1,1,4,1>(1,0), SAT);
            SumOfSquared(0,1)     = cm_sum<uint>(SumOfSquared_8x8.select<1,1,4,1>(1,0), SAT);

            sum_8x8(2,0)          = cm_sum<uint>(in.select<8,1,8,1>(16,0) , SAT);
            SumOfSquared_8x8(2,0) = cm_sum<uint>(squared.select<8,1,8,1>(16,0) , SAT);
            sum_8x8(2,1)          = cm_sum<uint>(in.select<8,1,8,1>(16,8) , SAT);
            SumOfSquared_8x8(2,1) = cm_sum<uint>(squared.select<8,1,8,1>(16,8) , SAT);
            sum_8x8(2,2)          = cm_sum<uint>(in.select<8,1,8,1>(24,0) , SAT);
            SumOfSquared_8x8(2,2) = cm_sum<uint>(squared.select<8,1,8,1>(24,0) , SAT);
            sum_8x8(2,3)          = cm_sum<uint>(in.select<8,1,8,1>(24,8) , SAT);
            SumOfSquared_8x8(2,3) = cm_sum<uint>(squared.select<8,1,8,1>(24,8) , SAT);

            sum(1,0)              = cm_sum<uint>(sum_8x8.select<1,1,4,1>(2,0), SAT);
            SumOfSquared(1,0)     = cm_sum<uint>(SumOfSquared_8x8.select<1,1,4,1>(2,0), SAT);

            sum_8x8(3,0)          = cm_sum<uint>(in.select<8,1,8,1>(16,16) , SAT);
            SumOfSquared_8x8(3,0) = cm_sum<uint>(squared.select<8,1,8,1>(16,16) , SAT);
            sum_8x8(3,1)          = cm_sum<uint>(in.select<8,1,8,1>(16,24) , SAT);
            SumOfSquared_8x8(3,1) = cm_sum<uint>(squared.select<8,1,8,1>(16,24) , SAT);
            sum_8x8(3,2)          = cm_sum<uint>(in.select<8,1,8,1>(24,16) , SAT);
            SumOfSquared_8x8(3,2) = cm_sum<uint>(squared.select<8,1,8,1>(24,16) , SAT);
            sum_8x8(3,3)          = cm_sum<uint>(in.select<8,1,8,1>(24,24) , SAT);
            SumOfSquared_8x8(3,3) = cm_sum<uint>(squared.select<8,1,8,1>(24,24) , SAT);

            sum(1,1)              = cm_sum<uint>(sum_8x8.select<1,1,4,1>(3,0), SAT);
            SumOfSquared(1,1)     = cm_sum<uint>(SumOfSquared_8x8.select<1,1,4,1>(3,0), SAT);

            pixAvg = sum >> 8;
            sum   *= sum;
            sum  >>= 8;

            VarianceLuma   = SumOfSquared - sum;
            VarianceLuma >>= 8;
            flatness_out   = (VarianceLuma < MBFlatnessThreshold);

            pixAvg_8x8 = sum_8x8 >> 6;
            sum_8x8   *= sum_8x8;
            sum_8x8  >>= 6;

            VarianceLuma_8x8   = SumOfSquared_8x8 - sum_8x8;
            VarianceLuma_8x8 >>= 6;
        }

        unsigned short NumMBperRow = width / 16;
        NumMBperRow               += ((width % 16)> 0);

        unsigned int offset_vp  = (2 * NUM_OF_DWORDS_PER_MB * 4 * (v_pos * NumMBperRow + h_pos));
        unsigned int offset2_vp = offset_vp + NumMBperRow * NUM_OF_BYTES_PER_MB;

        vector<uint, 44> writeVProc(0);
        vector<uint,  8> Element_Offset(Stat_offsets);
        vector<uint,  8> Element_Offset_(Stat_offsets_);
        vector<uint, 16> Element_Offset_2(Stat_offsets_2);
        vector<uint,  8> Element_Offset_2_(Stat_offsets_2_);

        offset_vp  >>= 2;
        offset2_vp >>= 2;

        if(EnableMBFlatnessCheck)
        {
            writeVProc.select<4,11>(0) = flatness_out.select_all();
        }

        if(EnableVarianceOut)
        {
            writeVProc.select<2,11>(1)  = VarianceLuma.row(0);
            writeVProc.select<2,11>(23) = VarianceLuma.row(1);

            if(Enable_8x8_Stats)
            {
                writeVProc.select<4,1>(2)  = VarianceLuma_8x8.row(0);
                writeVProc.select<4,1>(13) = VarianceLuma_8x8.row(1);
                writeVProc.select<4,1>(24) = VarianceLuma_8x8.row(2);
                writeVProc.select<4,1>(35) = VarianceLuma_8x8.row(3);
            }
        }

        if(EnablePixAvgOut)
        {
            writeVProc.select<2,11>(6)  = pixAvg.row(0);
            writeVProc.select<2,11>(28) = pixAvg.row(1);

            if(Enable_8x8_Stats)
            {
                writeVProc.select<4,1>(7)  = pixAvg_8x8.row(0);
                writeVProc.select<4,1>(18) = pixAvg_8x8.row(1);
                writeVProc.select<4,1>(29) = pixAvg_8x8.row(2);
                writeVProc.select<4,1>(40) = pixAvg_8x8.row(3);
            }
        }

        if(EnableVarianceOut || EnablePixAvgOut)
        {
            if((h_pos * 32 < width) && (v_pos * 32 < height))
            {
                if((h_pos * 32)+ 16 >= width)
                {
                    write(MB_VProc_Stats, offset_vp, Element_Offset,  writeVProc.select<8,1>(0));
                    write(MB_VProc_Stats, offset_vp, Element_Offset_, writeVProc.select<8,1>(3));
                }
                else
                {
                    write(MB_VProc_Stats, offset_vp, Element_Offset_2,  writeVProc.select<16,1>(0));
                    write(MB_VProc_Stats, offset_vp, Element_Offset_2_, writeVProc.select<8,1>(14));
                }

                if ((v_pos * 32) + 16 < height)
                {
                    if ((h_pos * 32) + 16 >= width)
                    {
                        write(MB_VProc_Stats, offset2_vp, Element_Offset,  writeVProc.select<8, 1>(22));
                        write(MB_VProc_Stats, offset2_vp, Element_Offset_, writeVProc.select<8, 1>(25));
                    }
                    else
                    {
                        write(MB_VProc_Stats, offset2_vp, Element_Offset_2,  writeVProc.select<16, 1>(22));
                        write(MB_VProc_Stats, offset2_vp, Element_Offset_2_, writeVProc.select<8, 1>(36));
                    }
                }
            }
        }
    }
}

extern "C" _GENX_MAIN_ void
hme_frame_downscale2(unsigned short  width,
                     unsigned short  height,
                     vector<uint, 7> reserved,
                     SurfaceIndex    ibuf,
                     SurfaceIndex    obuf)
{
    matrix<uchar,  32, 32> in;
    matrix<ushort, 16, 16> temp1;
    matrix<ushort,  8,  8> temp2;
    matrix<ushort,  4,  4> temp3;
    matrix<uchar,  16, 16> out;
    vector<int,        16> mask;

    vector<ushort, 1> replicate;
    replicate[0] = 0;

    ushort h_pos_write = get_thread_origin_x() << 4;
    ushort v_pos_write = get_thread_origin_y() << 4;
    ushort h_pos_read  = h_pos_write;

    vector<ushort, 1> v_pos_read       = v_pos_write;
    vector<ushort, 1> max_dst_row      = v_pos_write + 16;
    vector<ushort, 1> max_real_dst_row = (height >> 1);

    replicate.merge((max_dst_row - max_real_dst_row), (max_dst_row > max_real_dst_row));

    v_pos_read.merge(max_real_dst_row - 1, v_pos_write >= max_real_dst_row);
    replicate.merge(15, v_pos_write >= max_real_dst_row);

    read(ibuf, h_pos_read << 1, v_pos_read[0]  << 1,       in.select<8, 1, 32, 1>(0, 0));
    read(ibuf, h_pos_read << 1, (v_pos_read[0] << 1) + 8,  in.select<8, 1, 32, 1>(8, 0));
    read(ibuf, h_pos_read << 1, (v_pos_read[0] << 1) + 16, in.select<8, 1, 32, 1>(16,0));
    read(ibuf, h_pos_read << 1, (v_pos_read[0] << 1) + 24, in.select<8, 1, 32, 1>(24,0));

    #pragma unroll
    for (uint i = 0; i<2; i++)
    {
        #pragma unroll
        for (uint j = 0; j<2; j++)
        {
            temp1.select<8, 2,16, 1>(0, 0)  = in.select<8, 2,16, 1>(i << 4, j << 4) + in.select<8, 2, 16, 1>((i << 4) + 1, j << 4);
            temp2.select<8, 1, 8, 1>(0, 0)  = temp1.select<8, 2, 8, 2>(0, 0) + temp1.select<8, 2, 8, 2>(0, 1);

            temp2.select<8, 1, 8, 1>(0, 0) += 2;
            temp2.select<8, 1, 8, 1>(0, 0)  = temp2.select<8, 1, 8, 1>(0, 0) >> 2;

            out.select<8, 1, 8, 1>(i << 3, j << 3) = temp2.select<8, 1, 8, 1>(0, 0);
        }
    }

    if (!replicate[0])
    {
        write(obuf, h_pos_write, v_pos_write, out);
    }
    else
    {
        matrix<uchar, 1, 16> last_line = out.select<1, 1, 16, 1>(15 - replicate[0], 0);

        #pragma unroll
        for (uint i = 1; i < 16; i++)
        {
            mask = i > 15 - replicate[0];
            out.select<1, 1, 16, 1>(i, 0).merge(last_line, mask);
        }

        write(obuf, h_pos_write, v_pos_write, out);
    }
}
