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
//! \file     media_interfaces_mhw.h
//! \brief    Gen-specific factory creation of the mhw interfaces
//!

#ifndef __MEDIA_INTERFACES_MHW_H__
#define __MEDIA_INTERFACES_MHW_H__

#include "media_interfaces.h"
#include "igfxfmid.h"
#include "mos_utilities.h"
#include "mos_os.h"

// forward declarations
class MhwCpInterface;
class MhwMiInterface;
class MhwRenderInterface;
class MhwSfcInterface;
class XMHW_STATE_HEAP_INTERFACE;
class MhwVeboxInterface;
class MhwVeboxInterface;
class MhwVdboxMfxInterface;
class MhwVdboxHcpInterface;
class MhwVdboxHucInterface;
class MhwVdboxVdencInterface;

//!
//! \class    MhwInterfaces
//! \brief    MHW interfaces
//!
class MhwInterfaces
{
public:
    virtual ~MhwInterfaces() {}

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
                uint32_t m_reserved : 23;
            };
            uint32_t m_value;
        } Flags;

        uint8_t m_heapMode = 0; //!< To be deprecated when heap management unified
        bool m_isDecode = false; //!< Whether or not decode is in use, only valid for VDBOX creation
        bool m_isCp     = false; //!< Whether or not CP is in use, CP only need mi and cp interface.
    };

    //! \brief These interfaces are responsible for constructing instructions,
     //!           structures, and registers for hardware.
    MhwCpInterface *m_cpInterface = nullptr;
    MhwMiInterface *m_miInterface = nullptr;
    MhwRenderInterface *m_renderInterface = nullptr;
    MhwSfcInterface *m_sfcInterface = nullptr;
    XMHW_STATE_HEAP_INTERFACE *m_stateHeapInterface = nullptr;
    MhwVeboxInterface *m_veboxInterface = nullptr;
    MhwVdboxMfxInterface *m_mfxInterface = nullptr;
    MhwVdboxHcpInterface *m_hcpInterface = nullptr;
    MhwVdboxHucInterface *m_hucInterface = nullptr;
    MhwVdboxVdencInterface *m_vdencInterface = nullptr;

    //!
    //! \brief    Calls the factory function to initialize all requested interfaces.
    //! \param    [in] params
    //!           Configuration flags for the creation of MHW interfaces.
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MhwInterfaces*
    //!           returns a valid pointer if successful and nullptr if failed.
    //!
    static MhwInterfaces* CreateFactory(
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
    void Destroy();
};

extern template class MediaInterfacesFactory<MhwInterfaces>;

#endif // __MEDIA_INTERFACES_MHW_H__
