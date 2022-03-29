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

#include "mos_os.h"
#include "media_interfaces_mhw.h"
#include "mhw_cp_interface.h"
#include "mhw_blt.h"
#include "mhw_vebox.h"
#include "mhw_render.h"
#include "media_vebox_copy.h"

#define MCPY_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _stmt)
#define MCPY_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _stmt)
#define MCPY_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _ptr)
#define MCPY_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _ptr)
#define MCPY_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _message, ##__VA_ARGS__)
#define MCPY_NORMALMESSAGE(_message, ...)    MOS_NORMALMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _message, ##__VA_ARGS__)

class VphalSurfaceDumper;
typedef struct VPHAL_SURFACE* PVPHAL_SURFACE;

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
    virtual MOS_STATUS Initialize(PMOS_INTERFACE osInterface, MhwInterfaces *mhwInterfaces);

    //!
    //! \brief    check copy capability.
    //! \details  to determine surface copy is supported or not.
    //! \param    none
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS CapabilityCheck();

    //!
    //! \brief    surface copy pre process.
    //! \details  pre process before doing surface copy.
    //! \param    none
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS PreProcess(MCPY_METHOD preferMethod = MCPY_METHOD_BALANCE);

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
    //! \brief    query which copy caps engine.
    //! \details  to indicate which Engine support this surface copy, for internal debug purpose.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \param    HW_engine
    //!           [out] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    MCPY_ENGINE_CAPS* QueryCapsEngine(PMOS_RESOURCE src, PMOS_RESOURCE dst)
    {
        CapabilityCheck();
        return &m_mcpyEngineCaps;
    }

protected:

    //!
    //! \brief    dispatch copy task if support.
    //! \details  dispatch copy task to HW engine (vebox, EU, Blt) based on customer and default.
    //! \param    none
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    virtual MOS_STATUS TaskDispatch();

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
    //! \brief    dispatch copy task if support.
    //! \details  dispatch copy task to HW engine (vebox, EU, Blt) based on customer and default.
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
    //!
    MOS_STATUS CopyEnigneSelect(MCPY_METHOD preferMethod);

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
    
   #if (_DEBUG || _RELEASE_INTERNAL)
    //debug only
    MOS_STATUS CloneResourceInfo(PVPHAL_SURFACE pVphalSurface, PMOS_SURFACE pMosSurface);
   #endif

public:
    PMOS_INTERFACE      m_osInterface    = nullptr;
    MhwInterfaces      *m_mhwInterfaces  = nullptr;
    MCPY_ENGINE_CAPS    m_mcpyEngineCaps = {1,1,1,1};
    MCPY_ENGINE         m_mcpyEngine     = MCPY_ENGINE_RENDER;
    MCPY_STATE_PARAMS   m_mcpySrc        = {nullptr, MOS_MMC_DISABLED,MOS_TILE_LINEAR, MCPY_CPMODE_CLEAR, false}; // source surface.
    MCPY_STATE_PARAMS   m_mcpyDst        = {nullptr, MOS_MMC_DISABLED,MOS_TILE_LINEAR, MCPY_CPMODE_CLEAR, false}; // destination surface.
    bool                m_allowCPBltCopy  = false;  // allow cp call media copy only for output clear cases.
    VphalSurfaceDumper  *m_surfaceDumper  = nullptr;

protected:
    PMOS_MUTEX           m_inUseGPUMutex = nullptr; // Mutex for in-use GPU context
};
#endif
