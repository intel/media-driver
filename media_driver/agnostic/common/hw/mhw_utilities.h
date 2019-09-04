/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file      mhw_utilities.h 
//! \brief         This modules implements utilities which are shared by both the HW interface     and the state heap interface. 
//!
#ifndef __MHW_UTILITIES_H__
#define __MHW_UTILITIES_H__

#include "mos_os.h"
#include <math.h>
#include "mos_util_debug.h"
#include "mhw_mmio.h"

typedef struct _MHW_RCS_SURFACE_PARAMS MHW_RCS_SURFACE_PARAMS, *PMHW_RCS_SURFACE_PARAMS;
typedef struct _MHW_BATCH_BUFFER MHW_BATCH_BUFFER, *PMHW_BATCH_BUFFER;

#define MHW_CACHELINE_SIZE      64
#define MHW_PAGE_SIZE           0x1000

#define MHW_TIMEOUT_MS_DEFAULT  1000
#define MHW_EVENT_TIMEOUT_MS    5

#define MHW_WIDTH_IN_DW(w)  ((w + 0x3) >> 2)

#define MHW_AVS_TBL_COEF_PREC   6           //!< Table coef precision (after decimal point
#define MHW_TABLE_PHASE_PREC    5
#define MHW_TABLE_PHASE_COUNT   (1 << MHW_TABLE_PHASE_PREC)
#define MHW_SCALER_UV_WIN_SIZE  4
#define MHW_TBL_COEF_PREC       6

#define NUM_HW_POLYPHASE_TABLES 32          //!< must be the same as NUM_HW_POLYPHASE_TABLES_G9
#define NUM_POLYPHASE_TABLES    32
#define NUM_POLYPHASE_Y_ENTRIES 8
#define NUM_POLYPHASE_5x5_Y_ENTRIES 5
#define NUM_POLYPHASE_UV_ENTRIES 4

#define NUM_HW_POLYPHASE_TABLES_G8              17
#define POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8  (NUM_POLYPHASE_Y_ENTRIES  * NUM_HW_POLYPHASE_TABLES_G8 * sizeof(int32_t))
#define POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8 (NUM_POLYPHASE_UV_ENTRIES * NUM_HW_POLYPHASE_TABLES_G8 * sizeof(int32_t))

#define NUM_HW_POLYPHASE_TABLES_G9              32
#define POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9   (NUM_POLYPHASE_Y_ENTRIES  * NUM_HW_POLYPHASE_TABLES_G9 * sizeof(int32_t))
#define POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9  (NUM_POLYPHASE_UV_ENTRIES * NUM_HW_POLYPHASE_TABLES_G9 * sizeof(int32_t))

#define NUM_HW_POLYPHASE_TABLES_G10              32
#define POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G10   (NUM_POLYPHASE_Y_ENTRIES  * NUM_HW_POLYPHASE_TABLES_G10 * sizeof(int32_t))
#define POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G10  (NUM_POLYPHASE_UV_ENTRIES * NUM_HW_POLYPHASE_TABLES_G10 * sizeof(int32_t))



// Calculates the number of bits between the startbit and the endbit (0 based).
#ifndef MHW_BITFIELD_RANGE
#define MHW_BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#endif

#define _NAME_MERGE_(x, y)      x ## y
#define _NAME_LABEL_(name, id)  _NAME_MERGE_(name, id)
#define __CODEGEN_UNIQUE(name)  _NAME_LABEL_(name, __LINE__)

#ifndef SIZE32
#define SIZE32( x )         ((uint32_t)( sizeof(x) / sizeof(uint32_t) ))
#endif // SIZE32

#ifndef OP_LENGTH
#define OP_LENGTH( x )      ((uint32_t)(x) - 2 )
#endif // OP_LENGTH

// Calculates the number of bits between the startbit and the endbit (0 based).
#ifndef BITFIELD_RANGE
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#endif

// Definition declared for clarity when creating structs.
#ifndef BITFIELD_BIT
#define BITFIELD_BIT( bit )                   1
#endif

#define MHW_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define MHW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _expr)

#define MHW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _message, ##__VA_ARGS__)

#define MHW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _message, ##__VA_ARGS__)

#define MHW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _message, ##__VA_ARGS__)

