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
//! \file      cm_printf_host.h 
//! \brief     Contains Class PFParser definitions 
//!

#pragma once

#define CM_KERNEL_PRINTF_ON             1

#if CM_KERNEL_PRINTF_ON

#include <string>

#define PRINT_HEADER_SIZE 32

#define CM_PRINT_OBJECT_TYPE_ENTRY_INDEX 0
#define CM_PRINT_DATA___TYPE_ENTRY_INDEX 1
#define CM_PRINT_LOWER32BITS_ENTRY_INDEX 6
#define CM_PRINT_UPPER32BITS_ENTRY_INDEX 7

#define CM_PRINT_DATA_TYPE_CHAR   0
#define CM_PRINT_DATA_TYPE_UCHAR  1
#define CM_PRINT_DATA_TYPE_FLOAT  2
#define CM_PRINT_DATA_TYPE_INT    3
#define CM_PRINT_DATA_TYPE_UINT   4
#define CM_PRINT_DATA_TYPE_SHORT  5
#define CM_PRINT_DATA_TYPE_USHORT 6
#define CM_PRINT_DATA_TYPE_QWORD  7
#define CM_PRINT_DATA_TYPE_UQWORD 8
#define CM_PRINT_DATA_TYPE_DOUBLE 9

#define CM_PRINT_OBJECT_TYPE_UNKNOWN 0
#define CM_PRINT_OBJECT_TYPE_MATRIX  1
#define CM_PRINT_OBJECT_TYPE_VECTOR  2
#define CM_PRINT_OBJECT_TYPE_SCALAR  3
#define CM_PRINT_OBJECT_TYPE_STRING  4
#define CM_PRINT_OBJECT_TYPE_FORMAT  5

/// If you want to change the static
/// buffer change these two macros.
#define CM_PRINTF_STATIC_BUFFER_ID 1
#define CM_PRINT_BUFFER CM_STATIC_BUFFER_1

#define CM_PRINTF(f_, ...) fprintf((f_), __VA_ARGS__)

#define PRINT_BUFFER_HEADER_SIZE    32
#define PRINT_PAYLOAD_ALIGN         16
#define PRINT_HEADER_SIZE           32
#define PRINT_FORMAT_STRING_SIZE    128
#define CM_PRINT_SIZE_WITH_PAYLOAD(msize) (PRINT_HEADER_SIZE + (msize-1) / PRINT_PAYLOAD_ALIGN * PRINT_PAYLOAD_ALIGN + PRINT_PAYLOAD_ALIGN)

/// Structure of header:
/// vector<int, 8> header
/// [0]: Object type: matrix,vector,scalar,string, or format string.
/// [1]: Data type: [u]*int[136]*[6248]_t, float, or double.
/// [2]: width
/// [3]: height
/// [4]: tid
/// [5]: reserved3
/// [6]: Scalar lower 32bits: [u]*int[13]*[628]_t, float. Lower 32bits of double and [u]*int64_t.
/// [7]: Scalar upper 32bits: Upper 32bits of double and [u]*int64_t.

typedef struct _CM_PRINT_HEADER{
    unsigned int  objectType;
    unsigned int  dataType;
    unsigned int  width;
    unsigned int  height;
    unsigned int  tid;
    unsigned int  reserved3;
    unsigned long long scalar64;
}CM_PRINT_HEADER, *PCM_PRINT_HEADER;

enum  PRINT_FMT_STATUS
{
    PF_FAIL    = 0,
    PF_SUCCESS = 1
};

