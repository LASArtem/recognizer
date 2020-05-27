//******************************
//  This code developed for Telegram Data clustering Contest 20209 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "tgparser.h"
#include "CArticle.h"
#include "tgnews_mgr.h"
#include "tools.h"
#include <algorithm>

//*************************
//  CArticleSet
//*************************
tg::CArticleSet::CArticleSet() :
    m_MinRelevance   (0.f),
    m_EnoughRelevance(0.f),
    m_Articles       ()
{
    //empty
}

tg::CArticleSet::CArticleSet(const CArticleSet& Source) :
    m_MinRelevance   (Source.GetMinRelevance()),
    m_EnoughRelevance(Source.GetEnoughRelevance()),
    m_Articles       (Source.GetArticles())
{
    //empty
}

bool tg::CArticleSet::IsValid() const
{
    return 0 < m_MinRelevance
        && 0 < m_EnoughRelevance;
}

float tg::CArticleSet::CalcRelevance(CArticle& Article, const EDepth Depth) const
{
    EDepth knownDepth = EDepth::UNDEF;
    float relevance = GetArticleRelevance(Article, knownDepth);
    if (knownDepth >= Depth)
        return relevance;
    else if (1 < (Depth - knownDepth))
        relevance = CalcRelevance(Article, ++knownDepth);   //#TODO check recursion (Depth = 100500 - probably Locks)

    // Calc Next Depth
    std::string_view source;
    float newPartRelevance = -1.f;  //Missing one of EDepth info in Article leads to account previous twice;
    switch(Depth)                   //missing first few EDepth infos stores as -1.f relevance
    {
        case EDepth::TITLE:
        {
            source = Article.GetTitle();
        }
        case EDepth::DESC:
        {
            source = source.empty() ? Article.GetDescription() : source;
            newPartRelevance = CalcRelevance(source);
            break;
        }
        case EDepth::BODY:
        {
            DEBUG_ACTION(Statistic::OnHitCase("Read Body"));
            view_vector_t bodyText;
            Article.GetBody(bodyText);
            newPartRelevance = CalcRelevance(bodyText);
            break;
        }
        default:
        {
            DEBUG_ASSERT(false && "Unexpected EDepth");
            return 0.f;
        }
    }

    if (newPartRelevance >= 0.f)    //newPartRelevance = -1 if part does not exist in current Article
    {
        if (relevance < 0)
            relevance = newPartRelevance;
        else
        {
            relevance *= static_cast<float>(knownDepth);
            relevance += newPartRelevance;
            relevance /= static_cast<float>(Depth);         //mean of all read parts
        }
    }

    SetArticleRelevance(Article, Depth, relevance);
    return relevance;
}

bool tg::CArticleSet::TryPushArticle(CArticle* pArticle) //Assumed that Article relevance already calculated
{
    DEBUG_ASSERT(nullptr != pArticle && "Invalid Article");
    EDepth depth = EDepth::UNDEF;
    float relevance = GetArticleRelevance(*pArticle, depth);
    if (relevance < m_MinRelevance)
    {
        DEBUG_ASSERT(EDepth::BODY == depth && "The Article hasn't been explored enough");
        return false;
    }

    pArticle->ClearCache();
    m_Articles.emplace_back(relevance, pArticle);
    return true;
}

void tg::CArticleSet::MakeReady()
{
    m_Articles.sort(&CArticleSet::RelevancePred);
}

void tg::CArticleSet::merge(CArticleSet* Other)
{
    m_Articles.merge(Other->GetArticles(), &CArticleSet::RelevancePred);
}

size_t tg::CArticleSet::size() const
{
    return m_Articles.size();
}

const std::list<std::pair<float, tg::CArticle*>>& tg::CArticleSet::GetArticles() const
{
    return m_Articles;
}

void tg::CArticleSet::PushReport(std::string& retReport, const std::string& SpapcesPrefix/*=""*/) const
{
    retReport.reserve(retReport.size() + Consts::FileNameSizeExpected * GetArticles().size());
    retReport += ("\n" + SpapcesPrefix + "\"articles\": [\n");

    const std::pair<float, CArticle*>* pLast = &GetArticles().back();
    for (const std::pair<float, CArticle*>& relArticle : GetArticles())
    {
        retReport += (SpapcesPrefix + "  \"");
        retReport += relArticle.second->GetFileName();
        DEBUG_ACTION(retReport += " = ");
        DEBUG_ACTION(retReport += std::to_string(relArticle.first));
        retReport += &relArticle != pLast ? "\",\n" : "\"\n";
    }
    retReport += (SpapcesPrefix + "]\n");
}

