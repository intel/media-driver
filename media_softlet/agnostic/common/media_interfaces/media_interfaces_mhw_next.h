/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_interfaces_mhw_next.h
//! \brief    Gen-specific factory creation of the mhw interfaces
//!

#ifndef __MEDIA_INTERFACES_MHW_NEXT_H__
#define __MEDIA_INTERFACES_MHW_NEXT_H__

#include "media_factory.h"
#include "igfxfmid.h"
#include "mos_utilities.h"
#include "mos_os.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_mi_itf.h"
#include "mhw_blt_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_vebox_itf.h"
#include "mhw_sfc_itf.h"
#include "mhw_render_itf.h"
#include "mhw_vdbox_vvcp_itf.h"

// forward declarations
class MhwCpInterface;
class MhwSfcInterface;
class XMHW_STATE_HEAP_INTERFACE;
class MhwVeboxInterface;
class MhwVdboxMfxInterface;
class MhwVdboxHcpInterface;
class MhwVdboxHucInterface;
class MhwVdboxVdencInterface;
class MhwBltInterface;
class MhwVdboxAvpInterface;

//!
//! \class    MhwInterfacesNext
//! \brief    MHW interfacesNext
//!
class MhwInterfacesNext
{
public:
    virtual ~MhwInterfacesNext() {}

    //! \brief Determines which interfaces are created
    struct CreateParams
    {
        CreateParams()
        {
            Flags.m_value = 0;
        }

        union
        {
            struct
            {
                uint32_t m_render : 1;
                uint32_t m_sfc : 1;
                uint32_t m_stateHeap : 1;
                uint32_t m_vebox : 1;
                uint32_t m_vdboxAll : 1;
                uint32_t m_mfx : 1;
                uint32_t m_hcp : 1;
                uint32_t m_huc : 1;
                uint32_t m_vdenc : 1;
                uint32_t m_blt : 1;
                uint32_t m_avp : 1;
                uint32_t m_reserved : 21;
            };
            uint32_t m_value;
        } Flags;

        uint8_t m_heapMode = 0; //!< To be deprecated when heap management unified
        bool m_isDecode = false; //!< Whether or not decode is in use, only valid for VDBOX creation
        bool m_isCp     = false; //!< Whether or not CP is in use, CP only need mi and cp interface.
        bool m_isMos    = false; //!< Create it for mos, for example hws .
    };

    /* Below legacy interfaces are kept temporarily for backward compatibility */

    //! \brief These interfaces are responsible for constructing instructions,
     //!           structures, and registers for hardware.
    MhwCpInterface            *m_cpInterface        = nullptr;
    MhwSfcInterface           *m_sfcInterface       = nullptr;
    XMHW_STATE_HEAP_INTERFACE *m_stateHeapInterface = nullptr;
    MhwVeboxInterface         *m_veboxInterface     = nullptr;
    MhwBltInterface           *m_bltInterface       = nullptr;
    PMOS_INTERFACE            m_osInterface         = nullptr;
    /* New mhw sub interfaces*/
    std::shared_ptr<mhw::vdbox::avp::Itf>   m_avpItf    = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf  = nullptr;
    std::shared_ptr<mhw::vdbox::huc::Itf>   m_hucItf    = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf     = nullptr;
    std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpItf    = nullptr;
    std::shared_ptr<mhw::vdbox::mfx::Itf>   m_mfxItf    = nullptr;
    std::shared_ptr<mhw::vebox::Itf>        m_veboxItf  = nullptr;
    std::shared_ptr<mhw::sfc::Itf>          m_sfcItf    = nullptr;
    std::shared_ptr<mhw::blt::Itf>          m_bltItf    = nullptr;
    std::shared_ptr<mhw::render::Itf>       m_renderItf = nullptr;
    std::shared_ptr<mhw::vdbox::vvcp::Itf>  m_vvcpItf   = nullptr;

    //!
    //! \brief    Calls the factory function to initialize all requested interfaces.
    //! \param    [in] params
    //!           Configuration flags for the creation of MHW interfaces.
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MhwInterfaces*
    //!           returns a valid pointer if successful and nullptr if failed.
    //!
    static MhwInterfacesNext* CreateFactory(
        CreateParams params,
        PMOS_INTERFACE osInterface);

    //!
    //! \brief    Creates requested MHW interfaces.
    //! \param    [in] params
    //!           Configuration flags for the creation of MHW interfaces.
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface) = 0;

    //!
    //! \brief    Destroys all created MHW interfaces
    //! \details  If the HAL creation fails, this is used for cleanup
    //!
    virtual void Destroy();

    //!
    //! \brief    Set Interfaces Destroy State
    //! \details  If the interfaces has destroyed, set this state value on
    //!
    void SetDestroyState(bool destorystate) { m_isDestroyed = destorystate; };
    bool GetDestroyState() { return m_isDestroyed; };

private:
    bool m_isDestroyed = false;
MEDIA_CLASS_DEFINE_END(MhwInterfacesNext)
};

extern template class MediaFactory<uint32_t, MhwInterfacesNext>;

#endif // __MEDIA_INTERFACES_MHW_NEXT_H__
