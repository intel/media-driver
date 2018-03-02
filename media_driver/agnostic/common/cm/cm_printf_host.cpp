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
//! \file      cm_printf_host.cpp 
//! \brief     Contains Class PFParser definitions 
//!

#include "cm_printf_host.h"

#if CM_KERNEL_PRINTF_ON

#include "cm_debug.h"

void PFParser::getToken(void)
{
    mPrevToken = mCurrToken;
    mCurrToken = Token();

    // This is a lexer that has 2 modes
    // Inside a conversion specification and outside
    while (*mCurrLoc != '\0')
    {
        if (!mInSpec)
        {
            // We're just picking off characters until we see a %
            mCurrToken.mTokenType = Token::String;

            while (*mCurrLoc != '\0')
            {
                if (*mCurrLoc == '%')
                {
                    // Peek to check for %%
                    if (*(mCurrLoc+1) != '\0' && *(mCurrLoc+1) != '%')
                    {
                        // This is definitely a directive, not a %%
                        break;
                    }
                    // This IS %% so take another character off the input
                    mCurrToken.mTokenString += *mCurrLoc++;
                }
                mCurrToken.mTokenString += *mCurrLoc++;
            }

            if (*mCurrLoc == '%')
                mInSpec = true;

            if (mCurrToken.mTokenString.length() > 0)
                return;
        }

        if (mInSpec)
        {
            char currChar = *mCurrLoc++;
            switch(currChar)
            {
            default:
                // We've had an unexpected character
                mCurrToken.mTokenType = Token::Error;
                mCurrToken.mTokenString += currChar; // Preserve the error char
                mInSpec = false; // End of the format spec
                return;
            case '%':
                mCurrToken.mTokenType = Token::Percent;
                return;
            case '-':
                mCurrToken.mTokenType = Token::Minus;
                return;
            case '+':
                mCurrToken.mTokenType = Token::Plus;
                return;
            case ' ':
                mCurrToken.mTokenType = Token::Space;
                return;
            case '.':
                mCurrToken.mTokenType = Token::Period;
                return;
            case '#':
                mCurrToken.mTokenType = Token::Hash;
                return;
            case '*':
                mCurrToken.mTokenType = Token::Star;
                return;
            case 'h':
                // Have to deal with 'hh'
                if (*mCurrLoc == 'h')
                {
                    mCurrLoc++;
                    mCurrToken.mTokenType = Token::hh_Mod;
                    return;
                }
                mCurrToken.mTokenType = Token::h_Mod;
                return;
            case 'l':
                // Have to deal with 'll'
                if (*mCurrLoc == 'l')
                {
                    mCurrLoc++;
                    mCurrToken.mTokenType = Token::ll_Mod;
                    return;
                }
                mCurrToken.mTokenType = Token::l_Mod;
                return;
            case 'j':
                mCurrToken.mTokenType = Token::j_Mod;
                return;
            case 'z':
                mCurrToken.mTokenType = Token::z_Mod;
                return;
            case 't':
                mCurrToken.mTokenType = Token::t_Mod;
                return;
            case 'L':
                mCurrToken.mTokenType = Token::L_Mod;
                return;
            case 'c':
                mCurrToken.mTokenType = Token::c_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 's':
                mCurrToken.mTokenType = Token::s_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'd':
                mCurrToken.mTokenType = Token::d_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'i':
                mCurrToken.mTokenType = Token::i_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'o':
                mCurrToken.mTokenType = Token::o_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'x':
                mCurrToken.mTokenType = Token::x_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'X':
                mCurrToken.mTokenType = Token::X_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'u':
                mCurrToken.mTokenType = Token::u_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'f':
                mCurrToken.mTokenType = Token::f_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'F':
                mCurrToken.mTokenType = Token::F_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'e':
                mCurrToken.mTokenType = Token::e_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'E':
                mCurrToken.mTokenType = Token::E_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'a':
                mCurrToken.mTokenType = Token::a_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'A':
                mCurrToken.mTokenType = Token::A_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'g':
                mCurrToken.mTokenType = Token::g_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'G':
                mCurrToken.mTokenType = Token::G_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'n':
                mCurrToken.mTokenType = Token::n_Conv;
                mInSpec = false; // End of the format spec
                return;
            case 'p':
                mCurrToken.mTokenType = Token::p_Conv;
                mInSpec = false; // End of the format spec
                return;
            case '0':
                if (*mCurrLoc < '1' || *mCurrLoc > '9')
                {
                    // Next character not part of a larger integer
                    mCurrToken.mTokenType = Token::Zero;
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
                mCurrToken.mTokenString += currChar;
                while (*mCurrLoc >= '0' && *mCurrLoc <= '9')
                {
                    mCurrToken.mTokenString += *mCurrLoc++;
                }
                // Create integer
                // Since we've created this integer string and we know it is simple we can use
                // atoi knowing that it won't throw an error
                mCurrToken.mTokenInt = atoi(mCurrToken.mTokenString.c_str());
                mCurrToken.mTokenType = Token::Integer;
                return;
            }
        }
    }

    mCurrToken.mTokenType = Token::End;
}

bool PFParser::accept(PFParser::Token::TokenType s)
{
    if (mCurrToken == s)
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
    if (mCurrToken == Token::_None_)
    {
        getToken();
    }
    while(mCurrToken != Token::End && mCurrToken != Token::Error)
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
        mUnsupported = true;
    }
    else if (accept(Token::t_Mod))
    {
        mUnsupported = true;
    }
    else if (accept(Token::z_Mod))
    {
        mUnsupported = true;
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
        mUnsupported = true;
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
            case CM_PRINT_DATA_TYPE_CHAR   :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(char           ));

            case CM_PRINT_DATA_TYPE_UCHAR  :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned char  ));

            case CM_PRINT_DATA_TYPE_INT    :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(int            ));

            case CM_PRINT_DATA_TYPE_UINT   :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned int   ));

            case CM_PRINT_DATA_TYPE_FLOAT  :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(float          ));

            case CM_PRINT_DATA_TYPE_SHORT  :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(short          ));

            case CM_PRINT_DATA_TYPE_USHORT :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned short ));

            case CM_PRINT_DATA_TYPE_QWORD  :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(long long      ));

            case CM_PRINT_DATA_TYPE_UQWORD :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(unsigned long long ));

            case CM_PRINT_DATA_TYPE_DOUBLE :
                return CM_PRINT_SIZE_WITH_PAYLOAD(header->height*header->width*sizeof(double         ));

            default:
                CM_ASSERTMESSAGE("Error: Invalid print data type.");
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
            CM_ASSERTMESSAGE("Error: Invalid print data type.");
        }
        return PRINT_HEADER_SIZE;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Invalid print object type.");
        return PRINT_HEADER_SIZE;
    }
}

