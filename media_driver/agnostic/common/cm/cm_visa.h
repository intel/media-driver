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
//! \file      cm_visa.h 
//! \brief     Contains Class ISAfile definitions 
//!

#ifndef VISA_H
#define VISA_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <array>

namespace vISA {
    enum Datatype { ONE, TWO, FOUR, EIGHT, VARCHAR, VARCHAR_POOL, GDATA, STRUCT, REMOVED };

    //!
    //! \brief      Field Struct.
    //! \details    This struct represents any field from the ISA file.
    //!             Fields are tagged according their data size in bytes (1 to 8 bytes).
    //!             Regular strings are tagged as VARCHAR and strings from the String
    //!             Pool as VARCHAR_POOL.
    //!
    struct Field {
        Datatype type;
        uint8_t countField;
        uint32_t size;
        union {
            int8_t number8;
            int16_t number16;
            int32_t number32;
            int64_t number64;
            int8_t ui8[8];
            char *varchar;
            const uint8_t *gdata;
        };

        //!
        //! \brief      Constructor of Field struct.
        //!
        Field() : countField(0), number64(0) {}

        //!
        //! \brief      Constructor of Field struct.
        //!
        Field(Datatype t) : type(t), countField(0), size(0), number64(0) {}

        //!
        //! \brief      Constructor of Field struct.
        //!
        Field(Datatype t, uint8_t cf) : type(t), countField(cf), size(0), number64(0) {}

        //!
        //! \brief      Destructor of Field struct.
        //!
        ~Field() {
            if (type == Datatype::VARCHAR || type == Datatype::VARCHAR_POOL)
                delete[] varchar;
            else if (type == Datatype::GDATA)
                delete[] gdata;
        }
    };

    class Header;
    class KernelBody;
    class FunctionBody;

    //!
    //! \brief      ISAfile Class.
    //! \details     This class provides the functionality to manage ISA files: read
    //!             parse and write ISA files. Access to Header, Kernels and
    //!             Functions is also provided.
    //!             It supports from vISA 3.4.
    //!
    class ISAfile
    {
    private:
        unsigned version;
        const uint8_t *data;
        const uint8_t *end;
        unsigned size;
        const char *error;
        unsigned errorIndex;
        Header *header;
        bool kernel_data_loaded;
        bool function_data_loaded;
        std::vector<KernelBody*> kernel_data;
        std::vector<FunctionBody*> function_data;

        //!
        //! \brief      Reads and parses the Header from ISA file.
        //! \retval     True if sucessfully parsers the header.
        //!             False otherwise.
        //!
        bool loadHeader();

        //!
        //! \brief      Reads and parses the kernels from ISA file.
        //! \retval     True if sucessfully parsers all the kernels.
        //!             False otherwise.
        //!
        bool loadKernelData();

        //!
        //! \brief      Reads and parses the functions from ISA file.
        //! \retval     True if sucessfully parsers all the functions.
        //!             False otherwise.
        //!
        bool loadFunctionData();
    public:
        //!
        //! \brief      Constructor of ISAfile class.
        //! \param      [in] data.
        //!             Pointer to buffer data from ISA file.
        //! \param      [in] size.
        //!             Size of ISA file buffer.
        //!
        ISAfile(const uint8_t *data, unsigned size);

        //!
        //! \brief      Copy Constructor of ISAfile class.
        //! \param      [in] other.
        //!             Reference to object to copy.
        //!
        ISAfile(const ISAfile& other);

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy.
        //!
        ISAfile& operator= (const ISAfile& other);

        //!
        //! \brief      Destructor of ISAfile class.
        //!
        ~ISAfile();

        //!
        //! \brief      Reads the ISA file.
        //! \retval     True if it reads successfully.
        //!
        bool readFile();

        //!
        //! \brief      Returns the Header object.
        //! \retval     The pointer to Header object.
        //!
        Header *getHeader() { return header; }

        //!
        //! \brief      Returns the vector of kernels.
        //! \retval     The reference to the vector with the kernels.
        //!
        std::vector<KernelBody*> &getKernelsData();

        //!
        //! \brief      Returns the vector of functions.
        //! \retval     The reference to the vector with the functions.
        //!
        std::vector<FunctionBody*> &getFunctionsData();

        //!
        //! \brief      Reads a value from buffer and assigns to field.
        //! \param      [in] p.
        //!             Pointer to buffer data from ISA file.
        //! \param      [in] buffEnd.
        //!             Pointer to the end of buffer data from ISA file.
        //! \param      [out] field.
        //!             Reference to field.
        //! \param      [in] size.
        //!             Size of the value to be read.
        //! \retval     The pointer to buffer after reading the value.
        //!
        const uint8_t* readField(const uint8_t *p, const uint8_t *buffEnd, Field &field, unsigned size);

        //!
        //! \brief      Returns the error message.
        //! \retval     The pointer to string with the error message.
        //!
        const char *getError() { return error; }

        //!
        //! \brief      Returns the error index.
        //! \retval     The index to the error.
        //!
        unsigned getErrorOffset() { return errorIndex; }

        //!
        //! \brief      Sets the error message.
        //! \param      [int] e.
        //!             Error message.
        //! \param      [in] index.
        //!              Index to the error.
        //!
        const uint8_t *setError(const char * e, unsigned index);

        //!
        //! \brief      Returns the vISA version of current file.
        //! \retval     The vISA version as integer.
        //!
        unsigned getCurrentVISAVersion() { return version; }

        //!
        //! \brief      Sets the ISA file version
        //! \param      The ISA file version
        //!
        void setCurrentVISAVersion(unsigned v) { version = v; }

        //!
        //! \brief      Writes a new ISA file with the current information in this class.
        //! \param      [in] filename.
        //!             String with the file name.
        //! \param      [in] originalBuffer.
        //!             Buffer data of the original ISA file.
        //! \retval     True if successfully writes the ISA file.
        //!
        bool writeToFile(const char *filename, std::vector<uint8_t> &originalBuffer);

        //!
        //! \brief      Adds the value from a field to buffer.
        //!             This function is called when writing a ISA file.
        //! \param      [in] field.
        //!             The field to get its value.
        //! \param      [out] buffer.
        //!             Buffer data where the field's field is added.
        //!
        void addToBuffer(Field &field, std::vector<char> &buffer);

    };

    //!
    //! \brief      AttributeInfo Class.
    //! \details    This class represents the AttributeInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class AttributeInfo {
    public:
        std::array<Field, 3> fields = std::array<Field, 3>
        {
            Field(Datatype::FOUR), // name
                Field(Datatype::ONE), // size
                Field(Datatype::GDATA, 1) // value
        };

        //!
        //! \brief      Constructor of AttributeInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        AttributeInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Constructor of AttributeInfo class.
        //!
        AttributeInfo() { }

        //!
        //! \brief      Destructor of AttributeInfo class.
        //!
        ~AttributeInfo() {
        }

