/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     media_scalability_defs.h
//! \brief    Defines the structures for media scalability

#ifndef __MEDIA_SCALABILITY_DEFS_H__
#define __MEDIA_SCALABILITY_DEFS_H__

#include "mos_util_debug.h"
#include "mos_os.h"

#define SCALABILITY_CHK_NULL_RETURN(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_SCALABILITY, 0, _ptr)

#define SCALABILITY_CHK_STATUS_RETURN(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_SCALABILITY, 0, _stmt)

#define SCALABILITY_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...) \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_SCALABILITY, 0, _stmt, _message, ##__VA_ARGS__)

#define SCALABILITY_ASSERTMESSAGE(_message, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_SCALABILITY, 0, _message, ##__VA_ARGS__)

#define SCALABILITY_NORMALMESSAGE(_message, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_SCALABILITY, 0, _message, ##__VA_ARGS__)

#define SCALABILITY_VERBOSEMESSAGE(_message, ...) \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_SCALABILITY, 0, _message, ##__VA_ARGS__)

#define SCALABILITY_ASSERT(_expr) \
    MOS_ASSERT(MOS_COMPONENT_SCALABILITY, 0, _expr)

#define SCALABILITY_COND_CHECK(_expr, _message, ...) \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_SCALABILITY, 0, _expr, _message, ##__VA_ARGS__)

enum ScalabilityComponentType
{
    scalabilityEncoder = 0,
    scalabilityDecoder = 1,
    scalabilityVp      = 2,
    scalabilityTotal,
};
enum ScalabilitySyncType
{
    syncAllPipes          = 0,
    syncOnePipeWaitOthers = 1,
    syncOnePipeForAnother = 2,
    syncOtherPipesForOne  = 3,
};

struct ScalabilityPars : public ContextRequirement
{
    bool    enableVE       = false;
    bool    forceMultiPipe = false;

    uint32_t frameWidth  = 0;
    uint32_t frameHeight = 0;

    uint8_t numVdbox     = 0;
    uint16_t numTileRows = 0;
    uint16_t numTileColumns = 0;

    uint8_t numVebox = 0;
    bool    enableTileReplay = false;
    uint32_t raMode = 0;
    uint32_t protectMode = 0;
    bool     allowSwArbitarySplit = false;
};

class ScalabilityTrace
{
public:
    ScalabilityTrace(const char *name) : m_name(name)
    {
        SCALABILITY_VERBOSEMESSAGE("Enter function:%s\r\n", name);
    }

    ~ScalabilityTrace()
    {
        SCALABILITY_VERBOSEMESSAGE("Exit function:%s\r\n", m_name);
    }

protected:
    const char *m_name;
};

#define SCALABILITY_FUNCTION_ENTER ScalabilityTrace _trace(__FUNCTION__);

struct ComponentState
{
};

class MediaStatusReport;
struct StateParams
{
    uint8_t            currentPipe = 0;
    uint16_t           currentPass = 0;
    uint8_t            currentRow = 0;
    uint8_t            currentSubPass = 0;
    uint8_t            pipeIndexForSubmit = 0;
    bool               singleTaskPhaseSupported = false;
    MediaStatusReport *statusReport = nullptr;
    ComponentState    *componentState = nullptr;
};

#endif  // !__MEDIA_SCALABILITY_H__
