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
//! \file     codechal_kernel_hme.h
//! \brief    Defines the hme kernel base
//! \details  Hme kernel base includes all common functions and definitions for HME on all platforms
//!

#ifndef __CODECHAL_KERNEL_HME_H__
#define __CODECHAL_KERNEL_HME_H__
#include "codechal_kernel_base.h"

// clang-format off
static uint8_t genericBMEMethod[NUM_TARGET_USAGE_MODES + 1] =
{
    0, 4, 4, 6, 6, 6, 6, 4, 7
};

static uint8_t genericMEMethod[NUM_TARGET_USAGE_MODES + 1] =
{
    0, 4, 4, 6, 6, 6, 6, 4, 7
};

static uint32_t SuperCombineDist[NUM_TARGET_USAGE_MODES + 1] =
{
    0, 1, 1, 5, 5, 5, 9, 9, 0
};

// SearchPath Table, index [CodingType][MEMethod][]
static uint32_t codechalEncodeSearchPath[2][8][16] =
{
    // I-Frame & P-Frame
    {
        // MEMethod: 0
        {
            0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
            0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 1
        {
            0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
            0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 2
        {
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 3
        {
            0x01010101, 0x11010101, 0x01010101, 0x11010101, 0x01010101, 0x11010101, 0x01010101, 0x11010101,
            0x01010101, 0x11010101, 0x01010101, 0x00010101, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 4
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 5
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 6
        {
            0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
            0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 7 used for mpeg2 encoding P frames
        {
            0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x02F1F1F1, 0x1F201111,
            0xF1EFFF0C, 0xF01104F1, 0x10FF0A50, 0x000FF1C0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        }

    },
    // B-Frame
    {
        // MEMethod: 0
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 1
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 2
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000

        },
        // MEMethod: 3
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 4
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 5
        {
            0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
            0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
        },
        // MEMethod: 6
        {
            0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
            0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        // MEMethod: 7 used for mpeg2 encoding B frames
        {
            0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x02F1F1F1, 0x1F201111,
            0xF1EFFF0C, 0xF01104F1, 0x10FF0A50, 0x000FF1C0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        }
    }
};
// clang-format on
//!
//! \class    CodechalKernelHme
//! \brief    Codechal kernel hme
//!
class CodechalKernelHme : public CodechalKernelBase
{
public:
    //!
    //! \enum     HmeLevel
    //! \brief    Hme level
    //!
    enum HmeLevel
    {
        hmeLevelNone = 0,
        hmeLevel4x   = 1,
        hmeLevel16x  = 1 << 1,
        hmeLevel32x  = 1 << 2
    };

    //!
    //! \enum     KernelIndex
    //! \brief    Kernel index
    //!
    enum KernelIndex
    {
        hmeP = 0,
        hmeB = 1
    };

    //!
    //! \enum     SurfaceId
    //! \brief    SurfaceI id
    //!
    enum SurfaceId
    {
        me4xMvDataBuffer = 0,
        me16xMvDataBuffer = 1,
        me32xMvDataBuffer = 2,
        me4xDistortionBuffer = 3
    };

    //!
    //! \enum     BindingTableOffset
    //! \brief    Binding table offset
    //!
    enum BindingTableOffset
    {
        meOutputMvDataSurface = 0,
        meInputMvDataSurface  = 1,
        meDistortionSurface   = 2,
        meBrcDistortion       = 3,
        meCurrForFwdRef       = 5,
        meFwdRefIdx0          = 6,
        meFwdRefIdx1          = 8,
        meFwdRefIdx2          = 10,
        meFwdRefIdx3          = 12,
        meFwdRefIdx4          = 14,
        meFwdRefIdx5          = 16,
        meFwdRefIdx6          = 18,
        meFwdRefIdx7          = 20,
        meCurrForBwdRef       = 22,
        meBwdRefIdx0          = 23,
        meBwdRefIdx1          = 25,
        meSurfaceNum          = 27
    };

    //!
    //! \struct    CurbeParam
    //! \brief     Curbe parameter
    //!
    struct CurbeParam
    {
        bool          brcEnable         = false;
        uint8_t       subPelMode        = 3;
        uint8_t       sumMVThreshold    = 0;
        CODEC_PICTURE currOriginalPic   = {};
        uint32_t      qpPrimeY          = 0;
        uint32_t      targetUsage       = 0;
        uint32_t      maxMvLen          = 0;
        uint32_t      numRefIdxL1Minus1 = 0;
        uint32_t      numRefIdxL0Minus1 = 0;
        uint8_t*      meMethodTable     = nullptr;
        uint8_t*      bmeMethodTable    = nullptr;

        uint32_t list0RefID0FieldParity : MOS_BITFIELD_BIT(0);
        uint32_t list0RefID1FieldParity : MOS_BITFIELD_BIT(1);
        uint32_t list0RefID2FieldParity : MOS_BITFIELD_BIT(2);
        uint32_t list0RefID3FieldParity : MOS_BITFIELD_BIT(3);
        uint32_t list0RefID4FieldParity : MOS_BITFIELD_BIT(4);
        uint32_t list0RefID5FieldParity : MOS_BITFIELD_BIT(5);
        uint32_t list0RefID6FieldParity : MOS_BITFIELD_BIT(6);
        uint32_t list0RefID7FieldParity : MOS_BITFIELD_BIT(7);
        uint32_t list1RefID0FieldParity : MOS_BITFIELD_BIT(8);
        uint32_t list1RefID1FieldParity : MOS_BITFIELD_BIT(9);
    };

    //!
    //! \struct    SurfaceParams
    //! \brief     Surface parameters
    //!
    struct SurfaceParams
    {
        bool                  mbaffEnabled                      = false;
        bool                  vdencStreamInEnabled              = false;
        uint32_t              numRefIdxL0ActiveMinus1           = 0;
        uint32_t              numRefIdxL1ActiveMinus1           = 0;
        uint32_t              downScaledWidthInMb               = 0;
        uint32_t              downScaledHeightInMb              = 0;
        uint32_t              downScaledBottomFieldOffset       = 0;
        uint32_t              vdencStreamInSurfaceSize          = 0;
        uint32_t              verticalLineStride                = 0;
        uint32_t              verticalLineStrideOffset          = 0;
        uint32_t              meBrcDistortionBottomFieldOffset  = 0;
        PCODEC_REF_LIST *     refList                           = nullptr;
        PCODEC_PIC_ID         picIdx                            = nullptr;
        PCODEC_PICTURE        currOriginalPic                   = nullptr;
        PCODEC_PICTURE        refL0List                         = nullptr;
        PCODEC_PICTURE        refL1List                         = nullptr;
        PMOS_SURFACE          meBrcDistortionBuffer             = nullptr;
        PMOS_RESOURCE         meVdencStreamInBuffer             = nullptr;
        CODECHAL_ENCODE_BUFFER meSumMvandDistortionBuffer       = {};
        CmSurface2D          *meBrcDistortionSurface            = nullptr;
    };

    //!
    //! \brief  Constructor
    //!
    //! \param  [in] encoder
    //!         Codechal encoder state
    //! \param  [in] me4xDistBufferSupported
    //!         flag to support 4x Distortion buffer
    //!
    CodechalKernelHme(
        CodechalEncoderState *encoder,
        bool                 me4xDistBufferSupported);

    virtual ~CodechalKernelHme();

    //!
    //! \brief  Execute HME kernel
    //!
    //! \param  [in] curbeParam
    //!         Reference to CurbeParam
    //! \param  [in] surfaceParam
    //!         Reference to SurfaceParams
    //! \param  [in] hmeLevel
    //!         current Hme level to run the kernel
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Execute(CurbeParam &curbeParam, SurfaceParams &surfaceParam, HmeLevel hmeLevel);

    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS ReleaseResources() override;



    uint32_t GetBTCount() override { return BindingTableOffset::meSurfaceNum; }

    inline bool Is4xMeEnabled()
    {
        return m_4xMeSupported && (m_pictureCodingType != I_TYPE);
    }

    inline bool Is16xMeEnabled()
    {
        return m_16xMeSupported && (m_pictureCodingType != I_TYPE);
    }

    inline bool Is32xMeEnabled()
    {
        return m_32xMeSupported && (m_pictureCodingType != I_TYPE);
    }

    inline uint32_t Get4xMeMvBottomFieldOffset()
    {
        return m_4xMeMvBottomFieldOffset;
    }
    inline void Set4xMeMvBottomFieldOffset(uint32_t offset)
    {
        m_4xMeMvBottomFieldOffset = offset;
    }

    inline uint32_t GetDistortionBottomFieldOffset()
    {
        return m_meDistortionBottomFieldOffset;
    }
    inline void SetDistortionBottomFieldOffset(uint32_t offset)
    {
        m_meDistortionBottomFieldOffset = offset;
    }

    inline uint32_t Get16xMeMvBottomFieldOffset()
    {
        return m_16xMeMvBottomFieldOffset;
    }
    inline void Set16xMeMvBottomFieldOffset(uint32_t offset)
    {
        m_16xMeMvBottomFieldOffset = offset;
    }

    inline uint32_t Get32xMeMvBottomFieldOffset()
    {
        return m_32xMeMvBottomFieldOffset;
    }
    inline void Set32xMeMvBottomFieldOffset(uint32_t offset)
    {
        m_32xMeMvBottomFieldOffset = offset;
    }
    inline void setnoMEKernelForPFrame(bool flag)
    {
        m_noMEKernelForPFrame = flag;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpKernelOutput() override;
#endif

protected:
    MOS_STATUS AddPerfTag() override;
    MHW_KERNEL_STATE * GetActiveKernelState() override;
    CODECHAL_MEDIA_STATE_TYPE GetMediaStateType() override;
    MOS_STATUS SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState) override;
    MOS_STATUS InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam) override;

protected:
    bool          &m_4xMeSupported;
    bool          &m_16xMeSupported;
    bool          &m_32xMeSupported;
    bool          &m_noMEKernelForPFrame;
    bool          &m_useNonLegacyStreamIn;
    bool          m_4xMeDistortionBufferSupported = false;  //!< 4x Me Distortion buffer supported
    bool          m_4xMeInUse                     = false;  //!< 4x Me is in use
    bool          m_16xMeInUse                    = false;  //!< 16x Me is in use
    bool          m_32xMeInUse                    = false;  //!< 32x Me is in use
    uint32_t      m_4xMeMvBottomFieldOffset       = 0;      //!< 4x ME mv bottom field offset
    uint32_t      m_16xMeMvBottomFieldOffset      = 0;      //!< 16x ME mv bottom field offset
    uint32_t      m_32xMeMvBottomFieldOffset      = 0;      //!< 32x ME mv bottom field offset
    uint32_t      m_meDistortionBottomFieldOffset = 0;      //!< ME distortion bottom field offset
    uint8_t *     m_bmeMethodTable = genericBMEMethod;      //!< pointer to BME method table
    uint8_t *     m_meMethodTable  = genericMEMethod;       //!< pointer to ME method table
    CurbeParam    m_curbeParam                    = {};     //!< curbe paramters
    SurfaceParams m_surfaceParam                  = {};     //! surface parameters

    static const uint32_t scalingFactor4X  = 4;
    static const uint32_t scalingFactor16X = 16;
    static const uint32_t scalingFactor32X = 32;
};

#endif /* __CODECHAL_KERNEL_HME_H__ */