#define MHW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL)

#define MHW_FUNCTION_EXIT                                                      \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, eStatus)

#define MHW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)

#define MHW_CHK_STATUS_RETURN(_stmt)                                            \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)

#define MHW_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt, _message, ##__VA_ARGS__)

#define MHW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

#define MHW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

#define MHW_CHK_NULL_NO_STATUS_RETURN(_ptr) \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

#define MHW_CHK_COND(_condition,  _message, ...)                                \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, (_condition),  (_message),  ##__VA_ARGS__)

#define MHW_CHK_NULL_RETURN(_ptr)                                                      \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

#define MHW_CHK_STATUS_RETURN(_stmt)                                                   \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)

#define MHW_MI_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt)

#define MHW_MI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _stmt, _message, ##__VA_ARGS__)

#define MHW_MI_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, MOS_HW_SUBCOMP_ALL, _ptr)

enum GFX_OPCODE
{
    GFXOP_PIPELINED     = 0x0,
    GFXOP_VEBOX         = 0x4,
    GFXOP_MFX_VEBOX_SFC = 0xA       // Media MFX/VEBOX+SFC Mode
};

enum INSTRUCTION_PIPELINE
{
    PIPE_COMMON       = 0x0,
    PIPE_SINGLE_DWORD = 0x1,
    PIPE_COMMON_CTG   = 0x1,
    PIPE_MEDIA        = 0x2,
    PIPE_3D           = 0x3
};

enum INSTRUCTION_TYPE
{
    INSTRUCTION_MI      = 0x0,
    INSTRUCTION_2D      = 0x2,
    INSTRUCTION_GFX     = 0x3
};

