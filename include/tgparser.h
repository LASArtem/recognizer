//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef TGPARSER_H
#define TGPARSER_H

#include "tgtypes.h"
#include <filesystem>
#include <list>

namespace tg{

    class CArticle;

    class CArticleSet
    {
    public:
        CArticleSet();
        CArticleSet(const CArticleSet& Source);
        virtual ~CArticleSet() = default;
        virtual bool  Init(const key_node_t& Source, const ETask TaskType) = 0;
        virtual bool  IsValid() const;
        virtual float CalcRelevance(CArticle& Article, const EDepth Depth) const;
        virtual bool  TryPushArticle(CArticle* pArticle);
        virtual void  MakeReady();

        void  merge(CArticleSet* Other);
        size_t size() const;
        const std::list<std::pair<float, CArticle*>>& GetArticles() const;
        void  PushReport(std::string& retReport, const std::string& SpapcesPrefix = "") const;
        float GetMinRelevance()    const;
        float GetEnoughRelevance() const;

    protected:
        virtual void  SetArticleRelevance(CArticle& Article, const EDepth Depth, float Relevance) const = 0;
        virtual float GetArticleRelevance(const CArticle& Article, EDepth& retDepth) const = 0;
        virtual float CalcRelevance(const std::string_view& Str) const = 0;
        virtual float CalcRelevance(const view_vector_t& Text) const = 0;

        std::list<std::pair<float, CArticle*>>& GetArticles();
        void SetMinRelevance(   float Relevance);
        void SetEnoughRelevance(float Relevance);

    private:
        static bool RelevancePred(const std::pair<float, CArticle*>& lhs, const std::pair<float, CArticle*>& rhs);

        float m_MinRelevance;
        float m_EnoughRelevance;
        std::list<std::pair<float, CArticle*>> m_Articles;
    };

    class CNews;

    class CLanguage :
        public CArticleSet
    {
    public:
        CLanguage();
        CLanguage(const CLanguage& Other);
        ~CLanguage() override;
        bool  Init(const key_node_t& Source, const ETask TaskType) override;
        bool  IsValid() const override;
        float CalcRelevance (CArticle& Article, const EDepth Depth) const override;
        bool  TryPushArticle(CArticle* pArticle) override;
        void  MakeReady() override;

        void merge(CLanguage* Other);
        void  PushReport(std::string& retReport) const;
        Lang::Type   GetType() const;
        const CNews& GetNews() const;
        bool  HasNews() const;
        CNews* GetNewsPtr();

    protected:
        void  SetArticleRelevance(CArticle& Article, const EDepth Depth, float Relevance) const override;
        float GetArticleRelevance(const CArticle& Article, EDepth& retDepth) const override;
        float CalcRelevance(const std::string_view& Str) const override;
        float CalcRelevance(const view_vector_t& Text) const override;

    private:
        Lang::Type m_Type;
        CNews*     m_pNews;
    };

    class CNews :
        public CArticleSet
    {
    public:
        CNews();
        CNews(const CNews& Source);
        ~CNews() override = default;
        bool  Init(const key_node_t& Source, const ETask TaskType) override;
        bool  IsValid() const override;
        float CalcRelevance (CArticle& Article, const EDepth Depth) const override;
        bool  TryPushArticle(CArticle* pArticle) override;

        void  PushReport(std::string& retReport) const;

    protected:
        void  SetArticleRelevance(CArticle& Article, const EDepth Depth, float Relevance) const override;
        float GetArticleRelevance(const CArticle& Article, EDepth& retDepth) const override;
        float CalcRelevance(const std::string_view& Str) const override;
        float CalcRelevance(const view_vector_t& Text) const override;

    private:
        float CalcWordsRelevance(const view_vector_t& Words) const;

        std::vector<std::pair<const std::string, const float>> m_Keywords;
    };

    class CParser
    {
    public:
        CParser(const ETask TaskType);
        ~CParser() = default;
        bool Init();
        bool GatherArticles(const std::vector<std::filesystem::path>& FilePathes);
        CLanguage* GetLanguagePtr(Lang::Type Type);
        void merge(CParser* Other);
        void GetReport(std::string& retReport) const;

    protected:
        bool ExploreArticle(const std::filesystem::path& Path);
        ETask GetTaskType() const;
        //Reporters
        void PushLanguagesReport     (std::string& retReport) const;
        void PushNewsReport          (std::string& retReport) const;
        void PushNotImplementedReport(std::string& retReport) const;

    private:
        std::vector<CLanguage>            m_Languages;
        ETask m_Task;

#ifdef TG_DEBUG_INFO
    public:
        void dbgTryReportLowTitles();
        void dbgTryDumpLowArticles();
    private:
        void dbgPushTitlesReport(std::string& retReport, const std::list<std::pair<float, CArticle*>>& Articles);
        void dbgDumpArticles(const std::filesystem::path& DestDir, const std::list<std::pair<float, CArticle*>>& Articles);
#endif //TG_DEBUG_INFO
    };

} //namespace tg

#endif // TGPARSER_H
