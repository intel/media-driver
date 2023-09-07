/*
* Copyright (c) 2022, Intel Corporation
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
//! \file      vp_visa.cpp 
//! \brief     Contains ISAfile definitions 
//!

#include "vp_visa.h"
#include <fstream>
#include "mos_utilities.h"

using namespace vp::vISA;

ISAfile::ISAfile(const uint8_t *data, unsigned size) : version(0), data(data), end(data + size),
size(size), error(0), errorIndex(0), header(0), kernel_data_loaded(false), function_data_loaded(false) { }

ISAfile::ISAfile(const ISAfile& other) {
    version = other.version;
    data = other.data;
    end = other.end;
    size = other.size;
    int error_length = std::strlen(other.error);
    error = new char[error_length + 1];
    if (nullptr == error)
    {
        MOS_OS_ASSERTMESSAGE("create array failed!");
        return;
    }
    char *perror = const_cast<char*>(error);
    perror[error_length] = '\0';
    MOS_SecureMemcpy(perror, error_length + 1, other.error, error_length);
    kernel_data_loaded = other.kernel_data_loaded;
    function_data_loaded = other.function_data_loaded;
    errorIndex = other.errorIndex;
    header = new Header(other.version);
    *header = *other.header;
    for (KernelBody *kb : other.kernel_data) {
        KernelBody *kb2 = new KernelBody(other.version);
        *kb2 = *kb;
        kernel_data.push_back(kb2);
    }
    for (FunctionBody *fb : other.function_data) {
        FunctionBody *fb2 = new FunctionBody(other.version);
        *fb2 = *fb;
        function_data.push_back(fb2);
    }
}

ISAfile& ISAfile::operator= (const ISAfile& other) {
    if (this != &other) {
        version = other.version;
        data = other.data;
        end = other.end;
        size = other.size;
        delete[] error;
        int   error_length = std::strlen(other.error);
        error = new char[error_length + 1];
        if (nullptr == error)
        {
            MOS_OS_ASSERTMESSAGE("create array failed!");
            return *this;
        }
        char *perror = const_cast<char *>(error);
        perror[error_length] = '\0';
        MOS_SecureMemcpy(perror, error_length + 1, other.error, error_length);
        kernel_data_loaded = other.kernel_data_loaded;
        function_data_loaded = other.function_data_loaded;
        errorIndex = other.errorIndex;
        *header = *other.header;
        for (KernelBody *kb : kernel_data)
            delete kb;
        for (FunctionBody *fb : function_data)
            delete fb;
        for (KernelBody *kb : other.kernel_data)
            kernel_data.push_back(&*kb);
        for (FunctionBody *fb : other.function_data)
            function_data.push_back(&*fb);
    }
    return *this;
}

ISAfile::~ISAfile() {
    delete header;
    for (KernelBody *kb : kernel_data)
        delete kb;
    for (FunctionBody *f : function_data)
        delete f;
    if (error)
    {
        delete[] error;
        error = nullptr;
    }
}

bool ISAfile::readFile() {
    bool status = true;
    status &= loadHeader();
    if (version < 302)
        return false;
    status &= loadKernelData();
    status &= loadFunctionData();
    return status;
}

bool ISAfile::loadHeader() {
    header = new Header(version);
    const uint8_t *p = header->parse(data, end, this);
    if (!p) {
        delete header;
        return false; //error loading header
    }
    return true;
}

bool ISAfile::loadKernelData() {
    const uint8_t *p = 0;
    for (Kernel *k : header->getKernelInfo()) {
        KernelBody *kb = new KernelBody(version);
        p = kb->parse(data + k->getOffset(), end, this);
        if (!p) {
            delete kb;
            return false; //error loading kernel_data
        }
        kernel_data.push_back(kb);
    }
    kernel_data_loaded = true;
    return true;
}

bool ISAfile::loadFunctionData() {
    const uint8_t *p = 0;
    for (Function *f : header->getFunctionInfo()) {
        FunctionBody *fb = new FunctionBody(version);
        p = fb->parse(data + f->getOffset(), end, this);
        if (!p) {
            delete fb;
            return false; //error loading kernel_data
        }
        function_data.push_back(fb);
    }
    function_data_loaded = true;
    return true;
}

std::vector<KernelBody*> &ISAfile::getKernelsData() {
    if (!kernel_data_loaded) loadKernelData();
    return kernel_data;
}

std::vector<FunctionBody*> &ISAfile::getFunctionsData() {
    if (!function_data_loaded) loadFunctionData();
    return function_data;
}

const uint8_t* ISAfile::readField(const uint8_t *p, const uint8_t *buffEnd,
    Field &field, unsigned dataSize) {
    switch (field.type) {
    case Datatype::ONE: field.number8 = *((int8_t *)p); p++; break;
    case Datatype::TWO: field.number16 = *((int16_t *)p); p += 2; break;
    case Datatype::FOUR: field.number32 = *((int32_t *)p); p += 4; break;
    case Datatype::EIGHT: field.number64 = *((int64_t *)p); p += 8; break;
    case Datatype::VARCHAR:
    {
        if (p + dataSize > buffEnd) {
            // error: truncated
            p = 0;
            return 0;
        }
        char *string = new char[dataSize + 1];
        MOS_SecureMemcpy(string, dataSize + 1, p, dataSize);
        string[dataSize] = '\0';
        field.size = dataSize;
        field.varchar = string;
        p += dataSize;
        break;
    }
    case Datatype::VARCHAR_POOL:
    {
        const uint8_t *strEnd = (const uint8_t *)std::memchr(p, 0, end - p);
        auto len = strEnd - p;
        char *string = new char[len + 1];
        MOS_SecureMemcpy(string, len + 1, p, len);
        string[len] = '\0';
        field.size = (uint32_t)len + 1;
        field.varchar = string;
        p = strEnd + 1;
        break;
    }
    case Datatype::GDATA:
    {
        // copy only if no out of bound.
        if (p + dataSize < end) {
            uint8_t *gdata = new uint8_t[dataSize];
            MOS_SecureMemcpy(gdata, dataSize , p, dataSize);
            field.gdata = gdata;
            field.size = dataSize;
            p += dataSize;
        } else {
            field.gdata = nullptr;
            field.size = 0;
        }
        break;
    }
    default: break;
    }
    return p;
}

const uint8_t *ISAfile::setError(const char * e, unsigned index) {
    error = e;
    errorIndex = index;
    return 0;
}

bool ISAfile::writeToFile(const char *filename, std::vector<uint8_t> &originalBuffer) {
    if (!header) {
        setError("Header not loaded", 0);
        return false;
    }
    if (!kernel_data_loaded)
        loadKernelData();
    if (!function_data_loaded)
        loadFunctionData();

    std::ofstream newFile(filename, std::ios::out | std::ios::binary);
    if (!newFile) {
        setError("Error creating new file ", 0);
        return false;
    }

    // buffer
    std::vector<char> buffer;
    // header
    header->addToBuffer(buffer, this);

    // kernel body
    for (KernelBody* k : kernel_data) {
        k->addToBuffer(buffer, this);
    }

    // function body
    for (FunctionBody* f : function_data) {
        f->addToBuffer(buffer, this);
    }

    // gen binaries for this kernel
    for (Kernel *k : header->getKernelInfo()) {
        for (GenBinary *g : k->getGenBinaryInfo()) {
            uint32_t offset = g->getBinaryOffset();
            if (offset > originalBuffer.size()) {
                setError("Error writing GEN binary into ISA file, bad offset from original file", 0);
                return false;
            }
            for (uint32_t b = 0; b < g->getBinarySize(); b++) {
                buffer.push_back(static_cast<char>(originalBuffer[offset + b]));
            }
        }
    }

    newFile.write(buffer.data(), buffer.size());
    newFile.close();
    return true;
}

void ISAfile::addToBuffer(Field &field, std::vector<char> &buffer) {
    switch (field.type) {
    case Datatype::ONE: buffer.push_back(field.ui8[0]); break;
    case Datatype::TWO: buffer.push_back(field.ui8[0]); buffer.push_back(field.ui8[1]); break;
    case Datatype::FOUR: buffer.push_back(field.ui8[0]); buffer.push_back(field.ui8[1]);
        buffer.push_back(field.ui8[2]); buffer.push_back(field.ui8[3]); break;
    case Datatype::EIGHT: buffer.push_back(field.ui8[0]); buffer.push_back(field.ui8[1]);
        buffer.push_back(field.ui8[2]); buffer.push_back(field.ui8[3]);
        buffer.push_back(field.ui8[4]); buffer.push_back(field.ui8[5]);
        buffer.push_back(field.ui8[6]); buffer.push_back(field.ui8[7]); break;
    case Datatype::VARCHAR:
    {
        for (unsigned i = 0; i < field.size; i++) {
            buffer.push_back(static_cast<char>(field.varchar[i]));
        }
        break;
    }
    case Datatype::VARCHAR_POOL:
    {
        for (unsigned i = 0; i < field.size; i++) {
            buffer.push_back(static_cast<char>(field.varchar[i]));
        }
        break;
    }
    case Datatype::GDATA:
    {
        for (unsigned i = 0; i < field.size; i++) {
            buffer.push_back(static_cast<char>(field.gdata[i]));
        }
        break;
    }
    default: break;
    }
}