enum MHW_GFX3DSTATE_SURFACEFORMAT
{
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_FLOAT             = 0x000,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SINT              = 0x001,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_UINT              = 0x002,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_UNORM             = 0x003,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SNORM             = 0x004,
    MHW_GFX3DSTATE_SURFACEFORMAT_R64G64_FLOAT                   = 0x005,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32X32_FLOAT             = 0x006,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SSCALED           = 0x007,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_USCALED           = 0x008,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16B16_UNORM_SAMPLE_8X8        = 0x017,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_FLOAT                = 0x040,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_SINT                 = 0x041,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_UINT                 = 0x042,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_UNORM                = 0x043,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_SNORM                = 0x044,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_SSCALED              = 0x045,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32_USCALED              = 0x046,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UNORM             = 0x080,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SNORM             = 0x081,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SINT              = 0x082,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UINT              = 0x083,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_FLOAT             = 0x084,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_FLOAT                   = 0x085,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SINT                    = 0x086,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_UINT                    = 0x087,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT_X8X24_TYPELESS       = 0x088,
    MHW_GFX3DSTATE_SURFACEFORMAT_X32_TYPELESS_G8X24_UINT        = 0x089,
    MHW_GFX3DSTATE_SURFACEFORMAT_L32A32_FLOAT                   = 0x08A,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_UNORM                   = 0x08B,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SNORM                   = 0x08C,
    MHW_GFX3DSTATE_SURFACEFORMAT_R64_FLOAT                      = 0x08D,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16X16_UNORM             = 0x08E,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16X16_FLOAT             = 0x08F,
    MHW_GFX3DSTATE_SURFACEFORMAT_A32X32_FLOAT                   = 0x090,
    MHW_GFX3DSTATE_SURFACEFORMAT_L32X32_FLOAT                   = 0x091,
    MHW_GFX3DSTATE_SURFACEFORMAT_I32X32_FLOAT                   = 0x092,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SSCALED           = 0x093,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_USCALED           = 0x094,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_SSCALED                 = 0x095,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32G32_USCALED                 = 0x096,
    MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8A8_UNORM                 = 0x0C0,
    MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8A8_UNORM_SRGB            = 0x0C1,
    MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UNORM              = 0x0C2,
    MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UNORM_SRGB         = 0x0C3,
    MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UINT               = 0x0C4,
    MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10_SNORM_A2_UNORM       = 0x0C5,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM                 = 0x0C7,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM_SRGB            = 0x0C8,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SNORM                 = 0x0C9,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SINT                  = 0x0CA,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UINT                  = 0x0CB,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM                   = 0x0CC,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SNORM                   = 0x0CD,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SINT                    = 0x0CE,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UINT                    = 0x0CF,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_FLOAT                   = 0x0D0,
    MHW_GFX3DSTATE_SURFACEFORMAT_B10G10R10A2_UNORM              = 0x0D1,
    MHW_GFX3DSTATE_SURFACEFORMAT_B10G10R10A2_UNORM_SRGB         = 0x0D2,
    MHW_GFX3DSTATE_SURFACEFORMAT_R11G11B10_FLOAT                = 0x0D3,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_SINT                       = 0x0D6,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_UINT                       = 0x0D7,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT                      = 0x0D8,
    MHW_GFX3DSTATE_SURFACEFORMAT_R24_UNORM_X8_TYPELESS          = 0x0D9,
    MHW_GFX3DSTATE_SURFACEFORMAT_X24_TYPELESS_G8_UINT           = 0x0DA,
    MHW_GFX3DSTATE_SURFACEFORMAT_L16A16_UNORM                   = 0x0DF,
    MHW_GFX3DSTATE_SURFACEFORMAT_I24X8_UNORM                    = 0x0E0,
    MHW_GFX3DSTATE_SURFACEFORMAT_L24X8_UNORM                    = 0x0E1,
    MHW_GFX3DSTATE_SURFACEFORMAT_A24X8_UNORM                    = 0x0E2,
    MHW_GFX3DSTATE_SURFACEFORMAT_I32_FLOAT                      = 0x0E3,
    MHW_GFX3DSTATE_SURFACEFORMAT_L32_FLOAT                      = 0x0E4,
    MHW_GFX3DSTATE_SURFACEFORMAT_A32_FLOAT                      = 0x0E5,
    MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8X8_UNORM                 = 0x0E9,
    MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8X8_UNORM_SRGB            = 0x0EA,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8X8_UNORM                 = 0x0EB,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8X8_UNORM_SRGB            = 0x0EC,
    MHW_GFX3DSTATE_SURFACEFORMAT_R9G9B9E5_SHAREDEXP             = 0x0ED,
    MHW_GFX3DSTATE_SURFACEFORMAT_B10G10R10X2_UNORM              = 0x0EE,
    MHW_GFX3DSTATE_SURFACEFORMAT_L16A16_FLOAT                   = 0x0F0,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_UNORM                      = 0x0F1,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_SNORM                      = 0x0F2,
    MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10X2_USCALED            = 0x0F3,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SSCALED               = 0x0F4,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_USCALED               = 0x0F5,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SSCALED                 = 0x0F6,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_USCALED                 = 0x0F7,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_SSCALED                    = 0x0F8,
    MHW_GFX3DSTATE_SURFACEFORMAT_R32_USCALED                    = 0x0F9,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8B8G8A8_UNORM                 = 0x0FA,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G6R5_UNORM                   = 0x100,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G6R5_UNORM_SRGB              = 0x101,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G5R5A1_UNORM                 = 0x102,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G5R5A1_UNORM_SRGB            = 0x103,
    MHW_GFX3DSTATE_SURFACEFORMAT_B4G4R4A4_UNORM                 = 0x104,
    MHW_GFX3DSTATE_SURFACEFORMAT_B4G4R4A4_UNORM_SRGB            = 0x105,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM                     = 0x106,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SNORM                     = 0x107,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SINT                      = 0x108,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UINT                      = 0x109,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM                      = 0x10A,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_SNORM                      = 0x10B,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_SINT                       = 0x10C,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT                       = 0x10D,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_FLOAT                      = 0x10E,
    MHW_GFX3DSTATE_SURFACEFORMAT_I16_UNORM                      = 0x111,
    MHW_GFX3DSTATE_SURFACEFORMAT_L16_UNORM                      = 0x112,
    MHW_GFX3DSTATE_SURFACEFORMAT_A16_UNORM                      = 0x113,
    MHW_GFX3DSTATE_SURFACEFORMAT_L8A8_UNORM                     = 0x114,
    MHW_GFX3DSTATE_SURFACEFORMAT_I16_FLOAT                      = 0x115,
    MHW_GFX3DSTATE_SURFACEFORMAT_L16_FLOAT                      = 0x116,
    MHW_GFX3DSTATE_SURFACEFORMAT_A16_FLOAT                      = 0x117,
    MHW_GFX3DSTATE_SURFACEFORMAT_L8A8_UNORM_SRGB                = 0x118,
    MHW_GFX3DSTATE_SURFACEFORMAT_R5G5_SNORM_B6_UNORM            = 0x119,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G5R5X1_UNORM                 = 0x11A,
    MHW_GFX3DSTATE_SURFACEFORMAT_B5G5R5X1_UNORM_SRGB            = 0x11B,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SSCALED                   = 0x11C,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_USCALED                   = 0x11D,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_SSCALED                    = 0x11E,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16_USCALED                    = 0x11F,
    MHW_GFX3DSTATE_SURFACEFORMAT_P8A8_UNORM_PALETTE_0           = 0x122,
    MHW_GFX3DSTATE_SURFACEFORMAT_P8A8_UNORM_PALETTE_1           = 0x123,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM                       = 0x140,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_SNORM                       = 0x141,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_SINT                        = 0x142,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_UINT                        = 0x143,
    MHW_GFX3DSTATE_SURFACEFORMAT_A8_UNORM                       = 0x144,
    MHW_GFX3DSTATE_SURFACEFORMAT_I8_UNORM                       = 0x145,
    MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM                       = 0x146,
    MHW_GFX3DSTATE_SURFACEFORMAT_P4A4_UNORM_PALETTE_0           = 0x147,
    MHW_GFX3DSTATE_SURFACEFORMAT_A4P4_UNORM_PALETTE_0           = 0x148,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_SSCALED                     = 0x149,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8_USCALED                     = 0x14A,
    MHW_GFX3DSTATE_SURFACEFORMAT_P8_UNORM_PALETTE_0             = 0x14B,
    MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM_SRGB                  = 0x14C,
    MHW_GFX3DSTATE_SURFACEFORMAT_P8_UNORM_PALETTE_1             = 0x14D,
    MHW_GFX3DSTATE_SURFACEFORMAT_P4A4_UNORM_PALETTE_1           = 0x14E,
    MHW_GFX3DSTATE_SURFACEFORMAT_A4P4_UNORM_PALETTE_1           = 0x14F,
    MHW_GFX3DSTATE_SURFACEFORMAT_DXT1_RGB_SRGB                  = 0x180,
    MHW_GFX3DSTATE_SURFACEFORMAT_R1_UINT                        = 0x181,
    MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_NORMAL                   = 0x182,
    MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY                  = 0x183,
    MHW_GFX3DSTATE_SURFACEFORMAT_P2_UNORM_PALETTE_0             = 0x184,
    MHW_GFX3DSTATE_SURFACEFORMAT_P2_UNORM_PALETTE_1             = 0x185,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC1_UNORM                      = 0x186,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC2_UNORM                      = 0x187,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC3_UNORM                      = 0x188,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC4_UNORM                      = 0x189,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC5_UNORM                      = 0x18A,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC1_UNORM_SRGB                 = 0x18B,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC2_UNORM_SRGB                 = 0x18C,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC3_UNORM_SRGB                 = 0x18D,
    MHW_GFX3DSTATE_SURFACEFORMAT_MONO8                          = 0x18E,
    MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUV                   = 0x18F,
    MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY                    = 0x190,
    MHW_GFX3DSTATE_SURFACEFORMAT_DXT1_RGB                       = 0x191,
    MHW_GFX3DSTATE_SURFACEFORMAT_FXT1                           = 0x192,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_UNORM                   = 0x193,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_SNORM                   = 0x194,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_SSCALED                 = 0x195,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_USCALED                 = 0x196,
    MHW_GFX3DSTATE_SURFACEFORMAT_R64G64B64A64_FLOAT             = 0x197,
    MHW_GFX3DSTATE_SURFACEFORMAT_R64G64B64_FLOAT                = 0x198,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC4_SNORM                      = 0x199,
    MHW_GFX3DSTATE_SURFACEFORMAT_BC5_SNORM                      = 0x19A,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_UNORM                = 0x19C,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_SNORM                = 0x19D,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_SSCALED              = 0x19E,
    MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16_USCALED              = 0x19F,
    MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8                   = 0x1A5,
    MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_16                  = 0x1A6,
    MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_UNORM_SRGB              = 0x1A8,
    MHW_GFX3DSTATE_SURFACEFORMAT_RAW                            = 0x1FF,
    NUM_MHW_GFX3DSTATE_SURFACEFORMATS
};

