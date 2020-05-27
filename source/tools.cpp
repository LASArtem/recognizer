//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "tools.h"
#include <functional>

#ifdef TG_DEBUG_INFO
#include <thread>
#include <fstream>
#include <sstream>
#endif //TG_DEBUG_INFO

//*************************
//  Globals
//*************************

namespace tg::Lang
{
    struct SCharSet
    {
        typedef std::pair<std::string, std::string> charrange_t;
        std::vector<charrange_t> Ranges;
        std::string              Extra;
    };

    const std::map<Type, SCharSet> CharSets = {
    {
        Type::EN,
        {{
            {u8"A", u8"Z"},
            {u8"a", u8"z"},
        },
        u8"\x24\xC2\xA3\xE2\x82\xAC"}       //dollar, pound, euro signs
    },
    {
        Type::RU,
        {{
            {u8"\xD0\x90", u8"\xD1\x8F"}    //Alphabet from Uppercase to lower
        },
        u8"\xD1\x91\xD0\x81\xC2\xAB\xC2\xBB"}
    },
    {
        Type::COMMON,
        {{
            {u8" ", u8"@"},
        },
        u8"\x27\xE2\x80\x98\xE2\x80\x99\xE2\x80\x94\xE2\x80\x9C\xE2\x80\x9D"}
    },
    {
        Type::OTHER,
        {{},
        u8"\xC3\xA9\xC3\xB3\xC3\xB1"}       //Spanish... //#TODO Remove
    }};

    const int SpecificCharWeight = 3;
    const int AlienCharWeight   = -8;
    const std::map<Type ,std::string> SpecificChars = { //#TODO remove
    {
        Type::EN,
        u8""
    },
    {
        Type::RU,
        u8""    //IOio
    },
    {
        Type::COMMON,
        u8""
    },
    {
        Type::OTHER,
        u8""
    }};

    const std::map<Type ,std::string> AlienChars = {    //#TODO CharSet with ranges
    {   //Spanish, Portuguese, German, Danish-Norwegian, Turkish, Romanian, Catalan, French, Polish, Slovenian, Italian, Croatian
        Type::EN,   //#TODO Sort by probability
        u8"\xC3\xB1\xC3\xB3\xC3\xA9\xC3\xA1\xC3\xA2\xC3\xA0\xC3\xAA\xC3\xB4\xC3\xB5\xC3\xBA\xC2\xBF\xC2\xA1\xC3\xA4\xC3\xB6\xC3\xBC\xC3\x9F\xC3\xB8\xC3\xA5\xC3\xA6\xC4\xB1\xC3\xA7\xC4\x9F\xC5\x9F\xC4\x83\xC3\xAE\xC8\x99\xC8\x9B\xC3\xA8\xC3\xAF\xC3\xB2\xC3\xAB\xC3\xBB\xC3\xB9\xC3\xBF\xC4\x85\xC4\x87\xC4\x99\xC5\x84\xC5\x9B\xC5\x82\xC5\xBA\xC5\xBC\xC4\x8D\xC5\xBE\xC5\xA1\xC3\xAC\xC4\x91"
    },  //Capital Aliens removed for performance
    {   //Ukrainian, Kazakh, Tatar, Tajik
        Type::RU,
        u8"\xD1\x96\xD1\x97\xD1\x94\xD2\x91\xD2\x9B\xD3\x99\xD2\x93\xD2\xA3\xD2\xB1\xD3\xA9\xD2\xAF\xD2\xBB\xD1\x9E\xD2\xB3\xD2\xB7\xD2\xB3\xD3\xAF"
    },  //Capital Aliens removed for performance
    {
        Type::COMMON,
        u8""
    },
    {
        Type::OTHER,
        u8""
    }};
}

#ifdef TG_DEBUG_INFO
namespace tg::Statistic
{
    std::map<std::string, size_t> CasesHit = {};

    struct SCountTrack
    {
        const std::string Key;
        const size_t      Max;
        const size_t      Step;
        size_t            current;
        SCountTrack(const std::string& key, const size_t max, const size_t step) :
            Key (key ),
            Max (max ),
            Step(step),
            current(0)
        {
            //empty
        }
        ~SCountTrack() = default;
    };
    SCountTrack& operator++(SCountTrack& track)
    {
        DEBUG_IF_LOG(++track.current % track.Step == 0,
            track.Key<<" : "<<track.current<<" of "<<track.Max);
        return track;
    };

    std::map<std::string, SCountTrack> CountTracks = {};
}
#endif //TG_DEBUG_INFO

//*************************
//  Helpers
//*************************

