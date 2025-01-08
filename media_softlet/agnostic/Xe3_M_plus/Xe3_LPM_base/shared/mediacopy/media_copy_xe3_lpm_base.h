/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     media_copy_xe3_lpm_base.h
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#ifndef __MEDIA_COPY_XE3_LPM_BASE_H__
#define __MEDIA_COPY_XE3_LPM_BASE_H__

#include "media_copy.h"
#include "media_vebox_copy_xe3_lpm_base.h"
#include "media_render_copy_xe3_lpm_base.h"
#include "media_blt_copy_xe3_lpm_base.h"

class MediaCopyStateXe3_Lpm_Base: public MediaCopyBaseState
{
public:
    //!
    //! \brief    MediaCopyStateXe3_Lpm_Base constructor
    //! \details  Initialize the MediaCopy members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaCopyStateXe3_Lpm_Base();
    virtual ~MediaCopyStateXe3_Lpm_Base();

    //!
    //! \brief    init function.
    virtual MOS_STATUS Initialize(PMOS_INTERFACE osInterface, MhwInterfacesNext *mhwInterfaces);

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
    //!    //!
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

    virtual bool IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst);

    MhwInterfacesNext                  *m_mhwInterfaces  = nullptr;
    VeboxCopyStateXe3_Lpm_Base         *m_veboxCopyState = nullptr;
    RenderCopyxe3_Lpm                  *m_renderCopy = nullptr;
    BltStateXe3_Lpm                    *m_bltCopy     = nullptr;

MEDIA_CLASS_DEFINE_END(MediaCopyStateXe3_Lpm_Base)
};

#endif // __MEDIA_COPY_XE3_LPM_BASE_H__