enum MHW_MEDIASTATE_SURFACEFORMAT
{
    MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL       = 0,
    MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY      = 1,
    MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV       = 2,
    MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY        = 3,
    MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8       = 4,
    MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8       = 5,    // deinterlace only
    MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8       = 6,    // deinterlace only
    MHW_MEDIASTATE_SURFACEFORMAT_STMM_DN_STATISTICS = 7,    // deinterlace only
    MHW_MEDIASTATE_SURFACEFORMAT_R10G10B10A2_UNORM  = 8,    // sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM     = 9,    // sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R8B8_UNORM         = 10,   // CrCb, sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM           = 11,   // Cr/Cb, sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM           = 12,   // sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM     = 13,   // sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_B8G8R8A8_UNORM     = 14,   // sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R16G16B16A16       = 15,   // Sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_16      = 23,   // Sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R16B16_UNORM       = 24,   // Sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_R16_UNORM          = 25,   // Sample_8x8 only
    MHW_MEDIASTATE_SURFACEFORMAT_Y16_UNORM          = 26    // Sample_8x8 only
};

enum GFX3DSTATE_SURFACETYPE
{
    GFX3DSTATE_SURFACETYPE_1D      = 0,
    GFX3DSTATE_SURFACETYPE_2D      = 1,
    GFX3DSTATE_SURFACETYPE_3D      = 2,
    GFX3DSTATE_SURFACETYPE_CUBE    = 3,
    GFX3DSTATE_SURFACETYPE_BUFFER  = 4,
    GFX3DSTATE_SURFACETYPE_SREBUF  = 5,  // Structured buffer surface.
    GFX3DSTATE_SURFACETYPE_SCRATCH = 6,  // Scratch space buffer.
    GFX3DSTATE_SURFACETYPE_NULL    = 7
};

