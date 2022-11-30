/*
* Copyright (c) 2021, Intel Corporation
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

#ifndef __MEDIA_CAPSTABLE_H__
#define __MEDIA_CAPSTABLE_H__

#include "igfxfmid.h"
#include "stdint.h"
#include "media_class_trace.h"

// This struct is designed to store common information

typedef struct PlatformInfo
{
    // ipVersion -Intellectual Property
    uint32_t ipVersion;
    // usRevId   -user reversion ID
    uint32_t usRevId;

    // std::map key need to overload operator< function
    bool operator<(const PlatformInfo &rhs) const
    {
        // Make table in descending order.This is for GetCapsTablePlatform()
        // to get the highest version which is lower than request.
        // std::map in asending order according to the value of the key. By default, it uses std::less<KEY>.
        // sort example {{4865 0}<{4615 4}<{4615 1}<{4561 0}<{4608 0}}
        return (((this->ipVersion << 16) | (this->usRevId)) > ((rhs.ipVersion << 16) | (rhs.usRevId)));
    }

} PlatformInfo;

template <typename _attributeCapsTable>
class MediaCapsTable
{
public:
    typedef typename std::map<PlatformInfo, const _attributeCapsTable> OsCapsTable;
    typedef typename OsCapsTable::iterator                       Iterator;

    //!
    //! \brief  Constructor
    //!
    MediaCapsTable() {}

    //!
    //! \brief  virtual destructor
    //!
    virtual ~MediaCapsTable() {}

    // pltCaps: map to store the captable information
    static OsCapsTable &GetOsCapsTable()
    {
        static OsCapsTable pltCaps = {};
        return pltCaps;
    }

    //!
    // \brief     caps register for different Platform
    //! \para     [in] KeyType
    //!           Specific the concret platform
    //! \para     [in]_attributeCapsTable
    //!           value mapped to sepcifci KeyType(PlatformInfo)
    //! \return   bool
    //!           return fail if insert pair fail
    //!
    static bool RegisterCaps(PlatformInfo pltInfo, const _attributeCapsTable attrCapsTable , bool forceReplace = false)
    {
        Iterator attrCapsTableItem = GetOsCapsTable().find(pltInfo);
        if (attrCapsTableItem == GetOsCapsTable().end())
        {
            std::pair<Iterator, bool> result =
                GetOsCapsTable().insert(std::make_pair(pltInfo, attrCapsTable));
            return result.second;
        }
        else
        {
            if (forceReplace)
            {
                GetOsCapsTable().erase(attrCapsTableItem);
                std::pair<Iterator, bool> result =
                    GetOsCapsTable().insert(std::make_pair(pltInfo, attrCapsTable));
                return result.second;
            }
            return true;
        }
    }

    bool IsLastIterator(Iterator iterator)
    {
        //DDI_FUNCTION_ENTER;
        return (iterator == GetOsCapsTable().end());
    }

    //!
    //! \brief    Get attributeCapsTable
    //! \params   [out] iter
    //!           attributeCapsTable iter
    //! \param    [in] PlatformInfo
    //!           Specific the concret platform
    //! \return   bool
    //!           return true if success
    //!
    //!
    bool GetCapsTablePlatform(const PlatformInfo &pltInfo, Iterator &iter)
    {
        //DDI_FUNCTION_ENTER;

        //get the table item w/ (table.ipversion == request.ipversion, table.revision <= request.request)
        iter = GetOsCapsTable().lower_bound(pltInfo);
        if (iter == GetOsCapsTable().end())
        {
            //DDI_NORMALMESSAGE("can't find expected PlatformInfo in GetOsCapsTable()");
            return false;
        }
        if (iter->first.ipVersion != pltInfo.ipVersion)
        {
            //DDI_NORMALMESSAGE("Cannot find request ipVersion!");
            return false;
        }
        return true;
    }

MEDIA_CLASS_DEFINE_END(MediaCapsTable)
};

#endif