float tg::CArticleSet::GetMinRelevance() const
{
    return m_MinRelevance;
}

float tg::CArticleSet::GetEnoughRelevance() const
{
    return m_EnoughRelevance;
}

std::list<std::pair<float, tg::CArticle*>>& tg::CArticleSet::GetArticles()
{
    return m_Articles;
}

void tg::CArticleSet::SetMinRelevance(float Relevance)
{
    m_MinRelevance = Relevance;
}

void tg::CArticleSet::SetEnoughRelevance(float Relevance)
{
    m_EnoughRelevance = Relevance;
}

bool tg::CArticleSet::RelevancePred(const std::pair<float, CArticle*>& lhs, const std::pair<float, CArticle*>& rhs)
{
    return lhs.first > rhs.first;
}

//*************************
//  CLanguage
//*************************
tg::CLanguage::CLanguage() :
    m_Type (Lang::Type::UNDEF),
    m_pNews(nullptr)
{
    //empty
}

tg::CLanguage::CLanguage(const CLanguage& Source) :
    CArticleSet::CArticleSet(Source),
    m_Type (Source.GetType()),
    m_pNews(Source.HasNews() ? new CNews(Source.GetNews()) : nullptr)   //#TODO std::memcpy?
{
    //empty
}

tg::CLanguage::~CLanguage() //The only Right holder to delete articles
{
    delete m_pNews;
    for (std::pair<float, CArticle*>& p : GetArticles())
        delete p.second;
}

bool tg::CLanguage::Init(const key_node_t& Source, const ETask TaskType)
{
    m_Type = Lang::FromString(Source.first);
    const std::string_view& minRelevance    = Source.second["charRate"]["min"].GetText();
    const std::string_view& enoughRelevance = Source.second["charRate"]["enough"].GetText();
    SetMinRelevance(   tg::swtof(minRelevance));
    SetEnoughRelevance(tg::swtof(enoughRelevance));

    if (ETask::NEWS <= TaskType)
    {
        CNode newsNode = Source.second["newsInfo"];
        CNews* pNews = new CNews;
        if (!pNews->Init(std::make_pair("newsInfo", newsNode), TaskType))
        {
            delete pNews;
            return false;
        }
        m_pNews = pNews;
    }

    return IsValid();
}

bool tg::CLanguage::IsValid() const
{
    return CArticleSet::IsValid()
        && Lang::Type::UNDEF != m_Type
        && (!HasNews() || m_pNews->IsValid());
}

float tg::CLanguage::CalcRelevance(CArticle& Article, EDepth Depth) const
{
    return CArticleSet::CalcRelevance(Article, Depth);  //#TODO words comparison in border cases
}

bool tg::CLanguage::TryPushArticle(CArticle* pArticle) //Assumed that Article Language relevance already calculated
{
    if (!CArticleSet::TryPushArticle(pArticle))
    {
        delete pArticle;
        return false;
    }
    return !HasNews() || GetNewsPtr()->TryPushArticle(pArticle);
}

void tg::CLanguage::MakeReady()
{
    if (HasNews())
        GetNewsPtr()->MakeReady();
    CArticleSet::MakeReady();
}

void tg::CLanguage::merge(CLanguage* Other)
{
    if (HasNews())
        GetNewsPtr()->merge(Other->GetNewsPtr());

    CArticleSet::merge(Other);
}

void tg::CLanguage::PushReport(std::string& retReport) const
{
    std::string extraSpapces = "  ";
    retReport += "    \"lang_code\": \"";
    retReport += Lang::ToString(m_Type);
    retReport += "\",";
    extraSpapces += "  ";

    CArticleSet::PushReport(retReport, extraSpapces);
}

tg::Lang::Type tg::CLanguage::GetType() const
{
    return m_Type;
}

const tg::CNews& tg::CLanguage::GetNews() const
{
    return *m_pNews;
}

bool tg::CLanguage::HasNews() const
{
    return m_pNews != nullptr;
}

tg::CNews* tg::CLanguage::GetNewsPtr()
{
    return m_pNews;
}

void tg::CLanguage::SetArticleRelevance(CArticle& Article, const EDepth Depth, float Relevance) const
{
    Article.SetLangRelevance(GetType(), Depth, Relevance);
}

float tg::CLanguage::GetArticleRelevance(const CArticle& Article, EDepth& retDepth) const
{
    return Article.GetLangRelevance(GetType(), retDepth);
}