enum SurfaceCacheabilityControl
{
    surfaceCacheabilityControlBitsFromGtt = 0x0,
    surfaceCacheabilityControlL3Cache     = 0x1,
    surfaceCacheabilityControlLLCCache    = 0x2
};

enum GFX3DSTATE_MEDIA_BOUNDARY_PIXEL_MODE
{
    GFX3DSTATE_BOUNDARY_NORMAL                  = 0x0,
    GFX3DSTATE_BOUNDARY_PROGRESSIVE_FRAME       = 0x2,
    GFX3DSTATE_BOUNDARY_INTERLACED_FRAME        = 0x3
};

enum TILED_RESOURCE_MODE_LEGACY
{
    TRMODE_NONE     = 0,
    TRMODE_TILEYF   = 1,
    TRMODE_TILEYS   = 2,
};

enum MEDIASTATE_SFC_PIPE_MODE
{
    MEDIASTATE_SFC_PIPE_VD_TO_SFC          = 0,
    MEDIASTATE_SFC_PIPE_VE_TO_SFC          = 1,
    MEDIASTATE_SFC_PIPE_VE_TO_SFC_INTEGRAL = 4
};

enum MEDIASTATE_SFC_AVS_FILTER_MODE
{
    MEDIASTATE_SFC_AVS_FILTER_5x5 = 0,
    MEDIASTATE_SFC_AVS_FILTER_8x8 = 1,
    MEDIASTATE_SFC_AVS_FILTER_BILINEAR = 2
};

enum MEDIASTATE_SFC_CHROMA_SUBSAMPLING_MODE
{
    MEDIASTATE_SFC_CHROMA_SUBSAMPLING_400  = 0,
    MEDIASTATE_SFC_CHROMA_SUBSAMPLING_420  = 1,
    MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H = 2,
    MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444  = 4,
    MEDIASTATE_SFC_CHROMA_SUBSAMPLING_411  = 5
};

