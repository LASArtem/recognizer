//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "CArticle.h"

tg::CArticle::CArticle() :
    m_Content(),
    m_LangRelevancesCache(),
    m_NewsRelevanceCache (EDepth::UNDEF, -1.f)
{
    //empty
}

bool tg::CArticle::InitFromHtml(const fs::path& FileName)
{
    LoadFile(FileName);
    CNode parsedTags;
	if (!parsedTags.InitFromView(GetFileView()))
    {
        DEBUG_LOG("Warn: "<<FileName<<" has unexpected file structure, probably it corrupted"); //0 bytes read
    }
	m_Content  = parsedTags["html"];
    return !m_Content.empty();
}

const std::string_view& tg::CArticle::GetTitle() const
{
    const std::string_view& retTitle = m_Content["head"]["meta"]["og:title"].GetText();
    if (!retTitle.empty())
        return retTitle;

    return m_Content["body"]["article"]["h1"].GetText();
}

const std::string_view& tg::CArticle::GetDescription() const
{
    return m_Content["head"]["meta"]["og:description"].GetText();
}

const std::string_view& tg::CArticle::GetUrl() const
{
    return m_Content["head"]["meta"]["og:url"].GetText();
}

const std::string_view& tg::CArticle::GetSiteName() const
{
    return m_Content["head"]["meta"]["og:site_name"].GetText();
}

bool tg::CArticle::GetBody(view_vector_t& RetText) const
{
    return m_Content["body"]["article"].GatherTexts(RetText, "p");
}

bool tg::CArticle::GetBodyBold(view_vector_t& RetText) const
{
    std::vector<CNode> nodes;       //#TODO remove multiple copy of nodes
    m_Content["body"]["article"].GatherNodes(nodes, "p");

    RetText.clear();
    RetText.reserve(nodes.size());

    for (const CNode& node : nodes)
        node.GatherTexts(RetText, "b", true);

    return !RetText.empty();
}

const tg::CNode& tg::CArticle::GetContent() const
{
    return m_Content;
}

void tg::CArticle::SetLangRelevance(const Lang::Type Lang, const EDepth Depth, const float Relevance)
{
    DEBUG_ASSERT(EDepth::UNDEF <= Depth && Depth < EDepth::COUNT && "EDepth out of allowed bounds");
    lang_relevances_t::iterator itLang = m_LangRelevancesCache.find(Lang);
    if (m_LangRelevancesCache.end() == itLang)
    {
        m_LangRelevancesCache.emplace(Lang, std::make_pair(Depth, Relevance));
        return;
    }
    TrySetRelevance(itLang->second, Depth, Relevance);
}
float tg::CArticle::GetLangRelevance(const Lang::Type Lang, EDepth& retDepth) const
{
    lang_relevances_t::const_iterator itLang = m_LangRelevancesCache.find(Lang);
    if (m_LangRelevancesCache.end() == itLang)
    {
        retDepth = EDepth::UNDEF;
        return -1.f;            // -1.f means relevance undefined for given depth
    }
    retDepth = itLang->second.first;
    return itLang->second.second;
}

void  tg::CArticle::SetNewsRelevance(const EDepth Depth, const float Relevance)
{
    DEBUG_ASSERT(EDepth::UNDEF <= Depth && Depth < EDepth::COUNT && "EDepth out of allowed bounds");
    TrySetRelevance(m_NewsRelevanceCache, Depth, Relevance);
}

float tg::CArticle::GetNewsRelevance(EDepth& retDepth) const
{
    retDepth = m_NewsRelevanceCache.first;
    return m_NewsRelevanceCache.second;
}

void  tg::CArticle::ClearCache()
{
    m_LangRelevancesCache.clear();
    m_NewsRelevanceCache = std::make_pair(EDepth::UNDEF, -1.f);
}

void tg::CArticle::clear()
{
    ClearCache();
    m_Content.clear();
}

void  tg::CArticle::TrySetRelevance(depth_relevance_t& RelevanceCached, const EDepth Depth, const float Relevance) const
{
    if (RelevanceCached.first >= Depth)
    {
        DEBUG_LOG("Relevance cache overridden");
        return;
    }
    RelevanceCached.first  = Depth;
    RelevanceCached.second = Relevance;
}
