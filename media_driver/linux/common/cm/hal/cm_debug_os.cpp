/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_debug.h
//! \brief     Contains CM debug definitions
//!

#include  <sstream>
#include "cm_debug.h"


static int32_t commandBufferCounter = 0;
static int32_t surfaceStateCounter = 0;
static int32_t IDDCounter = 0;

uint32_t GetLogFileLocation(const char *filename, char fileNamePrefix[])
{
    //Get driver persistent location
    MOS_SecureStrcpy(fileNamePrefix, 260, filename);
    return 0;
}

int32_t GetCommandBufferDumpCounter(uint32_t valueID)
{
    return commandBufferCounter;
}

int32_t RecordCommandBufferDumpCounter(int32_t count, uint32_t ValueID)
{
    commandBufferCounter = count;
    return 0;
}

int32_t GetSurfaceStateDumpCounter(uint32_t valueID)
{
    return surfaceStateCounter;
}

int32_t RecordSurfaceStateDumpCounter(int32_t count, uint32_t ValueID)
{
    surfaceStateCounter = count;
    return 0;
}

int32_t GetInterfaceDescriptorDataDumpCounter(uint32_t valueID)
{
    return IDDCounter;
}

int32_t RecordInterfaceDescriptorDataDumpCounter(int32_t count, uint32_t ValueID)
{
    IDDCounter = count;
    return 0;
}
uint32_t GetCommandBufferHeaderDWords(PMOS_INTERFACE osInterface)
{
    return 0;
}