enum MEDIASTATE_SFC_INPUT_ORDERING_MODE
{
    MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8           = 0,
    MEDIASTATE_SFC_INPUT_ORDERING_VE_4x4           = 1,
    MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8_128pixel  = 2,
    MEDIASTATE_SFC_INPUT_ORDERING_VE_4x4_128pixel  = 3,
    MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_NOSHIFT = 0,
    MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_SHIFT   = 1,
    MEDIASTATE_SFC_INPUT_ORDERING_VD_8x8_JPEG      = 2,
    MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_JPEG    = 3,
    MEDIASTATE_SFC_INPUT_ORDERING_VD_16x16_VP8     = 4
};

// SFC Pre-AVS Chroma Downsampling Mode
enum  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_MODE
{
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_DISABLED   = 0x0,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO420   = 0x1,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO422   = 0x2,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_422TO420   = 0x3
};

// SFC Pre-AVS Chroma Downsampling Coefficient -- Fractional Position of the Bilinear Filter
enum MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF
{
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8   = 0x0,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_1_OVER_8   = 0x1,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_2_OVER_8   = 0x2,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_3_OVER_8   = 0x3,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8   = 0x4,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_5_OVER_8   = 0x5,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_6_OVER_8   = 0x6,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_7_OVER_8   = 0x7,
    MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8   = 0x8
};

enum MEDIASTATE_VDIRECTION
{
    MEDIASTATE_VDIRECTION_TOP_FIELD    = 1,
    MEDIASTATE_VDIRECTION_FULL_FRAME   = 2,
    MEDIASTATE_VDIRECTION_BOTTOM_FIELD = 3
};

typedef union _MHW_MEMORY_OBJECT_CONTROL_PARAMS
{
    struct
    {
        uint32_t   L3              : 1 ;
        uint32_t   CacheControl    : 1 ;
        uint32_t   GFDT            : 1 ;
        uint32_t                   : 1 ;
        uint32_t                   : 28;
    } Gen7;
    struct
    {
        uint32_t   L3              : 1 ;
        uint32_t   CacheControl    : 2 ;
        uint32_t                   : 1 ;
        uint32_t                   : 28;
    } Gen7_5;
    struct
    {
        uint32_t   Age             : 2 ;
        uint32_t                   : 1 ;
        uint32_t   TargetCache     : 2 ;
        uint32_t   CacheControl    : 2 ;
        uint32_t                   : 25;
    } Gen8;
    struct
    {
        uint32_t                   : 1 ;
        uint32_t   Index           : 6 ;
        uint32_t                   : 25;
    } Gen9;
    uint32_t       Value;
} MHW_MEMORY_OBJECT_CONTROL_PARAMS, *PMHW_MEMORY_OBJECT_CONTROL_PARAMS;

typedef struct _MHW_RENDER_PWR_CLK_STATE_PARAMS
{
    union
    {
        struct
        {
            // 0010: 2 EUs
            // 0100: 4 EUs
            // 0110: 6 EUs
            // 1000: 8 EUs
            uint32_t EUmin : BITFIELD_RANGE(0, 3); // Minimum number of EUs to power (per subslice if multiple subslices enabled)
            uint32_t EUmax : BITFIELD_RANGE(4, 7); // Maximum number of EUs to power (per subslice if multiple subslices enabled)
                                              //  To specify an exact number of subslices, set EUmax equal to EUmin

            uint32_t SubSliceCount : BITFIELD_RANGE(8, 10);

            uint32_t SSCountEn : BITFIELD_RANGE(11, 11);

            uint32_t SliceCount : BITFIELD_RANGE(12, 17);

            uint32_t SCountEn : BITFIELD_RANGE(18, 18);

            uint32_t Reserved1 : BITFIELD_RANGE(19, 30);

                                                    // Main trigger: Power Clock State Enable
                                                    //  0: No specific power state set, no message/wait with PMunit
                                                    //  1: CSunit sends the contents of this register to PMunit each time it is written, Send contents of this register to PMunit, wait for Ack.
            uint32_t PowerClkStateEn : BITFIELD_RANGE(31, 31);
        };

        uint32_t Data;
    };

}MHW_RENDER_PWR_CLK_STATE_PARAMS;

struct _MHW_BATCH_BUFFER
{
    MOS_RESOURCE            OsResource;
    int32_t                 iRemaining;                     //!< Remaining space in the BB
    int32_t                 iSize;                          //!< Command buffer size
    uint32_t                count;                          //!< Actual batch count in this resource. If larger than 1, multiple buffer has equal size and resource size count * size.
    int32_t                 iCurrent;                       //!< Current offset in CB
    bool                    bLocked;                        //!< True if locked in memory (pData must be valid)
    uint8_t                 *pData;                          //!< Pointer to BB data
#if (_DEBUG || _RELEASE_INTERNAL)
    int32_t                     iLastCurrent;                   //!< Save offset in CB (for debug plug-in/out)
#endif