float tg::CLanguage::CalcRelevance(const std::string_view& Str) const
{
    if (Str.empty())
        return -1.f;

    return static_cast<float>(Lang::CalcCharHits(Str, GetType())) / static_cast<float>(Str.size());
}

float tg::CLanguage::CalcRelevance(const view_vector_t& Text) const
{
    size_t totalCount  = 0;
    size_t relateCount = 0;
    for(const std::string_view& s : Text)
    {
        totalCount  += s.size();
        relateCount += Lang::CalcCharHits(s, GetType());
    }
    if (0 == totalCount)
        return -1.f;

    return static_cast<float>(relateCount) / static_cast<float>(totalCount);
}

//*************************
//  CNews
//*************************
tg::CNews::CNews() :
    m_Keywords()
{
    //empty
}

tg::CNews::CNews(const CNews& Source) :
    CArticleSet::CArticleSet(Source),
    m_Keywords(Source.m_Keywords)
{
    //empty
}

bool tg::CNews::Init(const key_node_t& Source, const ETask TaskType)
{
    const std::string_view& minRelevance    = Source.second["relevance"]["min"].GetText();
    const std::string_view& enoughRelevance = Source.second["relevance"]["enough"].GetText();
    SetMinRelevance(   tg::swtof(minRelevance));
    SetEnoughRelevance(tg::swtof(enoughRelevance));

    const key_node_map& keyWords = Source.second["keywords"].GetSubTags();
    for(const key_node_t& wordWeight : keyWords)
    {
        if (!wordWeight.first.empty() && !wordWeight.second.GetText().empty())
            m_Keywords.emplace_back(wordWeight.first, tg::swtof(wordWeight.second.GetText()));
    }

    if (ETask::CATEGORIES <= TaskType)   //#TODO
        return false;

    return IsValid();
}

bool tg::CNews::IsValid() const
{
    return CArticleSet::IsValid()
        && !m_Keywords.empty();
}

float tg::CNews::CalcRelevance(CArticle& Article, EDepth Depth) const
{
    if (EDepth::TITLE < Depth)
        return CArticleSet::CalcRelevance(Article, Depth);

    std::vector<float> subHeads = {
        CalcRelevance(Article.GetSiteName()),
        CalcRelevance(Article.GetUrl()),
        CalcRelevance(Article.GetTitle())
    };

    size_t subHeadsCount = 0;
    float retRelevance = 0.f;
    for (float rel : subHeads)
    {
        if (rel >= 0.f)         //rel = -1 if subHead does not exist in current Article
        {
            ++subHeadsCount;
            retRelevance += rel;
        }
    }
    retRelevance = subHeadsCount > 0 ? retRelevance / static_cast<float>(subHeadsCount) : -1.f;

    Article.SetNewsRelevance(EDepth::TITLE, retRelevance);
    return retRelevance;
}

bool tg::CNews::TryPushArticle(CArticle* pArticle)
{
    for (EDepth depth = EDepth::TITLE;
         depth < EDepth::COUNT; ++depth)
    {
        if (CalcRelevance(*pArticle, depth) > GetMinRelevance())
            return CArticleSet::TryPushArticle(pArticle);
    }
    return false;       //#TODO CATEGORIES
}

void tg::CNews::PushReport(std::string& retReport) const
{
    retReport.reserve(retReport.size() + Consts::FileNameSizeExpected * GetArticles().size());
    std::string extraSpapces = "  ";

    CArticleSet::PushReport(retReport, extraSpapces);
}

void tg::CNews::SetArticleRelevance(CArticle& Article, const EDepth Depth, float Relevance) const
{
    Article.SetNewsRelevance(Depth, Relevance);
}

float tg::CNews::GetArticleRelevance(const CArticle& Article, EDepth& retDepth) const
{
    return Article.GetNewsRelevance(retDepth);
}

float tg::CNews::CalcRelevance(const std::string_view& Str) const
{
    view_vector_t words;
    tg::split_words(Str, words);
    return CalcWordsRelevance(words);
}

float tg::CNews::CalcRelevance(const view_vector_t& Text) const
{
    view_vector_t words;
    tg::split_words(Text, words);
    return CalcWordsRelevance(words);
}

float tg::CNews::CalcWordsRelevance(const view_vector_t& Words) const
{
    if (Words.empty())
        return -1.f;

    float totalRelevance = 0.f;
    for (const std::string_view& word : Words)            //#TODO try binded conditionChain
    {
        for (const std::pair<std::string, float>& key : m_Keywords)
            if (tg::begins(word, key.first))
                totalRelevance += key.second;
    }

    return totalRelevance / Words.size();
}

