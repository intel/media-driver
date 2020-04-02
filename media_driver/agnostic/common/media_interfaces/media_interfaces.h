/*
* Copyright (c) 2013-2019, Intel Corporation
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
//! \file     media_interfaces.h
//! \brief    Helps with gen-specific factory creation.
//!

#ifndef __MEDIA_INTERFACES_H__
#define __MEDIA_INTERFACES_H__

#include <map>
#include "mos_utilities.h"

//!
//! \class    MediaInterfacesFactory
//! \brief    Media interfaces factory
//!
template <class T>
class MediaInterfacesFactory
{
public:
    typedef T *Type;
    typedef uint32_t KeyType;
    typedef Type (*Creator)();
    typedef std::map<KeyType, Creator>  Creators;
    typedef typename Creators::iterator Iterator;

    //!
    //! \brief    register one Class C with key.
    //! \details  Use the member template to register class C with key and C is the
    //!           derived class of base class T.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of uint32_t.
    //!
    //! \param    [in] forceReplace
    //!           if force to replace the exsiting Creator, default is false.
    //!
    //! \return   true is returned if class C is successfully registerted with key
    //!           false is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool RegisterHal(KeyType key, bool forceReplace = false)
    {
        Creators &creators = GetCreators();
        Iterator creator = creators.find(key);
        if (creator == creators.end())
        {
            std::pair<Iterator, bool> result =
                GetCreators().insert(std::make_pair(key, Create<C>));
            return result.second;
        }
        else
        {
            if (forceReplace)
            {
                creators.erase(creator);
                std::pair<Iterator, bool> result =
                    GetCreators().insert(std::make_pair(key, Create<C>));
                return result.second;
            }
            return true;
        }
    }

    //!
    //! \brief    create a new object that is registered with key.
    //! \details  create and return one new object that is registered with key. And Args is passed to create
    //!           the new object.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of uint32_t.
    //!
    //! \return   the derived object of T is returned if key is found and the object is created.
    //!           nullptr is returned if key is not found
    //!
    static Type CreateHal(
        KeyType key)
    {
        Creators &creators = GetCreators();
        Iterator creator = creators.find(key);
        if (creator != creators.end())
        {
            return (creator->second)();
        }

        return nullptr;
    }

private:

    //!
    //! \brief    the callback function with key.
    //! \details  The member template to create the derived object
    //!
    //! \param    [in] arg
    //!           ArgType, the type alias of class template parameter.
    //!
    //! \return   the created object with arg input for C constructor.
    //!
    template <class C>
    static Type Create()
    {
        return MOS_New(C);
    }


    //!
    //! \brief    obtain the static pair table of param@ key and callback function
    //! \details  obtain the static pair table that is registerted with param@key and callback function
    //!
    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;

        return creators;
    }
};

#define MEDIA_EXT_FLAG  0x10000000

#endif // __MEDIA_INTERFACES_H__
