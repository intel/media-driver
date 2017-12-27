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
//! \file      cm_mov_inst.h  
//! \brief     Contains Class MovInst_RT definitions  
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMMOVINST_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMMOVINST_H_

#include "cm_array.h"
#include "cm_mem.h"

// A simple implementation that encodes a mov instruction in align1 mode.
// Both operands must be direct GRF with no modifiers.
// predication is disabled, and all instruction control fields have default value.
// The format of the mov instruction should be identical for SNB/IVB/HSW
class MovInst_RT 
{

#define GENX_GRF_BYTE_SIZE   32

    uint32_t instDW[4];  //4 DWORDS per inst
    bool isBDW;
public:

    void* GetBinary() { return instDW; }
    void EnableDebug(){SetBits( 30, 30, 0x1);}    // DbgCtrl;
    void ClearDebug(){SetBits( 30, 30, 0x0);}    // DbgCtrl;

    typedef enum {
        REG_ARF = 0x0,
        REG_GRF = 0x1,
        REG_MRF = 0x2,
        REG_IMM = 0x3
    } Genx_Reg_File;

    typedef enum {
        EXEC_SIZE_1 = 0x0,
        EXEC_SIZE_2 = 0x1,
        EXEC_SIZE_4 = 0x2,
        EXEC_SIZE_8 = 0x3,
        EXEC_SIZE_16 = 0x4,
        EXEC_SIZE_32 = 0x5,
        EXEC_SIZE_ILLEGAL = 0x6
    } Genx_Exec_Size;

    typedef enum {
        REG_TYPE_UD = 0x0,
        REG_TYPE_D = 0x1,
        REG_TYPE_UW = 0x2,
        REG_TYPE_W = 0x3,
        REG_TYPE_UB = 0x4,
        REG_TYPE_B = 0x5,
        REG_TYPE_DF = 0x6,
        REG_TYPE_F = 0x7,
    } Genx_Reg_Type;

    typedef enum {
        VS_0 = 0x0,
        VS_1 = 0x1,
        VS_2 = 0x2,
        VS_4 = 0x3,
        VS_8 = 0x4,
        VS_16 = 0x5,
        VS_32 = 0x6
    } Genx_VStride;

    typedef enum {
        W_1 = 0x0,
        W_2 = 0x1,
        W_4 = 0x2,
        W_8 = 0x3,
        W_16 = 0x4
    } Genx_Width;

    typedef enum {
        HS_0 = 0x0,
        HS_1 = 0x1,
        HS_2 = 0x2,
        HS_4 = 0x3
    } Genx_HStride;

    MovInst_RT(bool is_BDW, bool is_hwdebug) {
        // initialize the instruction to
        // mov (8) r0.0<1>:ud r0.0<8;8,1>:ud {Align1}
        for( int i = 0; i < 4; i++ ) {
            instDW[i] = 0;
        }
        isBDW = is_BDW;
        SetBits( 0, 6, 0x1);    //opcode = move
        if(is_hwdebug) {
            EnableDebug();
        }    // DbgCtrl
        SetBits( 8, 8, 0);      //align mode = align1
        SetExecSize( EXEC_SIZE_8 );     //exec size = 8 
        SetDstRegFile( REG_GRF );  //dst reg file = GRF
        SetDstType( REG_TYPE_UD );        //dst type = UD
        SetSrcRegFile( REG_GRF );  //src0 reg file = GRF
        SetSrcType( REG_TYPE_UD );        //src0 type = UD
        SetDstRegNum( 0 );      //dst reg = 0
        SetDstSubregNum( 0 );   //dst subreg = 0
        SetDstRegion( HS_1 );      //dst region = <1>
        SetSrcRegNum( 0 );      //src reg = 0
        SetSrcSubregNum( 0 );   //src subreg = 0
        SetSrcRegion( VS_8, W_8, HS_1 );      //src region = <8;8,1>
    }

    static uint32_t GetTypeSize( Genx_Reg_Type type ) {
        switch( type ) {
            case REG_TYPE_UD:
            case REG_TYPE_D:
            case REG_TYPE_F:
            case REG_TYPE_DF:
                return 4;
            case REG_TYPE_UW:
            case REG_TYPE_W:
                return 2;
            case REG_TYPE_UB:
            case REG_TYPE_B:
                return 1;
            default:
                CM_ASSERTMESSAGE("Error: Invalid register data type.");
                return 1;
        }
    }