// A small class used to contain the state machine used when parsing the printf string
// The issue we have to deal with is that there is a format string with some directives in it - the
// format string is received as an object from the client as an object in memory
// Any arguments are then passed as separate objects in memory. How many objects are required is
// determined by the format string.
// The cleanest way to deal with this is to use a very simple recursive descent parser on the format
// string in order to process the arguments in the correct way
// Here's the grammar for printf format strings (using EBNF). Only one format directive is to be
// returned from the input at a time:
//
// format:
//       { STRING } directive
//
//
// directive:
//       PERCENT flags { width } { PERIOD precision } { length_modifier } conversion
//
// flags:
//       { MINUS } { PLUS } { SPACE } { ZERO } { HASH }
//
// width:
//         INTEGER
//       | STAR
//
// precision:
//         INTEGER
//       | STAR
//
// length_modifier:
//       hh_MOD | h_MOD | l_MOD | ll_MOD | j_MOD | t_MOD | z_MOD | L_MOD
//
// conversion:
//         PERCENT
//       | c_CONV
//       | s_CONV
//       | d_CONV
//       | i_CONV
//       | o_CONV
//       | x_CONV
//       | X_CONV
//       | u_CONV
//       | f_CONV
//       | F_CONV
//       | e_CONV
//       | E_CONV
//       | a_CONV
//       | A_CONV
//       | g_CONV
//       | G_CONV
//       | n_CONV
//       | p_CONV
//
// STRING    : [any ASCII character that isn't a %]+
// PERCENT   : '%'
// MINUS     : '-'
// PLUS      : '+'
// SPACE     : ' '
// ZERO      : '0'
// INTEGER   : [
// PERIOD    : '.'
// HASH      : '#'
// hh_MOD    : 'hh'
// h_MOD     : 'h'
// l_MOD     : 'l'
// ll_MOD    : 'll'
// j_MOD     : 'j'
// z_MOD     : 'z'
// t_MOD     : 't'
// L_MOD     : 'L'
// STAR      : '*'
// c_CONV    : 'c'
// s_CONV    : 's'
// d_CONV    : 'd'
// i_CONV    : 'i'
// o_CONV    : 'o'
// x_CONV    : 'x'
// X_CONV    : 'X'
// u_CONV    : 'u'
// f_CONV    : 'f'
// F_CONV    : 'F'
// e_CONV    : 'e'
// E_CONV    : 'E'
// a_CONV    : 'a'
// A_CONV    : 'A'
// g_CONV    : 'g'
// G_CONV    : 'G'
// n_CONV    : 'n'
// p_CONV    : 'p'

class PFParser
{
public:
    PFParser(FILE* streamout) : mInSpec(false), mInputStart(nullptr), mCurrLoc(nullptr), mArgsExpected(0),
                 mNumMultArg(0), mUnsupported(false), mError(false), mStreamOut(streamout) {};
    void setStart(char *start)
    {
        mInputStart= mCurrLoc = start;
        // Prime the system with the first token
        getToken();
    }
    void DumpMemory(unsigned char * memory);
    void flush(void);

protected:
private:
    class Token
    {
    public:
        enum TokenType { _None_, Error,
                         String, Percent, Minus, Plus, Space, Zero, Integer, Period, Hash, Star,
                         hh_Mod, h_Mod, l_Mod, ll_Mod, j_Mod, z_Mod, t_Mod, L_Mod,
                         c_Conv, s_Conv, d_Conv, i_Conv, o_Conv, x_Conv, X_Conv, u_Conv, f_Conv,
                         F_Conv, e_Conv, E_Conv, a_Conv, A_Conv, g_Conv, G_Conv, n_Conv, p_Conv,
                         End
        };

        Token() : mTokenType(_None_), mTokenInt(0) {};
        bool operator==(const Token &other) const {
            return mTokenType == other.mTokenType;
        }
        bool operator==(const TokenType &other) const {
            return mTokenType == other;
        }
        bool operator!=(const Token &other) const {
            return mTokenType != other.mTokenType;
        }
        bool operator!=(const TokenType &other) const {
            return mTokenType != other;
        }

        TokenType mTokenType;
        std::string mTokenString;
        int mTokenInt;
    };

    bool     mInSpec;      // Mode for lexer - in spec mode or not
    Token    mCurrToken;   // The currently lexed token
    Token    mPrevToken;   // The previously lexed token
    char     *mInputStart; // The start of the input string
    char     *mCurrLoc;    // The current point of processing
    int      mArgsExpected; // For multi-arg format directives - how many still to process
    int      mNumMultArg;   // Total number of multi-arg format directives in total
    int      mArgs[2];      // Up to 2 int args can be used in multi-arg format directives
    bool     mUnsupported;  // Has the latest parsed format directive contained unsupported
                            // directives (VS doesn't support them all so we can't print them)
    bool     mError;        // Error in latest parsed format directive
    FILE     *mStreamOut;   // Output stream for kernel print

    PRINT_FMT_STATUS GetNextFmtToken(char * tkn, size_t size);
    bool outputToken(const char *tkn, PCM_PRINT_HEADER header);
    void getToken(void);
    void reset(void)
    {
        mInputStart = mCurrLoc;
        mUnsupported = false;
        mError = false;
        mNumMultArg = mArgsExpected = 0;
    }

    void error()
    {
        // We don't throw an error in this case
        // Just set the error flag
        mError = true;
    }

    bool accept(Token::TokenType s);
    bool expect(Token::TokenType s);
    int  format(void); // This function returns the number of args for the next format 0,1,2 or 3
    int  directive(void);
    void flags(void);
    int  width(void);
    int  precision(void);
    void length_modifier(void);
    int  conversion(void);
};

void DumpAllThreadOutput( FILE *streamout, unsigned char * dumpMem, size_t buffersize);

#endif