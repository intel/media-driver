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
//! \file     media_ddi_prot.h
//! \brief    Defines base class for DDI media protected.
//!

#ifndef _MEDIA_DDI_PROT_H_
#define _MEDIA_DDI_PROT_H_

#include <va/va.h>
#include "cp_factory.h"

#ifndef VAProtectedSessionID
#define VAProtectedSessionID VAContextID
#endif

#define DDI_MEDIA_VACONTEXTID_OFFSET_PROT_LP        0
#define DDI_MEDIA_VACONTEXTID_OFFSET_PROT_CP        0x8000000

#define DDI_MEDIA_MASK_VAPROTECTEDSESSION_ID        0x7FFFFFF
#define DDI_MEDIA_MASK_VAPROTECTEDSESSION_TYPE      0x8000000

#define DDI_MEDIA_CONTEXT_TYPE_PROTECTED_LINK       1
#define DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT    2

#define DDI_PROTECTED_DEFAULT       1
#define DDI_PROTECTED_DISPLAY_LINK  2
#define DDI_PROTECTED_CONTENT       3

//!
//! \class  DdiMediaProtected
//! \brief  Ddi media protected
//!
class DdiMediaProtected
{
public:
    //!
    //! \brief Constructor
    //!
    DdiMediaProtected() {}

    //!
    //! \brief Destructor
    //!
    virtual ~DdiMediaProtected() {}

    //!
    //! \brief   Check VA entrypoint is supported or not.
    //!
    //! \param   [in] entrypoint
    //!          VAEntrypoint
    //!
    virtual bool CheckEntrypointSupported(
        VAEntrypoint entrypoint);

    //!
    //! \brief    Check the attribute list if is valid according to profile and entrypoint
    //!
    //! \param    [in] profile
    //!           VAProfile
    //! \param    [in] entrypoint
    //!           VAEntrypoint
    //! \param    [in] attrib
    //!           Pointer to a pointer of VAConfigAttrib
    //! \param    [in] numAttribs
    //!           number of of VAConfigAttrib
    //!
    //! \return   bool
    //!           true if valid
    //!
    virtual bool CheckAttribList(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttrib* attrib,
        uint32_t numAttribs);

    //!
    //! \brief   Create protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] configId
    //!          VA configuration ID
    //! \param   [out] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DdiMedia_CreateProtectedSession(
        VADriverContextP        ctx,
        VAConfigID              config_id,
        VAProtectedSessionID    *protected_session);

    //!
    //! \brief   Destroy protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DdiMedia_DestroyProtectedSession(
        VADriverContextP        ctx,
        VAProtectedSessionID    protected_session);

    //!
    //! \brief   Attach protected session to display or context
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] context
    //!          VA context ID to be attached if not 0.
    //! \param   [in] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DdiMedia_AttachProtectedSession(
        VADriverContextP        ctx,
        VAContextID             context,
        VAProtectedSessionID    protected_session);

    //!
    //! \brief   Detach protected session from display or context
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] context
    //!          VA context ID to be Detached if not 0.
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DdiMedia_DetachProtectedSession(
        VADriverContextP        ctx,
        VAContextID             context);

    //!
    //! \brief   TEE execution for the particular protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] protected_session
    //!          VA protected session ID
    //! \param   [in] data
    //!          VA buffer ID
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DdiMedia_ProtectedSessionExecute(
        VADriverContextP        ctx,
        VAProtectedSessionID    protected_session,
        VABufferID              data);

    //! \brief   Create protected session buffer
    //!
    //! \param   [in] ctx
    //!          Pointer to VA Driver Context
    //! \param   [in] context
    //!          VA protected session ID
    //! \param   [in] type
    //!          Va Buffer type
    //! \param   [in] size
    //!          Size of each element in buffer
    //! \param   [in] num_elements
    //!          Number of elements
    //! \param   [in] data
    //!          Buffer data
    //! \param   [out] bufId
    //!          Pointer to VABufferID
    //!
    //! \return  VAStatus
    //!          VA_STATUS_SUCCESS if successful, else fail reason
    //!
    static VAStatus DdiMedia_ProtectedSessionCreateBuffer(
        VADriverContextP        ctx,
        VAContextID             context,
        VABufferType            type,
        uint32_t                size,
        uint32_t                num_elements,
        void                    *data,
        VABufferID              *bufId);


    //! \brief   Get singleton object
    //!
    //! \param   [in] id
    //!          Type of singleton object
    //!
    //! \return  DdiMediaProtected*
    //!          DdiMediaProtected object
    //!
    static DdiMediaProtected* GetInstance(uint32_t id = DDI_PROTECTED_DEFAULT);

    //! \brief   Free singleton objects created in GetInstance()
    static void FreeInstances();

    //! \brief    Get protected surface tag
    //!
    //! \param   [in] ctx
    //!          Media context
    //!
    //! \return  uint64_t
    //!          Protected surface tag based on cp session created.
    //!
    virtual uint64_t GetProtectedSurfaceTag(PDDI_MEDIA_CONTEXT media_ctx);

protected:
    virtual VAStatus CreateProtectedSession(
        VADriverContextP        ctx,
        VAConfigID              config_id,
        VAProtectedSessionID    *protected_session);

    virtual VAStatus DestroyProtectedSession(
        VADriverContextP        ctx,
        VAProtectedSessionID    protected_session);

    virtual VAStatus AttachProtectedSession(
        VADriverContextP        ctx,
        VAContextID             context,
        VAProtectedSessionID    protected_session);

    virtual VAStatus DetachProtectedSession(
        VADriverContextP        ctx,
        VAContextID             context);

    virtual VAStatus ProtectedSessionExecute(
        VADriverContextP        ctx,
        VAProtectedSessionID    protected_session,
        VABufferID              data);

    virtual VAStatus ProtectedSessionCreateBuffer(
        VADriverContextP        ctx,
        VAProtectedSessionID    protected_session,
        VABufferType            type,
        uint32_t                size,
        uint32_t                num_elements,
        void                    *data,
        VABufferID              *bufId);

private:
    static std::map<uint32_t, DdiMediaProtected*> _impl;

    MEDIA_CLASS_DEFINE_END(DdiMediaProtected)
};

typedef CpFactoryWithoutArgs<DdiMediaProtected> DdiProtectedFactory;

void DdiMedia_FreeProtectedSessionHeap(
    VADriverContextP ctx,
    PDDI_MEDIA_HEAP contextHeap,
    int32_t vaContextOffset,
    int32_t ctxNums);

void* DdiMedia_GetContextFromProtectedSessionID(
    VADriverContextP ctx,
    VAProtectedSessionID vaID,
    uint32_t *ctxType);

#endif /*  _MEDIA_DDI_PROT_H_ */