    // Create a sequence of Gen move instructions to copy <size> byte data
    // from srcOffset to dstOffset.  The move instructions are appended
    // to the <movInsts> vector starting at the given <index>.  The 
    // function returns the total number of move instructions created 
    static uint32_t CreateMoves( uint32_t dstOffset, uint32_t srcOffset, uint32_t size, 
        CMRT_UMD::CmDynamicArray &movInsts, uint32_t index, bool is_BDW, bool is_hwdebug ) {

            uint32_t dstEnd = dstOffset + size;
            uint32_t moveSize = GENX_GRF_BYTE_SIZE;
            uint32_t remainder = dstOffset % GENX_GRF_BYTE_SIZE;
            uint32_t numMoves = 0;

            if( remainder != 0 ) {
                // dst address is not GRF-aligned, handle this portion first 
                // until we get a GRF-aligned dst address
                uint32_t dstGRFEnd = dstOffset + GENX_GRF_BYTE_SIZE - remainder;
                if( dstGRFEnd > dstEnd ) {
                    dstGRFEnd = dstEnd;
                }
                while( dstGRFEnd != dstOffset ) {
                    while( (dstGRFEnd - dstOffset) >= moveSize ) { 
                        MovInst_RT* inst = MovInst_RT::CreateSingleMove( 
                            dstOffset, srcOffset, moveSize, is_BDW, is_hwdebug); 
                        if( !movInsts.SetElement( index + numMoves, inst )) {
                            CmSafeDelete(inst);
                            CM_ASSERTMESSAGE("Error: SetElement failure.");
                        }
                        numMoves++;
                        uint32_t srcEndGRFNum = (srcOffset + moveSize - 1) / GENX_GRF_BYTE_SIZE;
                        if( srcEndGRFNum != srcOffset / GENX_GRF_BYTE_SIZE )
                        {
                            // src straddles two GRFs, we have to insert compensation moves
                            // for the elements in the 2nd GRF
                            uint32_t compSrcStart = srcEndGRFNum * GENX_GRF_BYTE_SIZE;
                            uint32_t compSize = srcOffset + moveSize - compSrcStart;
                            uint32_t compDstStart = dstOffset + (moveSize - compSize);
                            numMoves += CreateMoves( compDstStart, compSrcStart, compSize, movInsts, index + numMoves, is_BDW, is_hwdebug);
                        }
                        dstOffset += moveSize;
                        srcOffset += moveSize;
                    }
                    moveSize >>= 1;
                }
            }

            moveSize = GENX_GRF_BYTE_SIZE;
            while( dstEnd != dstOffset ) {
                while( (dstEnd - dstOffset) >= moveSize ) { 
                    MovInst_RT* inst = MovInst_RT::CreateSingleMove( 
                        dstOffset, srcOffset, moveSize, is_BDW, is_hwdebug); 
                    if( !movInsts.SetElement( index + numMoves, inst )) {
                        CM_ASSERTMESSAGE("Error: SetElement failure.");
                        CmSafeDelete(inst);
                    }
                    numMoves++;

                    uint32_t srcEndGRFNum = (srcOffset + moveSize - 1) / GENX_GRF_BYTE_SIZE;
                    if( srcEndGRFNum != srcOffset / GENX_GRF_BYTE_SIZE )
                    {
                        // src straddles two GRFs, we have to insert compensation moves
                        // for the elements in the 2nd GRF
                        uint32_t compSrcStart = srcEndGRFNum * GENX_GRF_BYTE_SIZE;
                        uint32_t compSize = srcOffset + moveSize - compSrcStart;
                        uint32_t compDstStart = dstOffset + (moveSize - compSize);
                        numMoves += CreateMoves( compDstStart, compSrcStart, compSize, movInsts, index + numMoves, is_BDW, is_hwdebug);
                    }

                    dstOffset += moveSize;
                    srcOffset += moveSize;
                }
                moveSize >>= 1;
            }
            return numMoves;
    }

