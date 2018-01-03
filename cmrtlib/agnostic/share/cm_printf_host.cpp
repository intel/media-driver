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

/**
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** ----------------------------------------------------------------------------
**/
#include "cm_include.h"
#include "cm_printf_host.h"
#include "cm_debug.h"
#include "cm_mem.h"

void PFParser::getToken(void)
{
    m_prevToken = m_currToken;
    m_currToken = Token();

    // This is a lexer that has 2 modes
    // Inside a conversion specification and outside
    while (*m_currLoc != '\0')
    {
        if (!m_inSpec)
        {
            // We're just picking off characters until we see a %
            m_currToken.tokenType = Token::String;

            while (*m_currLoc != '\0')
            {
                if (*m_currLoc == '%')
                {
                    // Peek to check for %%
                    if (*(m_currLoc+1) != '\0' && *(m_currLoc+1) != '%')
                    {
                        // This is definitely a directive, not a %%
                        break;
                    }
                    // This IS %% so take another character off the input
                    m_currToken.tokenString += *m_currLoc++;
                }
                m_currToken.tokenString += *m_currLoc++;
            }

            if (*m_currLoc == '%')
                m_inSpec = true;

            if (m_currToken.tokenString.length() > 0)
                return;
        }

        if (m_inSpec)
        {
            char currChar = *m_currLoc++;
            switch(currChar)
            {
            default:
                // We've had an unexpected character
                m_currToken.tokenType = Token::Error;
                m_currToken.tokenString += currChar; // Preserve the error char
                m_inSpec = false; // End of the format spec
                return;
            case '%':
                m_currToken.tokenType = Token::Percent;
                return;
            case '-':
                m_currToken.tokenType = Token::Minus;
                return;
            case '+':
                m_currToken.tokenType = Token::Plus;
                return;
            case ' ':
                m_currToken.tokenType = Token::Space;
                return;
            case '.':
                m_currToken.tokenType = Token::Period;
                return;
            case '#':
                m_currToken.tokenType = Token::Hash;
                return;
            case '*':
                m_currToken.tokenType = Token::Star;
                return;
            case 'h':
                // Have to deal with 'hh'
                if (*m_currLoc == 'h')
                {
                    m_currLoc++;
                    m_currToken.tokenType = Token::hh_Mod;
                    return;
                }
                m_currToken.tokenType = Token::h_Mod;
                return;
            case 'l':
                // Have to deal with 'll'
                if (*m_currLoc == 'l')
                {
                    m_currLoc++;
                    m_currToken.tokenType = Token::ll_Mod;
                    return;
                }
                m_currToken.tokenType = Token::l_Mod;
                return;
            case 'j':
                m_currToken.tokenType = Token::j_Mod;
                return;
            case 'z':
                m_currToken.tokenType = Token::z_Mod;
                return;
            case 't':
                m_currToken.tokenType = Token::t_Mod;
                return;
            case 'L':
                m_currToken.tokenType = Token::L_Mod;
                return;
            case 'c':
                m_currToken.tokenType = Token::c_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 's':
                m_currToken.tokenType = Token::s_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'd':
                m_currToken.tokenType = Token::d_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'i':
                m_currToken.tokenType = Token::i_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'o':
                m_currToken.tokenType = Token::o_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'x':
                m_currToken.tokenType = Token::x_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'X':
                m_currToken.tokenType = Token::X_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'u':
                m_currToken.tokenType = Token::u_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'f':
                m_currToken.tokenType = Token::f_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'F':
                m_currToken.tokenType = Token::F_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'e':
                m_currToken.tokenType = Token::e_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'E':
                m_currToken.tokenType = Token::E_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'a':
                m_currToken.tokenType = Token::a_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'A':
                m_currToken.tokenType = Token::A_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'g':
                m_currToken.tokenType = Token::g_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'G':
                m_currToken.tokenType = Token::G_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'n':
                m_currToken.tokenType = Token::n_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case 'p':
                m_currToken.tokenType = Token::p_Conv;
                m_inSpec = false; // End of the format spec
                return;
            case '0':
                if (*m_currLoc < '1' || *m_currLoc > '9')
                {
                    // Next character not part of a larger integer
                    m_currToken.tokenType = Token::Zero;
                    return;
                }
                // Deliberately drop through
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                m_currToken.tokenString += currChar;
                while (*m_currLoc >= '0' && *m_currLoc <= '9')
                {
                    m_currToken.tokenString += *m_currLoc++;
                }
                // Create integer
                // Since we've created this integer string and we know it is simple we can use
                // atoi knowing that it won't throw an error
                m_currToken.tokenString = atoi(m_currToken.tokenString.c_str());
                m_currToken.tokenType = Token::Integer;
                return;
            }
        }
    }

    m_currToken.tokenType = Token::End;
}