    // User defined
    bool                    bSecondLevel; // REMOVE REMOVE
    uint32_t                dwOffset;                       //!< Offset to the data in the OS resource

    // Batch Buffer synchronization logic
    bool                    bBusy;                          //!< Busy flag (clear when Sync Tag is reached)
    uint32_t                dwCmdBufId;                     //!< Command Buffer ID for the workload
    PMHW_BATCH_BUFFER       pNext;                          //!< Next BB in the sync list
    PMHW_BATCH_BUFFER       pPrev;                          //!< Prev BB in the sync list

    // Batch Buffer Client Private Data
    uint32_t                dwSyncTag;
    bool                    bMatch;
    int32_t                 iPrivateType;                   //!< Indicates the BB client
    int32_t                 iPrivateSize;                   //!< Size of the current render args
    void                    *pPrivateData;                   //!< Pointer to private BB data
};

typedef struct _MHW_BATCH_BUFFER_LIST
{
    PMHW_BATCH_BUFFER       pHead;                          //!< First element in the list
    PMHW_BATCH_BUFFER       pTail;                          //!< Last element in the list
    int32_t                 iCount;                         //!< Number of BBs in this list
    uint32_t                dwSize;                         //!< Total BB memory in this list
} MHW_BATCH_BUFFER_LIST,*PMHW_BATCH_BUFFER_LIST;

enum WRITE_FLAG
{
    WRITE     = 0x1,
    WRITE_WA  = 0x2,
};

typedef struct _MHW_RESOURCE_PARAMS
{
    PMOS_RESOURCE                       presResource;
    uint32_t                            dwOffset;
    uint32_t                            *pdwCmd;
    uint32_t                            dwLocationInCmd;
    uint32_t                            dwLsbNum;
    uint32_t                            dwOffsetInSSH;

    // Location of upper bound value relative to
    // allocated resource address. The upper bound
    // value will be set if this parameter is > zero
    uint32_t                            dwUpperBoundLocationOffsetFromCmd;
    uint32_t                            dwSize;

    MOS_HW_COMMAND                      HwCommandType;
    uint32_t                            dwSharedMocsOffset;
    uint32_t                            bIsWritable;

    // If the patching location does not start at bit 0 then the value to be patched needs to be shifted
    uint32_t                            shiftAmount;
    uint32_t                            shiftDirection;
    MOS_PATCH_TYPE                      patchType;
}MHW_RESOURCE_PARAMS, *PMHW_RESOURCE_PARAMS;

typedef struct _MHW_GENERIC_PROLOG_PARAMS
{
    PMOS_INTERFACE              pOsInterface;
    void                        *pvMiInterface;
    bool                        bMmcEnabled;
    PMOS_RESOURCE               presStoreData;
    uint32_t                    dwStoreDataOffset;
    uint32_t                    dwStoreDataValue;
} MHW_GENERIC_PROLOG_PARAMS, *PMHW_GENERIC_PROLOG_PARAMS;

MOS_STATUS Mhw_AddResourceToCmd_GfxAddress(
    PMOS_INTERFACE              pOsInterface,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_RESOURCE_PARAMS        pParams);

MOS_STATUS Mhw_AddResourceToCmd_PatchList(
    PMOS_INTERFACE              pOsInterface,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_RESOURCE_PARAMS        pParams);

MOS_STATUS Mhw_SurfaceFormatToType(
    uint32_t                    dwForceSurfaceFormat,
    PMOS_SURFACE                psSurface,
    uint32_t                    *pdwSurfaceType);

MOS_STATUS Mhw_SendGenericPrologCmd(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_GENERIC_PROLOG_PARAMS  pParams,
    MHW_MI_MMIOREGISTERS       *pMmioReg = nullptr);

MOS_STATUS Mhw_SetNearestModeTable(
    int32_t         *iCoefs,
    uint32_t        dwPlane,
    bool            bBalancedFilter);

