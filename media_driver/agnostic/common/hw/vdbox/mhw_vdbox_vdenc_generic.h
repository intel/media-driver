/*
* Copyright (c) 2017-2022, Intel Corporation
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

//! \file     mhw_vdbox_vdenc_generic.h
//! \brief    MHW interface for constructing Vdenc commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox Vdenc commands across all platforms 
//!

#ifndef _MHW_VDBOX_VDENC_GENERIC_H_
#define _MHW_VDBOX_VDENC_GENERIC_H_

#include "mhw_vdbox_vdenc_interface.h"

//!  MHW Vdbox Vdenc generic interface
/*!
This class defines the shared Vdenc command construction functions across all platforms as templates
*/
template <class TVdencCmds>
class MhwVdboxVdencInterfaceGeneric : public MhwVdboxVdencInterface
{
protected:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxVdencInterfaceGeneric(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterface(osInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief   Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceGeneric() {}

    MOS_STATUS AddVdPipelineFlushCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS  params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VD_PIPELINE_FLUSH_CMD cmd;

        cmd.DW1.HevcPipelineDone           = params->Flags.bWaitDoneHEVC;
        cmd.DW1.VdencPipelineDone          = params->Flags.bWaitDoneVDENC;
        cmd.DW1.MflPipelineDone            = params->Flags.bWaitDoneMFL;
        cmd.DW1.MfxPipelineDone            = params->Flags.bWaitDoneMFX;
        cmd.DW1.VdCommandMessageParserDone = params->Flags.bWaitDoneVDCmdMsgParser;
        cmd.DW1.HevcPipelineCommandFlush   = params->Flags.bFlushHEVC;
        cmd.DW1.VdencPipelineCommandFlush  = params->Flags.bFlushVDENC;
        cmd.DW1.MflPipelineCommandFlush    = params->Flags.bFlushMFL;
        cmd.DW1.MfxPipelineCommandFlush    = params->Flags.bFlushMFX;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencConstQPStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_VDENC_CQPT_STATE_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_CONST_QPT_STATE_CMD cmd;

        cmd.DW1_10.QpLambdaArrayIndex[0]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[1]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[2]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[3]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[4]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[5]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[6]  = 1;
        cmd.DW1_10.QpLambdaArrayIndex[7]  = 2;
        cmd.DW1_10.QpLambdaArrayIndex[8]  = 2;
        cmd.DW1_10.QpLambdaArrayIndex[9]  = 2;
        cmd.DW1_10.QpLambdaArrayIndex[10] = 2;
        cmd.DW1_10.QpLambdaArrayIndex[11] = 3;
        cmd.DW1_10.QpLambdaArrayIndex[12] = 3;
        cmd.DW1_10.QpLambdaArrayIndex[13] = 3;
        cmd.DW1_10.QpLambdaArrayIndex[14] = 4;
        cmd.DW1_10.QpLambdaArrayIndex[15] = 4;
        cmd.DW1_10.QpLambdaArrayIndex[16] = 5;
        cmd.DW1_10.QpLambdaArrayIndex[17] = 5;
        cmd.DW1_10.QpLambdaArrayIndex[18] = 6;
        cmd.DW1_10.QpLambdaArrayIndex[19] = 7;
        cmd.DW1_10.QpLambdaArrayIndex[20] = 7;
        cmd.DW1_10.QpLambdaArrayIndex[21] = 8;
        cmd.DW1_10.QpLambdaArrayIndex[22] = 9;
        cmd.DW1_10.QpLambdaArrayIndex[23] = 10;
        cmd.DW1_10.QpLambdaArrayIndex[24] = 12;
        cmd.DW1_10.QpLambdaArrayIndex[25] = 13;
        cmd.DW1_10.QpLambdaArrayIndex[26] = 15;
        cmd.DW1_10.QpLambdaArrayIndex[27] = 17;
        cmd.DW1_10.QpLambdaArrayIndex[28] = 19;
        cmd.DW1_10.QpLambdaArrayIndex[29] = 21;
        cmd.DW1_10.QpLambdaArrayIndex[30] = 23;
        cmd.DW1_10.QpLambdaArrayIndex[31] = 26;
        cmd.DW1_10.QpLambdaArrayIndex[32] = 30;
        cmd.DW1_10.QpLambdaArrayIndex[33] = 33;
        cmd.DW1_10.QpLambdaArrayIndex[34] = 37;
        cmd.DW1_10.QpLambdaArrayIndex[35] = 42;
        cmd.DW1_10.QpLambdaArrayIndex[36] = 47;
        cmd.DW1_10.QpLambdaArrayIndex[37] = 53;
        cmd.DW1_10.QpLambdaArrayIndex[38] = 59;
        cmd.DW1_10.QpLambdaArrayIndex[39] = 66;
        cmd.DW11.QpLambdaArrayIndex40 = 74;
        cmd.DW11.QpLambdaArrayIndex41 = 83;

        if (params->wPictureCodingType == P_TYPE)
        {
            cmd.DW12_24.SkipThresholdArrayIndex[0]  = 0;
            cmd.DW12_24.SkipThresholdArrayIndex[1]  = 0;
            cmd.DW12_24.SkipThresholdArrayIndex[2]  = 0;
            cmd.DW12_24.SkipThresholdArrayIndex[3]  = 0;
            cmd.DW12_24.SkipThresholdArrayIndex[4]  = 2;
            cmd.DW12_24.SkipThresholdArrayIndex[5]  = 4;
            cmd.DW12_24.SkipThresholdArrayIndex[6]  = 7;
            cmd.DW12_24.SkipThresholdArrayIndex[7]  = 11;
            cmd.DW12_24.SkipThresholdArrayIndex[8]  = 17;
            cmd.DW12_24.SkipThresholdArrayIndex[9]  = 25;
            cmd.DW12_24.SkipThresholdArrayIndex[10] = 35;
            cmd.DW12_24.SkipThresholdArrayIndex[11] = 50;
            cmd.DW12_24.SkipThresholdArrayIndex[12] = 68;
            cmd.DW12_24.SkipThresholdArrayIndex[13] = 91;
            cmd.DW12_24.SkipThresholdArrayIndex[14] = 119;
            cmd.DW12_24.SkipThresholdArrayIndex[15] = 153;
            cmd.DW12_24.SkipThresholdArrayIndex[16] = 194;
            cmd.DW12_24.SkipThresholdArrayIndex[17] = 241;
            cmd.DW12_24.SkipThresholdArrayIndex[18] = 296;
            cmd.DW12_24.SkipThresholdArrayIndex[19] = 360;
            cmd.DW12_24.SkipThresholdArrayIndex[20] = 432;
            cmd.DW12_24.SkipThresholdArrayIndex[21] = 513;
            cmd.DW12_24.SkipThresholdArrayIndex[22] = 604;
            cmd.DW12_24.SkipThresholdArrayIndex[23] = 706;
            cmd.DW12_24.SkipThresholdArrayIndex[24] = 819;
            cmd.DW12_24.SkipThresholdArrayIndex[25] = 944;

            if (!params->bBlockBasedSkip)
            {
                for (uint8_t i = 0; i < 26; i++)
                {
                    cmd.DW12_24.SkipThresholdArrayIndex[i] *= 3;
                }
            }
            else if (!params->bTransform8x8Flag)
            {
                for (uint8_t i = 0; i < 26; i++)
                {
                    cmd.DW12_24.SkipThresholdArrayIndex[i] /= 2;
                }
            }

            if (params->bFTQEnabled)
            {
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[0]  = 0x02;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[1]  = 0x02;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[2]  = 0x03;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[3]  = 0x04;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[4]  = 0x04;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[5]  = 0x05;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[6]  = 0x07;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[7]  = 0x09;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[8]  = 0x0b;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[9]  = 0x0e;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[10] = 0x12;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[11] = 0x14;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[12] = 0x18;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[13] = 0x1d;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[14] = 0x20;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[15] = 0x25;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[16] = 0x2a;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[17] = 0x34;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[18] = 0x39;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[19] = 0x3f;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[20] = 0x4e;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[21] = 0x51;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[22] = 0x5b;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[23] = 0x63;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[24] = 0x6f;
                cmd.DW26_38.SicForwardTransformCoeffThresholdMatrix0ArrayIndex[25] = 0x7f;

                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[0]  = 0x03;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[1]  = 0x04;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[2]  = 0x05;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[3]  = 0x05;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[4]  = 0x07;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[5]  = 0x09;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[6]  = 0x0b;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[7]  = 0x0e;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[8]  = 0x12;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[9]  = 0x17;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[10] = 0x1c;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[11] = 0x21;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[12] = 0x27;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[13] = 0x2c;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[14] = 0x33;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[15] = 0x3b;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[16] = 0x41;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[17] = 0x51;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[18] = 0x5c;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[19] = 0x1a;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[20] = 0x1e;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[21] = 0x21;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[22] = 0x22;
                cmd.DW40_45.SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[23] = 0x26;
                cmd.DW46.SicForwardTransformCoeffThresholdMatrix135ArrayIndex24       = 0x2c;
                cmd.DW46.SicForwardTransformCoeffThresholdMatrix135ArrayIndex25       = 0x30;

                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[0]  = 0x02;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[1]  = 0x02;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[2]  = 0x03;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[3]  = 0x04;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[4]  = 0x04;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[5]  = 0x05;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[6]  = 0x07;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[7]  = 0x09;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[8]  = 0x0b;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[9]  = 0x0e;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[10] = 0x12;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[11] = 0x14;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[12] = 0x18;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[13] = 0x1d;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[14] = 0x20;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[15] = 0x25;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[16] = 0x2a;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[17] = 0x34;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[18] = 0x39;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[19] = 0x0f;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[20] = 0x13;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[21] = 0x14;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[22] = 0x16;
                cmd.DW47_52.SicForwardTransformCoeffThresholdMatrix2ArrayIndex[23] = 0x18;
                cmd.DW53.SicForwardTransformCoeffThresholdMatrix2ArrayIndex24      = 0x1b;
                cmd.DW53.SicForwardTransformCoeffThresholdMatrix2ArrayIndex25      = 0x1f;

                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[0]  = 0x04;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[1]  = 0x05;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[2]  = 0x06;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[3]  = 0x09;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[4]  = 0x0b;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[5]  = 0x0d;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[6]  = 0x12;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[7]  = 0x16;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[8]  = 0x1b;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[9]  = 0x23;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[10] = 0x2c;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[11] = 0x33;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[12] = 0x3d;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[13] = 0x45;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[14] = 0x4f;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[15] = 0x5b;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[16] = 0x66;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[17] = 0x7f;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[18] = 0x8e;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[19] = 0x2a;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[20] = 0x2f;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[21] = 0x32;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[22] = 0x37;
                cmd.DW54_59.SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[23] = 0x3c;
                cmd.DW60.SicForwardTransformCoeffThresholdMatrix46ArrayIndex24       = 0x45;
                cmd.DW60.SicForwardTransformCoeffThresholdMatrix46ArrayIndex25       = 0x4c;
            }
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

public:
    inline uint32_t GetVdencAvcImgStateSize()
    {
        return TVdencCmds::VDENC_IMG_STATE_CMD::byteSize;
    }

    inline uint32_t GetVdencCmd3Size()
    {
        return 0;
    }

    inline uint32_t GetVdencAvcCostStateSize()
    {
        return 0;
    }

    inline uint32_t GetVdencAvcSlcStateSize()
    {
        return 0;
    }
};

#endif