bool PFParser::accept(PFParser::Token::TokenType s)
{
    if (m_currToken == s)
    {
        getToken();
        return true;
    }
    return false;
}

bool PFParser::expect(Token::TokenType s)
{
    if (accept(s))
        return true;
    error();
    return false;
}

int PFParser::format(void)
{
    if (m_currToken == Token::_None_)
    {
        getToken();
    }
    while(m_currToken != Token::End && m_currToken != Token::Error)
    {
        if (accept(Token::String))
        {
        }
        else if (accept(Token::Percent))
        {
            return directive();
        }
    }
    return 0;
}

int PFParser::directive(void)
{
    int numArgs = 0;
    flags();
    numArgs += width();
    numArgs += precision();
    length_modifier();

    int numConvArgs = conversion();
    if (numConvArgs == 0)
    {
        // Not expecting ANY arguments
        // Ignore any previous directives (width, precision etc.)
        numArgs = 0;
    }
    else
    {
        numArgs += numConvArgs;
    }

    return numArgs;
}

void PFParser::flags(void)
{
    if (accept(Token::Minus))
    {
    }

    if (accept(Token::Plus))
    {
    }

    if (accept(Token::Space))
    {
    }

    if (accept(Token::Zero))
    {
    }

    if (accept(Token::Hash))
    {
    }
}

int PFParser::width(void)
{
    if (accept(Token::Integer))
    {
        return 0;
    }
    else if (accept(Token::Star))
    {
        return 1;
    }
    return 0;
}

int PFParser::precision(void)
{
    if (accept(Token::Period))
    {
        if (accept(Token::Integer))
        {
            return 0;
        }
        else if (expect(Token::Star))
        {
            return 1;
        }
        return 0;
    }
    return 0;
}

void PFParser::length_modifier(void)
{
    if (accept(Token::hh_Mod))
    {
    }
    else if (accept(Token::h_Mod))
    {
    }
    else if (accept(Token::l_Mod))
    {
    }
    else if (accept(Token::ll_Mod))
    {
    }
    else if (accept(Token::j_Mod))
    {
        m_unsupported = true;
    }
    else if (accept(Token::t_Mod))
    {
        m_unsupported = true;
    }
    else if (accept(Token::z_Mod))
    {
        m_unsupported = true;
    }
    else if (accept(Token::L_Mod))
    {
    }
}

int PFParser::conversion(void)
{
    int numArgs = 1;
    if (accept(Token::Percent))
    {
        numArgs = 0;
    }
    else if (accept(Token::c_Conv))
    {
    }
    else if (accept(Token::s_Conv))
    {
    }
    else if (accept(Token::d_Conv))
    {
    }
    else if (accept(Token::i_Conv))
    {
    }
    else if (accept(Token::o_Conv))
    {
    }
    else if (accept(Token::x_Conv))
    {
    }
    else if (accept(Token::X_Conv))
    {
    }
    else if (accept(Token::u_Conv))
    {
    }
    else if (accept(Token::f_Conv))
    {
    }
    else if (accept(Token::F_Conv))
    {
    }
    else if (accept(Token::e_Conv))
    {
    }
    else if (accept(Token::E_Conv))
    {
    }
    else if (accept(Token::a_Conv))
    {
    }
    else if (accept(Token::A_Conv))
    {
    }
    else if (accept(Token::g_Conv))
    {
    }
    else if (accept(Token::G_Conv))
    {
    }
    else if (accept(Token::n_Conv))
    {
        m_unsupported = true;
    }
    else if (expect(Token::p_Conv))
    {
    }
    else
    {
        // Expect must have failed
        numArgs = 0;
    }
    return numArgs;
}

