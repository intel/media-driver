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
//! \file     media_blt.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_H__
#define __MEDIA_BLT_H__

#include "media_interfaces_mhw.h"
#include "mhw_blt.h"
#include "mhw_mi.h"
#include "mos_os.h"

#define BLT_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_BLT, MOS_BLT_SUBCOMP_SELF, _stmt)
#define BLT_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_BLT, MOS_BLT_SUBCOMP_SELF, _stmt)
#define BLT_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_BLT, MOS_BLT_SUBCOMP_SELF, _ptr)
#define BLT_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_BLT, MOS_BLT_SUBCOMP_SELF, _ptr)
#define BLT_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_BLT, MOS_BLT_SUBCOMP_SELF, _message, ##__VA_ARGS__)

//!
//! \brief  Structure for BLT parameter
//!
typedef struct _BLT_STATE_PARAM
{
    bool             bCopyMainSurface;
    PMOS_SURFACE     pSrcSurface;
    PMOS_SURFACE     pDstSurface;
}BLT_STATE_PARAM, *PBLT_STATE_PARAM;

typedef struct _BLT_STATE_PARAM2
{
    bool             bCopyMainSurface;
    PMOS_RESOURCE    pSrcSurface;
    PMOS_RESOURCE    pDstSurface;
}BLT_STATE_PARAM2, *PBLT_STATE_PARAM2;


class BltState
{
public:
    //!
    //! \brief    BltState constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltState(PMOS_INTERFACE     osInterface);

    virtual ~BltState();

    //!
    //! \brief    BltState initialize
    //! \details  Initialize the BltState, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_SURFACE src,
        PMOS_SURFACE dst);

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

protected:
    virtual MOS_STATUS SetupFastCopyBltParam(
        PMHW_FAST_COPY_BLT_PARAM mhwParams,
        PMOS_SURFACE             inputSurface,
        PMOS_SURFACE             outputSurface);

    //!
    //! \brief    Submit command
    //! \details  Submit BLT command
    //! \param    pBltStateParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD(
        PBLT_STATE_PARAM pBltStateParam);


    virtual MOS_STATUS SetupFastCopyBltParam(
        PMHW_FAST_COPY_BLT_PARAM mhwParams,
        PMOS_RESOURCE            inputSurface,
        PMOS_RESOURCE            outputSurface);

    //!
    //! \brief    Submit command
    //! \details  Submit BLT command
    //! \param    pBltStateParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD(
        PBLT_STATE_PARAM2 pBltStateParam);


public:
    PMOS_INTERFACE m_osInterface;
    MhwInterfaces *mhwInterfaces;
    MhwInterfaces::CreateParams params;
    MhwMiInterface *m_miInterface;
    PMHW_BLT_INTERFACE m_bltInterface;
};

#endif // __VPHAL_BLT_H__
