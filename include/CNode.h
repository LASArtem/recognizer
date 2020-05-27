//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef CNODE_H
#define CNODE_H

#include "tgtypes.h"
#include<list>

namespace tg{

    class CNode
    {
    public:
        CNode();
        CNode(const std::string_view& Text);
        ~CNode() = default;

        size_t InitFromView(const std::string_view& FileView, const std::string_view& SelfName = ""); //Return read bytesCount
        bool GatherNodes (std::vector<CNode>& retNodes, const std::string& key) const;
        bool GatherTexts (view_vector_t& retText, const std::string& key, bool append = false) const;
        const std::string_view& GetText() const;    //return first part between tags
        const view_vector_t&    GetTexts() const;   //return all texts in node;
        const key_node_map&     GetSubTags() const;

        void clear();
        size_t size() const;
        bool empty() const;
        const CNode& operator [] (const std::string& key) const;   //gives first found object by key

    protected:
        bool IsCloseTag         (const std::string_view& Tag,  const std::string_view& ExpectedName) const;
        bool TryParseComplexTag (std::string_view& Tag       );
        void TryParseMultiTag   (const std::string_view& Key,  const std::string_view& Tag);
        bool TryParseAttributes (const std::string_view& Tag );
        void FilterKeyValAliases(std::list<key_node_t>& Nodes) const;
        bool IsIgnoredTag       (const std::string_view& Tag ) const;
        bool IsSingleNodeTag    (const std::string_view& Tag ) const;

    private:
        view_vector_t m_Texts;
        key_node_map  m_SubTags;

        static const CNode Empty;
        static const strvector_t IgnoredTags;
        static const strvector_t SingleNodeTags;
        static const std::pair<std::string, std::string> KeyValAliases;

#ifdef TG_DEBUG_INFO
    public:
        void dbgPrint(size_t Indent = 0, std::string SelfKey = "") const;
#endif //TG_DEBUG_INFO
    };  //#TODO try ptr subnodes

} //namespace tg

#endif // CNODE_H