bool tg::is_same(const std::string_view& sw1, const std::string_view& sw2)
{
    if (sw1.size() != sw1.size())
        return false;

    for(std::string_view::const_iterator it1 = sw1.cbegin(),
        it2 = sw2.cbegin();
        it1 != sw1.cend();
        ++it1, ++it2)
        if (*it1 != *it2)
            return false;

    return true;
}

bool tg::begins(const std::string_view& sw_explored, const std::string& key)
{
    if (key.empty() || sw_explored.size() < key.size())
        return false;

    std::string_view::const_iterator it = sw_explored.cbegin();
    for (const char& ch : key)
        if (ch != *it++)
            return false;

    return true;
}

void tg::clear(std::string_view& str_w)
{
    str_w.remove_suffix(str_w.size());
}

void tg::trimm(std::string_view& str_w, const char* chars)
{
    size_t i = str_w.find_first_not_of(chars);
    i = i < str_w.size()? i : str_w.size();
    str_w.remove_prefix(i);
}

void tg::trimm_spaces(std::string_view& str_w)
{
    tg::trimm(str_w, " ");
}

void tg::split_words(const std::string_view& Str, view_vector_t& retWords, bool Append/*= false*/)
{
    const char* delimeters = " .,:-?!/"; //#TODO not only single char delimeters 0xxxxxxx
    if (!Append)
        retWords.clear();

    retWords.reserve(retWords.size() + Str.size() / Consts::WordLengthExpected);
    std::string_view line = Str;
    tg::trimm(line, delimeters);
    if (line.empty())
        return;

    size_t startPos = 0;
    size_t endPos = line.find_first_of(delimeters, startPos);
    while (endPos < line.size())
    {
        retWords.push_back(line.substr(startPos, endPos - startPos));  //#TODO iterators are faster?
        startPos = line.find_first_not_of(delimeters, endPos);
        endPos   = line.find_first_of(delimeters, startPos);
    }
    if (startPos < line.size())                                 //last word
        retWords.push_back(line.substr(startPos, line.size() - startPos ));
}

void tg::split_words(const view_vector_t& Text, view_vector_t& retWords)
{
    retWords.clear();
    for (const std::string_view line : Text)
        tg::split_words(line, retWords, true);
}

float tg::swtof(const std::string_view& str_w, float defaultVal/*= 0.f*/)
{
    try //#TODO no construct std::string https://stackoverflow.com/questions/45637697/how-to-convert-stdstring-view-to-double
    {
        defaultVal = std::stof(std::string(str_w));
    }
    catch (std::exception &ex)
    {
    }
    return defaultVal;
}

size_t tg::swtoul(const std::string_view& str_w, size_t defaultVal/*= 0*/)
{
    try
    {
        defaultVal = std::stoul(std::string(str_w));
    }
    catch (std::exception &ex)
    {
    }
    return defaultVal;
}

bool tg::swtobool(const std::string_view& str_w, bool defaultVal/*= false*/)
{
    if ("true"  == str_w)
        return true;
    if ("false" == str_w)
        return false;
    return defaultVal;
}

tg::Lang::Type tg::Lang::FromString(const std::string_view& sw)
{
    if ("en" == sw)
        return Type::EN;
    else if ("ru" == sw)
        return Type::RU;
    else if ("other" == sw)
        return Type::OTHER;
    else
        return Type::UNDEF;
}

const char* tg::Lang::ToString(Type v)
{
    switch (v)
    {
        case Type::EN :   return "en";
        case Type::RU :   return "ru";
        case Type::OTHER :return "other";
        default :         return "undef";
    }
}

namespace tg::Lang  //internal scope
{
    typedef std::string_view::const_iterator sw_it_t;
    typedef std::function<bool(sw_it_t& it)> node_func_t;

    bool IsCharOk(sw_it_t It, std::vector<node_func_t> ConditionsAny)
    {
        for (const node_func_t& func : ConditionsAny)
            if (func(It))
                return true;
        return false;
    }

    bool IsCharInRange(sw_it_t It, node_func_t From, node_func_t To)
    {
        return From(It) && To(It);
    }

    bool IsCharMore(sw_it_t It, const unsigned char Ch, node_func_t NextCheck)
    {
        unsigned char itChar = static_cast<unsigned char>(*It);
        return itChar > Ch
            || (itChar == Ch
                && NextCheck(++It));
    }

    bool IsCharLess(sw_it_t It, const unsigned char Ch, node_func_t NextCheck)
    {
        unsigned char itChar = static_cast<unsigned char>(*It);
        return itChar < Ch
            || (itChar == Ch
                && NextCheck(++It));
    }

    bool IsCharEqual(sw_it_t It, const unsigned char Ch, node_func_t NextCheck)
    {
        return Ch == static_cast<unsigned char>(*It)
            && NextCheck(++It);
    }