void PFParser::flush(void)
{
    if (mInputStart && mCurrLoc)
    {
        if (mCurrToken != Token::End &&
            mCurrToken != Token::_None_)
        {
            // Tidy up any remaining characters
            // Any characters that remain to be flushed need to be check for illegal directives (e.g. %n
            // will cause an exception if attempted to be printed with no argument)
            int numArgs = format();
            if (mUnsupported)
            {
                CM_PRINTF(mStreamOut,"Unsupported (but valid C++11) format string used : %s", mInputStart);
                reset();
            }
            else if (mError)
            {
                CM_PRINTF(mStreamOut,"Error in printf format string : %s", mInputStart);
                reset();
            }
            else if (numArgs > 0)
            {
                // Not enough arguments provided for remaining directives
                CM_PRINTF(mStreamOut,"Not enough (no) arguments supplied for format string : %s", mInputStart);
                reset();
            }
            else
            {
                CM_PRINTF(mStreamOut,"%s", mInputStart);
            }
        }
        reset();
    }
}

PRINT_FMT_STATUS PFParser::GetNextFmtToken(char* tkn, size_t size)
{
    memset(tkn, 0, size);

    if (mNumMultArg)
    {
        if (!mArgsExpected)
        {
            // Copy the whole of the format string into the token
            if ((size_t)(mCurrLoc - mInputStart) <= size)
            {
                memcpy(tkn, mInputStart, mCurrLoc - mInputStart);
                tkn[mCurrLoc - mInputStart] = '\0';
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
        if ((size_t)(mCurrLoc - mInputStart) <= size)
        {
            memcpy(tkn, mInputStart, mCurrLoc - mInputStart);
            tkn[mCurrLoc - mInputStart] = '\0';
            return PF_SUCCESS;
        }
        return PF_FAIL;
    case 2:
    case 3:
        mNumMultArg = numArgs - 1;
        mArgsExpected = numArgs - 1;
        return PF_SUCCESS;
    }
}

bool PFParser::outputToken(const char *tkn, PCM_PRINT_HEADER header)
{
    if (mNumMultArg && mArgsExpected)
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
                case CM_PRINT_DATA_TYPE_INT    :
                    mArgs[mNumMultArg - mArgsExpected] = *((int*            )&(header->scalar64));
                    break;
                case CM_PRINT_DATA_TYPE_UINT   :
                    mArgs[mNumMultArg - mArgsExpected] = *((unsigned int*   )&(header->scalar64));
                    break;
                case CM_PRINT_DATA_TYPE_CHAR   :
                    mArgs[mNumMultArg - mArgsExpected] = *((char*           )&(header->scalar64));
                    break;
                case CM_PRINT_DATA_TYPE_UCHAR  :
                    mArgs[mNumMultArg - mArgsExpected] = *((unsigned char*  )&(header->scalar64));
                    break;
                case CM_PRINT_DATA_TYPE_SHORT  :
                    mArgs[mNumMultArg - mArgsExpected] = *((short*         )&(header->scalar64));
                    break;
                case CM_PRINT_DATA_TYPE_USHORT :
                    mArgs[mNumMultArg - mArgsExpected] = *((unsigned short*)&(header->scalar64));
                    break;
            }
            mArgsExpected -= 1;
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
    if (mUnsupported)
    {
        CM_PRINTF(mStreamOut,"Unsupported (but valid C++11) printf format string : %s", tkn);
        reset();
        return true;
    }
    // Inform the user that they've got an error (illegal format string)
    if (mError)
    {
        CM_PRINTF(mStreamOut, "Error in printf format string : %s", tkn);
        reset();
        return true;
    }

    // Output as appropriate
    if (!mNumMultArg)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT    :
            CM_PRINTF(mStreamOut, tkn, *((int*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UINT   :
            CM_PRINTF(mStreamOut, tkn, *((unsigned int*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR   :
            CM_PRINTF(mStreamOut, tkn, *((char*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR  :
            CM_PRINTF(mStreamOut, tkn, *((unsigned char*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT  :
            CM_PRINTF(mStreamOut, tkn, *((float*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT  :
            CM_PRINTF(mStreamOut, tkn, *((short*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT :
            CM_PRINTF(mStreamOut, tkn, *((unsigned short*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE :
            CM_PRINTF(mStreamOut, tkn, *((double*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD  :
            CM_PRINTF(mStreamOut, tkn, *((long long*)&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD :
            CM_PRINTF(mStreamOut, tkn, *((unsigned long long*)&(header->scalar64)));
            break;
        }
    }
    else if (mNumMultArg == 1)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT    :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((int*            )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UINT   :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((unsigned int*   )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR   :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((char*           )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((unsigned char*  )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((float*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((short*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((unsigned short* )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((double*         )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((long long*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], *((unsigned long long* )&(header->scalar64)));
            break;
        }
    }
    else if (mNumMultArg == 2)
    {
        switch (header->dataType)
        {
        case CM_PRINT_DATA_TYPE_INT    :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((int*            )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UINT   :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((unsigned int*   )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_CHAR   :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((char*           )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UCHAR  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((unsigned char*  )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_FLOAT  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((float*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_SHORT  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((short*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_USHORT :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((unsigned short* )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_DOUBLE :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((double*         )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_QWORD  :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((long long*          )&(header->scalar64)));
            break;

        case CM_PRINT_DATA_TYPE_UQWORD :
            CM_PRINTF(mStreamOut,tkn, mArgs[0], mArgs[1], *((unsigned long long* )&(header->scalar64)));
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

    if(!mNumMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_MATRIX )
    {
        CM_PRINTF(mStreamOut,"\n Thread id %d, Matrix , Width %d, Height %d \n", threadid, header->width, header->height);
    }
    else if(!mNumMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_VECTOR)
    {
        CM_PRINTF(mStreamOut, " \n Thread id %d, Vector , Width %d\n", threadid, header->width);
    }
    else if(!mNumMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_FORMAT)
    {
        // Flush any remaining characters from existing format string (if any)
        flush();
        setStart((char *)memory);
    }
    else if(!mNumMultArg && header->objectType == CM_PRINT_OBJECT_TYPE_STRING)
    {
        char tkn[PRINT_FORMAT_STRING_SIZE];
        PRINT_FMT_STATUS status = GetNextFmtToken(tkn, PRINT_FORMAT_STRING_SIZE);
        if (status == PF_SUCCESS)
        {
            // Inform the user that they've used an unsupported format string if that is the case
            // (e.g. %jd, %td etc.)
            if (mUnsupported)
            {
                CM_PRINTF(mStreamOut, "Unsupported (but valid C++11) format string used : %s", tkn);
            }
            // Inform the user that they've got an error (illegal format string)
            if (mError)
            {
                CM_PRINTF(mStreamOut, "Error in printf format string : %s", tkn);
            }

            if (mUnsupported || mError)
            {
                reset();
                return;
            }

            CM_PRINTF(mStreamOut, tkn, (char*)memory);
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
                CM_PRINTF(mStreamOut, "Problem outputting with format string %s\n", tkn);
                mNumMultArg = mArgsExpected = 0;
            }
        }
        return;
    }
    else
    {
        if (mNumMultArg)
        {
            // Something has gone wrong in multi-arg so reset
            CM_PRINTF(mStreamOut, "Error in multi-arg directive\n");
            mNumMultArg = 0;
            mArgsExpected = 0;
        }
        else
        {
            CM_PRINTF(mStreamOut, "Unknown TYPE\n");
        }
        return;
    }
}

void DumpAllThreadOutput(FILE *streamout, unsigned char * dumpMem, size_t buffersize)
{

    unsigned int offsetFromHeader =   0;
    unsigned int off                =   PRINT_BUFFER_HEADER_SIZE;
    PFParser     pState(streamout);

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
    pState.flush();

}
#endif
