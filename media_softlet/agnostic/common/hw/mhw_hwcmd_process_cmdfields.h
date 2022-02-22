/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_hwcmd_process_cmdfields.h
//! \brief    process each field for a command
//! \details
//!

#ifdef DO_FIELDS
    #define DO_FIELD(dw, field, value) _MHW_CMD_ASSIGN_FIELD(dw, field, value)
        DO_FIELDS();
    #undef DO_FIELD
    #if MHW_HWCMDPARSER_ENABLED
        if (m_parseFieldsLayout)
        {
    #define DO_FIELD(dw, field, value) MHW_HWCMDPARSER_PARSEFIELDLAYOUT(dw, field)
            DO_FIELDS();
    #undef DO_FIELD
        }
    #endif  // MHW_HWCMDPARSER_ENABLED
    #undef DO_FIELDS
#endif  // DO_FIELDS
#ifndef NO_RETURN
        return MOS_STATUS_SUCCESS;
#endif  // NO_RETURN
