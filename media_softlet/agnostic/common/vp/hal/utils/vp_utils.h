/*
* Copyright (c) 2018-2023, Intel Corporation
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
#ifndef __VP_UTILS_H__
#define __VP_UTILS_H__

#include "mos_util_debug.h"
#include "vp_common.h"

using MosFormatArray = std::vector<MOS_FORMAT>;

#define VP_UNUSED(param) (void)(param)
//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_ENCODE sub-comp
//------------------------------------------------------------------------------
#define VP_HW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _expr)

#define VP_HW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VP_HW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VP_HW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VP_HW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW)

#define VP_HW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _stmt)

#define VP_HW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

#define VP_HW_CHK_NULL_RETURN(_ptr)                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_HW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_PUBLIC sub-comp
//------------------------------------------------------------------------------
#define VP_PUBLIC_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _expr)

#define VP_PUBLIC_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VP_PUBLIC_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VP_PUBLIC_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VP_PUBLIC_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC)

#define VP_PUBLIC_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VP_PUBLIC_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VP_PUBLIC_CHK_VALUE_RETURN(_value, _expect_value)                         \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC,                \
        ((_value) == (_expect_value)) ? MOS_STATUS_SUCCESS : MOS_STATUS_INVALID_PARAMETER)

#define VP_PUBLIC_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_PUBLIC_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_PUBLIC_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_CHK_SPACE_NULL_RETURN(_ptr)                                                            \
{                                                                                                 \
    if ((_ptr) == nullptr)                                                                        \
    {                                                                                             \
        MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, "Invalid (nullptr) Pointer."); \
        return MOS_STATUS_NO_SPACE;                                                               \
    }                                                                                             \
}

#define VP_PUBLIC_CHK_NOT_FOUND_RETURN(_handle, _group)                         \
{                                                                               \
    if (_handle == (_group)->end())                                             \
    {                                                                           \
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);                   \
    }                                                                           \
}
//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define VP_DEBUG_ASSERT(_expr)                                                    \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _expr)

#define VP_DEBUG_ASSERTMESSAGE(_message, ...)                                     \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VP_DEBUG_NORMALMESSAGE(_message, ...)                                     \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VP_DEBUG_VERBOSEMESSAGE(_message, ...)                                    \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VP_FUNCTION_VERBOSEMESSAGE(_message, ...)                                 \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_ENTRY_VERBOSE, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VP_DEBUG_FUNCTION_ENTER                                                   \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VP_DEBUG_CHK_STATUS(_stmt)                                                \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _stmt)

#define VP_DEBUG_CHK_STATUS_RETURN(_stmt)                                         \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _stmt)

#define VP_DEBUG_CHK_NULL(_ptr)                                                   \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

#define VP_DEBUG_CHK_NULL_RETURN(_ptr)                                            \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_DEBUG_CHK_NULL_NO_STATUS(_ptr)                                         \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define VP_RENDER_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define VP_RENDER_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VP_RENDER_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VP_RENDER_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VP_RENDER_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define VP_RENDER_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VP_RENDER_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VP_RENDER_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VP_RENDER_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define VP_RENDER_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VP_RENDER_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VP_RENDER_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DDI sub-comp
//------------------------------------------------------------------------------
#define VP_DDI_ASSERT(_expr)                                                         \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _expr)

#define VP_DDI_ASSERTMESSAGE(_message, ...)                                          \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_NORMALMESSAGE(_message, ...)                                          \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_VERBOSEMESSAGE(_message, ...)                                         \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_FUNCTION_ENTER                                                        \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI)

#define VP_DDI_CHK_STATUS(_stmt)                                                     \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt)

#define VP_DDI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                              \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

#define VP_DDI_CHK_NULL(_ptr)                                                        \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VP_DDI_CHK_NULL_RETURN(_ptr)                                                 \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VP_DDI_CHK_NULL_NO_STATUS(_ptr)                                              \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_HR(_ptr)                                                       \
    MOS_CHK_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_NULL_WITH_HR(_ptr)                                             \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)
namespace vp
{
class Trace
{
public:
    Trace(const char* name) : m_name(name)
    {
        if (g_perfutility && (g_perfutility->dwPerfUtilityIsEnabled & VP_HAL))
        {
            g_perfutility->startTick(name);
            m_enablePerfMeasure = true;
        }
        else // Always bypass function trace for perf measurement case.
        {
            VP_FUNCTION_VERBOSEMESSAGE("Enter function:%s\r\n", name);
        }
    }

    ~Trace()
    {
        if (m_enablePerfMeasure && g_perfutility)
        {
            g_perfutility->stopTick(m_name);
        }
        else
        {
            VP_FUNCTION_VERBOSEMESSAGE("Exit function:%s\r\n", m_name);
        }
    }

protected:
    const char* m_name              = nullptr;
    bool        m_enablePerfMeasure = false;
};
}

#if MOS_MESSAGES_ENABLED
// Function trace for vp hal layer.
#define VP_FUNC_CALL() vp::Trace trace(__FUNCTION__);
#else
#define VP_FUNC_CALL()
#endif

#define __VPHAL_VEBOX_OUTPUTPIPE_MODE                                   "VPOutputPipe Mode"
#define __VPHAL_VEBOX_FEATURE_INUSE                                     "VeBox Feature In use"
#define __VPHAL_VEBOX_DISABLE_SFC                                       "Disable SFC"
#define __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE               "Disable SFC DTR"
#define __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_CENTERING_DISABLE         "SFC Output Centering Disable"
#define __VPHAL_BYPASS_COMPOSITION                                      "Bypass Composition"
#define __MEDIA_USER_FEATURE_VALUE_VEBOX_TGNE_ENABLE_VP                 "Enable Vebox GNE"

#define __VPHAL_RNDR_SSD_CONTROL                                        "SSD Control"
#define __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE         "CSC Patch Mode Disable"
#define __MEDIA_USER_FEATURE_VALUE_DISABLE_DN                           "Disable Dn"
#define __MEDIA_USER_FEATURE_VALUE_DISABLE_PACKET_REUSE                 "Disable PacketReuse"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_PACKET_REUSE_TEAMS_ALWAYS     "Enable PacketReuse Teams mode Always"
#define __MEDIA_USER_FEATURE_VALUE_FORCE_ENABLE_VEBOX_OUTPUT_SURF       "Force Enable Vebox Output Surf"

#define __VPHAL_HDR_LUT_MODE                                            "HDR Lut Mode"
#define __VPHAL_HDR_GPU_GENERTATE_3DLUT                                 "HDR GPU generate 3DLUT"
#define __VPHAL_HDR_DISABLE_AUTO_MODE                                   "Disable HDR Auto Mode"
#define __VPHAL_HDR_SPLIT_FRAME_PORTIONS                                "VPHAL HDR Split Frame Portions"
#define __MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE                  "VP Apogeios Enabled"
#define __VPHAL_PRIMARY_MMC_COMPRESSMODE                                "VP Primary Surface Compress Mode"
#define __VPHAL_RT_MMC_COMPRESSMODE                                     "VP RT Compress Mode"
#define __VPHAL_RT_Cache_Setting                                        "VP RT Cache Setting"

#if (_DEBUG || _RELEASE_INTERNAL)
#define __VPHAL_RT_Old_Cache_Setting                                    "VP RT Old Cache Setting"
#define __VPHAL_ENABLE_COMPUTE_CONTEXT                                  "VP Enable Compute Context"
#define __VPHAL_RNDR_SCOREBOARD_CONTROL                                 "SCOREBOARD Control"
#define __VPHAL_RNDR_CMFC_CONTROL                                       "CMFC Control"
#define __VPHAL_ENABLE_1K_1DLUT                                         "Enable 1K 1DLUT"
#define __VPHAL_FORCE_3DLUT_INTERPOLATION                               "VPHAL Force 3DLUT Interpolation"
#define __VPHAL_VEBOX_HDR_MODE                                          "VeboxHDRMode"
#define __VPHAL_HDR_ENABLE_QUALITY_TUNING                               "VPHAL HDR Enable Quality Tuning"
#define __VPHAL_HDR_ENABLE_KERNEL_DUMP                                  "VPHAL HDR Enable Kernel Dump"
#define __VPHAL_HDR_H2S_RGB_TM                                          "VPHAL H2S TM RGB Based"
#define __VPHAL_HDR_3DLUT_CPU_PATH                                      "HDR 3DLut Table Use CPU Caculate"
#define __VPHAL_FORCE_VP_3DLUT_KERNEL_ONLY                              "Force VP 3DLut Kernel Only"

// Compression
#define __VPHAL_MMC_ENABLE                                              "VP MMC In Use"
#define __VPHAL_RT_MMC_COMPRESSIBLE                                     "VP RT Compressible"
#define __VPHAL_PRIMARY_MMC_COMPRESSIBLE                                "VP Primary Surface Compressible"

#define __VPHAL_RNDR_FORCE_VP_DECOMPRESSED_OUTPUT                       "FORCE VP DECOMPRESSED OUTPUT"
#define __VPHAL_COMP_8TAP_ADAPTIVE_ENABLE                               "8-TAP Enable"
#define __VPHAL_VEBOX_FORCE_VP_MEMCOPY_OUTPUTCOMPRESSED                 "Force VP Memorycopy Outputcompressed"
#define __VPHAL_ENABLE_SFC_NV12_P010_LINEAR_OUTPUT                      "Enable SFC NV12 P010 Linear Output"
#define __VPHAL_ENABLE_SFC_RGBP_RGB24_OUTPUT                            "Enable SFC RGBP RGB24 Output"

#define __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME                         "outxmlLocation"
#define __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME                     "startxmlFrame"
#define __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME                       "endxmlFrame"
#define __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY                               "Vphal Debug Dump Output Directory"
#define __VPHAL_DBG_PARA_DUMP_ENABLE_SKUWA_DUMP                         "enableSkuWaDump"

#define __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_START_COUNTER "InterFrameNinjaStartCounter"
#define __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_END_COUNTER "InterFrameNinjaEndCounter"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_IFNCC                         "EnableIFNCC"
// For OCL 3DLut
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_3DLUT                  "Enable OCL 3DLUT"
#define __MEDIA_USER_FEATURE_VALUE_VP_OCL_3DLUT_ENABLED                 "OCL 3DLUT Enabled"

// For OCL FC
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_FC                      "Enable OCL FC"
#define __MEDIA_USER_FEATURE_VALUE_DISABLE_VP_OCL_FC_FP                  "Disable OCL FC FP"
#define __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_SUPPORTED                   "OCL FC Supported"
#define __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_FEATURE_REPORT              "OCL FC Feature Report"
#define __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_REPORT                      "OCL FC Diff Report"
#define __MEDIA_USER_FEATURE_VALUE_VP_LEGACY_FC_IN_USE                   "Legacy FC In Use"

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VESFC_LINEAR_OUTPUT_BY_TILECONVERT "Enable VESFC Linearoutput By TileConvert"

#define  __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_ID_REPORT              "Enable VEBOX ID REPORT"
#define  __MEDIA_USER_FEATURE_VALUE_USED_VEBOX_ID                       "USED VEBOX ID"
#endif  //(_DEBUG || _RELEASE_INTERNAL)

class VpUtils
{
public:
    //!
    //! \brief    Allocates the Surface
    //! \details  Allocates the Surface
    //!           - if the surface is not already allocated OR
    //!           - resource dimenisions OR format changed
    //! \param    [in] pOsInterface
    //!           Pointer to MOS_INTERFACE
    //! \param    [in,out] pSurface
    //!           Pointer to VPHAL_SURFACE
    //! \param    [in] pSurfaceName
    //!           Pointer to surface name
    //! \param    [in] format
    //!           Expected MOS_FORMAT
    //! \param    [in] DefaultResType
    //!           Expected Resource Type
    //! \param    [in] DefaultTileType
    //!           Expected Surface Tile Type
    //! \param    [in] dwWidth
    //!           Expected Surface Width
    //! \param    [in] dwHeight
    //!           Expected Surface Height
    //! \param    [in] bCompressible
    //!           Surface being compressible or not
    //! \param    [in] CompressionMode
    //!           Compression Mode
    //! \param    [out] pbAllocated
    //!           true if allocated, false for not
    //! \param    [in] resUsageType
    //!           resource usage type for caching
    //! \param    [in] tileModeByForce
    //!           Forced tile mode
    //! \param    [in] memType
    //!           vidoe memory location
    //! \param    [in] isNotLockable
    //!           Flag to indicate whether resource being not lockable
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    static MOS_STATUS ReAllocateSurface(
        PMOS_INTERFACE        osInterface,                               
        PVPHAL_SURFACE        surface,                                   
        PCCHAR                surfaceName,                               
        MOS_FORMAT            format,                                     
        MOS_GFXRES_TYPE       defaultResType,                            
        MOS_TILE_TYPE         defaultTileType,                            
        uint32_t              dwWidth,                                    
        uint32_t              dwHeight,                                   
        bool                  bCompressible,                             
        MOS_RESOURCE_MMC_MODE compressionMode,                            
        bool                  *bAllocated,                                
        MOS_HW_RESOURCE_DEF   resUsageType    = MOS_HW_RESOURCE_DEF_MAX,  
        MOS_TILE_MODE_GMM     tileModeByForce = MOS_TILE_UNSET_GMM,       
        Mos_MemPool           memType         = MOS_MEMPOOL_VIDEOMEMORY,  
        bool                  isNotLockable   = false);                                         

    //!
    //! \brief
    //! \details  Get CSC matrix in a form usable by Vebox, SFC and IECP kernels
    //! \param    [in] SrcCspace
    //!           Source Cspace
    //! \param    [in] DstCspace
    //!           Destination Cspace
    //! \param    [out] pfCscCoeff
    //!           [3x3] Coefficients matrix
    //! \param    [out] pfCscInOffset
    //!           [3x1] Input Offset matrix
    //! \param    [out] pfCscOutOffset
    //!           [3x1] Output Offset matrix
    //! \return   void
    //!
    static void GetCscMatrixForVeSfc8Bit(
        VPHAL_CSPACE srcCspace,
        VPHAL_CSPACE dstCspace,
        float        *fCscCoeff,
        float        *fCscInOffset,
        float        *fCscOutOffset);

    //!
    //! \brief    Performs Color Space Convert for Sample 8 bit
    //! \details  Performs Color Space Convert from Src Color Spase to Dst Color Spase
    //! \param    [out] pOutput
    //!           Pointer to VPHAL_COLOR_SAMPLE_8
    //! \param    [in] pInput
    //!           Pointer to VPHAL_COLOR_SAMPLE_8
    //! \param    [in] srcCspace
    //!           Source Color Space
    //! \param    [in] dstCspace
    //!           Dest Color Space
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    static bool GetCscMatrixForRender8Bit(
        VPHAL_COLOR_SAMPLE_8 *output,
        VPHAL_COLOR_SAMPLE_8 *input,
        VPHAL_CSPACE          srcCspace,
        VPHAL_CSPACE          dstCspace);

    static bool IsSyncFreeNeededForMMCSurface(PVPHAL_SURFACE surface, PMOS_INTERFACE osInterface);

    static bool IsVerticalRotation(VPHAL_ROTATION rotation);

    //!
    //! \brief    Performs Color Space Convert for Sample Pixel
    //! \details  Performs Color Space Convert from Src Color Spase to Dst Color Spase
    //! \param    [out] pOutput
    //!           Pointer to float
    //! \param    [in] pInput
    //!           Pointer to VPHAL_COLOR_SAMPLE_8
    //! \param    [in] srcCspace
    //!           Source Color Space
    //! \param    [in] dstCspace
    //!           Dest Color Space
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!
    static MOS_STATUS GetPixelWithCSCForColorFill(
        VPHAL_COLOR_SAMPLE_8 &input,
        float                 output[4],
        VPHAL_CSPACE          srcCspace,
        VPHAL_CSPACE          dstCspace);

    //!
    //! \brief    Get Color Space Convert Normalized Matrix
    //! \details  Get Color Space Convert Normalized Matrix
    //! \param    [out] pCSC_Matrix
    //!           Pointer to float
    //! \param    [in] src
    //!           Source Color Space
    //! \param    [in] dst
    //!           Target Color Space
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!
    static MOS_STATUS GetNormalizedCSCMatrix(
        MEDIA_CSPACE src,
        MEDIA_CSPACE dst,
        float        cscMatrix[12]);

private:
    //!
    //! \brief    Performs Color Space Convert for Sample 8 bit Using Specified Coeff Matrix
    //! \details  Performs Color Space Convert from Src Color Spase to Dst Color Spase
    //            Using Secified input CSC Coeff Matrix
    //! \param    [out] output
    //!           Pointer to VPHAL_COLOR_SAMPLE_8
    //! \param    [in] input
    //!           Pointer to VPHAL_COLOR_SAMPLE_8
    //! \param    [in] srcCspace
    //!           Source Color Space
    //! \param    [in] dstCspace
    //!           Dest Color Space
    //! \param    [in] iCscMatrix
    //!           input CSC coeff Matrxi
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    static bool GetCscMatrixForRender8BitWithCoeff(
        VPHAL_COLOR_SAMPLE_8 *output,
        VPHAL_COLOR_SAMPLE_8 *input,
        VPHAL_CSPACE          srcCspace,
        VPHAL_CSPACE          dstCspace,
        int32_t              *iCscMatrix);

MEDIA_CLASS_DEFINE_END(VpUtils)
};

#endif // !__VP_UTILS_H__
