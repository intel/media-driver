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

#define STREAMIN_SIZE       64
#define LCU64_STREAMIN_SIZE (STREAMIN_SIZE * 4)
#define TILEINFO_SIZE       16

#define LCUSIZE32 0
#define LCUSIZE64 1

#define SHIFT_MB_TO_SUB_MB    (2)
#define SHIFT_PIXEL_TO_SUB_MB (2)

// Number of bytes in Curbe data
#define CURBEDATA_SIZE 160

#define BIT5  0x20
#define BIT4  0x10
#define BIT3  0x08

//---------------------------------------------------------------------------
// Binding table indexes
//---------------------------------------------------------------------------
#define NUM_SURFACE_IDX 14
#define UNI              4

//---------------------------------------------------------------------------
// Const vector for Streamin
//---------------------------------------------------------------------------
const char MBX_Indx[16]      = {0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3};
const char MBaddressIndx[16] = {0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3};

const char MBX_Indx_hevc[16]      = {0,1,0,1};
const char MBaddressIndx_hevc[16] = {0,0,1,1};

enum ROISOURCE
{
   ROIMAP_FROM_UNKNOWN          = 0,
   ROIMAP_FROM_APP              = 1,
   ROIMAP_FROM_APP_DIRTYRECT    = 2,
   ROIMAP_FROM_HME_STATICREGION = 3,
   ROIMAP_FROM_PIXELVAR         = 4
};


_GENX_  void
inline HME_SET_REF(vector_ref<uchar, CURBEDATA_SIZE> CURBEData, vector_ref<short, 2> input_refine, vector_ref<short, 2> output_refine, vector_ref<short, 2> pos)
{
    vector<short, 2> maxLens;

    maxLens(0) = 512;
    maxLens(1) = CURBEData.format<short>()[13] >> 2;

    vector<short,  2> in_refine = input_refine >> 2;
    vector<ushort, 2> search    = CURBEData.select<2,1>(22);
    vector<ushort, 2> tmp       = search - 16;
    vector<ushort, 2> widths    = tmp >> 1;

    vector<short,  2> pictureWidths;
    pictureWidths(0) = CURBEData(18) << 4;
    pictureWidths(1) = CURBEData(17) << 4;
    pictureWidths(1) = pictureWidths(1) + 16;

    vector<short, 2> VME_params = in_refine;
    VME_params                  = VME_params - widths;

    output_refine = VME_params + pos;
    output_refine.merge(pos + maxLens - tmp, in_refine + tmp >   maxLens);
    output_refine.merge(pos - maxLens,       in_refine - tmp < (-maxLens));

    output_refine.merge((pictureWidths - 1) & 0xFFFC, output_refine > (pictureWidths - 1));

    vector<short, 2> tmp_search = -search;
    output_refine.merge((5 - search) & 0xFFFC,  output_refine <= tmp_search);
    output_refine = output_refine - pos;
}