        //!
        //! \brief      Returns the integer value of the Name field.
        //! \details    Name field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getName() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the Name field.
        //! \details    Name field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setName(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getSize() {
            return (uint8_t)fields[1].number8;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint8_t value) {
            fields[1].number8 = value;
        }

        //!
        //! \brief      Returns the pointer to buffer data from Value field.
        //! \details    Value field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     A pointer to buffer data (const uint8_t*).
        //!
        const uint8_t* getValue() {
            return fields[2].gdata;
        }

        //!
        //! \brief      Sets the pointer to buffer data of the Value field.
        //! \details    Value field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to buffer data (uint8_t*) to be assigned.
        //!
        void setValue(uint8_t * value) {
            fields[2].gdata = value;
        }

        //!
        //! \brief      Parses one AttributeInfo object from ISA file.
        //! \details    Reads and parses all the fields of the AttributeInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one AttributeInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for AttributeInfo's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the AttributeInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's AttributeInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      InputInfo Class.
    //! \details    This class represents the InputInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class InputInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::ONE), // kind
                Field(Datatype::FOUR), // id
                Field(Datatype::TWO), // offset
                Field(Datatype::TWO) // size
        };

        //!
        //! \brief      Constructor of InputInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        InputInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of InputInfo class.
        //!
        ~InputInfo() {
        }

        //!
        //! \brief      Returns the integer value of the Kind field.
        //! \details    Kind field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        int8_t getKind() {
            return fields[0].number8;
        }

        //!
        //! \brief      Sets the integer value of the Kind field.
        //! \details    Kind field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setKind(int8_t value) {
            fields[0].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the Id field.
        //! \details    Id field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getId() {
            return (uint32_t)fields[1].number32;
        }

        //!
        //! \brief      Sets the integer value of the Id field.
        //! \details    Id field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setId(uint32_t value) {
            fields[1].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Offset field.
        //! \details    Offset field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        int16_t getOffset() {
            return fields[2].number16;
        }

        //!
        //! \brief      Sets the integer value of the Offset field.
        //! \details    Offset field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setOffset(int16_t value) {
            fields[2].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getSize() {
            return (uint16_t)fields[3].number16;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint16_t value) {
            fields[3].number16 = value;
        }

        //!
        //! \brief      Parses one InputInfo object from ISA file.
        //! \details    Reads and parses all the fields of the InputInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one InputInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for InputInfo's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the InputInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's InputInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[1] = Datatype::TWO;
        }
    };

    //!
    //! \brief      VmeInfo Class.
    //! \details    This class represents the VmeInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class VmeInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of VmeInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        VmeInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of VmeInfo class.
        //!
        ~VmeInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    VmeInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one VmeInfo object from ISA file.
        //! \details    Reads and parses all the fields of the VmeInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one VmeInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for VmeInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the VmeInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's VmeInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      SurfaceInfo Class.
    //! \details    This class represents the SurfaceInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class SurfaceInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of SurfaceInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        SurfaceInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of SurfaceInfo class.
        //!
        ~SurfaceInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    SurfaceInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one SurfaceInfo object from ISA file.
        //! \details    Reads and parses all the fields of the SurfaceInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one SurfaceInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for SurfaceInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the SurfaceInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's SurfaceInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      SamplerInfo Class.
    //! \details    This class represents the SamplerInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class SamplerInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of SamplerInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        SamplerInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of SamplerInfo class.
        //!
        ~SamplerInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    SamplerInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one SamplerInfo object from ISA file.
        //! \details    Reads and parses all the fields of the SamplerInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one SamplerInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for SamplerInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the SamplerInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's SamplerInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      LabelInfo Class.
    //! \details    This class represents the LabelInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class LabelInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::ONE), // kind
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of LabelInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        LabelInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of LabelInfo class.
        //!
        ~LabelInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Kind field.
        //! \details    Kind field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getKind() {
            return (uint8_t)fields[1].number8;
        }

        //!
        //! \brief      Sets the integer value of the Kind field.
        //! \details    Kind field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setKind(uint8_t value) {
            fields[1].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    LabelInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one LabelInfo object from ISA file.
        //! \details    Reads and parses all the fields of the LabelInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one LabelInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for LabelInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the LabelInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's LabelInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      PredicateInfo Class.
    //! \details    This class represents the PredicateInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class PredicateInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of PredicateInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        PredicateInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of PredicateInfo class.
        //!
        ~PredicateInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    PredicateInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one PredicateInfo object from ISA file.
        //! \details    Reads and parses all the fields of the PredicateInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one PredicateInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for PredicateInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the PredicateInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's PredicateInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      RelocationInfo Class.
    //! \details    This class represents the RelocationInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class RelocationInfo {
    public:
        std::array<Field, 2> fields = std::array<Field, 2>
        {
            Field(Datatype::TWO), // symbolic_index
                Field(Datatype::TWO) // resolved_index
        };

        //!
        //! \brief      Constructor of RelocationInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        RelocationInfo(unsigned version) {}

        //!
        //! \brief      Constructor of RelocationInfo class.
        //!
        RelocationInfo() {}

        //!
        //! \brief      Copy Constructor of RelocationInfo class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        RelocationInfo(const RelocationInfo& other) {
            fields = other.fields;
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        RelocationInfo& operator= (const RelocationInfo& other) {
            if (this != &other) {
                fields = other.fields;
            }
            return *this;
        }

        //!
        //! \brief      Destructor of RelocationInfo class.
        //!
        ~RelocationInfo() {
        }

        //!
        //! \brief      Returns the integer value of the SymbolicIndex field.
        //! \details    SymbolicIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getSymbolicIndex() {
            return (uint16_t)fields[0].number16;
        }

        //!
        //! \brief      Sets the integer value of the SymbolicIndex field.
        //! \details    SymbolicIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSymbolicIndex(uint16_t value) {
            fields[0].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the ResolvedIndex field.
        //! \details    ResolvedIndex field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getResolvedIndex() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the ResolvedIndex field.
        //! \details    ResolvedIndex field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setResolvedIndex(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Parses one RelocationInfo object from ISA file.
        //! \details    Reads and parses all the fields of the RelocationInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one RelocationInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for RelocationInfo's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the RelocationInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

    };

    //!
    //! \brief      AddressInfo Class.
    //! \details    This class represents the AddressInfo from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class AddressInfo {
    public:
        std::array<Field, 4> fields = std::array<Field, 4>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 2), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of AddressInfo class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        AddressInfo(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of AddressInfo class.
        //!
        ~AddressInfo() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    AddressInfo has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one AddressInfo object from ISA file.
        //! \details    Reads and parses all the fields of the AddressInfo object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one AddressInfo object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for AddressInfo's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the AddressInfo's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's AddressInfo.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
        }
    };

    //!
    //! \brief      Variable Class.
    //! \details    This class represents the Variable from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class Variable {
    public:
        std::array<Field, 8> fields = std::array<Field, 8>
        {
            Field(Datatype::FOUR), // name_index
                Field(Datatype::ONE), // bit_properties
                Field(Datatype::TWO), // num_elements
                Field(Datatype::FOUR), // alias_index
                Field(Datatype::TWO), // alias_offset
                Field(Datatype::ONE), // alias_scope_specifier
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 6), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of Variable class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        Variable(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of Variable class.
        //!
        ~Variable() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the BitProperties field.
        //! \details    BitProperties field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getBitProperties() {
            return (uint8_t)fields[1].number8;
        }

        //!
        //! \brief      Sets the integer value of the BitProperties field.
        //! \details    BitProperties field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setBitProperties(uint8_t value) {
            fields[1].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[2].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[2].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AliasIndex field.
        //! \details    AliasIndex field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getAliasIndex() {
            return (uint32_t)fields[3].number32;
        }

        //!
        //! \brief      Sets the integer value of the AliasIndex field.
        //! \details    AliasIndex field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAliasIndex(uint32_t value) {
            fields[3].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the AliasOffset field.
        //! \details    AliasOffset field is at index 4 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getAliasOffset() {
            return (uint16_t)fields[4].number16;
        }

        //!
        //! \brief      Sets the integer value of the AliasOffset field.
        //! \details    AliasOffset field is at index 4 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAliasOffset(uint16_t value) {
            fields[4].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AliasScopeSpecifier field.
        //! \details    AliasScopeSpecifier field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAliasScopeSpecifier() {
            return (uint8_t)fields[5].number8;
        }

        //!
        //! \brief      Sets the integer value of the AliasScopeSpecifier field.
        //! \details    AliasScopeSpecifier field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAliasScopeSpecifier(uint8_t value) {
            fields[5].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 6 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[6].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 6 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[6].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    Variable has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one Variable object from ISA file.
        //! \details    Reads and parses all the fields of the Variable object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one Variable object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Variable's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the Variable's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's Variable.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
            fields[3] = Datatype::TWO;
        }
    };

    //!
    //! \brief      StringPool Class.
    //! \details    This class represents the StringPool from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class StringPool {
    public:
        std::array<Field, 1> fields = std::array<Field, 1>
        {
            Field(Datatype::VARCHAR_POOL) // string
        };

        //!
        //! \brief      Constructor of StringPool class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        StringPool(unsigned version) {}

        //!
        //! \brief      Copy Constructor of StringPool class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        StringPool(const StringPool& other) {
            fields = other.fields;
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        StringPool& operator= (const StringPool& other) {
            if (this != &other) {
                fields = other.fields;
            }
            return *this;
        }

        //!
        //! \brief      Destructor of StringPool class.
        //!
        ~StringPool() {
        }

        //!
        //! \brief      Returns the string value of the String field.
        //! \details    String field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     A pointer to the string (const char*).
        //!
        const char * getString() {
            return fields[0].varchar;
        }

        //!
        //! \brief      Sets the string value of the String field.
        //! \details    String field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to string (char*) to be assigned.
        //!
        void setString(char * value) {
            fields[0].varchar = value;
        }

        //!
        //! \brief      Parses one StringPool object from ISA file.
        //! \details    Reads and parses all the fields of the StringPool object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one StringPool object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for StringPool's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the StringPool's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

    };

    //!
    //! \brief      GenBinary Class.
    //! \details    This class represents the GenBinary from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class GenBinary {
    public:
        std::array<Field, 3> fields = std::array<Field, 3>
        {
            Field(Datatype::ONE), // gen_platform
                Field(Datatype::FOUR), // binary_offset
                Field(Datatype::FOUR) // binary_size
        };

        //!
        //! \brief      Constructor of GenBinary class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        GenBinary(unsigned version) {}

        //!
        //! \brief      Constructor of GenBinary class.
        //!
        GenBinary() {}

        //!
        //! \brief      Copy Constructor of GenBinary class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        GenBinary(const GenBinary& other) {
            fields = other.fields;
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        GenBinary& operator= (const GenBinary& other) {
            if (this != &other) {
                fields = other.fields;
            }
            return *this;
        }

        //!
        //! \brief      Destructor of GenBinary class.
        //!
        ~GenBinary() {
        }

        //!
        //! \brief      Returns the integer value of the GenPlatform field.
        //! \details    GenPlatform field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getGenPlatform() {
            return (uint8_t)fields[0].number8;
        }

        //!
        //! \brief      Sets the integer value of the GenPlatform field.
        //! \details    GenPlatform field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setGenPlatform(uint8_t value) {
            fields[0].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the BinaryOffset field.
        //! \details    BinaryOffset field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getBinaryOffset() {
            return (uint32_t)fields[1].number32;
        }

        //!
        //! \brief      Sets the integer value of the BinaryOffset field.
        //! \details    BinaryOffset field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setBinaryOffset(uint32_t value) {
            fields[1].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the BinarySize field.
        //! \details    BinarySize field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getBinarySize() {
            return (uint32_t)fields[2].number32;
        }

        //!
        //! \brief      Sets the integer value of the BinarySize field.
        //! \details    BinarySize field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setBinarySize(uint32_t value) {
            fields[2].number32 = value;
        }

        //!
        //! \brief      Parses one GenBinary object from ISA file.
        //! \details    Reads and parses all the fields of the GenBinary object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one GenBinary object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for GenBinary's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the GenBinary's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

    };

    //!
    //! \brief      Function Class.
    //! \details    This class represents the Function from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class Function {
    public:
        std::array<Field, 9> fields = std::array<Field, 9>
        {
            Field(Datatype::ONE), // linkage
                Field(Datatype::TWO), // name_len
                Field(Datatype::VARCHAR, 1), // name
                Field(Datatype::FOUR), // offset
                Field(Datatype::FOUR), // size
                Field(Datatype::TWO), // num_syms_variable
                Field(Datatype::STRUCT, 5), // variable_reloc_symtab
                Field(Datatype::TWO), // num_syms_function
                Field(Datatype::STRUCT, 7), // function_reloc_symtab
        };
        std::vector<RelocationInfo*> variable_reloc_symtab;
        std::vector<RelocationInfo*> function_reloc_symtab;

        //!
        //! \brief      Constructor of Function class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        Function(unsigned version) {
            if (version <= 306) setVersion306();
        }

        //!
        //! \brief      Constructor of Function class.
        //!
        Function() {}

        //!
        //! \brief      Copy Constructor of Function class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Function(const Function& other) {
            fields = other.fields;
            for (RelocationInfo *r : other.variable_reloc_symtab) {
                RelocationInfo *s = new RelocationInfo();
                *s = *r;
                variable_reloc_symtab.push_back(s);
            }
            for (RelocationInfo *r : other.function_reloc_symtab) {
                RelocationInfo *s = new RelocationInfo();
                *s = *r;
                function_reloc_symtab.push_back(s);
            }
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Function& operator= (const Function& other) {
            if (this != &other) {
                fields = other.fields;
                for (RelocationInfo *r : variable_reloc_symtab)
                    delete r;
                for (RelocationInfo *r : other.variable_reloc_symtab) {
                    RelocationInfo *s = new RelocationInfo();
                    *s = *r;
                    variable_reloc_symtab.push_back(s);
                }
                for (RelocationInfo *r : function_reloc_symtab)
                    delete r;
                for (RelocationInfo *r : other.function_reloc_symtab) {
                    RelocationInfo *s = new RelocationInfo();
                    *s = *r;
                    function_reloc_symtab.push_back(s);
                }
            }
            return *this;
        }

        //!
        //! \brief      Destructor of Function class.
        //!
        ~Function() {
            for (RelocationInfo *s : variable_reloc_symtab) delete s;
            for (RelocationInfo *s : function_reloc_symtab) delete s;
        }

        //!
        //! \brief      Returns the integer value of the Linkage field.
        //! \details    Linkage field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getLinkage() {
            return (uint8_t)fields[0].number8;
        }

        //!
        //! \brief      Sets the integer value of the Linkage field.
        //! \details    Linkage field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setLinkage(uint8_t value) {
            fields[0].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields for version <= 306
        //! \retval     An integer.
        //!
        uint8_t getNameLen_Ver306() {
            return (uint8_t)fields[1].number8;
        }

        //!
        //! \brief      Returns the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields
        //! \retval     An integer.
        //!
        uint16_t getNameLen() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields for version <= 306
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameLen_Ver306(uint8_t value) {
            fields[1].number8 = value;
        }

        //!
        //! \brief      Sets the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameLen(uint16_t value)
        {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the string value of the Name field.
        //! \details    Name field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     A pointer to the string (const char*).
        //!
        const char * getName() {
            return fields[2].varchar;
        }

        //!
        //! \brief      Sets the string value of the Name field.
        //! \details    Name field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to string (char*) to be assigned.
        //!
        void setName(char * value) {
            fields[2].varchar = value;
        }

        //!
        //! \brief      Returns the integer value of the Offset field.
        //! \details    Offset field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getOffset() {
            return (uint32_t)fields[3].number32;
        }

        //!
        //! \brief      Sets the integer value of the Offset field.
        //! \details    Offset field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setOffset(uint32_t value) {
            fields[3].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 4 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getSize() {
            return (uint32_t)fields[4].number32;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 4 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint32_t value) {
            fields[4].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumSymsVariable field.
        //! \details    NumSymsVariable field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumSymsVariable() {
            return (uint16_t)fields[5].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumSymsVariable field.
        //! \details    NumSymsVariable field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumSymsVariable(uint16_t value) {
            fields[5].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of RelocationInfo objects.
        //! \details    Function has a vector of RelocationInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of RelocationInfo*.
        //!
        std::vector<RelocationInfo*> &getVariableRelocSymtab() {
            return variable_reloc_symtab;
        }

        //!
        //! \brief      Returns the integer value of the NumSymsFunction field.
        //! \details    NumSymsFunction field is at index 7 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumSymsFunction() {
            return (uint16_t)fields[7].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumSymsFunction field.
        //! \details    NumSymsFunction field is at index 7 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumSymsFunction(uint16_t value) {
            fields[7].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of RelocationInfo objects.
        //! \details    Function has a vector of RelocationInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of RelocationInfo*.
        //!
        std::vector<RelocationInfo*> &getFunctionRelocSymtab() {
            return function_reloc_symtab;
        }

        //!
        //! \brief      Parses one Function object from ISA file.
        //! \details    Reads and parses all the fields of the Function object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one Function object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Function's field", i);
                i++;
            }
            // RelocationInfo
            count = fields[fields[i].countField].number32;
            variable_reloc_symtab.resize(count);
            for (unsigned j = 0; j < count; j++) {
                RelocationInfo *r = new RelocationInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                variable_reloc_symtab[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Function's field", i);
                i++;
            }
            // RelocationInfo
            count = fields[fields[i].countField].number32;
            function_reloc_symtab.resize(count);
            for (unsigned j = 0; j < count; j++) {
                RelocationInfo *r = new RelocationInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                function_reloc_symtab[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the Function's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // RelocationInfo
            for (RelocationInfo *r : variable_reloc_symtab) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // RelocationInfo
            for (RelocationInfo *r : function_reloc_symtab) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

                //!
        //! \brief      Makes the changes needed to support 306 version's Function.
        //! \details    This function is called when the current ISA file has the 306 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport newer versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion306()
        {
            fields[1] = Datatype::ONE;
        }

    };

    //!
    //! \brief      GlobalVariable Class.
    //! \details    This class represents the GlobalVariable from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class GlobalVariable {
    public:
        std::array<Field, 7> fields = std::array<Field, 7>
        {
            Field(Datatype::ONE), // linkage
                Field(Datatype::TWO), // name_len
                Field(Datatype::VARCHAR, 1), // name
                Field(Datatype::ONE), // bit_properties
                Field(Datatype::TWO), // num_elements
                Field(Datatype::ONE), // attribute_count
                Field(Datatype::STRUCT, 5), // attribute_info
        };
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of GlobalVariable class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        GlobalVariable(unsigned version) {}

        //!
        //! \brief      Constructor of GlobalVariable class.
        //!
        GlobalVariable() {}

        //!
        //! \brief      Copy Constructor of GlobalVariable class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        GlobalVariable(const GlobalVariable& other) {
            fields = other.fields;
            for (AttributeInfo *r : other.attribute_info) {
                AttributeInfo *s = new AttributeInfo();
                *s = *r;
                attribute_info.push_back(s);
            }
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        GlobalVariable& operator= (const GlobalVariable& other) {
            if (this != &other) {
                fields = other.fields;
                for (AttributeInfo *r : attribute_info)
                    delete r;
                for (AttributeInfo *r : other.attribute_info) {
                    AttributeInfo *s = new AttributeInfo();
                    *s = *r;
                    attribute_info.push_back(s);
                }
            }
            return *this;
        }

        //!
        //! \brief      Destructor of GlobalVariable class.
        //!
        ~GlobalVariable() {
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the Linkage field.
        //! \details    Linkage field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getLinkage() {
            return (uint8_t)fields[0].number8;
        }

        //!
        //! \brief      Sets the integer value of the Linkage field.
        //! \details    Linkage field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setLinkage(uint8_t value) {
            fields[0].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNameLen() {
            return (uint16_t)fields[1].number16;
        }

        //!
        //! \brief      Sets the integer value of the NameLen field.
        //! \details    NameLen field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameLen(uint16_t value) {
            fields[1].number16 = value;
        }

        //!
        //! \brief      Returns the string value of the Name field.
        //! \details    Name field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     A pointer to the string (const char*).
        //!
        const char * getName() {
            return fields[2].varchar;
        }

        //!
        //! \brief      Sets the string value of the Name field.
        //! \details    Name field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to string (char*) to be assigned.
        //!
        void setName(char * value) {
            fields[2].varchar = value;
        }

        //!
        //! \brief      Returns the integer value of the BitProperties field.
        //! \details    BitProperties field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getBitProperties() {
            return (uint8_t)fields[3].number8;
        }

        //!
        //! \brief      Sets the integer value of the BitProperties field.
        //! \details    BitProperties field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setBitProperties(uint8_t value) {
            fields[3].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumElements field.
        //! \details    NumElements field is at index 4 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumElements() {
            return (uint16_t)fields[4].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumElements field.
        //! \details    NumElements field is at index 4 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumElements(uint16_t value) {
            fields[4].number16 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getAttributeCount() {
            return (uint8_t)fields[5].number8;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint8_t value) {
            fields[5].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    GlobalVariable has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Parses one GlobalVariable object from ISA file.
        //! \details    Reads and parses all the fields of the GlobalVariable object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one GlobalVariable object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for GlobalVariable's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the GlobalVariable's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

    };

    //!
    //! \brief      FunctionBody Class.
    //! \details    This class represents the FunctionBody from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class FunctionBody {
    public:
        std::array<Field, 24> fields = std::array<Field, 24>
        {
            Field(Datatype::FOUR), // string_count
                Field(Datatype::STRUCT, 0), // string_pool
                Field(Datatype::FOUR), // name_index
                Field(Datatype::FOUR), // variable_count
                Field(Datatype::STRUCT, 3), // var_info
                Field(Datatype::TWO), // address_count
                Field(Datatype::STRUCT, 5), // address_info
                Field(Datatype::TWO), // predicate_count
                Field(Datatype::STRUCT, 7), // predicate_info
                Field(Datatype::TWO), // label_count
                Field(Datatype::STRUCT, 9), // label_info
                Field(Datatype::ONE), // sampler_count
                Field(Datatype::STRUCT, 11), // sampler_info
                Field(Datatype::ONE), // surface_count
                Field(Datatype::STRUCT, 13), // surface_info
                Field(Datatype::ONE), // vme_count
                Field(Datatype::STRUCT, 15), // vme_info
                Field(Datatype::FOUR), // size
                Field(Datatype::FOUR), // entry
                Field(Datatype::ONE), // input_size
                Field(Datatype::ONE), // return_value_size
                Field(Datatype::TWO), // attribute_count
                Field(Datatype::STRUCT, 21), // attribute_info
                Field(Datatype::GDATA, 17) // instructions
        };
        std::vector<StringPool*> string_pool;
        std::vector<Variable*> var_info;
        std::vector<AddressInfo*> address_info;
        std::vector<PredicateInfo*> predicate_info;
        std::vector<LabelInfo*> label_info;
        std::vector<SamplerInfo*> sampler_info;
        std::vector<SurfaceInfo*> surface_info;
        std::vector<VmeInfo*> vme_info;
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of FunctionBody class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        FunctionBody(unsigned version) {
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of FunctionBody class.
        //!
        ~FunctionBody() {
            for (StringPool *s : string_pool) delete s;
            for (Variable *s : var_info) delete s;
            for (AddressInfo *s : address_info) delete s;
            for (PredicateInfo *s : predicate_info) delete s;
            for (LabelInfo *s : label_info) delete s;
            for (SamplerInfo *s : sampler_info) delete s;
            for (SurfaceInfo *s : surface_info) delete s;
            for (VmeInfo *s : vme_info) delete s;
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the StringCount field.
        //! \details    StringCount field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getStringCount() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the StringCount field.
        //! \details    StringCount field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setStringCount(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of StringPool objects.
        //! \details    FunctionBody has a vector of StringPool objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of StringPool*.
        //!
        std::vector<StringPool*> &getStringPool() {
            return string_pool;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[2].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[2].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the VariableCount field.
        //! \details    VariableCount field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getVariableCount() {
            return (uint32_t)fields[3].number32;
        }

        //!
        //! \brief      Sets the integer value of the VariableCount field.
        //! \details    VariableCount field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setVariableCount(uint32_t value) {
            fields[3].number32 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of Variable objects.
        //! \details    FunctionBody has a vector of Variable objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of Variable*.
        //!
        std::vector<Variable*> &getVarInfo() {
            return var_info;
        }

        //!
        //! \brief      Returns the integer value of the AddressCount field.
        //! \details    AddressCount field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getAddressCount() {
            return (uint16_t)fields[5].number16;
        }

        //!
        //! \brief      Sets the integer value of the AddressCount field.
        //! \details    AddressCount field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAddressCount(uint16_t value) {
            fields[5].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AddressInfo objects.
        //! \details    FunctionBody has a vector of AddressInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AddressInfo*.
        //!
        std::vector<AddressInfo*> &getAddressInfo() {
            return address_info;
        }

        //!
        //! \brief      Returns the integer value of the PredicateCount field.
        //! \details    PredicateCount field is at index 7 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getPredicateCount() {
            return (uint16_t)fields[7].number16;
        }

        //!
        //! \brief      Sets the integer value of the PredicateCount field.
        //! \details    PredicateCount field is at index 7 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setPredicateCount(uint16_t value) {
            fields[7].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of PredicateInfo objects.
        //! \details    FunctionBody has a vector of PredicateInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of PredicateInfo*.
        //!
        std::vector<PredicateInfo*> &getPredicateInfo() {
            return predicate_info;
        }

        //!
        //! \brief      Returns the integer value of the LabelCount field.
        //! \details    LabelCount field is at index 9 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getLabelCount() {
            return (uint16_t)fields[9].number16;
        }

        //!
        //! \brief      Sets the integer value of the LabelCount field.
        //! \details    LabelCount field is at index 9 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setLabelCount(uint16_t value) {
            fields[9].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of LabelInfo objects.
        //! \details    FunctionBody has a vector of LabelInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of LabelInfo*.
        //!
        std::vector<LabelInfo*> &getLabelInfo() {
            return label_info;
        }

        //!
        //! \brief      Returns the integer value of the SamplerCount field.
        //! \details    SamplerCount field is at index 11 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getSamplerCount() {
            return (uint8_t)fields[11].number8;
        }

        //!
        //! \brief      Sets the integer value of the SamplerCount field.
        //! \details    SamplerCount field is at index 11 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSamplerCount(uint8_t value) {
            fields[11].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of SamplerInfo objects.
        //! \details    FunctionBody has a vector of SamplerInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of SamplerInfo*.
        //!
        std::vector<SamplerInfo*> &getSamplerInfo() {
            return sampler_info;
        }

        //!
        //! \brief      Returns the integer value of the SurfaceCount field.
        //! \details    SurfaceCount field is at index 13 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getSurfaceCount() {
            return (uint8_t)fields[13].number8;
        }

        //!
        //! \brief      Sets the integer value of the SurfaceCount field.
        //! \details    SurfaceCount field is at index 13 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSurfaceCount(uint8_t value) {
            fields[13].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of SurfaceInfo objects.
        //! \details    FunctionBody has a vector of SurfaceInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of SurfaceInfo*.
        //!
        std::vector<SurfaceInfo*> &getSurfaceInfo() {
            return surface_info;
        }

        //!
        //! \brief      Returns the integer value of the VmeCount field.
        //! \details    VmeCount field is at index 15 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getVmeCount() {
            return (uint8_t)fields[15].number8;
        }

        //!
        //! \brief      Sets the integer value of the VmeCount field.
        //! \details    VmeCount field is at index 15 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setVmeCount(uint8_t value) {
            fields[15].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of VmeInfo objects.
        //! \details    FunctionBody has a vector of VmeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of VmeInfo*.
        //!
        std::vector<VmeInfo*> &getVmeInfo() {
            return vme_info;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 17 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getSize() {
            return (uint32_t)fields[17].number32;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 17 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint32_t value) {
            fields[17].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Entry field.
        //! \details    Entry field is at index 18 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getEntry() {
            return (uint32_t)fields[18].number32;
        }

        //!
        //! \brief      Sets the integer value of the Entry field.
        //! \details    Entry field is at index 18 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setEntry(uint32_t value) {
            fields[18].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the InputSize field.
        //! \details    InputSize field is at index 19 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getInputSize() {
            return (uint8_t)fields[19].number8;
        }

        //!
        //! \brief      Sets the integer value of the InputSize field.
        //! \details    InputSize field is at index 19 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setInputSize(uint8_t value) {
            fields[19].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the ReturnValueSize field.
        //! \details    ReturnValueSize field is at index 20 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getReturnValueSize() {
            return (uint8_t)fields[20].number8;
        }

        //!
        //! \brief      Sets the integer value of the ReturnValueSize field.
        //! \details    ReturnValueSize field is at index 20 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setReturnValueSize(uint8_t value) {
            fields[20].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 21 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getAttributeCount() {
            return (uint16_t)fields[21].number16;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 21 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint16_t value) {
            fields[21].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    FunctionBody has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Returns the pointer to buffer data from Instructions field.
        //! \details    Instructions field is at index 23 in the internal
        //!             array of Fields.
        //! \retval     A pointer to buffer data (const uint8_t*).
        //!
        const uint8_t* getInstructions() {
            return fields[23].gdata;
        }

        //!
        //! \brief      Sets the pointer to buffer data of the Instructions field.
        //! \details    Instructions field is at index 23 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to buffer data (uint8_t*) to be assigned.
        //!
        void setInstructions(uint8_t * value) {
            fields[23].gdata = value;
        }

        //!
        //! \brief      Parses one FunctionBody object from ISA file.
        //! \details    Reads and parses all the fields of the FunctionBody object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one FunctionBody object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // StringPool
            count = fields[fields[i].countField].number32;
            string_pool.resize(count);
            for (unsigned j = 0; j < count; j++) {
                StringPool *r = new StringPool(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                string_pool[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // Variable
            count = fields[fields[i].countField].number32;
            var_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                Variable *r = new Variable(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                var_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // AddressInfo
            count = fields[fields[i].countField].number32;
            address_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AddressInfo *r = new AddressInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                address_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // PredicateInfo
            count = fields[fields[i].countField].number32;
            predicate_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                PredicateInfo *r = new PredicateInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                predicate_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // LabelInfo
            count = fields[fields[i].countField].number32;
            label_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                LabelInfo *r = new LabelInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                label_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // SamplerInfo
            count = fields[fields[i].countField].number32;
            sampler_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                SamplerInfo *r = new SamplerInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                sampler_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // SurfaceInfo
            count = fields[fields[i].countField].number32;
            surface_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                SurfaceInfo *r = new SurfaceInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                surface_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // VmeInfo
            count = fields[fields[i].countField].number32;
            vme_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                VmeInfo *r = new VmeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                vme_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for FunctionBody's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the FunctionBody's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // StringPool
            for (StringPool *r : string_pool) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // Variable
            for (Variable *r : var_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AddressInfo
            for (AddressInfo *r : address_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // PredicateInfo
            for (PredicateInfo *r : predicate_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // LabelInfo
            for (LabelInfo *r : label_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // SamplerInfo
            for (SamplerInfo *r : sampler_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // SurfaceInfo
            for (SurfaceInfo *r : surface_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // VmeInfo
            for (VmeInfo *r : vme_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's FunctionBody.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
            fields[2] = Datatype::TWO;
            fields[3] = Datatype::TWO;
        }
    };

    //!
    //! \brief      KernelBody Class.
    //! \details    This class represents the KernelBody from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class KernelBody {
    public:
        std::array<Field, 24> fields = std::array<Field, 24>
        {
            Field(Datatype::FOUR), // string_count
                Field(Datatype::STRUCT, 0), // string_pool
                Field(Datatype::FOUR), // name_index
                Field(Datatype::FOUR), // variable_count
                Field(Datatype::STRUCT, 3), // var_info
                Field(Datatype::TWO), // address_count
                Field(Datatype::STRUCT, 5), // address_info
                Field(Datatype::TWO), // predicate_count
                Field(Datatype::STRUCT, 7), // predicate_info
                Field(Datatype::TWO), // label_count
                Field(Datatype::STRUCT, 9), // label_info
                Field(Datatype::ONE), // sampler_count
                Field(Datatype::STRUCT, 11), // sampler_info
                Field(Datatype::ONE), // surface_count
                Field(Datatype::STRUCT, 13), // surface_info
                Field(Datatype::ONE), // vme_count
                Field(Datatype::STRUCT, 15), // vme_info
                Field(Datatype::FOUR), // num_inputs
                Field(Datatype::STRUCT, 17), // input_info
                Field(Datatype::FOUR), // size
                Field(Datatype::FOUR), // entry
                Field(Datatype::TWO), // attribute_count
                Field(Datatype::STRUCT, 21), // attribute_info
                Field(Datatype::GDATA, 19) // instructions
        };
        std::vector<StringPool*> string_pool;
        std::vector<Variable*> var_info;
        std::vector<AddressInfo*> address_info;
        std::vector<PredicateInfo*> predicate_info;
        std::vector<LabelInfo*> label_info;
        std::vector<SamplerInfo*> sampler_info;
        std::vector<SurfaceInfo*> surface_info;
        std::vector<VmeInfo*> vme_info;
        std::vector<InputInfo*> input_info;
        std::vector<AttributeInfo*> attribute_info;

        //!
        //! \brief      Constructor of KernelBody class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        KernelBody(unsigned version) {
            if (version <= 304) setVersion304();
            if (version <= 303) setVersion303();
        }

        //!
        //! \brief      Destructor of KernelBody class.
        //!
        ~KernelBody() {
            for (StringPool *s : string_pool) delete s;
            for (Variable *s : var_info) delete s;
            for (AddressInfo *s : address_info) delete s;
            for (PredicateInfo *s : predicate_info) delete s;
            for (LabelInfo *s : label_info) delete s;
            for (SamplerInfo *s : sampler_info) delete s;
            for (SurfaceInfo *s : surface_info) delete s;
            for (VmeInfo *s : vme_info) delete s;
            for (InputInfo *s : input_info) delete s;
            for (AttributeInfo *s : attribute_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the StringCount field.
        //! \details    StringCount field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getStringCount() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the StringCount field.
        //! \details    StringCount field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setStringCount(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of StringPool objects.
        //! \details    KernelBody has a vector of StringPool objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of StringPool*.
        //!
        std::vector<StringPool*> &getStringPool() {
            return string_pool;
        }

        //!
        //! \brief      Returns the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNameIndex() {
            return (uint32_t)fields[2].number32;
        }

        //!
        //! \brief      Sets the integer value of the NameIndex field.
        //! \details    NameIndex field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameIndex(uint32_t value) {
            fields[2].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the VariableCount field.
        //! \details    VariableCount field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getVariableCount() {
            return (uint32_t)fields[3].number32;
        }

        //!
        //! \brief      Sets the integer value of the VariableCount field.
        //! \details    VariableCount field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setVariableCount(uint32_t value) {
            fields[3].number32 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of Variable objects.
        //! \details    KernelBody has a vector of Variable objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of Variable*.
        //!
        std::vector<Variable*> &getVarInfo() {
            return var_info;
        }

        //!
        //! \brief      Returns the integer value of the AddressCount field.
        //! \details    AddressCount field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getAddressCount() {
            return (uint16_t)fields[5].number16;
        }

        //!
        //! \brief      Sets the integer value of the AddressCount field.
        //! \details    AddressCount field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAddressCount(uint16_t value) {
            fields[5].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AddressInfo objects.
        //! \details    KernelBody has a vector of AddressInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AddressInfo*.
        //!
        std::vector<AddressInfo*> &getAddressInfo() {
            return address_info;
        }

        //!
        //! \brief      Returns the integer value of the PredicateCount field.
        //! \details    PredicateCount field is at index 7 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getPredicateCount() {
            return (uint16_t)fields[7].number16;
        }

        //!
        //! \brief      Sets the integer value of the PredicateCount field.
        //! \details    PredicateCount field is at index 7 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setPredicateCount(uint16_t value) {
            fields[7].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of PredicateInfo objects.
        //! \details    KernelBody has a vector of PredicateInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of PredicateInfo*.
        //!
        std::vector<PredicateInfo*> &getPredicateInfo() {
            return predicate_info;
        }

        //!
        //! \brief      Returns the integer value of the LabelCount field.
        //! \details    LabelCount field is at index 9 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getLabelCount() {
            return (uint16_t)fields[9].number16;
        }

        //!
        //! \brief      Sets the integer value of the LabelCount field.
        //! \details    LabelCount field is at index 9 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setLabelCount(uint16_t value) {
            fields[9].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of LabelInfo objects.
        //! \details    KernelBody has a vector of LabelInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of LabelInfo*.
        //!
        std::vector<LabelInfo*> &getLabelInfo() {
            return label_info;
        }

        //!
        //! \brief      Returns the integer value of the SamplerCount field.
        //! \details    SamplerCount field is at index 11 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getSamplerCount() {
            return (uint8_t)fields[11].number8;
        }

        //!
        //! \brief      Sets the integer value of the SamplerCount field.
        //! \details    SamplerCount field is at index 11 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSamplerCount(uint8_t value) {
            fields[11].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of SamplerInfo objects.
        //! \details    KernelBody has a vector of SamplerInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of SamplerInfo*.
        //!
        std::vector<SamplerInfo*> &getSamplerInfo() {
            return sampler_info;
        }

        //!
        //! \brief      Returns the integer value of the SurfaceCount field.
        //! \details    SurfaceCount field is at index 13 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getSurfaceCount() {
            return (uint8_t)fields[13].number8;
        }

        //!
        //! \brief      Sets the integer value of the SurfaceCount field.
        //! \details    SurfaceCount field is at index 13 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSurfaceCount(uint8_t value) {
            fields[13].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of SurfaceInfo objects.
        //! \details    KernelBody has a vector of SurfaceInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of SurfaceInfo*.
        //!
        std::vector<SurfaceInfo*> &getSurfaceInfo() {
            return surface_info;
        }

        //!
        //! \brief      Returns the integer value of the VmeCount field.
        //! \details    VmeCount field is at index 15 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getVmeCount() {
            return (uint8_t)fields[15].number8;
        }

        //!
        //! \brief      Sets the integer value of the VmeCount field.
        //! \details    VmeCount field is at index 15 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setVmeCount(uint8_t value) {
            fields[15].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of VmeInfo objects.
        //! \details    KernelBody has a vector of VmeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of VmeInfo*.
        //!
        std::vector<VmeInfo*> &getVmeInfo() {
            return vme_info;
        }

        //!
        //! \brief      Returns the integer value of the NumInputs field.
        //! \details    NumInputs field is at index 17 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getNumInputs() {
            return (uint32_t)fields[17].number32;
        }

        //!
        //! \brief      Sets the integer value of the NumInputs field.
        //! \details    NumInputs field is at index 17 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumInputs(uint32_t value) {
            fields[17].number32 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of InputInfo objects.
        //! \details    KernelBody has a vector of InputInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of InputInfo*.
        //!
        std::vector<InputInfo*> &getInputInfo() {
            return input_info;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 19 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getSize() {
            return (uint32_t)fields[19].number32;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 19 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint32_t value) {
            fields[19].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Entry field.
        //! \details    Entry field is at index 20 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getEntry() {
            return (uint32_t)fields[20].number32;
        }

        //!
        //! \brief      Sets the integer value of the Entry field.
        //! \details    Entry field is at index 20 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setEntry(uint32_t value) {
            fields[20].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 21 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getAttributeCount() {
            return (uint16_t)fields[21].number16;
        }

        //!
        //! \brief      Sets the integer value of the AttributeCount field.
        //! \details    AttributeCount field is at index 21 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setAttributeCount(uint16_t value) {
            fields[21].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of AttributeInfo objects.
        //! \details    KernelBody has a vector of AttributeInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of AttributeInfo*.
        //!
        std::vector<AttributeInfo*> &getAttributeInfo() {
            return attribute_info;
        }

        //!
        //! \brief      Returns the pointer to buffer data from Instructions field.
        //! \details    Instructions field is at index 23 in the internal
        //!             array of Fields.
        //! \retval     A pointer to buffer data (const uint8_t*).
        //!
        const uint8_t* getInstructions() {
            return fields[23].gdata;
        }

        //!
        //! \brief      Sets the pointer to buffer data of the Instructions field.
        //! \details    Instructions field is at index 23 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to buffer data (uint8_t*) to be assigned.
        //!
        void setInstructions(uint8_t * value) {
            fields[23].gdata = value;
        }

        //!
        //! \brief      Parses one KernelBody object from ISA file.
        //! \details    Reads and parses all the fields of the KernelBody object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one KernelBody object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // StringPool
            count = fields[fields[i].countField].number32;
            string_pool.resize(count);
            for (unsigned j = 0; j < count; j++) {
                StringPool *r = new StringPool(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                string_pool[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // Variable
            count = fields[fields[i].countField].number32;
            var_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                Variable *r = new Variable(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                var_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // AddressInfo
            count = fields[fields[i].countField].number32;
            address_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AddressInfo *r = new AddressInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                address_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // PredicateInfo
            count = fields[fields[i].countField].number32;
            predicate_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                PredicateInfo *r = new PredicateInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                predicate_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // LabelInfo
            count = fields[fields[i].countField].number32;
            label_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                LabelInfo *r = new LabelInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                label_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // SamplerInfo
            count = fields[fields[i].countField].number32;
            sampler_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                SamplerInfo *r = new SamplerInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                sampler_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // SurfaceInfo
            count = fields[fields[i].countField].number32;
            surface_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                SurfaceInfo *r = new SurfaceInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                surface_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // VmeInfo
            count = fields[fields[i].countField].number32;
            vme_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                VmeInfo *r = new VmeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                vme_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // InputInfo
            count = fields[fields[i].countField].number32;
            input_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                InputInfo *r = new InputInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                input_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            // AttributeInfo
            count = fields[fields[i].countField].number32;
            attribute_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                AttributeInfo *r = new AttributeInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                attribute_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for KernelBody's field", i);
                i++;
            }
            return p;
        }

        //!
        //! \brief      Adds all the KernelBody's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // StringPool
            for (StringPool *r : string_pool) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // Variable
            for (Variable *r : var_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AddressInfo
            for (AddressInfo *r : address_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // PredicateInfo
            for (PredicateInfo *r : predicate_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // LabelInfo
            for (LabelInfo *r : label_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // SamplerInfo
            for (SamplerInfo *r : sampler_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // SurfaceInfo
            for (SurfaceInfo *r : surface_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // VmeInfo
            for (VmeInfo *r : vme_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // InputInfo
            for (InputInfo *r : input_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // AttributeInfo
            for (AttributeInfo *r : attribute_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
        }

        //!
        //! \brief      Makes the changes needed to support 303 version's KernelBody.
        //! \details    This function is called when the current ISA file has the 303 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion303() {
            fields[0] = Datatype::TWO;
            fields[2] = Datatype::TWO;
            fields[3] = Datatype::TWO;
        }

        //!
        //! \brief      Makes the changes needed to support 304 version's KernelBody.
        //! \details    This function is called when the current ISA file has the 304 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport previous versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion304() {
            fields[17] = Datatype::ONE;
        }
    };

    //!
    //! \brief      Kernel Class.
    //! \details    This class represents the Kernel from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class Kernel {
    public:
        std::array<Field, 11> fields = std::array<Field, 11>
        {
            Field(Datatype::TWO), // name_len
                Field(Datatype::VARCHAR, 0), // name
                Field(Datatype::FOUR), // offset
                Field(Datatype::FOUR), // size
                Field(Datatype::FOUR), // input_offset
                Field(Datatype::TWO), // num_syms_variable
                Field(Datatype::STRUCT, 5), // variable_reloc_symtab
                Field(Datatype::TWO), // num_syms_function
                Field(Datatype::STRUCT, 7), // function_reloc_symtab
                Field(Datatype::ONE), // num_gen_binaries
                Field(Datatype::STRUCT, 9), // gen_binary_info
        };
        std::vector<RelocationInfo*> variable_reloc_symtab;
        std::vector<RelocationInfo*> function_reloc_symtab;
        std::vector<GenBinary*> gen_binary_info;

        //!
        //! \brief      Constructor of Kernel class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        Kernel(unsigned version) {
            if (version <= 306) setVersion306();
        }

        //!
        //! \brief      Constructor of Kernel class.
        //!
        Kernel() {}

        //!
        //! \brief      Copy Constructor of Kernel class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Kernel(const Kernel& other) {
            fields = other.fields;
            for (RelocationInfo *r : other.variable_reloc_symtab) {
                RelocationInfo *s = new RelocationInfo();
                *s = *r;
                variable_reloc_symtab.push_back(s);
            }
            for (RelocationInfo *r : other.function_reloc_symtab) {
                RelocationInfo *s = new RelocationInfo();
                *s = *r;
                function_reloc_symtab.push_back(s);
            }
            for (GenBinary *r : other.gen_binary_info) {
                GenBinary *s = new GenBinary();
                *s = *r;
                gen_binary_info.push_back(s);
            }
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Kernel& operator= (const Kernel& other) {
            if (this != &other) {
                fields = other.fields;
                for (RelocationInfo *r : variable_reloc_symtab)
                    delete r;
                for (RelocationInfo *r : other.variable_reloc_symtab) {
                    RelocationInfo *s = new RelocationInfo();
                    *s = *r;
                    variable_reloc_symtab.push_back(s);
                }
                for (RelocationInfo *r : function_reloc_symtab)
                    delete r;
                for (RelocationInfo *r : other.function_reloc_symtab) {
                    RelocationInfo *s = new RelocationInfo();
                    *s = *r;
                    function_reloc_symtab.push_back(s);
                }
                for (GenBinary *r : gen_binary_info)
                    delete r;
                for (GenBinary *r : other.gen_binary_info) {
                    GenBinary *s = new GenBinary();
                    *s = *r;
                    gen_binary_info.push_back(s);
                }
            }
            return *this;
        }

        //!
        //! \brief      Destructor of Kernel class.
        //!
        ~Kernel() {
            for (RelocationInfo *s : variable_reloc_symtab) delete s;
            for (RelocationInfo *s : function_reloc_symtab) delete s;
            for (GenBinary *s : gen_binary_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the NameLen field.
        //! \details    NameLen field is at index 0 in the internal
        //!             array of Fields for vesrion <= 306
        //! \retval     An integer.
        //!
        uint8_t getNameLen_Ver306() {
            return (uint8_t)fields[0].number8;
        }

        //!
        //! \brief      Returns the integer value of the NameLen field.
        //! \details    NameLen field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNameLen()
        {
            return (uint16_t)fields[0].number16;
        }

        //!
        //! \brief      Sets the integer value of the NameLen field.
        //! \details    NameLen field is at index 0 in the internal
        //!             array of Fields for version <= 306
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameLen_Ver306(uint8_t value) {
            fields[0].number8 = value;
        }

        //!
        //! \brief      Sets the integer value of the NameLen field.
        //! \details    NameLen field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNameLen(uint16_t value)
        {
            fields[0].number16 = value;
        }

        //!
        //! \brief      Returns the string value of the Name field.
        //! \details    Name field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     A pointer to the string (const char*).
        //!
        const char * getName() {
            return fields[1].varchar;
        }

        //!
        //! \brief      Sets the string value of the Name field.
        //! \details    Name field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Pointer to string (char*) to be assigned.
        //!
        void setName(char * value) {
            fields[1].varchar = value;
        }

        //!
        //! \brief      Returns the integer value of the Offset field.
        //! \details    Offset field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getOffset() {
            return (uint32_t)fields[2].number32;
        }

        //!
        //! \brief      Sets the integer value of the Offset field.
        //! \details    Offset field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setOffset(uint32_t value) {
            fields[2].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the Size field.
        //! \details    Size field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getSize() {
            return (uint32_t)fields[3].number32;
        }

        //!
        //! \brief      Sets the integer value of the Size field.
        //! \details    Size field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setSize(uint32_t value) {
            fields[3].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the InputOffset field.
        //! \details    InputOffset field is at index 4 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getInputOffset() {
            return (uint32_t)fields[4].number32;
        }

        //!
        //! \brief      Sets the integer value of the InputOffset field.
        //! \details    InputOffset field is at index 4 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setInputOffset(uint32_t value) {
            fields[4].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumSymsVariable field.
        //! \details    NumSymsVariable field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumSymsVariable() {
            return (uint16_t)fields[5].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumSymsVariable field.
        //! \details    NumSymsVariable field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumSymsVariable(uint16_t value) {
            fields[5].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of RelocationInfo objects.
        //! \details    Kernel has a vector of RelocationInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of RelocationInfo*.
        //!
        std::vector<RelocationInfo*> &getVariableRelocSymtab() {
            return variable_reloc_symtab;
        }

        //!
        //! \brief      Returns the integer value of the NumSymsFunction field.
        //! \details    NumSymsFunction field is at index 7 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumSymsFunction() {
            return (uint16_t)fields[7].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumSymsFunction field.
        //! \details    NumSymsFunction field is at index 7 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumSymsFunction(uint16_t value) {
            fields[7].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of RelocationInfo objects.
        //! \details    Kernel has a vector of RelocationInfo objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of RelocationInfo*.
        //!
        std::vector<RelocationInfo*> &getFunctionRelocSymtab() {
            return function_reloc_symtab;
        }

        //!
        //! \brief      Returns the integer value of the NumGenBinaries field.
        //! \details    NumGenBinaries field is at index 9 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getNumGenBinaries() {
            return (uint8_t)fields[9].number8;
        }

        //!
        //! \brief      Sets the integer value of the NumGenBinaries field.
        //! \details    NumGenBinaries field is at index 9 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumGenBinaries(uint8_t value) {
            fields[9].number8 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of GenBinary objects.
        //! \details    Kernel has a vector of GenBinary objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of GenBinary*.
        //!
        std::vector<GenBinary*> &getGenBinaryInfo() {
            return gen_binary_info;
        }

        //!
        //! \brief      Parses one Kernel object from ISA file.
        //! \details    Reads and parses all the fields of the Kernel object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one Kernel object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Kernel's field", i);
                i++;
            }
            // RelocationInfo
            count = fields[fields[i].countField].number32;
            variable_reloc_symtab.resize(count);
            for (unsigned j = 0; j < count; j++) {
                RelocationInfo *r = new RelocationInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                variable_reloc_symtab[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Kernel's field", i);
                i++;
            }
            // RelocationInfo
            count = fields[fields[i].countField].number32;
            function_reloc_symtab.resize(count);
            for (unsigned j = 0; j < count; j++) {
                RelocationInfo *r = new RelocationInfo(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                function_reloc_symtab[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Kernel's field", i);
                i++;
            }
            // GenBinary
            count = fields[fields[i].countField].number32;
            gen_binary_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                GenBinary *r = new GenBinary(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                gen_binary_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the Kernel's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // RelocationInfo
            for (RelocationInfo *r : variable_reloc_symtab) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // RelocationInfo
            for (RelocationInfo *r : function_reloc_symtab) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // GenBinary
            for (GenBinary *r : gen_binary_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

        //!
        //! \brief      Makes the changes needed to support 306 version's Kernel.
        //! \details    This function is called when the current ISA file has the 306 version.
        //!             Initially all the objects are created with last version's format, so
        //!             in order to suppport newer versions, changes of datatypes and insertion/removal
        //!             of fields can be needed.
        //!
        void setVersion306()
        {
            fields[0] = Datatype::ONE;
        }

    };

    //!
    //! \brief      Header Class.
    //! \details    This class represents the Header from vISA Object Format.
    //!             Provides getters, setters for its fields and structs as well as 
    //!             functions to read/write it from/to file.
    //!
    class Header {
    public:
        std::array<Field, 9> fields = std::array<Field, 9>
        {
            Field(Datatype::FOUR), // magic_number
                Field(Datatype::ONE), // major_version
                Field(Datatype::ONE), // minor_version
                Field(Datatype::TWO), // num_kernels
                Field(Datatype::STRUCT, 3), // kernel_info
                Field(Datatype::TWO), // num_variables
                Field(Datatype::STRUCT, 5), // file_scope_var_info
                Field(Datatype::TWO), // num_functions
                Field(Datatype::STRUCT, 7), // function_info
        };
        std::vector<Kernel*> kernel_info;
        std::vector<GlobalVariable*> file_scope_var_info;
        std::vector<Function*> function_info;

        //!
        //! \brief      Constructor of Header class.
        //! \param      [in] version.
        //!             Version of current ISA file.
        //!
        Header(unsigned version) {}

        //!
        //! \brief      Copy Constructor of Header class.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Header(const Header& other) {
            fields = other.fields;
            for (Kernel *r : other.kernel_info) {
                Kernel *s = new Kernel();
                *s = *r;
                kernel_info.push_back(s);
            }
            for (GlobalVariable *r : other.file_scope_var_info) {
                GlobalVariable *s = new GlobalVariable();
                *s = *r;
                file_scope_var_info.push_back(s);
            }
            for (Function *r : other.function_info) {
                Function *s = new Function();
                *s = *r;
                function_info.push_back(s);
            }
        }

        //!
        //! \brief      Assignment operator.
        //! \param      [in] other.
        //!             Reference to object to copy..
        //!
        Header& operator= (const Header& other) {
            if (this != &other) {
                fields = other.fields;
                for (Kernel *r : kernel_info)
                    delete r;
                for (Kernel *r : other.kernel_info) {
                    Kernel *s = new Kernel();
                    *s = *r;
                    kernel_info.push_back(s);
                }
                for (GlobalVariable *r : file_scope_var_info)
                    delete r;
                for (GlobalVariable *r : other.file_scope_var_info) {
                    GlobalVariable *s = new GlobalVariable();
                    *s = *r;
                    file_scope_var_info.push_back(s);
                }
                for (Function *r : function_info)
                    delete r;
                for (Function *r : other.function_info) {
                    Function *s = new Function();
                    *s = *r;
                    function_info.push_back(s);
                }
            }
            return *this;
        }

        //!
        //! \brief      Destructor of Header class.
        //!
        ~Header() {
            for (Kernel *s : kernel_info) delete s;
            for (GlobalVariable *s : file_scope_var_info) delete s;
            for (Function *s : function_info) delete s;
        }

        //!
        //! \brief      Returns the integer value of the MagicNumber field.
        //! \details    MagicNumber field is at index 0 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint32_t getMagicNumber() {
            return (uint32_t)fields[0].number32;
        }

        //!
        //! \brief      Sets the integer value of the MagicNumber field.
        //! \details    MagicNumber field is at index 0 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setMagicNumber(uint32_t value) {
            fields[0].number32 = value;
        }

        //!
        //! \brief      Returns the integer value of the MajorVersion field.
        //! \details    MajorVersion field is at index 1 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getMajorVersion() {
            return (uint8_t)fields[1].number8;
        }

        //!
        //! \brief      Sets the integer value of the MajorVersion field.
        //! \details    MajorVersion field is at index 1 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setMajorVersion(uint8_t value) {
            fields[1].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the MinorVersion field.
        //! \details    MinorVersion field is at index 2 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint8_t getMinorVersion() {
            return (uint8_t)fields[2].number8;
        }

        //!
        //! \brief      Sets the integer value of the MinorVersion field.
        //! \details    MinorVersion field is at index 2 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setMinorVersion(uint8_t value) {
            fields[2].number8 = value;
        }

        //!
        //! \brief      Returns the integer value of the NumKernels field.
        //! \details    NumKernels field is at index 3 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumKernels() {
            return (uint16_t)fields[3].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumKernels field.
        //! \details    NumKernels field is at index 3 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumKernels(uint16_t value) {
            fields[3].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of Kernel objects.
        //! \details    Header has a vector of Kernel objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of Kernel*.
        //!
        std::vector<Kernel*> &getKernelInfo() {
            return kernel_info;
        }

        //!
        //! \brief      Returns the integer value of the NumVariables field.
        //! \details    NumVariables field is at index 5 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumVariables() {
            return (uint16_t)fields[5].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumVariables field.
        //! \details    NumVariables field is at index 5 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumVariables(uint16_t value) {
            fields[5].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of GlobalVariable objects.
        //! \details    Header has a vector of GlobalVariable objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of GlobalVariable*.
        //!
        std::vector<GlobalVariable*> &getFileScopeVarInfo() {
            return file_scope_var_info;
        }

        //!
        //! \brief      Returns the integer value of the NumFunctions field.
        //! \details    NumFunctions field is at index 7 in the internal
        //!             array of Fields.
        //! \retval     An integer.
        //!
        uint16_t getNumFunctions() {
            return (uint16_t)fields[7].number16;
        }

        //!
        //! \brief      Sets the integer value of the NumFunctions field.
        //! \details    NumFunctions field is at index 7 in the internal
        //!             array of Fields.
        //! \param      [in] value.
        //!             Integer be assigned.
        //!
        void setNumFunctions(uint16_t value) {
            fields[7].number16 = value;
        }

        //!
        //! \brief      Returns the reference to the vector of Function objects.
        //! \details    Header has a vector of Function objects
        //!             that represents another entity within the vISA object format. 
        //! \retval     Reference to the vector of Function*.
        //!
        std::vector<Function*> &getFunctionInfo() {
            return function_info;
        }

        //!
        //! \brief      Parses one Header object from ISA file.
        //! \details    Reads and parses all the fields of the Header object
        //!             from the file buffer and returns the pointer immediately
        //!             after all this data.
        //!             If this class contains internal structs, their Parse
        //!             functions are called when corresponding.
        //! \param      [in] p.
        //!             Pointer to file buffer to start reading.
        //! \param      [in] end.
        //!             Pointer to end of file buffer.
        //! \param      [in] m.
        //!             Pointer ISAfile object.
        //! \retval     Pointer to file buffe after parsing one Header object.
        //!
        const uint8_t* parse(const uint8_t *p, const uint8_t *end, ISAfile *m) {
            unsigned i = 0, count = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Header's field", i);
                i++;
            }
            m->setCurrentVISAVersion(getMajorVersion() * 100 + getMinorVersion());
            // Kernel
            count = fields[fields[i].countField].number32;
            kernel_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                Kernel *r = new Kernel(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                kernel_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Header's field", i);
                i++;
            }
            // GlobalVariable
            count = fields[fields[i].countField].number32;
            file_scope_var_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                GlobalVariable *r = new GlobalVariable(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                file_scope_var_info[j] = r;
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                p = m->readField(p, end, fields[i], fields[fields[i].countField].number32);
                if (!p) return m->setError("bad offset/size for Header's field", i);
                i++;
            }
            // Function
            count = fields[fields[i].countField].number32;
            function_info.resize(count);
            for (unsigned j = 0; j < count; j++) {
                Function *r = new Function(m->getCurrentVISAVersion());
                p = r->parse(p, end, m);
                if (!p) {
                    delete r;
                    return 0;
                }
                function_info[j] = r;
            }
            i++;
            return p;
        }

        //!
        //! \brief      Adds all the Header's fields to a buffer.
        //! \details    Every field from this class is added to a buffer
        //!             in order to be written into an ISA file afterwards
        //!             If this class contains other internal structus, their addToBuffer
        //!             functions are called when corresponding.
        //! \param      [out] buffer.
        //!             The buffer where the fields data will be added.
        //! \param      [in] m.
        //!             Pointer to ISAfile object.
        //!
        void addToBuffer(std::vector<char> &buffer, ISAfile *m) {
            unsigned i = 0;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // Kernel
            for (Kernel *r : kernel_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // GlobalVariable
            for (GlobalVariable *r : file_scope_var_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
            while (i < fields.size() && fields[i].type != Datatype::STRUCT) {
                m->addToBuffer(fields[i], buffer);
                i++;
            }
            // Function
            for (Function *r : function_info) {
                r->addToBuffer(buffer, m);
            }
            i++;
        }

    };

};

#endif //VISA_H