int CalcSizeFromHeader(unsigned char * memory)
{
    PCM_PRINT_HEADER header = (PCM_PRINT_HEADER)memory;

    if((header->objectType == CM_PRINT_OBJECT_TYPE_MATRIX) ||
       (header->objectType == CM_PRINT_OBJECT_TYPE_VECTOR))
    {
        switch (header->dataType)
        {
            case CM_PRINT_DATA_TYPE_CHAR:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(char           ));

            case CM_PRINT_DATA_TYPE_UCHAR:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned char  ));

            case CM_PRINT_DATA_TYPE_INT:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(int            ));

            case CM_PRINT_DATA_TYPE_UINT:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned int   ));

            case CM_PRINT_DATA_TYPE_FLOAT:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(float          ));

            case CM_PRINT_DATA_TYPE_SHORT:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(short          ));

            case CM_PRINT_DATA_TYPE_USHORT:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned short ));

            case CM_PRINT_DATA_TYPE_QWORD:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(long long      ));

            case CM_PRINT_DATA_TYPE_UQWORD:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned long long ));

            case CM_PRINT_DATA_TYPE_DOUBLE:
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(double         ));

            default:
                CmAssert(0);
        }
        return PRINT_HEADER_SIZE;
    }
    else if(header->objectType == CM_PRINT_OBJECT_TYPE_STRING ||
            header->objectType == CM_PRINT_OBJECT_TYPE_FORMAT)
    {
        return PRINT_HEADER_SIZE + PRINT_FORMAT_STRING_SIZE;
    }
    else if(header->objectType == CM_PRINT_OBJECT_TYPE_SCALAR)
    {
        if(!((header->dataType == CM_PRINT_DATA_TYPE_CHAR)   ||
             (header->dataType == CM_PRINT_DATA_TYPE_UCHAR)  ||
             (header->dataType == CM_PRINT_DATA_TYPE_UINT)   ||
             (header->dataType == CM_PRINT_DATA_TYPE_INT)    ||
             (header->dataType == CM_PRINT_DATA_TYPE_USHORT) ||
             (header->dataType == CM_PRINT_DATA_TYPE_SHORT)  ||
             (header->dataType == CM_PRINT_DATA_TYPE_DOUBLE) ||
             (header->dataType == CM_PRINT_DATA_TYPE_QWORD)  ||
             (header->dataType == CM_PRINT_DATA_TYPE_UQWORD) ||
             (header->dataType == CM_PRINT_DATA_TYPE_FLOAT)))
        {
            CmAssert(0);
        }
        return PRINT_HEADER_SIZE;
    }
    else
    {
        CmAssert(0);
        return PRINT_HEADER_SIZE;
    }
}

void PFParser::Flush(void)
{
    if (m_inputStart && m_currLoc)
    {
        if (m_currToken != Token::End &&
            m_currToken != Token::_None_)
        {
            // Tidy up any remaining characters
            // Any characters that remain to be flushed need to be check for illegal directives (e.g. %n
            // will cause an exception if attempted to be printed with no argument)
            int numArgs = format();
            if (m_unsupported)
            {
                CM_PRINTF(m_streamOut,"Unsupported (but valid C++11) format string used : %s", m_inputStart);
                reset();
            }
            else if (m_error)
            {
                CM_PRINTF(m_streamOut,"Error in printf format string : %s", m_inputStart);
                reset();
            }
            else if (numArgs > 0)
            {
                // Not enough arguments provided for remaining directives
                CM_PRINTF(m_streamOut,"Not enough (no) arguments supplied for format string : %s", m_inputStart);
                reset();
            }
            else
            {
                CM_PRINTF(m_streamOut,"%s", m_inputStart);
            }
        }
        reset();
    }
}

