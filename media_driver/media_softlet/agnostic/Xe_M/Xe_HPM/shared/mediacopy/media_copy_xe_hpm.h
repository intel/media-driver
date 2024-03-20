/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_copy_xe_hpm.h
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#ifndef __MEDIA_COPY_XE_HPM_H__
#define __MEDIA_COPY_XE_HPM_H__

#include "media_copy.h"
#include "mos_defs.h"
#include "mos_os_specific.h"
class BltState_Xe_Hpm;
class MhwInterfaces;
class RenderCopy_Xe_Hpm;
class VeboxCopyState;
class MediaCopyState_Xe_Hpm: public MediaCopyBaseState
{
public:
    //!
    //! \brief    MediaCopyState_Xe_Hpm constructor
    //! \details  Initialize the MediaCopy members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaCopyState_Xe_Hpm();
    virtual ~MediaCopyState_Xe_Hpm();

    //!
    //! \brief    init function.
    virtual MOS_STATUS Initialize(  PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces);
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
    //! \brief    Is AIL force opition.
    //! \details  Is AIL force opition.
    //! \return   bool
    //!           Return true if support, otherwise return false.
    bool IsAILForceOption()
    {
        return true;
    }

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
    virtual MOS_STATUS MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst);

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
    virtual bool IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst);
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
    virtual MOS_STATUS CopyEnigneSelect(MCPY_METHOD &preferMethod, MCPY_ENGINE &mcpyEngine, MCPY_ENGINE_CAPS &caps);

protected:
    MhwInterfaces      *m_mhwInterfacesXeHpm  = nullptr;
    BltState_Xe_Hpm    *m_bltState       = nullptr;
    VeboxCopyState     *m_veboxCopyState = nullptr;
    RenderCopy_Xe_Hpm  *m_renderCopy     = nullptr;

MEDIA_CLASS_DEFINE_END(MediaCopyState_Xe_Hpm)
};
#endif // __MEDIA_COPY_XE_HPM_H__

