/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file        mos_defs.h 
//! \brief 
//!
//!
//! \file     mos_defs.h
//! \brief    Defines common types and macros across different platform
//! \details  Defines common types and macros across different platform
//!
#ifndef __MOS_DEFS_H__
#define __MOS_DEFS_H__

// The below definitions will prevent other files from redefining types.
#define BASIC_TYPES_DEFINED 1
#define BOOL_DEF            1  

#include <stdio.h>       // FILE
#include <stdint.h>
#include "mos_defs_specific.h"

//!
//! \brief Macros for enabling / disabling debug prints and asserts
//!

//! Asserts are only enabled for debug drivers:
#define MOS_ASSERT_ENABLED      (_DEBUG)

//! Debug messages are only enabled for debug or release-internal drivers.
//! If Asserts have been enabled manually, Messages should be enabled to:
#define MOS_MESSAGES_ENABLED    (_DEBUG || _RELEASE_INTERNAL || MOS_ASSERT_ENABLED)

//!
//! \brief Macros for enabling / disabling development features in MOS
//!

//! MediaSolo is only supported for non-production builds
#if ((_DEBUG || _RELEASE_INTERNAL ) && !defined(ANDROID) && _MEDIA_SOLO_SUPPORTED)
#define MOS_MEDIASOLO_SUPPORTED 1
#else
#define MOS_MEDIASOLO_SUPPORTED 0
#endif

//! Command buffer dumps are a debug feature so should not be enabled in release builds
#define MOS_COMMAND_BUFFER_DUMP_SUPPORTED   (_DEBUG || _RELEASE_INTERNAL)

//! Command res info dumps are a debug feature so should not be enabled in release builds
#define MOS_COMMAND_RESINFO_DUMP_SUPPORTED (_DEBUG || _RELEASE_INTERNAL)

typedef FILE*                   PFILE;                      //!< Pointer to a File
typedef FILE**                  PPFILE;                     //!< Pointer to a PFILE
typedef HMODULE*                PHMODULE;                   //!< Pointer to an HMODULE
typedef const void*             PCVOID;                     //!< Pointer to opaque const handle
typedef void**                  PPVOID;                     //!< Pointer to PVOID
typedef const char*             PCCHAR;                     //!< Pointer to 8 bit signed const value
typedef char**                  PPCHAR;                     //!< Pointer to a PCHAR

//------------------------------------------------------------------------------
// SECTION: Macros
//
// ABSTRACT: Provides basic helper utilities like mask bits, min, max, floor,
//      ceil, align, etc.
//------------------------------------------------------------------------------

//!
//! \brief Page Size
//!
#define MOS_PAGE_SIZE                       4096

//!
//! \brief PI
//!
#define MOS_PI                              3.14159265358979324f

//!
//! \def MOS_BYTES_TO_DWORDS(_b)
//!  Convert \a b Bytes -> DWORDs
//!
#define MOS_BYTES_TO_DWORDS(_b)             (((_b) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

//!
//! \def MOS_BYTES_TO_PAGES(_b)
//!  Convert \a b Bytes -> Pages
//!
#define MOS_BYTES_TO_PAGES(_b)              (((_b) + MOS_PAGE_SIZE - 1)/(MOS_PAGE_SIZE))

//!
//! \def MOS_PAGES_TO_BYTES(_p)
//!  Convert \a _p Pages -> Bytes
//!
#define MOS_PAGES_TO_BYTES(_p)              ((_p) * MOS_PAGE_SIZE)

//!
//! \def MOS_KB_TO_BYTES(_kb)
//!  Convert \a _kb Kilobyte -> Bytes
//!
#define MOS_KB_TO_BYTES(_kb)                ((_kb) * 1024)

//!
//! \def MOS_MB_TO_BYTES(_mb)
//!  Convert \a _mb Megabyte -> Bytes
//!
#define MOS_MB_TO_BYTES(_mb)                (( _mb) * 1024 * 1024)