// Main logic is in this function. The logic is shared by both P and B versions of the kernel.
_GENX_  void
inline HME( vector<uchar, CURBEDATA_SIZE> CURBEData,
            SurfaceIndex HME_MV_Data_Surface_index,
            SurfaceIndex HME_MV_Input_Data_Surface_index,
            SurfaceIndex DISTORTION_Surface,
            SurfaceIndex BRC_DISTORTION_Surface,
            SurfaceIndex Pred_Surface_L0,
            SurfaceIndex Pred_Surface_L1,
            SurfaceIndex StreamINSurface,
            SurfaceIndex StreamINSurface_input,
            SurfaceIndex SUM_Surface,
            SurfaceIndex TileInfo_Buffer,
            bool b,
            bool vdenc_enable,
            bool is_hevc_vp9_vdenc)
{
    ushort mb_x_pos = get_thread_origin_x();
    ushort mb_y_pos = get_thread_origin_y();

    vector<short, 2> pos;
    ushort x_pos = pos(0) = mb_x_pos << 4;
    ushort y_pos = pos(1) = mb_y_pos << 4;

    bool  useMVPrevStep       = (CURBEData(24) & BIT4) != 0;
    bool  EnableMVSum         = (CURBEData(24) & BIT5) != 0;
    bool  writeDistortions    = (CURBEData(24) & BIT3) != 0;
    uchar prevMVReadPosFactor = CURBEData(60);

    uchar MVShiftFactor       = CURBEData(61);
    uchar picture_heightMB    = CURBEData(17) + 1;
    uchar NumRefIdxL0         = CURBEData(52);
    uchar NumRefIdxL1         = CURBEData(53);
    uchar BRCMVThreshold      = CURBEData(20);

    vector<ushort, 2> ActualMBDim = 0;
    U8 HMEStreaminRefCost;
    U8 ROIMapEnable;

    if (!is_hevc_vp9_vdenc && vdenc_enable)
    {
        ActualMBDim = CURBEData.format<ushort>().select<2,1>(60);
        HMEStreaminRefCost = CURBEData(54);
        ROIMapEnable       = CURBEData(55) >> 5;
    }

    vector<U32, 8> BD_MV_Sum = 0;

    // used for determining whether to calculate refined MV on L0 and L1.
    // If the absulte value of the input (higher level in the hierarchy ) refined MV data is below this threshold,
    // we skip the refine calcualtion
    uchar SUPER_COMBINEDIST = CURBEData(25);

    // calculations on the reference region width and height
    vector<short,  2> ref_region_size    = CURBEData.select<2,1>(22) - 16;
    vector<ushort, 2> search_coordinates = ref_region_size    >> 1;
    vector<short,  2> rxy                = search_coordinates >> 2;

    // data structures used for VME commands:
    matrix<uchar, UNI, 32> UNIInput_tmp;
    matrix<uchar, UNI, 32> UNIInput;
    matrix<uchar,   4, 32> IMEInput;
    matrix<uchar,   4, 32> FBRInput;
    matrix<uchar,   9, 32> IME_output_MV_refine_L0;
    matrix<uchar,   7, 32> VME_ME_REFINE_L0;
    matrix<uchar,   9, 32> IME_output_MV_refine_L1;
    matrix<uchar,   7, 32> VME_ME_REFINE_L1;
    matrix<uchar,   7, 32> IME_output_MV_L0;
    matrix<uchar,   7, 32> VME_ME_L0;
    matrix<uchar,   7, 32> IME_output_MV_L1;
    matrix<uchar,   7, 32> VME_ME_L1;
    vector<short,       2> ref0;
    vector<ushort,     16> costCenter = 0;

    uchar FBRMbMode, FBRSubMbShape, FBRSubPredMode;

    // VME search control
    VMESearchCtrl searchControl = VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START;

    // initialize universal data
    UNIInput_tmp.row(0)                 = 0;
    UNIInput_tmp.format<ushort>()[0, 4] = x_pos;
    UNIInput_tmp.format<ushort>()[0, 5] = y_pos;
    UNIInput_tmp.row(0).format<uint>().select<2, 2>(3) = CURBEData.format<uint>().select<2, 2>(3);
    UNIInput_tmp(0, 20)                                = 0;
    UNIInput_tmp.row(1)                                = 0;
    UNIInput_tmp.row(1).format<uint>().select<2, 1>(0) = CURBEData.format<uint>().select<2, 1>(0);
    UNIInput_tmp.row(1).format<uint>().select<1, 1>(2) = CURBEData.format<uint>().select<1, 1>(2);
    UNIInput_tmp.select<1,1,2,1>(1, 10)                = rxy(0) + (rxy(1) << 4);
    UNIInput_tmp.row(1).format<uint>().select<1,1>(7)  = CURBEData.format<uint>().select<1, 1>(7);
    UNIInput_tmp.row(2).format<uint>()                 = CURBEData.format<uint>().select<8, 1>(8);

    // initialize IMEInput
    IMEInput.row(0).format<uint>()                 = CURBEData.format<uint>().select<8, 1>(16);
    IMEInput.row(1).format<uint>().select<6, 1>(0) = CURBEData.format<uint>().select<6, 1>(24);
    IMEInput.row(1).format<uint>().select<2, 1>(6) = 0;

    uchar current_iteration = 0;
    uint  picture_offset    = 0;

    // indicate whether we need to check L0 and L1 reference frames respectively
    vector<uchar, 1> checkL0 = 0;
    vector<uchar, 1> checkL1 = 0;

    // field_support
    char refFieldPolarityL0 = CURBEData(56);
    char refFieldPolarityL1 = CURBEData(57);

    uchar isField = (CURBEData(12) >> 7);

    vector<char, 8> vec1;
    vec1 = 0xff;

    vector<char, 8>ref0Polarity;
    ref0Polarity = 0;
    ref0Polarity.merge(vec1,refFieldPolarityL0);

    vector<char, 8>ref1Polarity;
    ref1Polarity = 0;

    if (b)
    {
        ref1Polarity.merge(vec1,refFieldPolarityL1);
    }

    /*some local declarations that were moved to the top*/
    matrix<uchar, 4, 32> MV;

    matrix<ushort, 4, 4> best_DistL0;
    matrix<uint,   4, 4> best_refId;
    matrix<uint,   4, 4> best_MV;
    matrix<uchar,  4, 4> ROIMask(0);

    //save values for streamIn
    matrix<uint, 2, 8> streamIn_MV = 0;
    uchar  PredictorSelect         = 0;
    ushort PredictorRefIdx         = 0;

    if (!is_hevc_vp9_vdenc && vdenc_enable)
    {
        char Streamin_MaxRef;

        best_DistL0     = 0xffff;
        best_refId      = 0;
        Streamin_MaxRef = (NumRefIdxL0 > 2 ? 2 : NumRefIdxL0);
        NumRefIdxL0     = Streamin_MaxRef;
    }

    if (is_hevc_vp9_vdenc && vdenc_enable)
    {
        NumRefIdxL0 = NumRefIdxL0 > 2 ? 2 : NumRefIdxL0;
        ActualMBDim = CURBEData.format<ushort>().select<2, 1>(60);
    }

    while (current_iteration <= NumRefIdxL0)
    {
        // initialize ME refinement cost to maximum
        VME_ME_REFINE_L0.row(0).format<ushort>()[4] = 0xFFFF;
        VME_ME_REFINE_L1.row(0).format<ushort>()[4] = 0xFFFF;

        UNIInput_tmp.row(1).format<char>().select<1,1>(5).merge(ref0Polarity(current_iteration), 0, isField);

        // SuperHME or UltaraHME
        if ( useMVPrevStep )
        {
            vector<uchar,     8> coarse_refine;
            vector_ref<short, 4> coarse_mv_refine = coarse_refine.format<short>();
            vector<short,     2> out_refine;

            // ===========================================
            //  Get higher level HME MV refinement data
            // ===========================================
            /*
            * The position of the MV of the last MB (in the vertical direction) should be
            * at read_y_pos = (picture_heightMB << prevMVReadPosFactor)
            * which means our input surface must have at least (picture_heightMB << prevMVReadPosFactor)
            * motion vectors in each column. Each thread write 4 MV in a column so we have to round this number
            * to the next multiplication of 4
            */
            uint subMB_offset_dscaled_pic = (( (picture_heightMB << prevMVReadPosFactor) + 3 ) & ~0x3);
            subMB_offset_dscaled_pic     *= current_iteration;

            /*
            * Read the MV of the current MB from the previous HME level.
            *   The desired MV located in the higher level in corresponding SubMB which has (pos << prevMVReadPosFactor)
            *   We look for the index in the array of subMB therfore we need to map between our MB index position
            *   (In the current resolution) to the SubMB index in the previous level.
            *
            *   The index of the x pos is the horizontal index of the current MB shifted by the
            *   read position factor given by the user. we add <<3 because each MV is 8 bytes
            *   The index for the y pos is the vertical index of the current MB shifted by the
            *   read position factor given by the user. we add subMB_offset_dscaled_pic so we get
            *   the MV from the correct reference (the input surface arranged as a list).
            */
            uint read_x_pos = mb_x_pos << prevMVReadPosFactor;
            uint read_y_pos = mb_y_pos << prevMVReadPosFactor;
            read(HME_MV_Input_Data_Surface_index, read_x_pos << 3, read_y_pos + subMB_offset_dscaled_pic, coarse_refine);

            // Compare input refine MV values to threshold
            vector<ushort, 4> v_mask = (cm_abs<short>(coarse_mv_refine) >> 2) < SUPER_COMBINEDIST;

            checkL0 = 1;
            checkL0.merge(0, v_mask(0) & v_mask(1));

            if (b)
            {
                checkL1 = 1;
                checkL1.merge(0, v_mask(2) & v_mask(3));
            }

            if (checkL0(0))
            {
                UNIInput.row(0) = UNIInput_tmp.row(0);
                UNIInput.row(1) = UNIInput_tmp.row(1);
                UNIInput.row(2) = UNIInput_tmp.row(2);
                UNIInput.row(3) = 0;
                UNIInput.row(1).select<4, 1>(24) = current_iteration;

                HME_SET_REF( CURBEData, coarse_mv_refine.select<2, 1>(0), out_refine, pos );
                UNIInput.format<short, 4, 16>().select<1, 1, 2, 1>(0, 0) = out_refine;

                ref0 = out_refine;
                run_vme_ime(UNIInput,
                           IMEInput.select<2, 1, 32, 1>(0,0),
                           VME_STREAM_OUT,
                           searchControl,
                           Pred_Surface_L0,
                           ref0,
                           0,
                           costCenter,
                           IME_output_MV_refine_L0.format<uchar, 9, 32>());

                UNIInput.format<int, 4, 8>()[2][5] =  IME_output_MV_refine_L0.format<int, 9, 8>()[0][6] & 0xFFFF00;
                UNIInput[2][20] = IME_output_MV_refine_L0[0][0] & 0x03;
                FBRInput.row(0) = IME_output_MV_refine_L0.row(1);
                FBRInput.row(1) = IME_output_MV_refine_L0.row(2);
                FBRInput.row(2) = IME_output_MV_refine_L0.row(3);
                FBRInput.row(3) = IME_output_MV_refine_L0.row(4);

                FBRMbMode      = VME_GET_UNIInput_FBRMbModeInput(UNIInput);
                FBRSubMbShape  = VME_GET_UNIInput_FBRSubMBShapeInput(UNIInput);
                FBRSubPredMode = VME_GET_UNIInput_FBRSubPredModeInput(UNIInput);

                run_vme_fbr(UNIInput,
                            FBRInput,
                            Pred_Surface_L0,
                            FBRMbMode,
                            FBRSubMbShape,
                            FBRSubPredMode,
                            VME_ME_REFINE_L0);
            }

            if (b)
            {
                if((current_iteration <= NumRefIdxL1) && checkL1(0))
                {
                    UNIInput_tmp.row(1).format<char>().select<1, 1>(5).merge(ref1Polarity(current_iteration), 0, isField);

                    UNIInput.row(0) = UNIInput_tmp.row(0);
                    UNIInput.row(1) = UNIInput_tmp.row(1);
                    UNIInput.row(2) = UNIInput_tmp.row(2);
                    UNIInput.row(3) = 0;
                    UNIInput.row(1).select<4,1>(24) = current_iteration;

                    HME_SET_REF(CURBEData, coarse_mv_refine.select<2, 1>(2), out_refine, pos);

                    UNIInput.format<short, 4, 16>().select<1, 1, 2, 1>(0, 0) = out_refine;
                    ref0 = out_refine;

                    run_vme_ime(UNIInput,
                                IMEInput.select<2, 1, 32, 1>(0, 0),
                                VME_STREAM_OUT,
                                searchControl,
                                Pred_Surface_L1,
                                ref0,
                                0,
                                costCenter,
                                IME_output_MV_refine_L1.format<uchar, 9, 32>());

                    UNIInput.format<int, 4, 8>()[2][5] =  IME_output_MV_refine_L1.format<int, 9, 8>()[0][6] & 0xFFFF00;
                    UNIInput[2][20] = IME_output_MV_refine_L1[0][0] & 0x03;
                    FBRInput.row(0) = IME_output_MV_refine_L1.row(1);
                    FBRInput.row(1) = IME_output_MV_refine_L1.row(2);
                    FBRInput.row(2) = IME_output_MV_refine_L1.row(3);
                    FBRInput.row(3) = IME_output_MV_refine_L1.row(4);

                    FBRMbMode       = VME_GET_UNIInput_FBRMbModeInput(UNIInput);
                    FBRSubMbShape   = VME_GET_UNIInput_FBRSubMBShapeInput(UNIInput);
                    FBRSubPredMode  = VME_GET_UNIInput_FBRSubPredModeInput(UNIInput);

                    run_vme_fbr(UNIInput,
                                FBRInput,
                                Pred_Surface_L1,
                                FBRMbMode,
                                FBRSubMbShape,
                                FBRSubPredMode,
                                VME_ME_REFINE_L1);

                    VME_ME_REFINE_L0.format<int, 7, 8>().select<4,1,4,2>(1,1) = VME_ME_REFINE_L1.format<int, 7, 8>().select<4, 1, 4, 2>(1, 0);
                }
            }

            VME_ME_REFINE_L0.format<short, 7, 16>().select<4, 1, 16, 1>(1, 0) <<=  MVShiftFactor;
        }

        UNIInput.row(0) = UNIInput_tmp.row(0);
        UNIInput.row(1) = UNIInput_tmp.row(1);
        UNIInput.row(2) = UNIInput_tmp.row(2);
        UNIInput.row(3) = 0;

        UNIInput.row(1).select<4, 1>(24)                         =  current_iteration;
        UNIInput.format<short, 4, 16>().select<1, 1, 2, 1>(0, 0) = -search_coordinates;
        ref0                                                     = -search_coordinates;

        if (checkL0(0))
        {
            IMEInput.row(2) = IME_output_MV_refine_L0.row(7);
            IMEInput.row(3) = IME_output_MV_refine_L0.row(8);
            run_vme_ime(UNIInput,
                        IMEInput,
                        VME_STREAM_IN,
                        searchControl,
                        Pred_Surface_L0,
                        ref0,
                        NULL,
                        costCenter,
                        IME_output_MV_L0);
        }
        else
        {
            run_vme_ime( UNIInput,
            IMEInput.select<2, 1, 32, 1>(0,0),
                VME_STREAM_DISABLE,
                searchControl,
                Pred_Surface_L0,
                ref0,
                NULL,
                costCenter,
                IME_output_MV_L0.format<uchar, 7, 32>());
        }

        UNIInput.format<int, 4, 8>()[2][5] = IME_output_MV_L0.format<int, 7, 8>()[0][6] & 0xFFFF00;
        UNIInput[2][20]                    = IME_output_MV_L0[0][0] & 0x03;
        FBRInput.row(0)                    = IME_output_MV_L0.row(1);
        FBRInput.row(1)                    = IME_output_MV_L0.row(2);
        FBRInput.row(2)                    = IME_output_MV_L0.row(3);
        FBRInput.row(3)                    = IME_output_MV_L0.row(4);

        FBRMbMode      = VME_GET_UNIInput_FBRMbModeInput(UNIInput);
        FBRSubMbShape  = VME_GET_UNIInput_FBRSubMBShapeInput(UNIInput);
        FBRSubPredMode = VME_GET_UNIInput_FBRSubPredModeInput(UNIInput);

        run_vme_fbr(UNIInput,
                    FBRInput,
                    Pred_Surface_L0,
                    FBRMbMode,
                    FBRSubMbShape,
                    FBRSubPredMode,
                    VME_ME_L0);

        if (b)
        {
            if(current_iteration <= NumRefIdxL1)
            {
                UNIInput_tmp.row(1).format<char>().select<1, 1>(5).merge(ref1Polarity(current_iteration), 0, isField);

                UNIInput.row(0) = UNIInput_tmp.row(0);
                UNIInput.row(1) = UNIInput_tmp.row(1);
                UNIInput.row(2) = UNIInput_tmp.row(2);
                UNIInput.row(3) = 0;

                UNIInput.row(1).select<4, 1>(24)                        =  current_iteration;
                UNIInput.format<short, 4, 16>().select<1, 1, 2, 1>(0,0) = -search_coordinates;
                ref0                                                    = -search_coordinates;

                if (checkL1(0))
                {
                    IMEInput.row(2) = IME_output_MV_refine_L1.row(7);
                    IMEInput.row(3) = IME_output_MV_refine_L1.row(8);

                    run_vme_ime(UNIInput,
                                IMEInput,
                                VME_STREAM_IN,
                                searchControl,
                                Pred_Surface_L1,
                                ref0,
                                NULL,
                                costCenter,
                                IME_output_MV_L1);
                }
                else
                {
                    run_vme_ime(UNIInput,
                                IMEInput.select<2, 1, 32, 1>(0, 0),
                                VME_STREAM_DISABLE,
                                searchControl,
                                Pred_Surface_L1,
                                ref0,
                                NULL,
                                costCenter,
                                IME_output_MV_L1.format<uchar, 7, 32>());
                }

                UNIInput.format<int, 4, 8>()[2][5] = IME_output_MV_L1.format<int, 7, 8>()[0][6] & 0xFFFF00;
                UNIInput[2][20] = IME_output_MV_L1[0][0] & 0x03;
                FBRInput.row(0) = IME_output_MV_L1.row(1);
                FBRInput.row(1) = IME_output_MV_L1.row(2);
                FBRInput.row(2) = IME_output_MV_L1.row(3);
                FBRInput.row(3) = IME_output_MV_L1.row(4);

                FBRMbMode       = VME_GET_UNIInput_FBRMbModeInput(UNIInput);
                FBRSubMbShape   = VME_GET_UNIInput_FBRSubMBShapeInput(UNIInput);
                FBRSubPredMode  = VME_GET_UNIInput_FBRSubPredModeInput(UNIInput);

                run_vme_fbr(UNIInput,
                            FBRInput,
                            Pred_Surface_L1,
                            FBRMbMode,
                            FBRSubMbShape,
                            FBRSubPredMode,
                            VME_ME_L1);

                VME_ME_L0.format<int, 7, 8>().select<4, 1, 4, 2>(1, 1) = VME_ME_L1.format<int, 7, 8>().select<4, 1, 4, 2>(1, 0);
            }
        }

        VME_ME_L0.format<short, 7, 16>().select<4, 1, 16, 1>(1, 0) <<= MVShiftFactor;

        matrix<ushort, 4, 8> vmeMask = VME_ME_REFINE_L0.row(0).format<ushort>()[4] < VME_ME_L0.row(0).format<ushort>()[4];

        MV.format<short, 4, 16>().select<4, 1, 8, 1>(0, 0).merge(
            VME_ME_REFINE_L0.format<short, 7, 16>().select<2, 2, 16, 1>(1, 0),
            VME_ME_L0.format<short, 7, 16>().select<2, 2, 16, 1>(1, 0), vmeMask);

        MV.format<short, 4, 16>().select<4, 1, 8, 1>(0, 8).merge(
            VME_ME_REFINE_L0.format<short, 7, 16>().select<2, 2, 16, 1>(2, 0),
            VME_ME_L0.format<short, 7, 16>().select<2, 2, 16, 1>(2, 0), vmeMask);

        if (b)
        {
            if(current_iteration <= NumRefIdxL1)
            {
                matrix<ushort, 4, 2> vmeMask1 = VME_ME_REFINE_L1.row(0).format<ushort>()[4] < VME_ME_L1.row(0).format<ushort>()[4];

                MV.format<int, 4, 8>().select<4, 1, 2, 2>(0, 1).merge(
                                VME_ME_REFINE_L0.format<int, 7, 8>().select<2, 2, 4, 2>(1, 1),
                                VME_ME_L0.format<int, 7, 8>().select<2, 2, 4, 2>(1, 1), vmeMask1);

                MV.format<int, 4, 8>().select<4, 1, 2, 2>(0, 5).merge(
                                VME_ME_REFINE_L0.format<int, 7, 8>().select<2, 2, 4, 2>(2, 1),
                                VME_ME_L0.format<int, 7, 8>().select<2, 2, 4, 2>(2, 1), vmeMask1);
            }
        }

        /*
        *  Each MV (there are 1 per SubMB, 16 total) will be used by a corresponding MB in the next level.
        *  Each MB in the next level would use the MV of the corresponding Sub MB in the current level
        */
        write( HME_MV_Data_Surface_index, x_pos << 1, (y_pos >> SHIFT_PIXEL_TO_SUB_MB) + picture_offset, MV);

        if (current_iteration==0)
        {
            matrix_ref<short, 4, 16>FinalMV = MV.format<short, 4, 16>();
            matrix<U16, 2, 2> mx            = cm_abs<U16, short>(FinalMV.select<2, 2, 2, 8>(0, 0));
            matrix<U16, 2, 2> my            = cm_abs<U16, short>(FinalMV.select<2, 2, 2, 8>(0, 1));

            matrix<U16, 2, 2> mvsum = 0;
            mvsum.merge(1, 0, (mx + my) < BRCMVThreshold);
            BD_MV_Sum(0) = cm_sum<U32, U16, 4>(mvsum.format<U16>());
        }

        if (is_hevc_vp9_vdenc && vdenc_enable)
        {
            streamIn_MV.select<2, 1, 2, 4>(0, current_iteration) = MV.format<uint, 4, 8>().select<2, 2, 2, 4>(0, 0);
            PredictorSelect = PredictorSelect | (1 << (2 * current_iteration));
            PredictorRefIdx = PredictorRefIdx | (current_iteration << (current_iteration << 2));

            if (b & (current_iteration == 0))
            {
                streamIn_MV.select<2, 1, 2, 4>(0, NumRefIdxL0 + 1) = MV.format<uint, 4, 8>().select<2, 2, 2, 4>(0, 1);

                PredictorSelect = PredictorSelect | (2 << (2 * (NumRefIdxL0 + 1)));
                PredictorRefIdx = PredictorRefIdx | (current_iteration << ((NumRefIdxL0 + 1) << 2));
            }
                      }

        if (b)
        {
            if(current_iteration <= NumRefIdxL1)
            {
                MV.format<int, 4, 8 >().select< 4, 1, 4, 2>(0,0) = MV.format<int, 4, 8 >().select< 4, 1, 4, 2>(0,1);
                MV.format<int, 4, 8 >().select< 4, 1, 4, 2>(0,1) = 0;
                write(HME_MV_Data_Surface_index, x_pos << 1, (y_pos >> SHIFT_PIXEL_TO_SUB_MB) + picture_offset + (picture_heightMB << 5) , MV);
            }
        }

        if (writeDistortions)
        {
            matrix<uchar, 4, 8> DistL0;
            matrix<uchar, 4, 8> DistL1;

            matrix<ushort, 4, 8> distMask = VME_ME_REFINE_L0.row(0).format<ushort>()[4] < VME_ME_L0.row(0).format<ushort>()[4];
            DistL0.merge(VME_ME_REFINE_L0.row(5).format<uchar, 4, 8>(), VME_ME_L0.row(5).format<uchar, 4, 8>(), distMask);

            if (b)
            {
                if(current_iteration <= NumRefIdxL1)
                {
                    distMask = VME_ME_REFINE_L1.row(0).format<ushort>()[4] < VME_ME_L1.row(0).format<ushort>()[4];
                    DistL1.merge(VME_ME_REFINE_L1.row(5).format<uchar, 4, 8>(), VME_ME_L1.row(5).format<uchar, 4, 8>(), distMask);
                }
            }

            if (current_iteration == 0)
            {
                matrix<uchar, 4, 8> BRCdist = DistL0;

                if (b)
                {
                    if (current_iteration <= NumRefIdxL1)
                    {
                        BRCdist.format<ushort>() = cm_min<ushort>(DistL0.format<ushort>(), DistL1.format<ushort>());
                    }
                }

                if (!vdenc_enable)
                {
                    write(BRC_DISTORTION_Surface, x_pos >> 1, y_pos >> 2, BRCdist);
                }

                BD_MV_Sum(1) = cm_sum<U32, U16, 16>(BRCdist.format<U16>());

                vector<uint, 8> local_offset = 7;
                vector<uint, 8> ret;

                local_offset[0] = 0;
                local_offset[1] = 1;

                if (EnableMVSum)
                {
                    write<uint, 8>(SUM_Surface, ATOMIC_ADD, 0, local_offset, BD_MV_Sum, ret);
                }
            }

            matrix<uchar, 4, 8> dist;
            dist = DistL0;
            dist.format<ushort, 4, 4>().select<4, 1, 1, 1>(0, 1) = dist.format<ushort, 4, 4>().select<4, 1, 1, 1>(0, 0);
            dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 2) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 0);
            dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 0) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(0, 0);
            dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(0, 2) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 2);

            write(DISTORTION_Surface, x_pos >> 1, (y_pos >> 2) + picture_offset, dist);

            if (!is_hevc_vp9_vdenc && vdenc_enable)
            {
                vector<uchar,1>factor = 0;
                factor.merge(HMEStreaminRefCost, current_iteration != 0);

                matrix<ushort, 4, 4>sumDist    = dist.format<ushort>() + factor(0);
                matrix<uint,   4, 4>curr_refID = current_iteration;
                matrix<uchar,  4, 4>dist_mask  = (best_DistL0 >= sumDist);

                best_DistL0.merge(sumDist,dist_mask);
                best_refId.merge(curr_refID,dist_mask);
                best_MV.merge(MV.format<uint, 4, 8>().select<4, 1, 4, 2>(0, 0), dist_mask);
            }

            if (b)
            {
                if (current_iteration <= NumRefIdxL1)
                {
                    dist = DistL1;
                    dist.format<ushort, 4, 4>().select<4, 1, 1, 1>(0, 1) = dist.format<ushort, 4, 4>().select<4, 1, 1, 1>(0, 0);
                    dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 2) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 0);
                    dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 0) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(0, 0);
                    dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(0, 2) = dist.format<ushort, 4, 4>().select<2, 2, 2, 1>(1, 2);
                    write(DISTORTION_Surface, x_pos >> 1, (y_pos >> 2) + picture_offset + (picture_heightMB << 5), dist);
                }
            }
        }

        /*
        * All the MV of the references are arranged in the input surface as a list.
        * Assume that each reference has X MB in a column so the MV of the top left block of the
        * first reference is located at (0,0) and the top left MV of the next reference is located
        * at ( X << SHIFT_MB_TO_SUB_MB, 0)
        */
        picture_offset += (picture_heightMB << SHIFT_MB_TO_SUB_MB);

        if (0 == current_iteration && (!is_hevc_vp9_vdenc && vdenc_enable) && ROIMapEnable == 3)
        {
            ROIMask = !((best_DistL0 < 10) & (best_MV.format<ushort,4, 8>().select<4, 1, 4, 2>(0, 0) == 0) & (best_MV.format<ushort, 4, 8>().select<4, 1, 4, 2>(0, 1) == 0));
        }

        current_iteration++;
    }

    if (is_hevc_vp9_vdenc && vdenc_enable)
    {
        vector<ushort, 2> Sizein32x32RoundDown = (ActualMBDim) >> 5;
        vector<ushort, 2> Sizein32x32RoundUp   = (ActualMBDim + 31) >> 5;
        vector<ushort, 2> mask                 = ((CURBEData[24] & 0x04) != 0);
        Sizein32x32RoundUp.merge(((ActualMBDim + 63) >> 6) << 1, mask);

        uint mb_xpos_32 = mb_x_pos << 1;
        uint mb_ypos_32 = mb_y_pos << 1;

        bool tiling = (CURBEData(24) & 0x40) != 0;

        vector<uint, 32> Streamin_read_row1 = 0;
        vector<uint, 32> Streamin_read_row2 = 0;

        vector_ref<uint, 16> Streamin_read_row1_blk1 = Streamin_read_row1.select<16, 1>(0);
        vector_ref<uint, 16> Streamin_read_row2_blk1 = Streamin_read_row2.select<16, 1>(0);

        vector<uint,   4> streamin_offset;
        vector<ushort, 8> tileinfo;

        if (tiling)
        {
            vector<uint,   1> tile_info_offset;
            vector<uint,   1> Offset_tile_streamin;
            vector<ushort, 2> ActualMBDimLCU = Sizein32x32RoundUp;

            if ((CURBEData[24] & 0x04))
            {
                ActualMBDimLCU = ActualMBDimLCU >> 1;

                tile_info_offset = (mb_x_pos + (mb_y_pos * ActualMBDimLCU(0))) * TILEINFO_SIZE;
                read(TileInfo_Buffer, tile_info_offset(0), tileinfo);

                Offset_tile_streamin(0) = tileinfo(4) * tileinfo(5) + (tileinfo(4) * (tileinfo(7) - tileinfo(5)))
                                           + (tileinfo(5) * (ActualMBDimLCU(0) - tileinfo(4)));

                Offset_tile_streamin(0) = Offset_tile_streamin(0)  * LCU64_STREAMIN_SIZE;
                streamin_offset(0)      = (mb_y_pos - tileinfo(5)) * (tileinfo(6) - tileinfo(4)) + mb_x_pos - tileinfo(4);

                streamin_offset(0) = Offset_tile_streamin(0) + (streamin_offset(0) * LCU64_STREAMIN_SIZE);
                streamin_offset(1) = streamin_offset(0) + STREAMIN_SIZE;
                streamin_offset(2) = streamin_offset(0) + (2 * STREAMIN_SIZE);
                streamin_offset(3) = streamin_offset(0) + (3 * STREAMIN_SIZE);
            }
            else
            {
                #pragma unroll
                for (ushort i = 0; i < 4; i++)
                {
                    ushort offset_x = i & 0x01;
                    ushort offset_y = (i & 0x02) >> 1;
                    tile_info_offset = ((mb_xpos_32 + offset_x) + ((mb_ypos_32 + offset_y) * Sizein32x32RoundUp(0))) * TILEINFO_SIZE;

                        read(TileInfo_Buffer, tile_info_offset(0), tileinfo);

                    Offset_tile_streamin(0) = tileinfo(4) * tileinfo(5) + (tileinfo(4) * (tileinfo(7) - tileinfo(5)))
                    + (tileinfo(5) * (Sizein32x32RoundUp(0) - tileinfo(4)));

                    Offset_tile_streamin(0) = Offset_tile_streamin(0) * STREAMIN_SIZE;

                    streamin_offset(i) = ((mb_ypos_32 + offset_y) - tileinfo(5)) * (tileinfo(6) - tileinfo(4)) + (mb_xpos_32 + offset_x) - tileinfo(4);
                    streamin_offset(i) = Offset_tile_streamin(0) + (streamin_offset(i) * STREAMIN_SIZE);
                }
            }
        }
        else
        {
            streamin_offset(0) = (mb_xpos_32 + (mb_ypos_32 * Sizein32x32RoundUp(0))) * STREAMIN_SIZE;
            streamin_offset(2) = streamin_offset(0) + (Sizein32x32RoundUp(0) * STREAMIN_SIZE);

            streamin_offset.select<1, 1>(0).merge((((mb_x_pos << 2) + ((mb_y_pos << 1) * Sizein32x32RoundUp(0))) * STREAMIN_SIZE), ((CURBEData[24] & 0x04) != 0));
            streamin_offset.select<1, 1>(2).merge((streamin_offset.select<1, 1>(0) + (2 * STREAMIN_SIZE)), ((CURBEData[24] & 0x04) != 0));

            streamin_offset(1) = streamin_offset(0) + STREAMIN_SIZE;
            streamin_offset(3) = streamin_offset(2) + STREAMIN_SIZE;
        }

        if (!(CURBEData[24] & 0x02))
        {
            Streamin_read_row1.select<2, 16>(0)     = CURBEData.format<uint>()(31);
            Streamin_read_row2.select<2, 16>(0)     = CURBEData.format<uint>()(31);
            Streamin_read_row1.select<4, 1>(1)      = CURBEData.format<uint>().select<4, 1>(32);
            Streamin_read_row1.select<4, 1>(17)     = CURBEData.format<uint>().select<4, 1>(32);
            Streamin_read_row2.select<4, 1>(1)      = CURBEData.format<uint>().select<4, 1>(32);
            Streamin_read_row2.select<4, 1>(17)     = CURBEData.format<uint>().select<4, 1>(32);
            Streamin_read_row1.select<2, 16>(6)     = CURBEData.format<uint>()(36);
            Streamin_read_row2.select<2, 16>(6)     = CURBEData.format<uint>()(36);
            Streamin_read_row1.select<2, 16>(7)     = CURBEData.format<uint>()(37);
            Streamin_read_row2.select<2, 16>(7)     = CURBEData.format<uint>()(37);
            Streamin_read_row1.select<2, 16>(14)    = CURBEData.format<uint>()(38);
            Streamin_read_row2.select<2, 16>(14)    = CURBEData.format<uint>()(38);

            if (((mb_xpos_32 + 1) < Sizein32x32RoundDown(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundDown(1)))
            {
                Streamin_read_row1.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(0, 4);
                Streamin_read_row2.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(1, 0);
                Streamin_read_row2.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(1, 4);

                Streamin_read_row1.format<uchar>().select<2, 64>(31)  = PredictorSelect;
                Streamin_read_row2.format<uchar>().select<2, 64>(31)  = PredictorSelect;

                Streamin_read_row1.format<ushort>().select<2, 32>(24) = PredictorRefIdx;
                Streamin_read_row2.format<ushort>().select<2, 32>(24) = PredictorRefIdx;
            }
            else if (((mb_xpos_32 + 1) < Sizein32x32RoundDown(0)) & ((mb_ypos_32) < Sizein32x32RoundDown(1)))
            {
                Streamin_read_row1.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(0, 4);

                Streamin_read_row1.format<uchar>().select<2, 64>(31)  = PredictorSelect;
                Streamin_read_row1.format<ushort>().select<2, 32>(24) = PredictorRefIdx;
            }
            else if (((mb_xpos_32) < Sizein32x32RoundDown(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundDown(1)))
            {
                Streamin_read_row1_blk1.select<4, 1>(8) = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row2_blk1.select<4, 1>(8) = streamIn_MV.select<1, 1, 4, 1>(1, 0);

                Streamin_read_row1_blk1.format<uchar>()(31)  = PredictorSelect;
                Streamin_read_row2_blk1.format<uchar>()(31)  = PredictorSelect;

                Streamin_read_row1_blk1.format<ushort>()(24) = PredictorRefIdx;
                Streamin_read_row2_blk1.format<ushort>()(24) = PredictorRefIdx;

            }
            else if (mb_xpos_32 < Sizein32x32RoundDown(0) && mb_ypos_32  < Sizein32x32RoundDown(1))
            {
                Streamin_read_row1_blk1.select<4, 1>(8)      = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1_blk1.format<uchar>()(31)  = PredictorSelect;
                Streamin_read_row1_blk1.format<ushort>()(24) = PredictorRefIdx;
            }

            // Note that in the if else blocks Sizein32x32RoundUp is being used. This is because when stream-in input is disabled, kernel has to populate other stream in fields(Num merge candidates etc) from CURBE
            // For HME predictors we need to round down the frame size. For other fields we need to round up
            if (((mb_xpos_32 + 1) < Sizein32x32RoundUp(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundUp(1)))
            {
                write(StreamINSurface, streamin_offset(0), Streamin_read_row1.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));
                write(StreamINSurface, streamin_offset(2), Streamin_read_row2.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(3), Streamin_read_row2.select<16, 1>(16));
            }
            else if (((mb_xpos_32 + 1) < Sizein32x32RoundUp(0)) & ((mb_ypos_32) < Sizein32x32RoundUp(1)))
            {
                write(StreamINSurface, streamin_offset(0), Streamin_read_row1.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));
            }
            else if (((mb_xpos_32) < Sizein32x32RoundUp(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundUp(1)))
            {
                write(StreamINSurface, streamin_offset(0), Streamin_read_row1_blk1);
                write(StreamINSurface, streamin_offset(2), Streamin_read_row2_blk1);
            }
            else
            {
                write(StreamINSurface, streamin_offset(0), Streamin_read_row1_blk1);
            }
        }
        else
        {
            if (((mb_xpos_32 + 1) < Sizein32x32RoundDown(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundDown(1)))
            {
                read(StreamINSurface_input, streamin_offset(0), Streamin_read_row1.select<16,1>(0));
                read(StreamINSurface_input, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));
                read(StreamINSurface_input, streamin_offset(2), Streamin_read_row2.select<16, 1>(0));
                read(StreamINSurface_input, streamin_offset(3), Streamin_read_row2.select<16, 1>(16));

                Streamin_read_row1.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(0, 4);
                Streamin_read_row2.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(1, 0);
                Streamin_read_row2.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(1, 4);


                Streamin_read_row1.format<uchar>().select<2, 64>(31)  = PredictorSelect;
                Streamin_read_row2.format<uchar>().select<2, 64>(31)  = PredictorSelect;

                Streamin_read_row1.format<ushort>().select<2, 32>(24) = PredictorRefIdx;
                Streamin_read_row2.format<ushort>().select<2, 32>(24) = PredictorRefIdx;


                write(StreamINSurface, streamin_offset(0), Streamin_read_row1.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));
                write(StreamINSurface, streamin_offset(2), Streamin_read_row2.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(3), Streamin_read_row2.select<16, 1>(16));
            }
            else if (((mb_xpos_32 + 1) < Sizein32x32RoundDown(0)) & ((mb_ypos_32) < Sizein32x32RoundDown(1)))
            {
                read(StreamINSurface_input, streamin_offset(0), Streamin_read_row1.select<16, 1>(0));
                read(StreamINSurface_input, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));

                Streamin_read_row1.select<4, 1>(8)  = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1.select<4, 1>(24) = streamIn_MV.select<1, 1, 4, 1>(0, 4);

                Streamin_read_row1.format<uchar>().select<2, 64>(31)  = PredictorSelect;
                Streamin_read_row1.format<ushort>().select<2, 32>(24) = PredictorRefIdx;

                write(StreamINSurface, streamin_offset(0), Streamin_read_row1.select<16, 1>(0));
                write(StreamINSurface, streamin_offset(1), Streamin_read_row1.select<16, 1>(16));
            }
            else if (((mb_xpos_32) < Sizein32x32RoundDown(0)) & ((mb_ypos_32 + 1) < Sizein32x32RoundDown(1)))
            {
                read(StreamINSurface_input, streamin_offset(0), Streamin_read_row1_blk1);
                read(StreamINSurface_input, streamin_offset(2), Streamin_read_row2_blk1);

                Streamin_read_row1_blk1.select<4, 1>(8)      = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row2_blk1.select<4, 1>(8)      = streamIn_MV.select<1, 1, 4, 1>(1, 0);

                Streamin_read_row1_blk1.format<uchar>()(31)  = PredictorSelect;
                Streamin_read_row2_blk1.format<uchar>()(31)  = PredictorSelect;

                Streamin_read_row1_blk1.format<ushort>()(24) = PredictorRefIdx;
                Streamin_read_row2_blk1.format<ushort>()(24) = PredictorRefIdx;

                write(StreamINSurface, streamin_offset(0), Streamin_read_row1_blk1);
                write(StreamINSurface, streamin_offset(2), Streamin_read_row2_blk1);
            }
            else if (mb_xpos_32 < Sizein32x32RoundDown(0) && mb_ypos_32  < Sizein32x32RoundDown(1))
            {
                read(StreamINSurface_input, streamin_offset(0), Streamin_read_row1_blk1);

                Streamin_read_row1_blk1.select<4, 1>(8)      = streamIn_MV.select<1, 1, 4, 1>(0, 0);
                Streamin_read_row1_blk1.format<uchar>()(31)  = PredictorSelect;
                Streamin_read_row1_blk1.format<ushort>()(24) = PredictorRefIdx;

                write(StreamINSurface, streamin_offset(0), Streamin_read_row1_blk1);
            }
        }
    }

    if (!is_hevc_vp9_vdenc && vdenc_enable)
    {
        uint initVal, streamin_offset;

        vector<uint, 16> finalOffset;
        vector<uint, 16> finalOffset_RefID;
        vector<uint, 16> finalOffset_ROI;
        vector<short,16> v_MBaddressIndx(MBaddressIndx );
        vector<short,16> v_MB_X_Indx( MBX_Indx );
        vector<uint, 16> lastMBaddr, lastMBmask;

        vector<uint,  16> offset_debug = 0xffff0000;
        vector<uint, 256> input_streamInsurface;

        uint globOffset = ((x_pos + (y_pos * ActualMBDim(0))) << 2);

        streamin_offset = ActualMBDim(0) * 16;

        finalOffset       = (v_MBaddressIndx * streamin_offset) + (v_MB_X_Indx * 16) + 2;
        finalOffset_RefID = finalOffset + 2;
        finalOffset_ROI   = finalOffset - 2;

        lastMBaddr = ((y_pos>>2) + v_MBaddressIndx + 1) * streamin_offset;
        lastMBmask = ((finalOffset + globOffset) > lastMBaddr);

        uint block1 = globOffset * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(0));

        block1 = (globOffset + 32) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(32));

        block1 = (globOffset + streamin_offset) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(64));

        block1 = (globOffset + 32 + streamin_offset) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(96));

        block1 = (globOffset + streamin_offset * 2) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(128));

        block1 = (globOffset + 32 + streamin_offset * 2) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(160));

        block1 = (globOffset + streamin_offset * 3) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(192));

        block1 = (globOffset + 32 + streamin_offset * 3) * 4;
        read((StreamINSurface_input), block1, input_streamInsurface.select<32, 1>(224));

        input_streamInsurface.select<16, 16>(2) = best_MV.format<uint>();
        input_streamInsurface.select<16, 16>(4) = best_refId.format<uint>();

        if (ROIMapEnable == 3)
        {
            input_streamInsurface.select<16, 16>(0) = (input_streamInsurface.select<16, 16>(0) & 0xFFFFFF00) | (ROIMask.format<uchar>() & 0x000000FF);
        }

        uchar boundary_result = lastMBmask.any();
        if ( boundary_result == 0)
        {
            block1 = globOffset * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(0));

            block1 = (globOffset + 32) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(32));

            block1 = (globOffset + streamin_offset) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(64));

            block1 = (globOffset + 32 + streamin_offset) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(96));

            block1 = (globOffset + streamin_offset * 2) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(128));

            block1 = (globOffset + 32 + streamin_offset * 2) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(160));

            block1 = (globOffset + streamin_offset * 3) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(192));

            block1 = (globOffset + 32 + streamin_offset * 3) * 4;
            write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(224));
        }
        else
        {
            block1 = globOffset * 4;
            if ((lastMBmask(0) == 0) && (lastMBmask(1) == 0))
            {
                write((StreamINSurface),block1, input_streamInsurface.select<32, 1>(0));
            }
            else if (lastMBmask(0) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(0));
            }

            block1 = (globOffset + 32) * 4;
            if ((lastMBmask(2) == 0) && (lastMBmask(3) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(32));
            }
            else if (lastMBmask(2) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(32));
            }

            block1 = (globOffset + streamin_offset) * 4;
            if ((lastMBmask(4) == 0) && (lastMBmask(5) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(64));
            }
            else if (lastMBmask(4) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(64));
            }

            block1 = (globOffset + 32 + streamin_offset) * 4;
            if ((lastMBmask(6) == 0) && (lastMBmask(7) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(96));
            }
            else if (lastMBmask(6) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(96));
            }

            block1 = (globOffset + streamin_offset * 2) * 4;
            if ((lastMBmask(8) == 0) && (lastMBmask(9) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(128));
            }
            else if (lastMBmask(8) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(128));
            }

            block1 = (globOffset + 32 + streamin_offset * 2) * 4;
            if ((lastMBmask(10) == 0) && (lastMBmask(11) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(160));
            }
            else if (lastMBmask(10) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(160));
            }

            block1 = (globOffset + streamin_offset * 3) * 4;
            if ((lastMBmask(12) == 0) && (lastMBmask(13) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(192));
            }
            else if (lastMBmask(12) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(192));
            }

            block1 = (globOffset + 32 + streamin_offset * 3) * 4;
            if ((lastMBmask(14) == 0 ) && (lastMBmask(15) == 0))
            {
                write((StreamINSurface), block1, input_streamInsurface.select<32, 1>(224));
            }
            else if (lastMBmask(14) == 0)
            {
                write((StreamINSurface), block1, input_streamInsurface.select<16, 1>(224));
            }
        }
    }
}