//*************************
//  CParser
//*************************
tg::CParser::CParser(ETask TaskType) :
    m_Languages(),
    m_Task(TaskType)
{
    DEBUG_ASSERT(ETask::LANGUAGES <= TaskType && TaskType < ETask::COUNT && "Unexpected Task type");
    Init();
}

bool tg::CParser::Init()
{
    const CNode& config = CNewsMgr::GetInstance()->GetConfig();
    const CNode& langs = config["langConfig"];
    if (langs.empty())
        return false;

    m_Languages.clear();    //languages
    m_Languages.reserve(langs.size());
    for(const key_node_t& node : langs.GetSubTags())
    {
        m_Languages.emplace_back();
        if (!m_Languages.back().Init(node, GetTaskType()))
        {
            DEBUG_LOG("Language does not initialized");
            return false;
        }
    }

    return !m_Languages.empty();
}

bool tg::CParser::GatherArticles(const std::vector<fs::path>& FilePathes)
{
    if (m_Languages.empty() || !m_Languages[0].IsValid())
        return false;

    DEBUG_ACTION(Statistic::InitCountTrack("Info: reading", FilePathes.size(), 1000, false));
    for (std::vector<fs::path>::const_iterator pathIt = FilePathes.begin(); pathIt != FilePathes.end(); ++pathIt)
        ExploreArticle(*pathIt);

    for (CLanguage& lang : m_Languages)
        lang.MakeReady();

    if (ETask::NEWS == GetTaskType()) //Store news from all languages by relevance
        for (size_t i = 1; i < m_Languages.size(); ++i)
            m_Languages[0].GetNewsPtr()->merge(m_Languages[i].GetNewsPtr());

    return true;
}

tg::CLanguage* tg::CParser::GetLanguagePtr(Lang::Type Type)
{
    std::vector<CLanguage>::iterator pFound = std::find_if(m_Languages.begin(), m_Languages.end(),
        [Type](CLanguage& lang) -> bool
        {
            return lang.GetType() == Type;
        });
    if (pFound !=  m_Languages.end())
        return &*pFound;
    return nullptr;
}

void tg::CParser::merge(CParser* Other)
{
    for(CLanguage& lang : m_Languages)
    {
        CLanguage* pOtherLang = Other->GetLanguagePtr(lang.GetType());
        if (pOtherLang)
            lang.merge(pOtherLang);
    }
}

void tg::CParser::GetReport(std::string& retReport) const
{
    retReport.clear();
    retReport.reserve(Consts::FileNameSizeExpected);

    switch(GetTaskType())
    {
        case ETask::LANGUAGES:
            PushLanguagesReport(retReport);
            return;
        case ETask::NEWS:
            PushNewsReport(retReport);
            return;
        case ETask::CATEGORIES:
            PushNotImplementedReport(retReport);
            return;
        default:
            PushNotImplementedReport(retReport);
    }
}

bool tg::CParser::ExploreArticle(const fs::path& Path)
{
    DEBUG_ACTION(Statistic::OnHitCase("Info: reading", false));
    CArticle* pCurArticle = new CArticle;
    if (!pCurArticle->InitFromHtml(Path))
    {
        delete pCurArticle;
        return false;
    }

    CLanguage* pBestLang = nullptr;
    std::vector<CLanguage*> pLanguages;
    pLanguages.reserve(m_Languages.size());
    std::for_each(m_Languages.begin(), m_Languages.end(),
        [&pLanguages](CLanguage& lang){pLanguages.push_back(&lang);});

    float bestLangRelevance = 0.f;
    for (EDepth depth = EDepth::TITLE;
         depth < EDepth::COUNT; ++depth)
    {
        for (std::vector<CLanguage*>::iterator it = pLanguages.begin();
             it != pLanguages.end(); ++it)
        {
            CLanguage* lang = *it;
            float relevance = lang->CalcRelevance(*pCurArticle, depth);

            if (relevance >= lang->GetEnoughRelevance())    //Language found, because  high enough relevance
                return lang->TryPushArticle(pCurArticle);
            else if (relevance < lang->GetMinRelevance()    //If cur depth has less then min relevance ignore this Language
                     && relevance >= 0.f)
            {
                pLanguages.erase(it--);
                if (pLanguages.empty())
                {
                    delete pCurArticle;
                    return false;
                }

            }
            else if (relevance >= bestLangRelevance)        //Suitable Language still not chosen
            {
                bestLangRelevance = relevance;
                pBestLang = lang;
            }
        }
    }
    if (pBestLang)
        return pBestLang->TryPushArticle(pCurArticle);

    delete pCurArticle;
    return false;
}

