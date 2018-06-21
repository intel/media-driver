/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_encode_sw_scoreboard.h
//! \brief    Defines base class for SW scoreboard init kernel
//!

#ifndef __CODECHAL_ENCODE_SW_SCOREBOARD_H__
#define __CODECHAL_ENCODE_SW_SCOREBOARD_H__

#include "codechal.h"
#include "codechal_hw.h"

#define CODECHAL_ENCODE_SW_SCOREBOARD_SURFACE_NUM MOS_MAX(dependencyPatternNum, CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)

enum DependencyPattern
{
    dependencyWavefrontNone = 0x00,
    dependencyWavefrontHorizontal = 0x01,
    dependencyWavefrontVertical = 0x02,
    dependencyWavefront45Degree = 0x03,
    dependencyWavefront26Degree = 0x04,
    dependencyWavefront45XDegree = 0x05,
    dependencyWavefront26XDegree = 0x06,
    dependencyWavefront45XVp9Degree = 0x07,
    dependencyWavefront26ZDegree = 0x08,
    dependencyWavefront26ZigDegree = 0x09,
    dependencyWavefront45DDegree = 0x0A,
    dependencyWavefront26DDegree = 0x0B, // not support yet
    dependencyWavefront45XDegreeAlt = 0x0C,
    dependencyWavefront26XDegreeAlt = 0x0D,
    dependencyWavefront45XDDegree = 0x0E, // 45X diamond
    dependencyWavefront26XDDegree = 0x0F, // 26X diamond, not support yet
    dependencyWavefrontCustom = 0x10,
    dependencyPatternNum = dependencyWavefrontCustom + 1
};

//!
//! SW scoreboard init kernel base class
//! \details  Entry point to create SW scoreboard init class instance 
//! 
//! This class defines the base class for SW scoreboard init feature, it includes
//! common member fields, functions, interfaces etc shared by all Gens.
//!
//! To create an instance, client needs to call #CodechalEncodeSWScoreboard::CreateSWScoreboardState()
//!
class CodechalEncodeSwScoreboard
{
public:
    //!
    //! \brief    SW scoreboard init kernel binding table
    //!
    enum KernelBTI
    {
        swScoreboardInitSurface = 0,
        swScoreboardInitLcuInfoSurface = 1,
        swScoreboardNumSurfaces = 2,
    };

    //!< \cond SKIP_DOXYGEN
    //!
    //! \brief    Curbe params for SW scoreboard init kernel
    //!
    struct CurbeParams
    {
        uint32_t                                 scoreboardWidth = 0;
        uint32_t                                 scoreboardHeight = 0;
        bool                                     isHevc = false;
        uint32_t                                 numberOfWaveFrontSplit = 0;
        uint32_t                                 numberOfChildThread = 0;
    };

    //!
    //! \brief    Surface params for SW scoreboard init kernel
    //!
    struct SurfaceParams
    {
        bool                                     isHevc = false;
        uint32_t                                 swScoreboardSurfaceWidth = 0;
        uint32_t                                 swScoreboardSurfaceHeight = 0;
        MOS_SURFACE                              swScoreboardSurface[CODECHAL_ENCODE_SW_SCOREBOARD_SURFACE_NUM] = {};
        uint32_t                                 surfaceIndex = 0;
        PMOS_SURFACE                             lcuInfoSurface = nullptr;
    };

    //!
    //! \brief    Kernel params for SW scoreboard init kernel
    //!
    struct KernelParams
    {
        uint32_t                                 scoreboardWidth = 0;
        uint32_t                                 scoreboardHeight = 0;
        bool                                     isHevc = false;
        uint32_t                                 numberOfWaveFrontSplit = 0;
        uint32_t                                 numberOfChildThread = 0;
        uint32_t                                 swScoreboardSurfaceWidth = 0;
        uint32_t                                 swScoreboardSurfaceHeight = 0;
        uint32_t                                 surfaceIndex = 0;
        PMOS_SURFACE                             lcuInfoSurface = nullptr;
    };

    //!
    //! \brief    Get current SW scoreboard init surface
    //!
    //! \param    [in] index
    //!           Index to SW scorboard surface array
    //!
    //! \return   void
    //!
    void SetCurSwScoreboardSurfaceIndex(uint32_t index) { m_surfaceParams.surfaceIndex = index; }

    //!
    //! \brief    Set current dependency pattern
    //!
    //! \return   Currern dependency pattern
    //!
    void SetDependencyPattern(DependencyPattern pattern) { m_dependencyPatternIdx = pattern; }

