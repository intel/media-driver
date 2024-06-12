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
//! \file     mhw_hwcmd_process_cmdfields.h
//! \brief    process each field for a command
//! \details
//!

#if defined(DO_FIELDS) || defined(DO_FIELDS_EXT)
#define DO_FIELD(dw, field, value) _MHW_CMD_ASSIGN_FIELD(dw, field, value)
#if defined(DO_FIELDS) && !defined(DISABLE_DO_FIELDS)
    DO_FIELDS();
#endif  // defined(DO_FIELDS) && !defined(DISABLE_DO_FIELDS)
#if defined(DO_FIELDS_EXT) && !defined(DISABLE_DO_FIELDS_EXT)
    DO_FIELDS_EXT();
#endif  // defined(DO_FIELDS_EXT) && !defined(DISABLE_DO_FIELDS_EXT)
#undef DO_FIELD
#if MHW_HWCMDPARSER_ENABLED
    {
        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance && instance->ParseFieldsLayoutEn())
        {
#define DO_FIELD(dw, field, value) MHW_HWCMDPARSER_PARSEFIELDLAYOUT(instance, dw, field)
#if defined(DO_FIELDS) && !defined(DISABLE_DO_FIELDS)
            DO_FIELDS();
#endif  // defined(DO_FIELDS) && !defined(DISABLE_DO_FIELDS)
#if defined(DO_FIELDS_EXT) && !defined(DISABLE_DO_FIELDS_EXT)
            DO_FIELDS_EXT();
#endif  // defined(DO_FIELDS_EXT) && !defined(DISABLE_DO_FIELDS_EXT)
#undef DO_FIELD
        }
    }
#endif  // MHW_HWCMDPARSER_ENABLED
#ifdef DO_FIELDS
#undef DO_FIELDS
#endif  // DO_FIELDS
#ifdef DO_FIELDS_EXT
#undef DO_FIELDS_EXT
#endif  // DO_FIELDS_EXT
#endif  // defined(DO_FIELDS) || defined(DO_FIELDS_EXT)
#if !_MEDIA_RESERVED
    MHW_CHK_STATUS_RETURN(this->ApplyExtSettings(params, reinterpret_cast<uint32_t *>(&cmd)));
#endif  // !_MEDIA_RESERVED
    return MOS_STATUS_SUCCESS;