//!
//! \def MOS_IS_ALIGNED(_a, _alignment)
//!  Is \a _a aligned to \a _alignment already?
//!
#define MOS_IS_ALIGNED(_a, _alignment)      ((((_a) & ((_alignment)-1)) == 0) ? 1 : 0)

//!
//! \def MOS_ALIGN_CEIL(_a, _alignment)
//!  Ceiling Align \a _a to \a _alignment
//!
#define MOS_ALIGN_CEIL(_a, _alignment)      (((_a) + ((_alignment)-1)) & (~((_alignment)-1)))

//!
//! \def MOS_ALIGN_OFFSET(_a, _alignment)
//!  Offset to align \a _a to \a _alignment
//!
#define MOS_ALIGN_OFFSET(_a, _alignment)    ((~(_a) + 1) & (_alignment-1))

//!
//! \def MOS_ALIGN_FLOOR(_a, _alignment)
//!  Floor align \a _a to \a _alignment
//!
#define MOS_ALIGN_FLOOR(_a, _alignment)     ((_a) & (~((_alignment)-1)))

//!
//! \def MOS_ROUNDUP_SHIFT(_a, _shift)
//!  Roundup-shift \a _a by \a _shift
//!
#define MOS_ROUNDUP_SHIFT(_a, _shift)       (((_a) + (1 << (_shift)) - 1) >> (_shift))

//!
//! \def MOS_ROUNDUP_DIVIDE(_x, _divide)
//!  Returns the \a x / \a divide rounded up
//!
#define MOS_ROUNDUP_DIVIDE(_x, _divide)     (((_x) + (_divide) - 1) / (_divide))

//!
//! \def MOS_NUMBER_OF_ELEMENTS(_A)
//!  Returns the number of elements in array \a _A
//!
#define MOS_NUMBER_OF_ELEMENTS(_A)          (sizeof(_A) / sizeof((_A)[0]))

//!
//! \def MOS_MIN(_a, _b)
//!  Returns the lesser of \a _a and \a _b
//!
#define MOS_MIN(_a, _b)                     (((_a) < (_b)) ? (_a) : (_b))

//!
//! \def MOS_MIN3(_a, _b, _c)
//! Returns the lesser of \a _a, \a _b and \a _c
//!
#define MOS_MIN3(_a, _b, _c)                MOS_MIN(MOS_MIN(_a, _b), _c)

//!
//! \def MOS_MAX(_a, _b)
//!  Returns the greater of \a a and \a _b
//!
#define MOS_MAX(_a, _b)                     (((_a) > (_b)) ? (_a) : (_b))

//!
//! \def MOS_MAX3(_a, _b, _c)
//!  Returns the greater of \a _a, \a _b and \a _c
//!
#define MOS_MAX3(_a, _b, _c)                MOS_MAX(MOS_MAX(_a, _b), _c)

//!
//! \def MOS_CLAMP_MIN_MAX(_a, min, max)
//! Return value between min and max (either _a, min or max)
//!
#define MOS_CLAMP_MIN_MAX(_a, min, max)     ((_a) < (min) ? (min) : MOS_MIN ((_a), (max)))

//!
//! \def MOS_UF_ROUND(a)
//!  Rounds float \a a to an uint32_t
//!
#define MOS_UF_ROUND(a)          ((uint32_t) ((a) + 0.5F))

//!
//! \def MOS_F_ROUND(a) 
//!  Rounds float \a a to a int32_t
//!
#define MOS_F_ROUND(a)           ((int32_t) ((a) + ((a) < 0 ? -0.5F : 0.5F)))

//!
//! \def MOS_MASK(_low, _high)
//!  Returns a mask of bits set in the range from \a _low to \a _high
//!
#define MOS_MASK(_low, _high)    ((((uint32_t)1) << (_high)) |    \
                                 ((((uint32_t)1) << (_high)) -    \
                                  (((uint32_t)1) << (_low))))
//!
//! \def MOS_MASKBITS32(_low, _high)
//!  Returns a mask of bits set in the range from \a _low to \a _high
//!
#define MOS_MASKBITS32(_low, _high)         ((((((uint32_t)1) << (_high+1)) - 1) >> _low) << _low)

