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
//! \file     codechal_kernel_intra_dist.h
//! \brief    Defines the intra distortion kernel base
//! \details  Intra distortion base includes all common functions and definitions for intra distortion
//!
#ifndef __CODECHAL_KERNEL_INTRA_DIST_H__
#define __CODECHAL_KERNEL_INTRA_DIST_H__
#include "codechal_kernel_base.h"

//!
//! \class    CodechalKernelIntraDist
//! \brief    Codechal kernel intra dist
//!
class CodechalKernelIntraDist : public CodechalKernelBase
{
public:
    //!
    //! \enum     KernelIndex
    //! \brief    Kernel index
    //!
    enum KernelIndex
    {
        intraDistortion = 0
    };

    //!
    //! \enum     BindingTableOffset
    //! \brief    Binding table offset
    //!
    enum BindingTableOffset
    {
        intraDistCurrent4xY   = 0,
        intraDistOutputSurf   = 1,
        intraDistVmeIntraPred = 2,
        intraDistSurfaceNum   = 3
    };

    //!
    //! \class    CurbeParam
    //! \brief    Curbe parameter
    //!
    struct CurbeParam
    {
        uint32_t downScaledWidthInMb4x = 0;
        uint32_t downScaledHeightInMb4x = 0;
    };

    //!
    //! \class    SurfaceParams
    //! \brief    Surface parameters
    //!
    struct SurfaceParams
    {
        uint32_t     intraDistBottomFieldOffset = 0;
        PMOS_SURFACE input4xDsSurface = nullptr;
        PMOS_SURFACE intraDistSurface = nullptr;
        PMOS_SURFACE input4xDsVmeSurface = nullptr;
    };

    // clang-format off
    //!
    //! \class    Curbe
    //! \brief    Curbe
    //!
    class Curbe
    {
    public:
        Curbe()
        {
            memset(&m_data, 0, sizeof(CurbeData));
            m_data.DW1.interSAD = 2;
            m_data.DW1.intraSAD = 2;
            m_data.DW8.value    = 0xFFFFFFFF;
            m_data.DW9.value    = 0xFFFFFFFF;
            m_data.DW10.value   = 0xFFFFFFFF;
        }
        //!
        //! \struct    CurbeData
        //! \brief     Curbe data
        //!
        struct CurbeData
        {
            //DW0
            union
            {
                struct
                {
                    uint32_t picWidthInLumaSamples  : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t picHeightInLumaSamples : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t value;
            } DW0;

            //DW1
            union
            {
                struct
                {
                    uint32_t srcSize              : MOS_BITFIELD_RANGE(0, 1);
                    uint32_t reserved2            : MOS_BITFIELD_RANGE(2, 13);
                    uint32_t skipType             : MOS_BITFIELD_BIT(14);
                    uint32_t reserved15           : MOS_BITFIELD_BIT(15);
                    uint32_t intraChromaMode      : MOS_BITFIELD_BIT(16);
                    uint32_t ftSkipCheckEnable    : MOS_BITFIELD_BIT(17);
                    uint32_t reserved18           : MOS_BITFIELD_BIT(18);
                    uint32_t blockBasedSkipEnable : MOS_BITFIELD_BIT(19);
                    uint32_t interSAD             : MOS_BITFIELD_RANGE(20, 21);
                    uint32_t intraSAD             : MOS_BITFIELD_RANGE(22, 23);
                    uint32_t reserved24           : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t value;
            } DW1;

            //DW2
            union
            {
                struct
                {
                    uint32_t intraPartMask     : MOS_BITFIELD_RANGE(0, 4);
                    uint32_t nonSkipZmvAdded   : MOS_BITFIELD_BIT(5);
                    uint32_t nonSkipModeAdded  : MOS_BITFIELD_BIT(6);
                    uint32_t intraCornerSwap   : MOS_BITFIELD_BIT(7);
                    uint32_t reserved8         : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t mvCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
                    uint32_t bilinearEnable    : MOS_BITFIELD_BIT(18);
                    uint32_t reserved19        : MOS_BITFIELD_BIT(19);
                    uint32_t weightedSADHaar   : MOS_BITFIELD_BIT(20);
                    uint32_t acOnlyHaar        : MOS_BITFIELD_BIT(21);
                    uint32_t refIDCostMode     : MOS_BITFIELD_BIT(22);
                    uint32_t reserved23        : MOS_BITFIELD_BIT(23);
                    uint32_t skipCenterMask    : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t value;
            } DW2;

            //DW3
            union
            {
                struct
                {
                    uint32_t reserved;
                };
                uint32_t value;
            } DW3;

            //DW4
            union
            {
                struct
                {
                    uint32_t reserved;
                };
                uint32_t value;
            } DW4;

            //DW5
            union
            {
                struct
                {
                    uint32_t reserved;
                };
                uint32_t value;
            } DW5;

            //DW6
            union
            {
                struct
                {
                    uint32_t reserved;
                };
                uint32_t value;
            } DW6;

            //DW7
            union
            {
                struct
                {
                    uint32_t reserved;
                };
                uint32_t value;
            } DW7;

            //DW8
            union
            {
                struct
                {
                    uint32_t currPic4xBTI;
                };
                uint32_t value;
            } DW8;

            //DW9
            union
            {
                struct
                {
                    uint32_t intraDistSurfaceBTI;
                };
                uint32_t value;
            } DW9;

            //DW10
            union
            {
                struct
                {
                    uint32_t vmeIntraPredSurfaceBTI;
                };
                uint32_t value;
            } DW10;
        } m_data;
        static const uint32_t m_curbeSize = sizeof(CurbeData);
    };
    // clang-format on

    CodechalKernelIntraDist(CodechalEncoderState *encoder);

    virtual ~CodechalKernelIntraDist();

    //!
    //! \brief  Execute Intra Distortion kernel
    //!
    //! \param  [in] curbeParam
    //!         Reference to CodechalKernelIntraDist::CurbeParam
    //! \param  [in] surfaceParam
    //!         Reference to CodechalKernelIntraDist::SurfaceParams
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS Execute(CurbeParam &curbeParam, SurfaceParams &surfaceParam);

    uint32_t GetBTCount() { return BindingTableOffset::intraDistSurfaceNum; }

protected:
    MOS_STATUS AllocateResources();
    MOS_STATUS ReleaseResources();

    virtual uint32_t   GetCurbeSize() { return Curbe::m_curbeSize; }
    virtual MOS_STATUS SetCurbe(MHW_KERNEL_STATE *kernelState);
    virtual MOS_STATUS SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState);
    virtual MOS_STATUS AddPerfTag();
    virtual MHW_KERNEL_STATE *GetActiveKernelState();
    virtual MOS_STATUS InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam);
    virtual CODECHAL_MEDIA_STATE_TYPE GetMediaStateType();

    CurbeParam    m_curbeParam;    //!< curbe paramters
    SurfaceParams m_surfaceParam;  //! surface parameters
};

#endif /* __CODECHAL_KERNEL_INTRA_DIST_H__ */
