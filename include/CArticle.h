//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef CARTICLE_H
#define CARTICLE_H

#include "CNode.h"
#include "CFileHolder.h"

namespace tg{

    class CArticle :
        public CFileHolder
    {
    public :
        CArticle();
        ~CArticle() override = default;
        bool  InitFromHtml  (const fs::path& FileName);
        const std::string_view& GetTitle () const;
        const std::string_view& GetDescription() const;
        const std::string_view& GetUrl() const;
        const std::string_view& GetSiteName() const;
        bool  GetBody    (view_vector_t& RetText ) const; //#TODO with <b>,<i> or not?
        bool  GetBodyBold(view_vector_t& RetText ) const; //#TODO <marked>, italic etc.
        const CNode& GetContent() const;
        void  SetLangRelevance(const Lang::Type Lang, const EDepth Depth, const float Relevance);
        float GetLangRelevance(const Lang::Type Lang, EDepth& retDepth) const;
        void  SetNewsRelevance(const EDepth Depth, const float Relevance);
        float GetNewsRelevance(EDepth& retDepth) const;
        void  ClearCache();
        void  clear();

    protected:
        typedef std::pair<EDepth, float> depth_relevance_t;
        typedef std::map<Lang::Type, depth_relevance_t> lang_relevances_t;
        void  TrySetRelevance(depth_relevance_t& RelevanceCached, const EDepth Depth, const float Relevance) const;

    private:
        CNode             m_Content;
        lang_relevances_t m_LangRelevancesCache;
        depth_relevance_t m_NewsRelevanceCache;
    };
}
#endif // CARTICLE_H
