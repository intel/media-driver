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
//! \file     codechal_encode_wp.h
//! \brief    Defines base class for weighted prediction kernel
//!

#ifndef __CODECHAL_ENCODE_WP_H__
#define __CODECHAL_ENCODE_WP_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_encoder_base.h"

//!
//! \class    CodechalEncodeWP
//! \brief    Weighted prediction kernel base class
//! \details  Entry point to create weighted prediction class instance 
//!          
//!         This class defines the base class for weighted prediction feature, it includes
//!         common member fields, functions, interfaces etc shared by all Gens.
//!
//!         To create an instance, client needs to call #CodechalEncodeWP::CreateWPState()
//!  
class CodechalEncodeWP
{
public:
    //!
    //! \struct      SliceParams
    //! \brief       Slice parameters
    //!
    struct SliceParams
    {
        /*! \brief Specifies the weights and offsets used for explicit mode weighted prediction.
        *
        *    Weigths[i][j][k][m]:
        *        \n - i: the reference picture list (0 or 1)
        *        \n - j: reference to entry j in RefPicList (has range [0...31])
        *        \n - k: the YUV component (0 = luma, 1 = Cb chroma, 2 = Cr chroma)
        *        \n - m: the weight or offset used in the weighted prediction process (0 = weight, 1 = offset)
        */
        uint16_t        weights[2][32][3][2] = {};
        uint8_t         luma_log2_weight_denom = 0; //!< Same as AVC syntax element.    
    };

    //!
    //! \struct      CurbeParams
    //! \brief     Curbe params for WP kernel
    //!
    struct CurbeParams
    {
        uint8_t                          refPicListIdx = 0;
        uint32_t                         wpIdx = 0;
        SliceParams                      *slcParams = nullptr;
    };

    //!
    //! \struct      SurfaceParams
    //! \brief       Surface params for WP kernel
    //!
    struct SurfaceParams
    {
        uint8_t                          wpOutListIdx = 0;
        PMOS_SURFACE                     refFrameInput = nullptr;
        MOS_SURFACE                      weightedPredOutputPicList[CODEC_NUM_WP_FRAME] = {};
        bool                             refIsBottomField = false;
    };

    //!
    //! \struct      KernelParams
    //! \brief       Kernel params for WP kernel
    //!
    struct KernelParams
    {
        bool                             useRefPicList1 = false;
        uint32_t                         wpIndex = 0;
        SliceParams                      *slcWPParams = nullptr;
        PMOS_SURFACE                     refFrameInput = nullptr;
        bool                             refIsBottomField = false;
        bool                             *useWeightedSurfaceForL0 = nullptr;
        bool                             *useWeightedSurfaceForL1 = nullptr;
    };

    //!
    //! \enum      KernelBTI
    //! \brief     Weighted prediction kernel binding table
    //!
    enum KernelBTI
    {
        wpInputRefSurface = 0,
        wpOutputScaledSurface = 1,
        wpNumSurfaces = 2,
    };

    //!
    //! \brief    Set kernel base
    //!
    //! \param    [in] kernelBase
    //!           Kernel base
    //!
    void SetKernelBase(uint8_t *kernelBase) { m_kernelBase = kernelBase; }

    //!
    //! \brief    Get WP output picture list
    //!
    //! \param    [in] index
    //!           Index
    //!
    //! \return   PMOS_SURFACE
    //!           Pointer to MOS surface
    //!    
    PMOS_SURFACE GetWPOutputPicList(uint8_t index) { return &m_surfaceParams.weightedPredOutputPicList[index]; }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeWP();

    //!
    //! \brief    Get weighted prediction BT count
    //!
    //! \return   Number of BTI
    //!
    uint8_t GetBTCount();

    //!
    //! \brief    Initialize weighted prediction kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState();

    //!
    //! \brief    Weighted prediction kernel function
    //!
    //! \param    [in] params
    //!           Pointer to KernelParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(KernelParams *params);

    //!
    //! \brief    Allocate weighted prediction surface
    //!
    //! \param    [in] binary
    //!           Kernel binary
    //!
    //! \param    [in] operation
    //!           Encode operation
    //!
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //!
    //! \param    [in] krnHeader
    //!           Kernel header
    //!
    //! \param    [in] krnSize
    //!           Kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*pfnGetKernelHeaderAndSize) (
        void                            *binary,
        EncOperation                    operation,
        uint32_t                        krnStateIdx,
        void                            *krnHeader,
        uint32_t                        *krnSize) {};

