//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef TOOLS_H
#define TOOLS_H

#include "tgtypes.h"
#include<string_view>

namespace tg{

    bool is_same(const std::string_view& sw1, const std::string_view& sw2);
    bool begins(const std::string_view& sw_explored, const std::string& key);
    void clear(std::string_view& str_w);
    void trimm(std::string_view& str_w, const char* chars);
    void trimm_spaces(std::string_view& str_w);
    void split_words(const std::string_view& Str, view_vector_t& retWords, bool Append = false);
    void split_words(const view_vector_t& Text,   view_vector_t& retWords);

    float  swtof(   const std::string_view& str_w, float  defaultVal = 0.f);
    size_t swtoul(  const std::string_view& str_w, size_t defaultVal = 0  );
    bool   swtobool(const std::string_view& str_w, bool defaultVal = false);

    namespace Lang
    {
        enum class Type;

        Type FromString(const std::string_view& sw);
        const char* ToString(Type v);
        size_t CalcCharHits(const std::string_view& source,const Type LangType);
    }

    namespace utf8
    {
        bool   next(std::string_view::const_iterator& it, const std::string_view& source);
        size_t count_width(std::string_view::const_iterator& it, const std::string_view& source);
    }

#ifdef TG_DEBUG_INFO
    bool WriteFile(const std::string& FileName, const std::string& Str);

    namespace Statistic
    {
        void OnHitCase(const std::string& Key, bool FromAllThreads = true);
        void PrintCasesHit();
        void InitCountTrack(const std::string& Key, size_t Max, size_t Step, bool FromAllThreads = true);
        std::string GetCurrentThreadId();
    }
#endif //TG_DEBUG_INFO
} //namespace tg

#endif // TOOLS_H
