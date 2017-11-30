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
//! \file     media_ddi_factory.h
//! \brief    Defines one template class to create the instance of DDI media encode/decoder.
//!

#ifndef _MEDIA_DDI_FACTORY_H_
#define _MEDIA_DDI_FACTORY_H_

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <cstdarg>

template <class T, class Arg>
class MediaDdiFactory
{
public:
    typedef T *Type;
    typedef Arg *ArgType;
    typedef Type (*Creator)(ArgType);
    typedef std::string                 KeyType;
    typedef std::map<KeyType, Creator>  Creators;
    typedef typename Creators::iterator iterator;

    //!
    //! \brief    register one Class C with key.
    //! \details  Use the member template to register class C with key and C is the
    //!           derived class of base class T.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \return   true is returned if class C is successfully registerted with key
    //!           false is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool RegisterCodec(const KeyType &key)
    {
        std::pair<iterator, bool> result =
            GetCreators().insert(std::make_pair(key, create<C>));

        return result.second;
    }

    //!
    //! \brief    create a new object that is registered with key.
    //! \details  create and return one new object that is registered with key. And Args is passed to create
    //!           the new object.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \param    [in] arg
    //!           ArgType, the type alias of Arg template parameter

    //! \return   the derived object of T is returned if key is found and the object is created.
    //!           nullptr is returned if key is not found
    //!
    static Type CreateCodec(
        const KeyType &key,
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
    //! \brief    the callback function with key.
    //! \details  The member template to create the derived object
    //!
    //! \param    [in] arg
    //!           ArgType, the type alias of class template parameter.
    //!
    //! \return   the created object with arg input for C constructor.
    //!
    template <class C>
    static Type create(ArgType arg)
    {
        return MOS_New(C, arg);
    }

    //!
    //! \brief    obtain the static pair table of key and callback function
    //! \details  obtain the static pair table that is registerted with key and callback function
    //!

    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;

        return creators;
    }
};

template <class T>
class MediaDdiFactoryNoArg
{
public:
    typedef T *Type;
    typedef Type (*Creator)();
    typedef std::string                 KeyType;
    typedef std::map<KeyType, Creator>  Creators;
    typedef typename Creators::iterator iterator;

    //!
    //! \brief    register one Class C with key.
    //! \details  Use the member template to register class C with key and C is the
    //!           derived class of base class T.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \return   true is returned if class C is successfully registerted with key
    //!           false is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool RegisterCodec(const KeyType &key)
    {
        std::pair<iterator, bool> result =
            GetCreators().insert(std::make_pair(key, create<C>));

        return result.second;
    }

    //!
    //! \brief    create a new object that is registered with key.
    //! \details  create and return one new object that is registered with key. And Args is passed to create
    //!           the new object.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \return   the derived object of T is returned if key is found and the object is created.
    //!           nullptr is returned if key is not found
    //!
    static Type CreateCodec(const KeyType &key)
    {
        Creators &creators = GetCreators();
        iterator  creator  = creators.find(key);
        if (creator != creators.end())
            return (creator->second)();

        return nullptr;
    }

private:
    //!
    //! \brief    the callback function with key.
    //! \details  The member template to create the derived object
    //!
    //!
    //! \return   the created object.
    //!
    template <class C>
    static Type create()
    {
        return MOS_New(C);
    }

    //!
    //! \brief    obtain the static pair table of key and callback function
    //! \details  obtain the static pair table that is registerted with param@key and callback function
    //!

    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;

        return creators;
    }
};

#endif /*  _MEDIA_DDI_FACTORY_H_ */
