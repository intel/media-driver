/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     cp_factory.h
//! \brief    Helps with cp specific factory creation.
//!

#ifndef _CP_FACTORY_H_
#define _CP_FACTORY_H_

#include <map>
#include "mos_utilities.h"

//!
//! \class    CpFactory
//! \brief    Cp factory
//!
template <class T, typename... Args>
class CpFactory
{
public:
    typedef T *Type;
    typedef uint32_t KeyType;
    typedef Type (*Creator)(Args... args);
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
    //! \return   true is returned if class C is successfully registered with key
    //!           false is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool Register(KeyType key)
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
    static Type Create(
        KeyType key, Args... args)
    {
        Creators &creators = GetCreators();
        Iterator creator = creators.find(key);
        if (creator != creators.end())
        {
            return (creator->second)(args...);
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
    static Type Create(Args... args)
    {
        return MOS_New(C, args...);
    }


    //!
    //! \brief    obtain the static pair table of param@ key and callback function
    //! \details  obtain the static pair table that is registered with param@key and callback function
    //!
    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;
        return creators;
    }
};


//!
//! \class  CpFactoryWithoutArgs
//! \brief  Cp factory no argument class
//!
template <class T>
class CpFactoryWithoutArgs
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
    //! \return   true is returned if class C is successfully registered with key
    //!           false is returned if key is already registered and doesn't register
    //!           class C with key.
    //!
    template <class C>
    static bool Register(KeyType key, bool forceReplace = false)
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
    static Type Create(KeyType key)
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
    //! \return   the created object
    //!
    template <class C>
    static Type Create()
    {
        return MOS_New(C);
    }

    //!
    //! \brief    obtain the static pair table of param@ key and callback function
    //! \details  obtain the static pair table that is registered with param@key and callback function
    //!
    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;
        return creators;
    }
};

#endif // _CP_FACTORY_H_