PRINT_FMT_STATUS PFParser::GetNextFmtToken(char* tkn, size_t size)
{
    memset(tkn, 0, size);

    if (m_numMultArg)
    {
        if (!m_argsExpected)
        {
            // Copy the whole of the format string into the token
            if ((size_t)(m_currLoc - m_inputStart) <= size)
            {
                CmSafeMemCopy(tkn, m_inputStart, m_currLoc - m_inputStart);
                tkn[m_currLoc - m_inputStart] = '\0';
                return PF_SUCCESS;
            }
            return PF_FAIL;
        }
        // Still processing input arguments
        return PF_SUCCESS;
    }

    int numArgs = format();
    switch (numArgs)
    {
    default:
        return PF_FAIL; // Something has gone wrong
    case 0:
    case 1:
        // Copy the whole of the format string into the token
        if ((size_t)(m_currLoc - m_inputStart) <= size)
        {
            CmSafeMemCopy(tkn, m_inputStart, m_currLoc - m_inputStart);
            tkn[m_currLoc - m_inputStart] = '\0';
            return PF_SUCCESS;
        }
        return PF_FAIL;
    case 2:
    case 3:
        m_numMultArg = numArgs - 1;
        m_argsExpected = numArgs - 1;
        return PF_SUCCESS;
    }
}