    // Create a single move instruction for the given dst/src offset and size,
    // all in unit of bytes.  The size must be a legal Gen execution size,
    // i.e., power-of-twos from 1 to 32.
    static MovInst_RT *CreateSingleMove( uint32_t dstOffset, uint32_t srcOffset, uint32_t size, bool is_BDW, bool is_hwdebug ) 
    {
        Genx_Exec_Size eSize;
        Genx_Reg_Type type;
        Genx_VStride VStride;
        Genx_Width width;
        Genx_HStride HStride;

        switch( size ) {
            case 32:
                //type = UD, esize = 8
                eSize = MovInst_RT::EXEC_SIZE_8;
                type = MovInst_RT::REG_TYPE_UD;
                VStride = MovInst_RT::VS_8;
                width = MovInst_RT::W_8;
                HStride = MovInst_RT::HS_1;
                break;
            case 16:
                //type = UD, esize = 4
                eSize = MovInst_RT::EXEC_SIZE_4;
                type = MovInst_RT::REG_TYPE_UD;
                VStride = MovInst_RT::VS_4;
                width = MovInst_RT::W_4;
                HStride = MovInst_RT::HS_1;
                break;
            case 8:
                //type = UD, esize = 2;
                eSize = MovInst_RT::EXEC_SIZE_2;
                type = MovInst_RT::REG_TYPE_UD;
                VStride = MovInst_RT::VS_2;
                width = MovInst_RT::W_2;
                HStride = MovInst_RT::HS_1;
                break;
            case 4:
                //type = UD, esize = 1;
                eSize = MovInst_RT::EXEC_SIZE_1;
                type = MovInst_RT::REG_TYPE_UD;
                VStride = MovInst_RT::VS_0;
                width = MovInst_RT::W_1;
                HStride = MovInst_RT::HS_0;
                break;
            case 2:
                //type = UW, esize = 1;
                eSize = MovInst_RT::EXEC_SIZE_1;
                type = MovInst_RT::REG_TYPE_UW;
                VStride = MovInst_RT::VS_0;
                width = MovInst_RT::W_1;
                HStride = MovInst_RT::HS_0;
                break;
            case 1:
                //type = UB, esize = 1;
                eSize = MovInst_RT::EXEC_SIZE_1;
                type = MovInst_RT::REG_TYPE_UB;
                VStride = MovInst_RT::VS_0;
                width = MovInst_RT::W_1;
                HStride = MovInst_RT::HS_0;
                break;
            default:
                // below assignment is to avoid variables being used uninitialized in this function 
                eSize = MovInst_RT::EXEC_SIZE_1;
                type = MovInst_RT::REG_TYPE_UD;
                VStride = MovInst_RT::VS_0;
                width = MovInst_RT::W_1;
                HStride = MovInst_RT::HS_0;
                CM_ASSERTMESSAGE("Error: Invalid Gen execution size.");
        }

        MovInst_RT *inst = new (std::nothrow) MovInst_RT(is_BDW, is_hwdebug);
        if(!inst)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return nullptr;
        }

        uint32_t dstSubRegNum = dstOffset % GENX_GRF_BYTE_SIZE;
        uint32_t srcSubRegNum = srcOffset % GENX_GRF_BYTE_SIZE;