    bool CharOk(sw_it_t It)
    {
        return true;
    }

    bool CharNotOk(sw_it_t It)
    {
        return false;
    }

    node_func_t MakeRangeRecognizer(const SCharSet::charrange_t& Range)
    {
        std::reverse_iterator<std::string::const_iterator> chIt = Range.first.crbegin();
        node_func_t conditionFrom = std::bind(IsCharMore, std::placeholders::_1, *chIt, &CharOk);
        for(++chIt; chIt != Range.first.crend(); ++chIt)
            conditionFrom = std::bind(IsCharMore, std::placeholders::_1, *chIt, conditionFrom);

        chIt = Range.second.crbegin();
        node_func_t conditionTo = std::bind(IsCharLess, std::placeholders::_1, *chIt, &CharOk);
        for(++chIt; chIt != Range.second.crend(); ++chIt)
            conditionTo = std::bind(IsCharLess, std::placeholders::_1, *chIt, conditionTo); //#TODO first char Equal in range by IsEqual (Ru \xD0\x90\xD0\xAF -> \xD0 got one check)

        return std::bind(IsCharInRange, std::placeholders::_1, conditionFrom, conditionTo); //since c++20 constexpr bind would be much faster
    }

    node_func_t MakeGlyphRecognizer(const std::string_view& Glyph)
    {
        std::string_view::const_reverse_iterator chIt = Glyph.crbegin();
        node_func_t conditionEqual = std::bind(IsCharEqual, std::placeholders::_1, *chIt, &CharOk);
        for(++chIt; chIt != Glyph.crend(); ++chIt)
            conditionEqual = std::bind(IsCharEqual, std::placeholders::_1, *chIt, conditionEqual);

        return conditionEqual;
    }

    node_func_t MakeSetRecognizer(SCharSet Set)
    {
        std::vector<node_func_t> ConditionsVector;
        for (const SCharSet::charrange_t& rng : Set.Ranges)
        {
            if (rng.first.empty() || rng.second.empty())
            {
                DEBUG_LOG("Invalid CharRange in Language char set");
                continue;
            }
            ConditionsVector.push_back(MakeRangeRecognizer(rng));
        }

        std::string_view singleSymbols = Set.Extra;         //TODO no CopyPaste
        for(std::string_view::const_iterator itFirst = singleSymbols.cbegin(),
            itLast = singleSymbols.cbegin();
            utf8::count_width(itLast, singleSymbols);)
        {
            std::string_view glyph = std::string_view(itFirst, std::distance(itFirst, itLast));
            ConditionsVector.push_back(MakeGlyphRecognizer(glyph));
            itFirst = itLast;
        }

        return std::bind(IsCharOk, std::placeholders::_1, ConditionsVector);
    }

    const std::map<Type, node_func_t> MakeLangRecognizers()
    {
        std::map<Type, node_func_t> recognizers;
        Type lang = Type::UNDEF;
        for (++lang; lang < Type::COUNT; ++lang)
            recognizers.emplace(lang,
                MakeSetRecognizer(CharSets.at(lang)));

        return recognizers;
    }

    node_func_t MakeGlyphCollectionRecognizer(const std::string_view& GlyphCollection)
    {
        if (GlyphCollection.empty())
            return CharNotOk;

        std::vector<node_func_t> ConditionsVector;
        for(std::string_view::const_iterator itFirst = GlyphCollection.cbegin(),
            itLast = GlyphCollection.cbegin();
            utf8::count_width(itLast, GlyphCollection);)
        {
            std::string_view glyph = std::string_view(itFirst, std::distance(itFirst, itLast));
            ConditionsVector.push_back(MakeGlyphRecognizer(glyph));
            itFirst = itLast;
        }

        return std::bind(IsCharOk, std::placeholders::_1, ConditionsVector);
    }

    const std::map<Type, node_func_t> MakeSpecialRecognizers(const std::map<Type ,std::string>& Chars)
    {
        std::map<Type, node_func_t> recognizers;
        Type lang = Type::UNDEF;
        for (++lang; lang < Type::COUNT; ++lang)
            recognizers.emplace(lang,
                MakeGlyphCollectionRecognizer(Chars.at(lang)));

        return recognizers;
    }

    const std::map<Type ,node_func_t> LangRecognizers     = MakeLangRecognizers();
    const std::map<Type ,node_func_t> SpecificRecognizers = MakeSpecialRecognizers(Lang::SpecificChars);
    const std::map<Type ,node_func_t> AlienRecognizers    = MakeSpecialRecognizers(Lang::AlienChars);
} //end namespace tg::Lang