extern "C" _GENX_MAIN_ void
HME_P( vector<uchar, CURBEDATA_SIZE> CURBEData,
       SurfaceIndex HME_MV_Data_Surface_index,
       SurfaceIndex HME_MV_Input_Data_Surface_index,
       SurfaceIndex DISTORTION_Surface,
       SurfaceIndex BRC_DISTORTION_Surface,
       SurfaceIndex Pred_Surface_L0,
       SurfaceIndex Pred_Surface_L1,
       SurfaceIndex StreamINSurface,
       SurfaceIndex StreamINSurface_input,
       SurfaceIndex SUM_Surface,
       SurfaceIndex TileInfo_Buffer
    )
{
    HME(CURBEData,
        HME_MV_Data_Surface_index,
        HME_MV_Input_Data_Surface_index,
        DISTORTION_Surface,
        BRC_DISTORTION_Surface,
        Pred_Surface_L0,
        Pred_Surface_L1,
        StreamINSurface,
        StreamINSurface_input,
        SUM_Surface,
        TileInfo_Buffer,
        0,
        0,
        0);
}

extern "C" _GENX_MAIN_ void
HME_B( vector<uchar, CURBEDATA_SIZE> CURBEData,
      SurfaceIndex HME_MV_Data_Surface_index,
      SurfaceIndex HME_MV_Input_Data_Surface_index,
      SurfaceIndex DISTORTION_Surface,
      SurfaceIndex BRC_DISTORTION_Surface,
      SurfaceIndex Pred_Surface_L0,
      SurfaceIndex Pred_Surface_L1,
      SurfaceIndex StreamINSurface,
      SurfaceIndex StreamINSurface_input,
      SurfaceIndex SUM_Surface,
      SurfaceIndex TileInfo_Buffer
      )
{
    HME(CURBEData ,
        HME_MV_Data_Surface_index,
        HME_MV_Input_Data_Surface_index,
        DISTORTION_Surface,
        BRC_DISTORTION_Surface,
        Pred_Surface_L0,
        Pred_Surface_L1,
        StreamINSurface,
        StreamINSurface_input,
        SUM_Surface,
        TileInfo_Buffer,
        1,
        0,
        0);
}

extern "C" _GENX_MAIN_ void
HME_VDENC_STREAMIN( vector<uchar, CURBEDATA_SIZE> CURBEData,
                    SurfaceIndex HME_MV_Data_Surface_index,
                    SurfaceIndex HME_MV_Input_Data_Surface_index,
                    SurfaceIndex DISTORTION_Surface,
                    SurfaceIndex BRC_DISTORTION_Surface,
                    SurfaceIndex Pred_Surface_L0,
                    SurfaceIndex Pred_Surface_L1,
                    SurfaceIndex StreamINSurface,
                    SurfaceIndex StreamINSurface_input,
                    SurfaceIndex SUM_Surface,
                    SurfaceIndex TileInfo_Buffer)
{
    HME(CURBEData ,
        HME_MV_Data_Surface_index,
        HME_MV_Input_Data_Surface_index,
        DISTORTION_Surface,
        BRC_DISTORTION_Surface,
        Pred_Surface_L0,
        Pred_Surface_L1,
        StreamINSurface,
        StreamINSurface_input,
        SUM_Surface,
        TileInfo_Buffer,
        0,
        1,
        0);
}
