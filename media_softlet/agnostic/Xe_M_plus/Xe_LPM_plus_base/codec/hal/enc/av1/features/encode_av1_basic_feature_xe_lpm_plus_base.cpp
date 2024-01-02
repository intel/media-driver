/*
* Copyright (c) 2021 - 2023, Intel Corporation
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
//! \file     encode_av1_basic_feature_xe_lpm_plus_base.cpp
//! \brief    Defines the Xe_LPM_plus+ common class for encode av1 basic feature
//!

#include "encode_av1_basic_feature_xe_lpm_plus_base.h"
#include "encode_av1_vdenc_const_settings_xe_lpm_plus_base.h"
#include "encode_av1_superres.h"

namespace encode
{

MOS_STATUS Av1BasicFeatureXe_Lpm_Plus_Base::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::Update(params));

    Av1SuperRes *superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_RETURN(superResFeature);
    if (superResFeature->IsEnabled())
    {
        m_rawSurfaceToEnc = superResFeature->GetRawSurfaceToEnc();
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeatureXe_Lpm_Plus_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_SURFACE_STATE)(params));

    if (!m_is10Bit)
    {
        params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
    }
    else
    {
        if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }
        else
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
        }
    }

    return MOS_STATUS_SUCCESS;
}

#if _MEDIA_RESERVED
#include "encode_av1_basic_feature_xe_lpm_plus_base_ext.h"
#else
static uint32_t ComputeRdMult(uint16_t qIndex, bool is10Bit)
{
    static uint32_t rdmult_lut[2][256] = {{58, 234, 234, 297, 366, 443, 528,
        528, 619, 718, 825, 938, 1059, 1188, 1323, 1323, 1466, 1617, 1774, 
        1939, 2112, 2291, 2478, 2478, 2673, 2874, 3083, 3300, 3523, 3754, 
        3754, 3993, 4238, 4491, 4752, 5019, 5294, 5294, 5577, 5866, 6163, 
        6468, 6779, 6779, 7098, 7425, 7758, 8099, 8448, 8448, 8803, 9166, 
        9537, 9914, 10299, 10299, 10692, 11091, 11498, 11913, 11913, 12334, 
        12763, 13200, 13643, 14094, 14094, 14553, 15018, 15491, 15972, 
        15972, 16459, 16954, 17457, 17966, 17966, 18483, 19008, 19539, 
        20078, 20078, 20625, 21178, 21739, 22308, 22308, 22883, 23466, 
        24057, 24057, 24654, 25259, 25872, 26491, 26491, 27753, 28394, 
        29700, 31034, 31713, 33091, 33792, 35214, 35937, 37403, 38148, 
        39658, 40425, 41979, 42768, 44366, 45177, 46819, 47652, 49338, 
        50193, 51054, 52800, 53683, 55473, 57291, 59139, 61017, 62923, 
        65838, 67818, 69828, 71866, 73934, 76032, 78158, 80314, 82500, 
        84714, 86958, 89232, 91534, 95043, 98618, 101038, 104723, 108474, 
        111012, 114873, 118800, 121454, 125491, 128219, 132366, 135168, 
        139425, 145203, 149614, 154091, 158634, 163243, 167918, 172659, 
        177466, 182339, 187278, 193966, 199059, 205953, 211200, 216513, 
        223699, 229166, 234699, 242179, 249777, 257491, 265323, 271274, 
        279312, 287466, 295738, 304128, 312634, 321258, 330000, 338858, 
        350097, 359219, 368459, 380174, 389678, 399300, 411491, 423866, 
        433898, 446603, 459492, 472563, 485818, 499257, 512878, 526683, 
        540672, 554843, 572091, 586666, 604398, 619377, 637593, 656073, 
        674817, 693825, 713097, 732633, 755758, 779243, 799659, 827291, 
        851854, 876777, 905699, 935091, 964953, 999108, 1029966, 1065243, 
        1105137, 1145763, 1187123, 1229217, 1276366, 1328814, 1382318, 
        1436878, 1501866, 1568292, 1636154, 1715472, 1796666, 1884993, 
        1986218, 2090091, 2202291, 2323258, 2459457, 2605713, 2768923, 
        2943658, 3137291, 3344091, 3579194, 3829774, 4104334, 4420548,
        4756843, 5140138, 5565354, 6026254, 6544618},
        {4, 19, 23, 39, 52, 66, 92, 111, 143, 180, 220, 265, 314, 367, 
        424, 506, 573, 644, 745, 825, 939, 1060, 1155, 1289, 1394, 1541,
        1695, 1856, 1982, 2156, 2338, 2527, 2723, 2926, 3084, 3300, 3524,
        3755, 3993, 4239, 4492, 4686, 4952, 5225, 5506, 5794, 6089, 6315,
        6623, 6938, 7261, 7591, 7843, 8186, 8536, 8894, 9167, 9537, 9915,
        10300, 10593, 10991, 11396, 11705, 12123, 12441, 12872, 13310, 13644,
        14095, 14438, 14902, 15373, 15731, 16215, 16583, 17080, 17457, 17967, 
        18354, 18876, 19273, 19674, 20215, 20625, 21179, 21599, 22023, 22595,
        23029, 23614, 24057, 24505, 25108, 25565, 26026, 26961, 28073, 29044,
        30031, 31204, 32227, 33266, 34322, 35575, 36667, 37775, 38900, 40041,
        41199, 42373, 43564, 44771, 45995, 47235, 48492, 49765, 51055, 52361,
        53684, 55023, 57063, 58907, 61017, 63164, 65104, 67321, 69323, 71610,
        73675, 76032, 78159, 80315, 82775, 84994, 87241, 89518, 92115, 95044,
        98318, 101648, 104724, 108160, 111651, 114873, 118141, 121789, 125153,
        128563, 132019, 135873, 140141, 144839, 149245, 153716, 158254, 163244,
        167919, 172660, 177467, 181931, 188108, 193967, 199487, 205519, 211640,
        217852, 223700, 229625, 236093, 243123, 250256, 257978, 265324, 272273,
        279818, 287467, 296260, 304656, 313706, 322345, 331101, 339974, 350097,
        359794, 370205, 380175, 390875, 401117, 412721, 424490, 435793, 447884,
        459492, 472564, 485819, 499257, 512879, 526684, 541376, 556985, 572092,
        587400, 604399, 621640, 639123, 656073, 675604, 694623, 714715, 735094,
        756591, 779244, 802230, 827292, 852739, 878571, 907523, 936018, 966835, 
        999108, 1032884, 1068210, 1106144, 1145764, 1187124, 1232404, 1279614,
        1331023, 1384571, 1441473, 1503040, 1568292, 1639831, 1716726, 1799234,
        1888939, 1986219, 2090092, 2205134, 2329100, 2465467, 2610352, 2772111,
        2946945, 3140684, 3349346, 3581006, 3831649, 4112097, 4424575, 4763110,
        5142310, 5567614, 4289813442, 4290334454}};

    return rdmult_lut[is10Bit][qIndex];
}
#endif

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BasicFeatureXe_Lpm_Plus_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_PIC_STATE)(params));

    params.rdmult = ComputeRdMult(params.baseQindex, m_is10Bit);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
