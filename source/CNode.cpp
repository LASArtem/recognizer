//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "CNode.h"
#include "tools.h"
#include <algorithm>

namespace tg
{
    typedef key_node_map::const_iterator key_node_iterator;
    typedef std::pair<key_node_iterator, key_node_iterator> key_node_range_t;

    const CNode CNode::Empty = CNode("");
    const std::vector<std::string> CNode::IgnoredTags(   {"!DOCTYPE html"});
    const std::vector<std::string> CNode::SingleNodeTags({"meta"});
    const std::pair<std::string, std::string> CNode::KeyValAliases({"property", "content"});
}

tg::CNode::CNode() :
    m_Texts(),
    m_SubTags()
{
    //empty
}

tg::CNode::CNode(const std::string_view& Text) :
    m_Texts  ({Text}),
    m_SubTags()
{
    //empty
}

size_t tg::CNode::InitFromView(const std::string_view& FileView, const std::string_view& SelfName)
{
    if (FileView.empty())
        return 0;

    size_t length = 0;

	for(std::string_view::const_iterator itFirst = FileView.cbegin(), itLast = nullptr;
        utf8::next(itLast, FileView);)
    {
        if ('<'== *itLast)          //tagName opened
        {
            length = std::distance(itFirst, itLast);    //#TODO Check sinhle *itLast
            std::string_view text = std::string_view(itFirst, length);
            tg::trimm(text, " \n");
            if (!text.empty())
                m_Texts.push_back(text);

            itFirst = itLast;
            ++itFirst;              //skip '<'
        }

        else if ('>'== *itLast)     //tagName closed
        {
            length = std::distance(itFirst, itLast);
            std::string_view tagName = std::string_view(itFirst, length);
            ++itLast;               //skip '>'

            if (IsCloseTag(tagName, SelfName))          //go to parent node
                return std::distance(FileView.cbegin(), itLast);

            else if(!IsIgnoredTag(tagName)              //make child node
                    && (!TryParseComplexTag(tagName)
                        || !tagName.empty()))
            {
                length = std::distance(itLast, FileView.cend());
                std::string_view subView = std::string_view(itLast, length);

                CNode subTag;
                size_t bytesRead = subTag.InitFromView(subView, tagName);
                m_SubTags.emplace(tagName, subTag);
                std::advance(itLast, bytesRead);
            }
            itFirst = itLast;
            --itLast;              //restore '>' - do not miss next character!
        }
    }

    return FileView.size();
}

bool tg::CNode::GatherNodes(std::vector<CNode>& retNodes, const std::string& key) const
{
    key_node_range_t rng = m_SubTags.equal_range(key);
    retNodes.clear();
    retNodes.reserve(std::distance(rng.first, rng.second));

    for (key_node_iterator i = rng.first; i != rng.second; ++i)
        if (!i->second.empty())
            retNodes.emplace_back(i->second);

    return !retNodes.empty();
}

bool tg::CNode::GatherTexts(view_vector_t& retText, const std::string& key, bool append /*= false*/) const
{
    key_node_range_t rng = m_SubTags.equal_range(key);
    if (!append)
        retText.clear();

    retText.reserve(retText.size() + std::distance(rng.first, rng.second));
    for (key_node_iterator node = rng.first;
         node != rng.second; ++node)
        for(const std::string_view& v : node->second.m_Texts)
            retText.emplace_back(v);

    return !retText.empty();
}

const std::string_view& tg::CNode::GetText() const
{
    return m_Texts.size() ? m_Texts.front() : CNode::Empty.GetText();
}

const tg::view_vector_t& tg::CNode::GetTexts() const
{
    return m_Texts;
}

const tg::key_node_map& tg::CNode::GetSubTags() const
{
    return m_SubTags;
}

void tg::CNode::clear()
{
    m_Texts.clear();
    m_SubTags.clear();
}

size_t tg::CNode::size() const
{
    return m_SubTags.size();
}

bool tg::CNode::empty() const
{
    return m_SubTags.empty()
           && (m_Texts.empty()
               || this == &tg::CNode::Empty);
}

const tg::CNode& tg::CNode::operator [] (const std::string& key) const
{
    key_node_iterator pNode = m_SubTags.find(key);
    return pNode != m_SubTags.cend()? pNode->second : tg::CNode::Empty;
}

bool tg::CNode::IsCloseTag(const std::string_view& Tag, const std::string_view& ExpectedName) const
{
    if (Tag.empty() || '/' != Tag.front())
        return false;

    std::string_view tagName = Tag.substr(1);
    return tg::is_same(ExpectedName, tagName);
}