//!
//! \def MOS_MASKBITS64(_low, _high)
//!  Returns a mask of bits set in the range from \a _low to \a _high
//!
#define MOS_MASKBITS64(_low, _high)         ((((((uint64_t)1) << (_high+1)) - 1) >> _low) << _low)

//!
//! \def MOS_BITFIELD_BIT_N(n)
//!  Returns an uint32_t with bit n set to 1
//!
#define MOS_BITFIELD_BIT_N(n)               ((uint32_t)1 << (n))

//!
//! \def MOS_BITFIELD_RANGE(_startbit, _endbit)
//!  Calculates the number of bits between the startbit and the endbit (0 based).
//!
#define MOS_BITFIELD_RANGE(_startbit, _endbit)  ((_endbit) - (_startbit) + 1)

//!
//! \def MOS_BITFIELD_BIT(_bit)
//!  Definition declared for clarity when creating structs.
//!
#define MOS_BITFIELD_BIT(_bit)                      1

//!
//! \def MOS_BIT_ON(_a, _bit)
//!  Sets \a _bit in \a _a
//!
#define MOS_BIT_ON(_a, _bit)                ((_a) |= (_bit))

//!
//! \def MOS_BIT_OFF(_a, _bit)
//!  Unsets \a _bit in \a _a
//!
#define MOS_BIT_OFF(_a, _bit)               ((_a) &= ~(_bit))

//!
//! \def MOS_IS_BIT_SET(_a, _bit)
//!  Determines if \a _bit in \a _a is 1
//!
#define MOS_IS_BIT_SET(_a, _bit)            ((_a) & (_bit))

//!
//! \def MOS_ABS(_x)
//! Calculate the Absolute value of \a _x.
//!
#define MOS_ABS(_x)                         (((_x) > 0) ? (_x) : -(_x))

//!
//! \def MOS_WITHIN_RANGE(_x, _min, _max)
//! Check that value within provided (_min, _max) range
//!
#define MOS_WITHIN_RANGE(_x, _min, _max)  (((_x >= _min) && (_x <= _max)) ? (true) : (false))

//!
//! \def MOS_ALIGNED(_alignment)
//!  Structure Align to \a _alignment
//!
#if defined(_MSC_VER)
#define MOS_ALIGNED(_alignment) __declspec(align(_alignment))
#else
#define MOS_ALIGNED(_alignment) __attribute__ ((aligned(_alignment)))
#endif

//!
//! \def MOS_UNUSED(param)
//! Fix compiling warning for unused parameter
//!
#define MOS_UNUSED(param) (void)(param)

#define MOS_BITFIELD_VALUE(_x, _bits)         ((_x) & ((1 << (_bits)) - 1))

//------------------------------------------------------------------------------
//
//                          Structures and enumerations
//
//------------------------------------------------------------------------------