bool PFParser::outputToken(const char *tkn, PCM_PRINT_HEADER header)
{
    if (m_numMultArg && m_argsExpected)
    {
        // Processing items for multi-arg directives
        if (header->objectType == CM_PRINT_OBJECT_TYPE_SCALAR &&
            header->dataType != CM_PRINT_DATA_TYPE_FLOAT &&
            header->dataType != CM_PRINT_DATA_TYPE_DOUBLE &&
            header->dataType != CM_PRINT_DATA_TYPE_QWORD &&
            header->dataType != CM_PRINT_DATA_TYPE_UQWORD)
        {
            // Received an int type argument as expected
            switch (header->dataType)
            {
                case CM_PRINT_DATA_TYPE_INT:
                    m_args[m_numMultArg - m_argsExpected] = *((int*            )&(header->scalarLow32));
                    break;
                case CM_PRINT_DATA_TYPE_UINT:
                    m_args[m_numMultArg - m_argsExpected] = *((unsigned int*   )&(header->scalarLow32));
                    break;
                case CM_PRINT_DATA_TYPE_CHAR:
                    m_args[m_numMultArg - m_argsExpected] = *((char*           )&(header->scalarLow32));
                    break;
                case CM_PRINT_DATA_TYPE_UCHAR:
                    m_args[m_numMultArg - m_argsExpected] = *((unsigned char*  )&(header->scalarLow32));
                    break;
                case CM_PRINT_DATA_TYPE_SHORT:
                    m_args[m_numMultArg - m_argsExpected] = *((short*         )&(header->scalarLow32));
                    break;
                case CM_PRINT_DATA_TYPE_USHORT:
                    m_args[m_numMultArg - m_argsExpected] = *((unsigned short*)&(header->scalarLow32));
                    break;
            }
            m_argsExpected -= 1;
            return true;
        }
        else
        {
            // Not received the expected argument
            // Dump what is in the format string and attempt to recover as well as possible
            return false;
        }
    }

    // Inform the user that they've used an unsupported format string if that is the case
    // (e.g. %jd, %td etc.)
    if (m_unsupported)
    {
        CM_PRINTF(m_streamOut,"Unsupported (but valid C++11) printf format string : %s", tkn);
        reset();
        return true;
    }
    // Inform the user that they've got an error (illegal format string)
    if (m_error)
    {
        CM_PRINTF(m_streamOut, "Error in printf format string : %s", tkn);
        reset();
        return true;
    }

    // Output as appropriate
    if (!m_numMultArg)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT:
            CM_PRINTF(m_streamOut, tkn, *((int*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UINT:
            CM_PRINTF(m_streamOut, tkn, *((unsigned int*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR:
            CM_PRINTF(m_streamOut, tkn, *((char*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR:
            CM_PRINTF(m_streamOut, tkn, *((unsigned char*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT:
            CM_PRINTF(m_streamOut, tkn, *((float*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT:
            CM_PRINTF(m_streamOut, tkn, *((short*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT:
            CM_PRINTF(m_streamOut, tkn, *((unsigned short*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE:
            CM_PRINTF(m_streamOut, tkn, *((double*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD:
            CM_PRINTF(m_streamOut, tkn, *((long long*)&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD:
            CM_PRINTF(m_streamOut, tkn, *((unsigned long long*)&(header->scalarLow32)));
            break;
        }
    }
    else if (m_numMultArg == 1)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((int*            )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UINT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((unsigned int*   )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((char*           )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((unsigned char*  )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((float*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((short*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((unsigned short* )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((double*         )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((long long*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD:
            CM_PRINTF(m_streamOut,tkn, m_args[0], *((unsigned long long* )&(header->scalarLow32)));
            break;
        }
    }
    else if (m_numMultArg == 2)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((int*            )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UINT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((unsigned int*   )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((char*           )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((unsigned char*  )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((float*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((short*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((unsigned short* )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((double*         )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((long long*          )&(header->scalarLow32)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD:
            CM_PRINTF(m_streamOut,tkn, m_args[0], m_args[1], *((unsigned long long* )&(header->scalarLow32)));
            break;
        }
    }

    reset();
    return true;
}
void PFParser::DumpMemory(unsigned char * memory)
{
    PCM_PRINT_HEADER header = (PCM_PRINT_HEADER)memory;
    memory += PRINT_HEADER_SIZE;
    int threadid = header->tid;

    if(!m_numMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_MATRIX )
    {
        CM_PRINTF(m_streamOut,"\n Thread id %d, Matrix , Width %d, Height %d \n", threadid, header->width, header->height);
    }
    else if(!m_numMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_VECTOR)
    {
        CM_PRINTF(m_streamOut, " \n Thread id %d, Vector , Width %d\n", threadid, header->width);
    }
    else if(!m_numMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_FORMAT)
    {
        // Flush any remaining characters from existing format string (if any)
        Flush();
        SetStart((char *)memory);
    }
    else if(!m_numMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_STRING)
    {
        char tkn[PRINT_FORMAT_STRING_SIZE];
        PRINT_FMT_STATUS status = GetNextFmtToken(tkn, PRINT_FORMAT_STRING_SIZE);
        if (status == PF_SUCCESS)
        {
            // Inform the user that they've used an unsupported format string if that is the case
            // (e.g. %jd, %td etc.)
            if (m_unsupported)
            {
                CM_PRINTF(m_streamOut, "Unsupported (but valid C++11) format string used : %s", tkn);
            }
            // Inform the user that they've got an error (illegal format string)
            if (m_error)
            {
                CM_PRINTF(m_streamOut, "Error in printf format string : %s", tkn);
            }

            if (m_unsupported || m_error)
            {
                reset();
                return;
            }

            CM_PRINTF(m_streamOut, tkn, (char*)memory);
            reset();
        }

        return;
    }
    else if(header->objectType == CM_PRINT_OBJECT_TYPE_SCALAR)
    {
        char tkn[PRINT_FORMAT_STRING_SIZE];
        PRINT_FMT_STATUS status = GetNextFmtToken(tkn, PRINT_FORMAT_STRING_SIZE);
        if (status == PF_SUCCESS)
        {
            if (!outputToken(tkn, header))
            {
                // Something has gone wrong
                // Reset multi-arg at least
                CM_PRINTF(m_streamOut, "Problem outputting with format string %s\n", tkn);
                m_numMultArg = m_argsExpected = 0;
            }
        }
        return;
    }
    else
    {
        if (m_numMultArg)
        {
            // Something has gone wrong in multi-arg so reset
            CM_PRINTF(m_streamOut, "Error in multi-arg directive\n");
            m_numMultArg = 0;
            m_argsExpected = 0;
        }
        else
        {
            CM_PRINTF(m_streamOut, "Unknown TYPE\n");
        }
        return;
    }
}

void DumpAllThreadOutput(FILE *streamOut, unsigned char * dumpMem, size_t buffersize)
{

    unsigned int offsetFromHeader =   0;
    unsigned int off              =   PRINT_BUFFER_HEADER_SIZE;
    PFParser     pState(streamOut);

    while(1)
    {
        if((off + PRINT_HEADER_SIZE) >= buffersize)
            break;

        if(off >= (*(unsigned int *)dumpMem))
            break;

        offsetFromHeader = CalcSizeFromHeader(dumpMem + off);
        if( (off + offsetFromHeader) >= buffersize )
            break;

        pState.DumpMemory(dumpMem + off);

        off += offsetFromHeader;
    }

    // Flush any remaining characters in the format buffer
    pState.Flush();

}