MOS_STATUS Mhw_CalcPolyphaseTablesY(
    int32_t         *iCoefs,
    float           fScaleFactor,
    uint32_t        dwPlane,
    MOS_FORMAT      srcFmt,
    float           fHPStrength,
    bool            bUse8x8Filter,
    uint32_t        dwHwPhase);

MOS_STATUS Mhw_CalcPolyphaseTablesUV(
    int32_t  *piCoefs,
    float    fLanczosT,
    float    fInverseScaleFactor);

MOS_STATUS Mhw_CalcPolyphaseTablesUVOffset(
    int32_t     *piCoefs,
    float       fLanczosT,
    float       fInverseScaleFactor,
    int32_t     iUvPhaseOffset);

MOS_STATUS Mhw_AllocateBb(
    PMOS_INTERFACE          pOsInterface,
    PMHW_BATCH_BUFFER       pBatchBuffer,
    PMHW_BATCH_BUFFER       pBatchBufferList,
    uint32_t                dwSize,
    uint32_t                batchCount=1);

MOS_STATUS Mhw_FreeBb(
    PMOS_INTERFACE          pOsInterface,
    PMHW_BATCH_BUFFER       pBatchBuffer,
    PMHW_BATCH_BUFFER       pBatchBufferList);

MOS_STATUS Mhw_LockBb(
    PMOS_INTERFACE          pOsInterface,
    PMHW_BATCH_BUFFER       pBatchBuffer);

MOS_STATUS Mhw_UnlockBb(
    PMOS_INTERFACE          pOsInterface,
    PMHW_BATCH_BUFFER       pBatchBuffer,
    bool                    bResetBuffer);

uint32_t Mhw_ConvertToTRMode(
    MOS_TILE_TYPE           Type);

//*-----------------------------------------------------------------------------
//| Purpose:    Function to add command to batch buffer
//|             Also used to skip commands if pCmd provided is nullptr
//| Return:     MOS_STATUS_SUCCESS if call succeeds
//*-----------------------------------------------------------------------------
static __inline MOS_STATUS Mhw_AddCommandBB(
    PMHW_BATCH_BUFFER           pBatchBuffer,   // [in] Pointer to Batch Buffer
    void                        *pCmd,           // [in] Command Pointer
    uint32_t                    dwCmdSize)      // [in] Size of command in bytes
{
    uint8_t     *pbBatchPtr;
    uint32_t    dwCmdSizeDwAligned;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------------
    MHW_CHK_NULL(pBatchBuffer);
    MHW_CHK_NULL(pBatchBuffer->pData);
    //---------------------------------------------

    pbBatchPtr = pBatchBuffer->pData + pBatchBuffer->iCurrent;

    dwCmdSizeDwAligned = MOS_ALIGN_CEIL(dwCmdSize, sizeof(uint32_t));

    pBatchBuffer->iCurrent += dwCmdSizeDwAligned;

    if (pCmd)
    {
        pBatchBuffer->iRemaining -= dwCmdSizeDwAligned;
        if (pBatchBuffer->iRemaining < 0)
        {
            MHW_ASSERTMESSAGE("Unable to add command (no space).");
            return MOS_STATUS_UNKNOWN;
        }

        eStatus = MOS_SecureMemcpy(pbBatchPtr, dwCmdSize, pCmd, dwCmdSize);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Function to add command to batch buffer
//| Return:     MOS_STATUS_SUCCESS if call succeeds
//*-----------------------------------------------------------------------------
static __inline MOS_STATUS Mhw_AddCommandCmdOrBB(
    void       *pCmdBuffer,     // [in] Pointer to Command Buffer
    void       *pBatchBuffer,   // [in] Pointer to Batch Buffer
    void       *pCmd,           // [in] Command Pointer
    uint32_t   dwCmdSize)      // [in] Size of command in bytes
{
    if (pCmdBuffer)
    {
        return ((MOS_STATUS)Mos_AddCommand((PMOS_COMMAND_BUFFER)pCmdBuffer, pCmd, dwCmdSize));
    }
    else
    {
        return (Mhw_AddCommandBB((PMHW_BATCH_BUFFER)pBatchBuffer, pCmd, dwCmdSize));
    }
}

#endif // __MHW_UTILITIES_H__