//!
//! \brief Function return codes.
//!
typedef enum _MOS_STATUS
{
    MOS_STATUS_SUCCESS                           =  0,
    MOS_STATUS_NO_SPACE                          =  1,
    MOS_STATUS_INVALID_PARAMETER                 =  2,
    MOS_STATUS_INVALID_HANDLE                    =  3,
    MOS_STATUS_INVALID_FILE_SIZE                 =  4,
    MOS_STATUS_NULL_POINTER                      =  5,
    MOS_STATUS_FILE_EXISTS                       =  6,
    MOS_STATUS_FILE_NOT_FOUND                    =  7,
    MOS_STATUS_FILE_OPEN_FAILED                  =  8,
    MOS_STATUS_FILE_READ_ONLY                    =  9,
    MOS_STATUS_FILE_READ_FAILED                  = 10,
    MOS_STATUS_FILE_WRITE_FAILED                 = 11,
    MOS_STATUS_DIR_CREATE_FAILED                 = 12,
    MOS_STATUS_SET_FILE_POINTER_FAILED           = 13,
    MOS_STATUS_LOAD_LIBRARY_FAILED               = 14,
    MOS_STATUS_MORE_DATA                         = 15,
    MOS_STATUS_USER_CONTROL_MAX_NAME_SIZE        = 16,
    MOS_STATUS_USER_CONTROL_MIN_DATA_SIZE        = 17,
    MOS_STATUS_USER_CONTROL_MAX_DATA_SIZE        = 18,
    MOS_STATUS_USER_FEATURE_KEY_READ_FAILED      = 19,
    MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED     = 20,
    MOS_STATUS_EVENT_WAIT_REGISTER_FAILED        = 21,
    MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED      = 22,
    MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED      = 23,
    MOS_STATUS_HLT_INIT_FAILED                   = 24,
    MOS_STATUS_UNIMPLEMENTED                     = 25,
    MOS_STATUS_EXCEED_MAX_BB_SIZE                = 26,
    MOS_STATUS_PLATFORM_NOT_SUPPORTED            = 27,
    MOS_STATUS_CLIENT_AR_NO_SPACE                = 28,
    MOS_STATUS_HUC_KERNEL_FAILED                 = 29,
    MOS_STATUS_NOT_ENOUGH_BUFFER                 = 30,
    MOS_STATUS_UNINITIALIZED                     = 31,
    MOS_STATUS_GPU_CONTEXT_ERROR                 = 32,
    MOS_STATUS_STILL_DRAWING                     = 33,
    MOS_STATUS_UNKNOWN                           = 34
} MOS_STATUS;

//!
//! \def MOS_SUCCEEDED(_status)
//!  Generic test for success on any MOS_STATUS value
//!  Currently only MOS_STATUS_SUCCESS indicates success
//!
#define MOS_SUCCEEDED(_status)                                                               \
    (_status == MOS_STATUS_SUCCESS)

//!
//! \def MOS_FAILED(_status)
//!  Generic test for failure on any MOS_STATUS value
//!  Currently any non MOS_STATUS_SUCCESS value indicates failure
//!
#define MOS_FAILED(_status)                                                                 \
    (_status != MOS_STATUS_SUCCESS)

//!
//! \brief Feature Null Rendering Control Flags
//!
typedef struct _MOS_NULL_RENDERING_FLAGS {
    union
    {
        struct
        {
            uint32_t CodecGlobal         : 1;
            uint32_t CtxRender           : 1;
            uint32_t CtxRender2          : 1;
            uint32_t CtxVideo            : 1;
            uint32_t CtxVideo2           : 1;
            uint32_t CtxVideo3           : 1;
            uint32_t CtxVDBox2Video      : 1;
            uint32_t CtxVDBox2Video2     : 1;
            uint32_t CtxVDBox2Video3     : 1;
            uint32_t Reserved1           : 3;
            uint32_t VPBitCopy           : 1;
            uint32_t VPDnDi              : 1;
            uint32_t VPDrDb              : 1;
            uint32_t VPFrc               : 1;
            uint32_t VPIs                : 1;
            uint32_t VPComp              : 1;
            uint32_t VP3P                : 1;
            uint32_t VPLgca              : 1;
            uint32_t VPGobal             : 1;
            uint32_t VPDpRotation        : 1;
            uint32_t VPFDFB              : 1;
            uint32_t VPEu3dLut           : 1;
            uint32_t Cm                  : 1;
            uint32_t Mmc                 : 1;
            uint32_t Reserved3           : 6;
        };
        uint32_t     Value;
    };
} MOS_NULL_RENDERING_FLAGS;

//!
//! \brief GPU Context definitions
//!