protected:
    //!
    //! \brief    Weighted prediction kernel header struct
    //!
    struct KernelHeader
    {
        int                     kernelCount = 0;
        CODECHAL_KERNEL_HEADER  header = {};
    };

    //!
    //! \brief    Weighted prediction kernel Curbe data
    //!
    struct CurbeData
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   defaultWeight    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   defaultOffset    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t   roi0XLeft        : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t   roi0XRight       : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t   roi0Weight       : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t   roi1XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi1YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t   roi1XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi1YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t   roi1Weight       : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi1Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t   roi2XLeft        : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi2YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t   roi2XRight       : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi2YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t   roi2Weight       : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi2Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t   roi3XLeft        : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi3YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t   roi3XRight       : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi3YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t   roi3Weight       : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi3Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t   roi4XLeft        : MOS_BITFIELD_RANGE (0, 15);
                uint32_t   roi4YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t   roi4XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi4YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t   roi4Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi4Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW15;

        // DW16
        union
        {
            struct
            {
                uint32_t   roi5XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi5YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                uint32_t   roi5XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi5YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                uint32_t   roi5Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi5Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                uint32_t   roi6XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi6YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                uint32_t   roi6XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi6YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                uint32_t   roi6Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi6Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                uint32_t   roi7XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi7YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                uint32_t   roi7XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi7YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                uint32_t   roi7Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi7Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                uint32_t   roi8XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi8YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                uint32_t   roi8XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi8YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                uint32_t   roi8Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi8Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                uint32_t   roi9XLeft        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi9YTop         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                uint32_t   roi9XRight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi9YBottom      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW29;

        // DW30
        union
        {
            struct
            {
                uint32_t   roi9Weight       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi9Offset       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t   roi10XLeft       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi10YTop        : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t   roi10XRight      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi10YBottom     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t   roi10Weight      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi10Offset      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t   roi11XLeft       : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi11YTop        : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t   roi11XRight      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi11YBottom     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t   roi11Weight      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi11Offset      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t   roi12XLeft      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi12YTop       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t   roi12XRight     : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi12YBottom    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t   roi12Weight     : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi12Offset     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t   roi13XLeft      : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi13YTop       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t   roi13XRight     : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi13YBottom    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t   roi13Weight    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi13Offset    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t   roi14XLeft     : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi14YTop      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t   roi14XRight    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi14YBottom   : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t   roi14Weight    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi14Offset    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t   roi15XLeft     : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi15YTop      : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t   roi15XRight    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi15YBottom   : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW47;

        // DW48
        union
        {
            struct
            {
                uint32_t   roi15Weight    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   roi15Offset    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t   inputSurface   : MOS_BITFIELD_RANGE( 0, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW49;

        // DW50
        union
        {
            struct
            {
                uint32_t   outputSurface  : MOS_BITFIELD_RANGE( 0, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW50;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CurbeData)) == 51);

    //!
    //! \brief    Constructor
    //!            
    CodechalEncodeWP(CodechalEncoderState* encoder);

    CodechalEncoderState*       m_encoder = nullptr;                         //!< Pointer to ENCODER base class
    MOS_INTERFACE               *m_osInterface = nullptr;                    //!< OS interface
    CodechalHwInterface         *m_hwInterface = nullptr;                    //!< HW interface
    CodechalDebugInterface      *m_debugInterface = nullptr;                 //!< Debug interface
    MhwMiInterface              *m_miInterface = nullptr;                    //!< Common Mi Interface
    MhwRenderInterface          *m_renderInterface = nullptr;                //!< Render engine interface
    XMHW_STATE_HEAP_INTERFACE   *m_stateHeapInterface = nullptr;             //!< State heap class interface
    MHW_KERNEL_STATE            *m_kernelState = nullptr;                    //!< WP kernel state

    uint32_t                    m_curbeLength = 0;                           //!< WP kernel Curbe length
    uint32_t                    m_kernelUID = 0;                             //!< WP kernel UID
    uint32_t                    m_combinedKernelSize = 0;                    //!< Combined kernel size
    uint8_t                     *m_kernelBase = nullptr;                     //!< kernel binary base address
    CurbeParams                 m_curbeParams = {};                          //!< Curbe parameters
    SurfaceParams               m_surfaceParams = {};                        //!< Surface parameters

    //!
    //! Reference to data members in Encoder class
    //! 
    bool&                       m_useHwScoreboard;
    bool&                       m_renderContextUsesNullHw;
    bool&                       m_groupIdSelectSupported;
    bool&                       m_singleTaskPhaseSupported;
    bool&                       m_firstTaskInPhase;
    bool&                       m_lastTaskInPhase;
    bool&                       m_hwWalker;
    uint8_t&                    m_groupId;
    uint16_t&                   m_pictureCodingType;
    uint32_t&                   m_mode;
    uint32_t&                   m_verticalLineStride;
    uint32_t&                   m_maxBtCount;
    uint32_t&                   m_vmeStatesSize;
    uint32_t&                   m_storeData;
    uint32_t&                   m_frameWidth;
    uint32_t&                   m_frameHeight;
    uint32_t&                   m_frameFieldHeight;
    CODEC_PICTURE&              m_currOriginalPic;
    MHW_WALKER_MODE&            m_walkerMode;

protected:
    //!
    //! \brief    Allocate weighted prediction surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources();

    //!
    //! \brief    Release weighted prediction surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void ReleaseResources();

    //!
    //! \brief    Setup Curbe for weighted prediction kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbe();

    //!
    //! \brief    Send surface for weighted prediction kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSurface(PMOS_COMMAND_BUFFER cmdBuffer);
};

#endif  // __CODECHAL_ENCODE_WP_H__
