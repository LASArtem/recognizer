//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef TGTYPES_H_INCLUDED
#define TGTYPES_H_INCLUDED

#include <string>
#include <string_view>
#include <map>
#include <vector>

#ifdef TG_DEBUG_INFO
#include <iostream>
#include <assert.h>
#endif //TG_DEBUG_INFO

namespace tg{
    namespace Consts
    {
        const size_t FilesCountExpected   = 200000;
        const size_t FileSizeExpected     = 5120;
        const size_t FileNameSizeExpected = 30;
        const size_t WordLengthExpected   = 5;
        const size_t TitleSizeExprcted    = 200;
    } //namespace Consts

    class CNode;

    typedef std::vector<std::string>               strvector_t;
    typedef std::vector<std::string_view>          view_vector_t;
    typedef std::pair<std::string_view, CNode>     key_node_t;
    typedef std::multimap<std::string_view, CNode> key_node_map;

    enum class ETask
    {
        LANGUAGES  = 0,
        NEWS       = 1,
        CATEGORIES = 2,
        THREADS    = 3,
        TOP        = 4,
        COUNT
    };

    namespace Lang
    {
        enum class Type
        {
            UNDEF  = 0,
            EN     = 1,
            RU     = 2,
            COMMON = 3,
            OTHER  = 4,
            COUNT
        };
    }   //namespace tg::Lang

    enum class EDepth   //Deepness of article exploration
    {
        UNDEF  = 0,
        TITLE  = 1,
        DESC   = 2,
        BODY   = 3,
        COUNT
    };

    template<class E>
    E operator++(E& num)
    {
        if (num < E::UNDEF)
            num = E::UNDEF;
        else if (num >= E::COUNT)
            num = E::COUNT;
        else
            num = static_cast<E>(static_cast<int>(num) + 1);
        return num;
    }

    template<class E>
    int operator-(const E& lhs, const E& rhs)
    {
        return static_cast<int>(lhs) - static_cast<int>(rhs);
    }

#ifdef TG_DEBUG_INFO
#define DEBUG_LOG(msg) std::cout<<msg<<std::endl
#define DEBUG_IF_LOG(cond, msg) if (cond){ DEBUG_LOG(msg); }
#define DEBUG_ASSERT(condMsg) assert(condMsg)
#define DEBUG_ACTION(line) line
#else
#define DEBUG_LOG(msg)
#define DEBUG_IF_LOG(cond, msg)
#define DEBUG_ASSERT(condMsg)
#define DEBUG_ACTION(line)
#endif //TG_DEBUG_INFO

} //namespace tg

#endif // TGTYPES_H_INCLUDED