bool tg::CNode::TryParseComplexTag(std::string_view& Tag)
{
    bool isSingleTag = '/' == Tag.back();

    std::size_t spasePos = Tag.find(' ');
    if (!spasePos || spasePos > Tag.size())
    {
        if (isSingleTag)
            tg::clear(Tag);
        return isSingleTag;         //true : like <br/> single tag, so ignore
    }                               //false: like <regular> expect </regular>, so it is simple tag

    //there are single tag and space: like<asd parameter="2357"/>, so no tag to close expected
    std::string_view tagName = Tag.substr(0, spasePos);
    std::string_view rest    = Tag;
    rest.remove_prefix(spasePos + 1);
    tg::trimm_spaces(rest);

    TryParseMultiTag(tagName, rest);
    if (isSingleTag)
        tg::clear(Tag);
    else
        Tag = tagName;

    return true;
}

void tg::CNode::TryParseMultiTag(const std::string_view& Key, const std::string_view& Tag)
{
    std::string_view rest = Tag;
    CNode subTag;
    if (!subTag.TryParseAttributes(rest))
        return;

    if (IsSingleNodeTag(Key))
    {
        key_node_map::iterator pExistNodeIt = m_SubTags.find(Key);
        if (pExistNodeIt != m_SubTags.end())    //try find existing <meta .../> Tag
        {
            pExistNodeIt->second.m_SubTags.merge(subTag.m_SubTags);
            return;
        }
    }
    m_SubTags.emplace(Key, subTag);
}

bool tg::CNode::TryParseAttributes(const std::string_view& Tag)    // like: ..property="1" content="a"/>
{
    std::list<key_node_t> attributes;
    std::string_view rest = Tag;

    std::size_t tokenPos = rest.find("=\"");
    while (tokenPos && tokenPos < rest.size())
    {
        std::string_view tagName = rest.substr(0, tokenPos);

        rest.remove_prefix(tokenPos + 2);
        tokenPos = rest.find('\"');
        if (tokenPos < rest.size())
        {
            CNode newNode = CNode(rest.substr(0, tokenPos));
            attributes.emplace_back(tagName, newNode);
            rest.remove_prefix(tokenPos + 1);
            tg::trimm_spaces(rest);
        }
        tokenPos = rest.find("=\"");
    }

    FilterKeyValAliases(attributes);

    for(key_node_t& node : attributes)
        m_SubTags.insert(node);

    return !attributes.empty();
}

void tg::CNode::FilterKeyValAliases(std::list<key_node_t>& Nodes) const
{
    std::list<key_node_t>::iterator itKey = std::find_if(Nodes.begin(), Nodes.end(),
        [](const key_node_t& node) -> bool
        {
            return tg::is_same(KeyValAliases.first, node.first);
        });
    if (itKey == Nodes.end())
        return;

    std::list<key_node_t>::iterator itVal = std::find_if(itKey, Nodes.end(),
        [](const key_node_t& node) -> bool
        {
            return tg::is_same(KeyValAliases.second, node.first);
        });
    if (itVal == Nodes.end())
        return;

    if (1 != itKey->second.GetTexts().size())
        return;

    const std::string_view& newKey = itKey->second.GetTexts().front();
    itKey->first  = newKey;
    itKey->second = itVal->second;
    Nodes.erase(itVal);
}

bool tg::CNode::IsIgnoredTag(const std::string_view& Tag) const
{
    return tg::CNode::IgnoredTags.cend() != std::find(
           tg::CNode::IgnoredTags.cbegin(),
           tg::CNode::IgnoredTags.cend(),
           Tag);
}

bool tg::CNode::IsSingleNodeTag(const std::string_view& Tag) const
{
    return tg::CNode::SingleNodeTags.cend() != std::find(
           tg::CNode::SingleNodeTags.cbegin(),
           tg::CNode::SingleNodeTags.cend(),
           Tag);
}

#ifdef TG_DEBUG_INFO
void tg::CNode::dbgPrint(size_t Indent, std::string SelfKey) const
{
    std::string prefix = std::string(2 * Indent, ' ');
    view_vector_t val = GetTexts();

    DEBUG_LOG(prefix<<SelfKey<<" = "<<(val.empty() ? "" : val.front()));
    for (size_t i = 1; i < val.size(); ++i)
    {
        DEBUG_LOG(prefix<<std::string(SelfKey.size(), ' ')<<" = "<<val[i]);
    }
    for(const key_node_t& node : m_SubTags)
    {
        std::string key = std::string(node.first);
        node.second.dbgPrint(Indent + 1, key);
    }
}
#endif //TG_DEBUG_INFO
