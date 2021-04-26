/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mhw_blt.h
//! \brief    MHW interface for constructing commands for the BLT
//!
#ifndef __MHW_BLT_H__
#define __MHW_BLT_H__

#include "mos_os.h"
#include "mhw_utilities.h"

//!
//! \brief  Fast copy blt parameters
//!
typedef struct _MHW_FAST_COPY_BLT_PARAM
{
    uint32_t        dwColorDepth;
    uint32_t        dwSrcPitch;
    uint32_t        dwDstPitch;
    uint32_t        dwSrcTop;
    uint32_t        dwSrcLeft;
    uint32_t        dwDstTop;
    uint32_t        dwDstBottom;
    uint32_t        dwDstLeft;
    uint32_t        dwDstRight;
    PMOS_RESOURCE   pSrcOsResource;
    PMOS_RESOURCE   pDstOsResource;
}MHW_FAST_COPY_BLT_PARAM, *PMHW_FAST_COPY_BLT_PARAM;

class mhw_blt_state
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1

    //!
    //! \brief XY_FAST_COPY_BLT
    //! \details
    //!     This BLT instruction performs a color source copy where the only
    //!     operands involved are a color source and destination of the same bit
    //!     width. The source and destination surfaces CANNOT overlap. The hardware
    //!     assumes this whenever this Fast_Copy command is given to it. For
    //!     overlapping Blits, use the traditional XY_SRC_COPY_BLT command (for
    //!     overlap determination, read the description given in the XY_SRC_COPY_BLT
    //!     command). Note that this command does not support Clipping operations.
    //!     This new blit command will happen in large numbers, consecutively,
    //!     possibly an entire batch will comprise only new blit commands Legacy
    //!     commands and new blit command will not be interspersed. If they are,
    //!     they will be separated by implied HW flush: Whenever there is a
    //!     transition between this new Fast Blit command and the Legacy Blit
    //!     commands (2D BLT instructions other than XY_BLOCK_COPY_BLT,
    //!     XY_FAST_COPY_BLT and XY_FAST_COLOR_BLT), the HW will impose an automatic
    //!     flush BEFORE the execution (at the beginning) of the next blitter
    //!     command. New blit command can use any combination of memory surface type
    //!     - linear, tiledX, tiledY, and the tiling information is conveyed as part
    //!     of the new Fast Copy command. The Fast Copy Blit supports the new 64KB
    //!     Tiling defined for SKL.The starting pixel of Fast Copy blit for both
    //!     source and destination should be on an OWord boundary.
    //!     Note that when two sequential fast copy blits have different source
    //!     surfaces, but their destinations refer to the same destination surfaces
    //!     and therefore destinations overlap it is imperative that a Flush be
    //!     inserted between the two blits.
    //!

    struct XY_FAST_COPY_BLT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 12)    ; //!< Reserved
                uint32_t                 DestinationTilingMethod                          : __CODEGEN_BITFIELD(13, 14)    ; //!< DESTINATION_TILING_METHOD
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 19)    ; //!< Reserved
                uint32_t                 SourceTilingMethod                               : __CODEGEN_BITFIELD(20, 21)    ; //!< SOURCE_TILING_METHOD
                uint32_t                 InstructionTargetOpcode                          : __CODEGEN_BITFIELD(22, 28)    ; //!< INSTRUCTION_TARGETOPCODE
                uint32_t                 Client                                           : __CODEGEN_BITFIELD(29, 31)    ; //!< CLIENT
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 DestinationPitch                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination Pitch
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Reserved
                uint32_t                 ColorDepth                                       : __CODEGEN_BITFIELD(24, 26)    ; //!< COLOR_DEPTH
                uint32_t                 Reserved59                                       : __CODEGEN_BITFIELD(27, 29)    ; //!< Reserved
                uint32_t                 TileYTypeForDestination                          : __CODEGEN_BITFIELD(30, 30)    ; //!< TILE_Y_TYPE_FOR_DESTINATION
                uint32_t                 TileYTypeForSource                               : __CODEGEN_BITFIELD(31, 31)    ; //!< TILE_Y_TYPE_FOR_SOURCE
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 DestinationX1CoordinateLeft                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination X1 Coordinate (Left)
                uint32_t                 DestinationY1CoordinateTop                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Destination Y1 Coordinate (Top)
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DestinationX2CoordinateRight                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination X2 Coordinate (Right)
                uint32_t                 DestinationY2CoordinateBottom                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Destination Y2 Coordinate (Bottom)
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint64_t                 DestinationBaseAddress                                                           ; //!< Destination Base Address
            };
            uint32_t                     Value[2];
        } DW4_5;
        union
        {
            struct
            {
                uint32_t                 SourceX1CoordinateLeft                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Source X1 Coordinate (Left)
                uint32_t                 SourceY1CoordinateTop                            : __CODEGEN_BITFIELD(16, 31)    ; //!< Source Y1 Coordinate (Top)
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 SourcePitch                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Source Pitch
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint64_t                 SourceBaseAddress                                                                ; //!< Source Base Address
            };
            uint32_t                     Value[2];
        } DW8_9;

        //! \name Local enumerations

        //! \brief DESTINATION_TILING_METHOD
        //! \details
        //!     SW is required to flush the HW before changing the polarity of these
        //!     bits for subsequent blits.
        enum DESTINATION_TILING_METHOD
        {
            DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED                  = 0, //!< No additional details
            DESTINATION_TILING_METHOD_LEGACYTILE_X                           = 1, //!< No additional details
            DESTINATION_TILING_METHOD_TILE_Y                                 = 2, //!< Choosing between 'Legacy Tile-Y' or the 'New 4K Tile-YF' can be done in DWord 1, Bit[30].
            DESTINATION_TILING_METHOD_64KBTILING                             = 3, //!< No additional details
        };

        //! \brief SOURCE_TILING_METHOD
        //! \details
        //!     SW is required to flush the HW before changing the polarity of these
        //!     bits for subsequent blits.
        enum SOURCE_TILING_METHOD
        {
            SOURCE_TILING_METHOD_LINEAR_TILINGDISABLED                       = 0, //!< No additional details
            SOURCE_TILING_METHOD_LEGACYTILE_X                                = 1, //!< No additional details
            SOURCE_TILING_METHOD_TILE_Y                                      = 2, //!< Choosing between 'Legacy Tile-Y' or the 'New 4K Tile-YF' can be done in DWord 1, Bit[31].
            SOURCE_TILING_METHOD_64KBTILING                                  = 3, //!< No additional details
        };

        enum INSTRUCTION_TARGETOPCODE
        {
            INSTRUCTION_TARGETOPCODE_UNNAMED66                               = 66, //!< No additional details
        };

        enum CLIENT
        {
            CLIENT_2DPROCESSOR                                               = 2, //!< No additional details
        };

        enum COLOR_DEPTH
        {
            COLOR_DEPTH_8BITCOLOR                                            = 0, //!< No additional details
            COLOR_DEPTH_16BITCOLOR_565                                       = 1, //!< No additional details
            COLOR_DEPTH_32BITCOLOR                                           = 3, //!< No additional details
            COLOR_DEPTH_64BITCOLOR_FOR64KBTILING                             = 4, //!< No additional details
            COLOR_DEPTH_128BITCOLOR_FOR64KBTILING                            = 5, //!< No additional details
        };

        //! \brief TILE_Y_TYPE_FOR_DESTINATION
        //! \details
        //!     Destination being Tile-Y can be selected in DWord 0, Bit[14:13].
        enum TILE_Y_TYPE_FOR_DESTINATION
        {
            TILE_Y_TYPE_FOR_DESTINATION_NEW4KTILE_YF                         = 1, //!< No additional details
        };

        //! \brief TILE_Y_TYPE_FOR_SOURCE
        //! \details
        //!     Source being Tile-Y can be selected in DWord 0, Bit[21:20].
        enum TILE_Y_TYPE_FOR_SOURCE
        {
            TILE_Y_TYPE_FOR_SOURCE_NEW4KTILE_YF                              = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        XY_FAST_COPY_BLT_CMD();

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

};

class MhwBltInterface
{
public:
    MhwBltInterface(PMOS_INTERFACE pOsInterface);

    virtual ~MhwBltInterface()
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Add fast copy blt
    //! \details  MHW function to add fast copy blt command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pFastCopyBltParam
    //!           Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddFastCopyBlt(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_FAST_COPY_BLT_PARAM    pFastCopyBltParam);

    MOS_STATUS (*pfnAddResourceToCmd)(
        PMOS_INTERFACE          pOsInterface,
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        PMHW_RESOURCE_PARAMS    pParams);

public:
    PMOS_INTERFACE m_osInterface = nullptr;
};

typedef class MhwBltInterface *PMHW_BLT_INTERFACE;

#endif  // __MHW_BLT_H__