tg::ETask tg::CParser::GetTaskType() const
{
    return m_Task;
}

void tg::CParser::PushLanguagesReport(std::string& retReport) const
{
    const CLanguage* pLast = &m_Languages.back();
    retReport += "[\n";
    for(const CLanguage& lang : m_Languages)
    {
        retReport += "  {\n";
        lang.PushReport(retReport);
        retReport += &lang != pLast ? "  },\n" : "  }\n";
    }
    retReport += "]";
}

void tg::CParser::PushNewsReport(std::string& retReport) const
{
    retReport += "{";
    m_Languages[0].GetNews().PushReport(retReport);
    retReport += "}";
}

void tg::CParser::PushNotImplementedReport(std::string& retReport) const
{
    retReport += "Not implemented yet";
}

#ifdef TG_DEBUG_INFO
void tg::CParser::dbgTryReportLowTitles()
{
    for(const CLanguage& lang : m_Languages)
    {
        std::string report;
        if (ETask::LANGUAGES == GetTaskType())
        {
            dbgPushTitlesReport(report,  lang.GetArticles());
            std::string fileName = Lang::ToString(lang.GetType()) + std::string(".txt");
            WriteFile(fileName, report);
        }
        else if (ETask::NEWS == GetTaskType())
        {
            dbgPushTitlesReport(report,  lang.GetNews().GetArticles());
            WriteFile("news.txt", report);
            return;
        }
        else
        {
            DEBUG_LOG("ETask Not supported");   //#TODO FromString
            return;
        }
    }
}

void tg::CParser::dbgTryDumpLowArticles()
{
    const CNode& config = CNewsMgr::GetInstance()->GetConfig();
    std::string toPath = std::string(config["dbgTuning"]["lowRelevancePath"].GetText());
    if (toPath.empty())
        return;

    fs::path destinationDir = fs::path(toPath);
    fs::remove_all(destinationDir);
    fs::create_directories(destinationDir);

    for(const CLanguage& lang : m_Languages)
    {
        if (ETask::LANGUAGES == GetTaskType())
        {
            fs::path subDir = destinationDir / fs::path(Lang::ToString(lang.GetType()));
            fs::create_directories(subDir);
            dbgDumpArticles(subDir,  lang.GetArticles());
        }
        else if (ETask::NEWS == GetTaskType())
        {
            dbgDumpArticles(destinationDir,  lang.GetNews().GetArticles());
        }
        else
        {
            DEBUG_LOG("ETask Not supported");   //#TODO FromString
            return;
        }
    }
}

void tg::CParser::dbgPushTitlesReport(std::string& retReport, const std::list<std::pair<float, CArticle*>>& Articles)
{
    retReport.reserve(retReport.size() + Articles.size() * Consts::TitleSizeExprcted);
    const CNode& config = CNewsMgr::GetInstance()->GetConfig();
    std::string upToRelevance = std::string(config["dbgTuning"]["upToRelevance"].GetText());
    float topRelevance = tg::swtof(upToRelevance);

    for (const std::pair<float, CArticle*>& art : Articles)
    {
        float curRelevance = art.first;
        if (curRelevance > topRelevance)
            continue;

        retReport += std::to_string(curRelevance);
        retReport += " = ";
        retReport += art.second->GetTitle();
        retReport += " (";
        retReport += art.second->GetFileName();
        retReport += ");\n";
    }
}

void tg::CParser::dbgDumpArticles(const fs::path& DestDir, const std::list<std::pair<float, CArticle*>>& Articles)
{
    const CNode& config = CNewsMgr::GetInstance()->GetConfig();
    std::string upToRelevance = std::string(config["dbgTuning"]["upToRelevance"].GetText());
    float topRelevance = tg::swtof(upToRelevance);

    for (const std::pair<float, CArticle*>& art : Articles)
    {
        float curRelevance = art.first;
        if (curRelevance > topRelevance)
            continue;

        try
        {
            fs::copy(fs::path(art.second->GetRelativePath())
                    , DestDir / fs::path(art.second->GetFileName()));
            DEBUG_LOG("Dump: relevance = "<<curRelevance<<" file: "<<art.second->GetFileName());
        }
        catch(std::exception &ex)
        {
            DEBUG_LOG("Warn: Can't dump "<<art.second->GetFileName()<<" relevane = "<<curRelevance);
            DEBUG_LOG(" because of: "<<ex.what());
        }
    }
}
#endif //TG_DEBUG_INFO