typedef enum _MOS_GPU_CONTEXT
{
    MOS_GPU_CONTEXT_RENDER          = 0,
    MOS_GPU_CONTEXT_RENDER2         = 1,
    MOS_GPU_CONTEXT_VIDEO           = 2,
    MOS_GPU_CONTEXT_VIDEO2          = 3,
    MOS_GPU_CONTEXT_VIDEO3          = 4,
    MOS_GPU_CONTEXT_VIDEO4          = 5,
    MOS_GPU_CONTEXT_VEBOX           = 6,
    MOS_GPU_OVERLAY_CONTEXT         = 7,
    MOS_GPU_CONTEXT_VDBOX2_VIDEO    = 8,  // VDBox2 Decode
    MOS_GPU_CONTEXT_VDBOX2_VIDEO2   = 9,  // VDBox2 Decode Was
    MOS_GPU_CONTEXT_VDBOX2_VIDEO3   = 10, // VDBox2 PAK
    MOS_GPU_CONTEXT_RENDER3         = 11, // MDF(aka. CM)
    MOS_GPU_CONTEXT_RENDER4         = 12, // MDF Custom context
    MOS_GPU_CONTEXT_VEBOX2          = 13, // Vebox2
    MOS_GPU_CONTEXT_COMPUTE         = 14, //Compute Context
    MOS_GPU_CONTEXT_CM_COMPUTE      = 15, // MDF Compute
    MOS_GPU_CONTEXT_RENDER_RA       = 16, // render context for RA mode
    MOS_GPU_CONTEXT_COMPUTE_RA      = 17, // compute context for RA mode
    MOS_GPU_CONTEXT_VIDEO5          = 18, // Decode Node 0 Split 2
    MOS_GPU_CONTEXT_VIDEO6          = 19, // Encode Node 0 Split 2
    MOS_GPU_CONTEXT_VIDEO7          = 20, // Decode Node 0 Split 3
    MOS_GPU_CONTEXT_BLT             = 21,
    MOS_GPU_CONTEXT_RTE             = 22, // RTE context
    MOS_GPU_CONTEXT_MAX             = 23,
    MOS_GPU_CONTEXT_INVALID_HANDLE  = 0xFFFFA
} MOS_GPU_CONTEXT, *PMOS_GPU_CONTEXT;

/*****************************************************************************\
ENUM: MOS_GPU_COMPONENT_ID
\*****************************************************************************/
typedef enum _MOS_GPU_COMPONENT_ID
{
    MOS_GPU_COMPONENT_VP,
    MOS_GPU_COMPONENT_CM,
    MOS_GPU_COMPONENT_DECODE,
    MOS_GPU_COMPONENT_ENCODE,
    MOS_GPU_COMPONENT_DEFAULT,
    MOS_GPU_COMPONENT_ID_MAX
} MOS_GPU_COMPONENT_ID;

//!
//! \brief Enum for Stereoscopic 3D channel
//!
enum MOS_S3D_CHANNEL
{
    MOS_S3D_NONE,
    MOS_S3D_LEFT,
    MOS_S3D_RIGHT
};

//!
//! \brief Structure to OS plane offset
//!
struct MOS_PLANE_OFFSET
{
    int iSurfaceOffset;    //!< Plane surface offset
    int iXOffset;          //!< X offset - horizontal offset in pixels
    int iYOffset;          //!< Y offset - vertical offset in pixels
    int iLockSurfaceOffset;//!< Locked surface offset
};
typedef struct MOS_PLANE_OFFSET *PMOS_PLANE_OFFSET;

//!
//! \brief Structure to Resource offsets
//!
struct MOS_RESOURCE_OFFSETS
{
    uint32_t BaseOffset;  //!< Nearest page aligned byte offset to surface origin
    uint32_t XOffset;     //!< X coordinate to surface origin. In Bytes
    uint32_t YOffset;     //!< Y coordinate to surface origin. In rows.
};

//!
//! \brief Enum for Memory Compression states
//!
enum MOS_MEMCOMP_STATE
{
    MOS_MEMCOMP_DISABLED = 0,
    MOS_MEMCOMP_HORIZONTAL,
    MOS_MEMCOMP_VERTICAL,
    MOS_MEMCOMP_MC,
    MOS_MEMCOMP_RC
};
typedef enum MOS_MEMCOMP_STATE *PMOS_MEMCOMP_STATE;
typedef uint32_t               GPU_CONTEXT_HANDLE;

#define MOS_MAX_ENGINE_INSTANCE_PER_CLASS   8

//APO wrapper
extern uint32_t g_apoMosEnabled;
#define MOS_INVALID_HANDLE 0

#endif // __MOS_DEFS_H__
