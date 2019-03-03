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
//! \file     media_libva_caps_factory.h
//! \brief    Defines one template class to create the instance of DDI MediaLibvaCaps 
//!

#ifndef _MEDIA_LIBVA_CAPS_FACTORY_H_
#define _MEDIA_LIBVA_CAPS_FACTORY_H_

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <cstdarg>

//!
//! \class  MediaLibvaCapsFactory
//! \brief  Media libva caps factory
//!
template <class T, class Arg>
class MediaLibvaCapsFactory
{
public:
    typedef T *Type;
    typedef Arg *ArgType;
    typedef Type (*Creator)(ArgType);
    typedef uint32_t KeyType;
    typedef std::map<KeyType, Creator>  Creators;
    typedef typename Creators::iterator iterator;

    //!
    //! \brief    Register one Class C with key.
    //! \details  Use the member template to register class C with key and C is the
    //!           derived class of base class T.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \return   True is returned if class C is successfully registerted with key
    //!           False is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool RegisterCaps(KeyType key)
    {
        std::pair<iterator, bool> result =
            GetCreators().insert(std::make_pair(key, create<C>));

        return result.second;
    }

    //!
    //! \brief    Create a new object that is registered with key.
    //! \details  Create and return one new object that is registered with key. And Args is passed to create
    //!           the new object.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \param    [in] arg
    //!           ArgType, the type alias of Arg template parameter
    //!
    //! \return   The derived object of T is returned if key is found and the object is created.
    //!           nullptr is returned if key is not found
    //!
    static Type CreateCaps(
        KeyType key,
        ArgType        arg)
    {
        Creators &creators = GetCreators();
        iterator  creator  = creators.find(key);
        if (creator != creators.end())
            return (creator->second)(arg);

        return nullptr;
    }

private:
    //!
    //! \brief    The callback function with key.
    //! \details  The member template to create the derived object
    //!
    //! \param    [in] arg
    //!           ArgType, the type alias of class template parameter.
    //!
    //! \return   The created object with arg input for C constructor.
    //!
    template <class C>
    static Type create(ArgType arg)
    {
        return MOS_New(C, arg);
    }

    //!
    //! \brief    Obtain the static pair table of key and callback function
    //! \details  Obtain the static pair table that is registerted with key and callback function
    //!
    //! \return   Return the static pair table about the param @key and callback function
    //!
    static Creators &GetCreators()
    {
        static Creators creators;

        return creators;
    }
};

#endif /*  _MEDIA_DDI_FACTORY_H_ */
