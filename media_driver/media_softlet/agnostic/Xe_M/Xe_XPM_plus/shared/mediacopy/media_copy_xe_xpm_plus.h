/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_copy_xe_xpm_plus.h
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#ifndef __MEDIA_COPY_XE_XPM_PLUS_H__
#define __MEDIA_COPY_XE_XPM_PLUS_H__

#include "mos_defs.h"
#include "mos_os_specific.h"
#include "media_copy.h"
class BltStateXe_Xpm_Plus;
class MhwInterfaces;
class RenderCopy_Xe_Xpm_Plus;

class MediaCopyStateXe_Xpm_Plus: public MediaCopyBaseState
{
public:
    //!
    //! \brief    MediaCopyStateXe_Xpm_Plus constructor
    //! \details  Initialize the MediaCopy members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaCopyStateXe_Xpm_Plus();
    virtual ~MediaCopyStateXe_Xpm_Plus();

    //!
    //! \brief    init function.
    virtual MOS_STATUS Initialize(PMOS_INTERFACE osInterface, MhwInterfaces *mhwInterfaces);
    using MediaCopyBaseState::Initialize;

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
    virtual bool RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst);

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
    virtual MOS_STATUS FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst, MCPY_ENGINE_CAPS& caps);

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
    MOS_STATUS CopyEnigneSelect(MCPY_METHOD& preferMethod, MCPY_ENGINE& mcpyEngine, MCPY_ENGINE_CAPS& caps);

protected:

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
    virtual MOS_STATUS MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst);

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
    virtual MOS_STATUS MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst);

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
    virtual MOS_STATUS MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst){return MOS_STATUS_INVALID_PARAMETER;}

    MhwInterfaces                 *m_mhwInterfacesXeXpmPlus = nullptr;
    BltStateXe_Xpm_Plus           *m_bltState       = nullptr;
    RenderCopy_Xe_Xpm_Plus        *m_renderCopy     = nullptr;
MEDIA_CLASS_DEFINE_END(MediaCopyStateXe_Xpm_Plus)
};

#endif // __MEDIA_COPY_XE_XPM_PLUS_H__