size_t tg::Lang::CalcCharHits(const std::string_view& source, const Type LangType)  //return byte count of suitable chars
{                                                                                   //Type::COMMON symbols assumed suitable to any Language
    int retCount   = 0;
    size_t charLength = 1;
    const node_func_t& IsFromLang = LangRecognizers.at(LangType);
    const node_func_t& IsCommon   = LangRecognizers.at(Type::COMMON);
    const node_func_t& IsAlien    = AlienRecognizers.at(LangType);

	for(std::string_view::const_iterator charIt = source.cbegin();
        charIt < source.cend();)
    {
        bool isHit = IsFromLang(charIt) || IsCommon(charIt);
        int weight = isHit ?
            1 : IsAlien(charIt) ?
                Lang::AlienCharWeight : 0;

        charLength = utf8::count_width(charIt, source);
        retCount  += weight * static_cast<int>(charLength);
    }

    return 0 < retCount ? static_cast<size_t>(retCount) : 0;
}

bool tg::utf8::next(std::string_view::const_iterator& it, const std::string_view& source) //if return 0 - iterator invalid
{                                                                                         //source.cend() assumed invalid
    if (!it)
    {
        it = source.cbegin();
        return true;
    }

    unsigned char ch = *it;                                //#TODO try signed math
    if ( ch < 0x80)     //bx 0xxx xxxx - one byte sequence
        return ++it < source.cend();

    else if (ch < 0xC0) //bx 10xx xxxx - tail byte - find next valid utf8 sequence
    {
        return next(++it, source);
    }
    else if (ch < 0xE0) //bx 110x xxxx - two byte head
    {
        std::advance(it, 2);
        return it < source.cend();
    }
    else if (ch < 0xF0) //bx 1110 xxxx - three byte head
    {
        std::advance(it, 3);
        return it < source.cend();
    }
    else if (ch < 0xF8) //bx 1111 0xxx - four byte head
    {
        std::advance(it, 4);
        return it < source.cend();
    }

    return next(++it, source);
}

size_t tg::utf8::count_width(std::string_view::const_iterator& it, const std::string_view& source)  //moves iterator to next
{                                                                                                   //source.cend() assumed valid as well
    unsigned char ch = *it;
    if ( ch < 0x80)     //bx 0xxx xxxx - one byte sequence
        return ++it <= source.cend();

    else if (ch < 0xC0) //bx 10xx xxxx - tail byte - find next valid utf8 sequence
    {
        size_t step = next(++it, source);
        return step + (it <= source.cend());
    }
    else if (ch < 0xE0) //bx 110x xxxx - two byte head
    {
        std::advance(it, 2);
        return 2 * (it <= source.cend());
    }
    else if (ch < 0xF0) //bx 1110 xxxx - three byte head
    {
        std::advance(it, 3);
        return 3 * (it <= source.cend());
    }
    else if (ch < 0xF8) //bx 1111 0xxx - four byte head
    {
        std::advance(it, 4);
        return 4 * (it <= source.cend());
    }

    size_t step = next(++it, source);   //#TODO composite sequences not supported
    return step + (it <= source.cend());
}

#ifdef TG_DEBUG_INFO
bool tg::WriteFile(const std::string& FileName, const std::string& Str) //#TOFO fs::path
{
    std::ofstream file(FileName, std::ios_base::out);
	if (!file.is_open())
    {
        DEBUG_LOG("Warn: Can't open file: \""<<FileName<<"\"");
        return false;
    }
    file<<Str<<std::endl;
    file.close();
    return true;
}

void tg::Statistic::OnHitCase(const std::string& Key, bool FromAllThreads/*= true*/)
{
    std::string mapKey = Key;
    if (!FromAllThreads)
        mapKey += GetCurrentThreadId();

    std::map<std::string, SCountTrack>::iterator itTracks = CountTracks.find(mapKey);
    std::map<std::string, size_t>::iterator      itHits   = CasesHit.find(   mapKey);

    if (itTracks != CountTracks.end())
        ++itTracks->second;
    else if (itHits != CasesHit.end())
        ++itHits->second;
    else
        Statistic::CasesHit.emplace(mapKey, 1);
}

void tg::Statistic::PrintCasesHit()
{
    std::cout<<"HitCases:"<<std::endl;
    for (const std::pair<std::string, size_t>& p : Statistic::CasesHit)
        std::cout<<p.first<<"\t = "<<p.second<<std::endl;
}

void tg::Statistic::InitCountTrack(const std::string& Key, size_t Max, size_t Step, bool FromAllThreads/*= true*/)
{
    std::string mapKey = Key;
    if (!FromAllThreads)
        mapKey += GetCurrentThreadId();

    CountTracks.emplace(mapKey, SCountTrack(Key, Max, Step));
}

std::string tg::Statistic::GetCurrentThreadId()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}
#endif //TG_DEBUG_INFO
