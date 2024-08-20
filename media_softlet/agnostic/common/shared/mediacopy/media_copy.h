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
//! \file     media_copy.h
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#ifndef __MEDIA_COPY_H__
#define __MEDIA_COPY_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_defs_specific.h"
#include "mos_os_specific.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "mos_os.h"
#include "mos_interface.h"

class CommonSurfaceDumper;

typedef struct _MCPY_ENGINE_CAPS
{
    uint32_t engineVebox   :1;
    uint32_t engineBlt     :1;
    uint32_t engineRender  :1;
    uint32_t reversed      :29;
}MCPY_ENGINE_CAPS;

enum MCPY_ENGINE
{
    MCPY_ENGINE_VEBOX = 0,
    MCPY_ENGINE_BLT,
    MCPY_ENGINE_RENDER,
};

enum MCPY_CPMODE
{
    MCPY_CPMODE_CP = 0,
    MCPY_CPMODE_CLEAR,
};

enum MCPY_METHOD
{
    MCPY_METHOD_DEFAULT = 0,  
    MCPY_METHOD_POWERSAVING,  // use BCS engine
    MCPY_METHOD_PERFORMANCE,  // use EU to get the best perf.
    MCPY_METHOD_BALANCE,      // use vebox engine.
};

typedef struct _MCPY_STATE_PARAMS
{
    MOS_RESOURCE         *OsRes;              // mos resource
    MOS_RESOURCE_MMC_MODE CompressionMode;    // MC, RC, uncompressed
    MOS_TILE_TYPE         TileMode;           // linear, TILEY, TILE4
    MCPY_CPMODE           CpMode;             // CP content.
    bool                  bAuxSuface;
}MCPY_STATE_PARAMS;

class MediaCopyBaseState
{
public:
    //!
    //! \brief    MediaCopy constructor
    //! \details  Initialize the MediaCopy members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaCopyBaseState();
    virtual ~MediaCopyBaseState();

    //!
    //! \brief    init Media copy
    //! \details  init func.
    //! \param    none
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if success, otherwise return failed.
    //!
    virtual MOS_STATUS Initialize(PMOS_INTERFACE osInterface);

    //!
    //! \brief    check copy capability.
    //! \details  to determine surface copy is supported or not.
    //! \param    format
    //!           [in] surface format
    //! \param    mcpySrc
    //!           [in] Pointer to source paramters
    //! \param    mcpyDst
    //!           [in] Pointer to destination paramters
    //! \param    caps
    //!           [in] reference of featue supported engine's caps
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS CapabilityCheck(
        MOS_FORMAT         format,
        MCPY_STATE_PARAMS &mcpySrc,
        MCPY_STATE_PARAMS &mcpyDst,
        MCPY_ENGINE_CAPS  &caps,
        MCPY_METHOD        preferMethod);

    //!
    //! \brief    surface copy pre process.
    //! \details  pre process before doing surface copy.
    //! \param    src
    //!          [in]Media copy state's input parmaters
    //! \param    dest
    //!          [in]Media copy state's output parmaters
    //! \param    preferMethod
    //!          [in]Media copy Method
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS PreCheckCpCopy(
        MCPY_STATE_PARAMS src, MCPY_STATE_PARAMS dest, MCPY_METHOD preferMethod);

    //!
    //! \brief    surface copy func.
    //! \details  copy surface.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS SurfaceCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst, MCPY_METHOD preferMethod = MCPY_METHOD_PERFORMANCE);

    //!
    //! \brief    aux surface copy.
    //! \details  copy surface.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS AuxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst);

    //!
    //! \brief    Is AIL force opition.
    //! \details  Is AIL force opition.
    //! \return   bool
    //!           Return true if support, otherwise return false.
    virtual bool IsAILForceOption()
    {
        return false;
    }

    virtual PMOS_INTERFACE GetMosInterface();

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual void SetRegkeyReport(bool flag)
    {
        m_bRegReport = flag;
    }
#endif

protected:

    //!
    //! \brief    dispatch copy task if support.
    //! \details  dispatch copy task to HW engine (vebox, EU, Blt) based on customer and default.
    //! \param    mcpySrc
    //!           [in] Pointer to source surface
    //! \param    mcpyDst
    //!           [in] Pointer to destination surface
    //! \param    mcpyEngine
    //!           [in] reference of featue supported engine
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS TaskDispatch(MCPY_STATE_PARAMS mcpySrc, MCPY_STATE_PARAMS mcpyDst, MCPY_ENGINE mcpyEngine);

    //!
    //! \brief    vebox format support.
    //! \details  surface format support.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   bool
    //!           Return true if support, otherwise return false.
    //!
    virtual bool IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {
        return false;
    }

    //!
    //! \brief    render format support.
    //! \details  surface format support.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   bool
    //!           Return true if support, otherwise return false.
    //!
    virtual bool RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {return false;}

    //!
    //! \brief    feature support check on specific check.
    //! \details  media copy feature support.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \param    caps
    //!           [in] reference of featue supported engine
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst,
            MCPY_STATE_PARAMS& mcpy_src, MCPY_STATE_PARAMS& mcpy_dst, MCPY_ENGINE_CAPS& caps)
    {return MOS_STATUS_SUCCESS;}

    //!
    //! \brief    select copy enigne
    //! \details  media copy select copy enigne.
    //! \param    preferMethod
    //!           [in] copy method
    //! \param    mcpyEngine
    //!           [in] copy engine
    //! \param    caps
    //!           [in] reference of featue supported engine
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS CopyEnigneSelect(MCPY_METHOD& preferMethod, MCPY_ENGINE &mcpyEngine, MCPY_ENGINE_CAPS &caps);

    //!
    //! \brief    use blt engie to do surface copy.
    //! \details  implementation media blt copy.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {return MOS_STATUS_SUCCESS;}

    //!
    //! \brief    use Render engie to do surface copy.
    //! \details  implementation media Render copy.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {return MOS_STATUS_SUCCESS;}

    //!
    //! \brief    use vebox engie to do surface copy.
    //! \details  implementation media vebox copy.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {return MOS_STATUS_SUCCESS;}

    MOS_STATUS CheckResourceSizeValidForCopy(const MOS_SURFACE &res, const MCPY_ENGINE method);
    MOS_STATUS ValidateResource(const MOS_SURFACE &src, const MOS_SURFACE &dst, MCPY_ENGINE method);

public:
    PMOS_INTERFACE       m_osInterface    = nullptr;

protected:
    PMOS_MUTEX           m_inUseGPUMutex        = nullptr; // Mutex for in-use GPU context
#if (_DEBUG || _RELEASE_INTERNAL)
    CommonSurfaceDumper *m_surfaceDumper        = nullptr;
    int                  m_MCPYForceMode        = 0;
    bool                 m_enableVeCopySmallRes = false;
    bool                 m_bRegReport           = true;
    char                 m_dumpLocation_in[MAX_PATH]  = {};
    char                 m_dumpLocation_out[MAX_PATH] = {};
#endif
MEDIA_CLASS_DEFINE_END(MediaCopyBaseState)
};
#endif