    //!
    //! \brief    Get current dependency pattern
    //!
    //! \return   Currern dependency pattern
    //!
    DependencyPattern GetDependencyPattern() { return (DependencyPattern)m_dependencyPatternIdx; }

    //!
    //! \brief    Get current SW scoreboard init surface
    //!
    //! \return   Pointer to current SW scoreboard init surface
    //!
    PMOS_SURFACE GetCurSwScoreboardSurface() { return &m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex]; }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeSwScoreboard();

    //!
    //! \brief    Get SW scoreboard init BT count
    //!
    //! \return   Number of BTI
    //!
    virtual uint8_t GetBTCount();

    //!
    //! \brief    Initialize SW scoreboard init kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    SW scoreboard init kernel function
    //!
    //! \param    [in] params
    //!           Pointer to KernelParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(KernelParams *params);

    //!
    //! \brief    Release SW scoreboard init surface
    //!
    //! \return   void
    //!
    void ReleaseResources();

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeSwScoreboard(CodechalEncoderState* encoder);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeSwScoreboard(const CodechalEncodeSwScoreboard&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeSwScoreboard& operator=(const CodechalEncodeSwScoreboard&) = delete;

    //! \endcond
protected:
    //!
    //! \brief    SW scoreboard init kernel header struct
    //!
    struct KernelHeader
    {
        int                     kernelCount = 0;
        CODECHAL_KERNEL_HEADER  header = {};
    };

    CodechalEncoderState*       m_encoder = nullptr;                         //!< Pointer to ENCODER base class
    MOS_INTERFACE               *m_osInterface = nullptr;                    //!< OS interface
    CodechalHwInterface         *m_hwInterface = nullptr;                    //!< HW interface
    CodechalDebugInterface      *m_debugInterface = nullptr;                 //!< Debug interface
    MhwMiInterface              *m_miInterface = nullptr;                    //!< Common Mi Interface
    MhwRenderInterface          *m_renderInterface = nullptr;                //!< Render engine interface
    XMHW_STATE_HEAP_INTERFACE   *m_stateHeapInterface = nullptr;             //!< State heap class interface
    MHW_KERNEL_STATE            *m_kernelState = nullptr;                    //!< SW scoreboard init kernel state

    uint32_t                    m_curbeLength = 0;                          //!< SW scoreboard init kernel Curbe length
    uint32_t                    m_kernelUID = 0;                            //!< SW scoreboard init kernel UID
    uint32_t                    m_kuidCommon = 0;                           //!< Combined kernel UID
    uint32_t                    m_combinedKernelSize = 0;                   //!< Combined kernel size
    uint8_t                     *m_kernelBase = nullptr;                     //!< kernel binary base address
    CurbeParams                 m_curbeParams = {};                         //!< Curbe parameters
    SurfaceParams               m_surfaceParams = {};                       //!< Surface parameters
    uint8_t                     m_dependencyPatternIdx = 0;                 //!< Dependency pattern

    //!
    //! Reference to data members in Encoder class
    //!
    bool&                       m_useHwScoreboard;
    bool&                       m_renderContextUsesNullHw;
    bool&                       m_groupIdSelectSupported;
    bool&                       m_singleTaskPhaseSupported;
    bool&                       m_firstTaskInPhase;
    bool&                       m_lastTaskInPhase;
    uint8_t&                    m_groupId;
    uint16_t&                   m_pictureCodingType;
    uint32_t&                   m_mode;
    uint32_t&                   m_verticalLineStride;
    uint32_t&                   m_maxBtCount;
    uint32_t&                   m_vmeStatesSize;
    uint32_t&                   m_storeData;
    MHW_WALKER_MODE&            m_walkerMode;

protected:
    //!
    //! \brief    SW scoreboard init kernel Curbe data
    //!
    struct CurbeData
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   scoreboardWidth          : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   scoreboardHeight         : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   isHevc                   : MOS_BITFIELD_BIT  (     0);
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 1, 31);
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
                uint32_t   numberOfWaveFrontsSplits : MOS_BITFIELD_RANGE( 0,  7);
                uint32_t   numberofChildThreads     : MOS_BITFIELD_RANGE( 8, 15);
                uint32_t   reserved                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   dependencyPattern        : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   reserved                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   softwareScoreboard       : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   lcuInfoSurface           : MOS_BITFIELD_RANGE( 0, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW17;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CurbeData)) == 18);

    //!
    //! \brief    Allocate SW scoreboard init surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources();

    //!
    //! \brief    Setup Curbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbe();

    //!
    //! \brief    Send surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSurface(PMOS_COMMAND_BUFFER cmdBuffer);
};

#endif  // __CODECHAL_ENCODE_SW_SCOREBOARD_H__
