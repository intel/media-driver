/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_blt_hwcmd_xe_lpm_plus_next.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_blt_hwcmd_xe_lpm_plus_next.h"
#include <string.h>

mhw::blt::xe_lpm_plus_next::Cmd::XY_BLOCK_COPY_BLT_CMD::XY_BLOCK_COPY_BLT_CMD()
{
    DW0.Value                                            = 0x50400014;
     //DW0.DwordLength                                    = DWORD_LENGTH_EXCLUDES_DWORD_0_1;
     //DW0.NumberOfMultisamples                           = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_1;
     //DW0.SpecialModeofOperation                         = SPECIAL_MODE_OF_OPERATION_NONE;
     //DW0.ColorDepth                                     = COLOR_DEPTH_8_BIT_COLOR;
     //DW0.InstructionTarget_Opcode                       = 0x0;
     //DW0.Client                                         = CLIENT_2D_PROCESSOR;
     DW1.Value                                            = 0x0;
     //DW1.DestinationPitch                               = 0x0;
     //DW1.DestinationAuxiliarysurfacemode                = DESTINATION_AUXILIARY_SURFACE_MODE_AUX_NONE;
     //DW1.DestinationMOCSvalue                           = 0x0;
     //DW1.DestinationControlSurfaceType                  = DESTINATION_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE;
     //DW1.DestinationCompressionEnable                   = DESTINATION_COMPRESSION_ENABLE_COMPRESSION_DISABLE;
     //DW1.DestinationTiling                              = DESTINATION_TILING_LINEAR;
     DW2.Value                                            = 0x0;
     //DW2.DestinationX1Coordinate_Left                   = 0x0;
     //DW2.DestinationY1Coordinate_Top                    = 0x0;
     DW3.Value                                            = 0x0;
     //DW3.DestinationX2Coordinate_Right                  = 0x0;
     //DW3.DestinationY2Coordinate_Bottom                 = 0x0;
     DW4_5.Value[0] = DW4_5.Value[1]                      = 0x0;
     //DW4_5.DestinationBaseAddress                         = 0x0;
     DW6.Value                                            = 0x0;
     //DW6.DestinationXoffset                             = 0x0;
     //DW6.DestinationYoffset                             = 0x0;
     //DW6.DestinationTargetMemory                        = DESTINATION_TARGET_MEMORY_LOCAL_MEM;
     DW7.Value                                            = 0x0;
     //DW7.SourceX1Coordinate_Left                        = 0x0;
     //DW7.SourceY1Coordinate_Top                         = 0x0;
     DW8.Value                                            = 0x0;
     //DW8.SourcePitch                                    = 0x0;
     //DW8.SourceAuxiliarysurfacemode                     = SOURCE_AUXILIARY_SURFACE_MODE_AUX_NONE;
     //DW8.SourceMOCS                                     = 0x0;
     //DW8.SourceControlSurfaceType                       = SOURCE_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE;
     //DW8.SourceCompressionEnable                        = SOURCE_COMPRESSION_ENABLE_COMPRESSION_DISABLE;
     //DW8.SourceTiling                                   = SOURCE_TILING_LINEAR;
     DW9_10.Value[0] = DW9_10.Value[1]                    = 0x0;
     //DW9.SourceBaseAddress                              = 0x0;
     DW11.Value                                            = 0x0;
     //DW11.SourceXoffset                                  = 0x0;
     //DW11.SourceYoffset                                  = 0x0;
     //DW11.SourceTargetMemory                             = SOURCE_TARGET_MEMORY_LOCAL_MEM;
     DW12.Value                                            = 0x0;
     //DW12.SourceCompressionFormat                        = 0x0;
     //DW12.SourceClearValueEnable                         = SOURCE_CLEAR_VALUE_ENABLE_DISABLE;
     //DW12.SourceClearAddressLow                          = 0x0;
     DW13.Value                                            = 0x0;
     //DW13.SourceClearAddressHigh                         = 0x0;
     DW14.Value                                            = 0x0;
     //DW14.DestinationCompressionFormat                   = 0x0;
     //DW14.DestinationClearValueEnable                    = DESTINATION_CLEAR_VALUE_ENABLE_DISABLE;
     //DW14.DestinationClearAddressLow                     = 0x0;
     DW15.Value                                            = 0x0;
     //DW15.DestinationClearAddressHigh                    = 0x0;
     DW16.Value = 0x0;
     //DW16.DestinationSurfaceHeight                       = 0x0;
     //DW16.DestinationSurfaceWidth                        = 0x0;
     //DW16.DestinationSurfaceType                         = DESTINATION_SURFACE_TYPE_SURFTYPE_1D;
     DW17.Value                                            = 0x0;
     //DW17.DestinationLOD                                 = 0x0;
     //DW17.DestinationSurfaceQpitch                       = 0x0;
     //DW17.DestinationSurfaceDepth                        = 0x0;
     DW18.Value                                            = 0x0;
     //DW18.DestinationHorizontalAlign                     = 0x0;
     //DW18.DestinationVerticalAlign                       = 0x0;
     //DW18.DestinationMipTailStartLOD                     = 0x0;
     //DW18.DestinationDepthStencilResource                = 0x0;
     //DW18.DestinationArrayIndex                          = 0x0;
     DW19.Value                                            = 0x0;
     //DW19.SourceSurfaceHeight                            = 0x0;
     //DW19.SourceSurfaceWidth                             = 0x0;
     //DW19.SourceSurfaceType                              = SOURCE_SURFACE_TYPE_SURFTYPE_1D;
     DW20.Value                                            = 0x0;
     //DW20.SourceLOD                                      = 0x0;
     //DW20.SourceSurfaceQpitch                            = 0x0;
     //DW20.SourceSurfaceDepth                             = 0x0;
     DW21.Value                                            = 0x0;
     //DW21.SourceHorizontalAlign                          = 0x0;
     //DW21.SourceVerticalAlign                            = 0x0;
     //DW21.SourceMipTailStartLOD                          = 0x0;
     //DW21.SourceDepthStencilResource                     = 0x0;
     //DW21.SourceArrayIndex
}

mhw::blt::xe_lpm_plus_next::Cmd::XY_FAST_COPY_BLT_CMD::XY_FAST_COPY_BLT_CMD()
{
    DW0.Value                                        = 0x50800008;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.DestinationTilingMethod                      = DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
    //DW0.SourceTilingMethod                           = SOURCE_TILING_METHOD_LINEAR_TILINGDISABLED;
    //DW0.InstructionTargetOpcode                      = INSTRUCTION_TARGETOPCODE_UNNAMED66;
    //DW0.Client                                       = CLIENT_2DPROCESSOR;

    DW1.Value                                        = 0x00000000;
    //DW1.ColorDepth                                   = COLOR_DEPTH_8BITCOLOR;
    //DW1.TileYTypeForDestination                      = 0;
    //DW1.TileYTypeForSource                           = 0;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;

}

