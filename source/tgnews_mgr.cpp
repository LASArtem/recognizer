//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "tgnews_mgr.h"
#include "tgparser.h"
#include "tools.h"
#include <thread>
#include <iostream>

#include <algorithm>

const std::string tg::CNewsMgr::ConfigPath("config.xml");
tg::CNewsMgr* tg::CNewsMgr::m_pInstance = nullptr;
bool tg::CNewsMgr::LowMemoryMode = true;

tg::CNewsMgr* tg::CNewsMgr::GetInstance()
{
    if (!m_pInstance)
        m_pInstance = new CNewsMgr();
    return m_pInstance;
}

bool tg::CNewsMgr::IsLowMemory()
{
    return CNewsMgr::LowMemoryMode;
}

const tg::CNode& tg::CNewsMgr::GetConfig()
{
    if (!m_Config.empty())
        return m_Config;

    LoadFile(fs::path(CNewsMgr::ConfigPath));
	m_Config.InitFromView(GetFileView());
	DEBUG_ACTION(m_Config.dbgPrint());

    const std::string_view& strLowMemory = m_Config["performance"]["lowMemory"].GetText();
    CNewsMgr::LowMemoryMode = tg::swtobool(strLowMemory, true);

    return m_Config;
}

bool tg::CNewsMgr::GetPathesByExtension(std::vector<fs::path>& retFilePathes, const std::string& relDirPath /*= "."*/, const std::string& ext /*= ".html"*/)
{
    try
    {
        retFilePathes.clear();
        retFilePathes.reserve(Consts::FilesCountExpected);

        fs::recursive_directory_iterator first(relDirPath);
        fs::recursive_directory_iterator last;

        std::copy_if(first, last, std::back_inserter(retFilePathes),
            [&ext](const fs::path& path)
            {
                return fs::is_regular_file(path) && (path.extension() == ext);
            });
    }
    catch (std::exception &ex)
    {
        std::cout<<"Error: Can't gather files in directory: "<<relDirPath<<std::endl;    //#TODO multiple try
        return false;
    }

    DEBUG_LOG("Info: got "<<retFilePathes.size()<<" files to read");
    return true;
}

bool tg::CNewsMgr::GetDirFileNames(strvector_t& retFileNames, const std::string& relDirPath /*= "."*/)
{
    try
    {
        fs::directory_iterator dirIt(relDirPath);
        for (auto& p : dirIt)
            retFileNames.push_back(p.path().string());      //#TODO handle file locks
    }
    catch (std::exception &ex)
    {
        std::cout<<"Error: Can't open the directory: "<<relDirPath<<std::endl;
        return false;
    }

    return true;

}

bool tg::CNewsMgr::DoWork(std::string path, ETask task)
{
    std::vector<fs::path> mainPathes;
    if (!GetPathesByExtension(mainPathes, path))
        return false;

    std::vector<std::vector<fs::path>> threadPathes;
    SplitPathes(mainPathes, threadPathes, GetThreadCount());
    CParser commonParser(task);

    std::vector<std::thread> workers;
    workers.reserve(threadPathes.size());
    std::vector<CParser> parsers(threadPathes.size(), commonParser);

    std::vector<CParser>::iterator itParser = parsers.begin();
    for (const std::vector<fs::path>& job : threadPathes)
        workers.emplace_back(&CParser::GatherArticles, itParser++, std::ref(job));

    commonParser.GatherArticles(mainPathes);
    for(size_t i = 0; i < threadPathes.size(); ++i)
    {
        if (workers[i].joinable())
        {
            workers[i].join();
            commonParser.merge(&parsers[i]);
        }
        else
            std::cout<<"Warn: can't join thread : "<<workers[i].get_id()<<std::endl;
    }

    std::string report;
    commonParser.GetReport(report);
    std::cout<<report<<std::endl;

    DEBUG_ACTION(WriteFile("Report.json", report));
    DEBUG_ACTION(Statistic::PrintCasesHit());
    DEBUG_ACTION(commonParser.dbgTryReportLowTitles());

    return true;
}

bool tg::CNewsMgr::runCategories(std::string path)
{
    std::cout<<"Not implemented yet"<<std::endl;
    return true;
}

bool tg::CNewsMgr::runThreads(std::string path)
{
    std::cout<<"Not implemented yet"<<std::endl;
    return true;
}

bool tg::CNewsMgr::runTop(std::string path)
{
    std::cout<<"Not implemented yet"<<std::endl;
    return true;
}

void tg::CNewsMgr::SplitPathes(std::vector<fs::path>& main, std::vector<std::vector<fs::path>>& retOthers, size_t count)
{
    retOthers.clear();
    retOthers.reserve(count - 1);   //first range remains for main thread

    size_t regularStep = main.size() / count;
    std::vector<fs::path>::iterator itFrom = main.begin();
    std::advance(itFrom, regularStep);
    std::vector<fs::path>::iterator itTo = itFrom;
    for (size_t i = 1; i < count; ++i)
    {
        if (i < count - 1)
            std::advance(itTo, regularStep);
        else
            itTo = main.end();      //push rest files to last thread
        retOthers.emplace_back(itFrom, itTo);
        itFrom = itTo;
    }

    main.resize(regularStep);
}

size_t tg::CNewsMgr::GetThreadCount()
{
    const CNode& config = GetConfig();
    const std::string_view& strCount = config["performance"]["threadCount"].GetText();
    return tg::swtoul(strCount, 1);
}