        inst->SetDstRegNum( dstOffset / GENX_GRF_BYTE_SIZE );
        inst->SetDstSubregNum( dstSubRegNum );
        inst->SetSrcRegNum( srcOffset / GENX_GRF_BYTE_SIZE );
        inst->SetSrcSubregNum( srcSubRegNum );
        inst->SetExecSize( eSize );
        inst->SetDstType( type );
        inst->SetSrcType( type );
        inst->SetDstRegion( MovInst_RT::HS_1 );
        inst->SetSrcRegion( VStride, width, HStride );
        return inst;
    }

    void SetBits(const unsigned char LowBit, const unsigned char HighBit, const uint32_t value) {
    CM_ASSERT(HighBit <= 127);
    CM_ASSERT((HighBit - LowBit) <= 31);
    CM_ASSERT(HighBit >= LowBit);
    CM_ASSERT((HighBit/32) == (LowBit/32));
    uint32_t maxvalue = ((1 << (HighBit - LowBit)) - 1) | (1 << (HighBit - LowBit));
    uint32_t newvalue = value;
    newvalue &= maxvalue;
    uint32_t Dword = HighBit/32;

    uint32_t mask = (unsigned long)(0xffffffff >> (32 - (HighBit - LowBit + 1)));
    uint32_t shift = LowBit - (Dword*32);
    mask <<= shift;
    instDW[Dword] &= ~mask;
    instDW[Dword] |= (newvalue << shift);
  }

    uint32_t GetBits(const int LowBit, const int HighBit) {
    CM_ASSERT(HighBit <= 127);
    CM_ASSERT((HighBit - LowBit) <= 31);
    CM_ASSERT(HighBit >= LowBit);
    CM_ASSERT((HighBit/32) == (LowBit/32));

    uint32_t retValue;
    uint32_t Dword = HighBit/32;
    uint32_t mask = (uint32_t)(0xffffffff >> (32 - (HighBit - LowBit + 1)));
    uint32_t shift = LowBit - (Dword*32);

    retValue = instDW[Dword] & (mask << shift);
    retValue >>= shift;
    retValue &= mask;

    return retValue;
  }

    void SetExecSize( const Genx_Exec_Size size ) {
        SetBits( 21, 23, size );
    }

    Genx_Exec_Size GetExecSize( void ) {
        return (Genx_Exec_Size) GetBits( 21, 23 );
    }

    void SetDstType( const Genx_Reg_Type type ) {
        if( isBDW )
        {
            SetBits( 37, 40, type );
        }
        else
        {
            SetBits( 34, 36, type );
        }
    }

    Genx_Reg_Type GetDstType( void ) {
        if( isBDW )
        {
            return (Genx_Reg_Type) GetBits( 37, 40 );
        }
        else
        {
             return (Genx_Reg_Type) GetBits( 34, 36 );
        }
    }

    void SetDstRegFile( Genx_Reg_File regFile ) {
        if( isBDW )
        {
            SetBits( 35, 36, regFile );
        }
        else
        {
            SetBits( 32, 33, regFile );
        }
    }

    void SetSrcType( const Genx_Reg_Type type ) {
        if( isBDW )
        {
            SetBits( 43, 46, type );
        }
        else
        {
            SetBits( 39, 41, type );
        }
    }

    Genx_Reg_Type GetSrcType( void ) {
        if( isBDW )
        {
            return (Genx_Reg_Type) GetBits( 43, 46 );
        }
        else
        {
            return (Genx_Reg_Type) GetBits( 39, 41 );
        }
    }

    void SetSrcRegFile( Genx_Reg_File regFile ) {
        if( isBDW )
        {
            SetBits( 41, 42, regFile );
        }
        else
        {
            SetBits( 37, 38, REG_GRF );
        }
    }

    void SetDstRegNum( const unsigned long reg ) {
        CM_ASSERT( reg < 128 );
        SetBits( 53, 60, reg );
    }

    uint32_t GetDstRegNum( void ) {
        return GetBits( 53, 60 );
    }

    void SetDstSubregNum( const unsigned long subreg ) {
        CM_ASSERT( subreg < 32 );
        SetBits( 48, 52, subreg );
    }

    uint32_t GetDstSubregNum( void ) {
        return GetBits( 48, 52 );
    }

    void SetDstRegion( const Genx_HStride hstride ) {
        SetBits( 61, 62, hstride );
    }

    Genx_HStride GetDstRegion( void ) {
        return (Genx_HStride) GetBits( 61, 62 );
    }

    void SetSrcRegNum( const unsigned long reg ) {
        CM_ASSERT( reg < 128 );
        SetBits( 69, 76, reg );
    }

    uint32_t GetSrcRegNum( void ) {
        return GetBits( 69, 76 );
    }

    void SetSrcSubregNum( const unsigned long subreg ) {
        CM_ASSERT( subreg < 32 );
        SetBits( 64, 68, subreg );
    }

    uint32_t GetSrcSubregNum( void ) {
        return GetBits( 64, 68 );
    }

    void SetSrcRegion( const Genx_VStride vstride, const Genx_Width width, const Genx_HStride hstride ) {
        SetBits( 85, 88, vstride );
        SetBits( 82, 84, width );
        SetBits( 80, 81, hstride );
    }

    Genx_VStride GetSrcVStride( void ) {
        return (Genx_VStride) GetBits( 85, 88 );
    }

    Genx_Width GetSrcWidth( void ) {
        return (Genx_Width) GetBits( 82, 84 );
    }

    Genx_HStride GetSrcHStride( void ) {
        return (Genx_HStride) GetBits( 80, 81 );
    }

    uint32_t GetExecSizeValue( Genx_Exec_Size size ) {
        switch( size ) {
            case EXEC_SIZE_1:
                return 1;
            case EXEC_SIZE_2:
                return 2;
            case EXEC_SIZE_4:
                return 4;
            case EXEC_SIZE_8:
                return 8;
            case EXEC_SIZE_16:
                return 16;
            case EXEC_SIZE_32:
                return 32;
            default:
                CM_ASSERTMESSAGE("Error: Invalid Gen execution operand size.");
                return 0;
        }
    }

    const char* GetTypeStr( Genx_Reg_Type type ) {
        switch( type ) {
            case REG_TYPE_UD:
                return "ud";
            case REG_TYPE_D:
                return "d";
            case REG_TYPE_UW:
                return "uw";
            case REG_TYPE_W:
                return "w";
            case REG_TYPE_UB:
                return "ub";
            case REG_TYPE_B:
                return "b";
            case REG_TYPE_DF:
                return "df";
            case REG_TYPE_F:
                return "f";
            default:
                CM_ASSERTMESSAGE("Error: Invalid register data type.");
                return "";
        }
    }

    uint32_t GetVStrideValue( Genx_VStride vstride ) {
        switch( vstride ) {
            case VS_0:
                return 0;
            case VS_1:
                return 1;
            case VS_2:
                return 2;
            case VS_4:
                return 4;
            case VS_8:
                return 8;
            case VS_16:
                return 16;
            case VS_32:
                return 32;
            default:
                CM_ASSERTMESSAGE("Error: Invalid value of vertical stride.");
                return 0;
        }
    }

    uint32_t GetWidthValue( Genx_Width width ) {
        switch( width ) {
            case W_1:
                return 1;
            case W_2:
                return 2;
            case W_4:
                return 4;
            case W_8:
                return 8;
            case W_16:
                return 16;
            default:
                CM_ASSERTMESSAGE("Error: Invalid genx width value.");
                return 0;
        }
    }

    uint32_t GetHStrideValue( Genx_HStride hstride ) {
        switch (hstride ) {
            case HS_0:
                return 0;
            case HS_1:
                return 1;
            case HS_2:
                return 2;
            case HS_4:
                return 4;
            default:
                CM_ASSERTMESSAGE("Error: Invalid value of horizontal stride.");
                return 0;
        }
    }

    void emit() {
        uint32_t execSize, dstHStride, srcVStride, srcWidth, srcHStride, dstSubregNum, srcSubregNum;
        const char *dstType, *srcType;
        execSize = GetExecSizeValue( GetExecSize() );
        dstHStride = GetHStrideValue( GetDstRegion() );
        dstType = GetTypeStr( GetDstType() );
        srcType = GetTypeStr( GetSrcType() );
        srcVStride = GetVStrideValue( GetSrcVStride() );
        srcWidth = GetWidthValue( GetSrcWidth() );
        srcHStride = GetHStrideValue( GetSrcHStride() );
        dstSubregNum = GetDstSubregNum() / GetTypeSize( GetDstType() );
        srcSubregNum = GetSrcSubregNum() / GetTypeSize( GetSrcType() );

        CM_NORMALMESSAGE("mov (%d) r%d.%d<%d>:%s r%d.%d<%d;%d,%d>:%s.",
            execSize, GetDstRegNum(), dstSubregNum, dstHStride, dstType, 
            GetSrcRegNum(), srcSubregNum, srcVStride, srcWidth, srcHStride, srcType ); 
    }
};

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMMOVINST_H_
