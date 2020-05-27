//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef TGNEWS_MGR_H
#define TGNEWS_MGR_H

#include "CFileHolder.h"
#include "CNode.h"

namespace tg
{
    class CNewsMgr : //Interface, multithread stuff
        public CFileHolder
    {
    public:
        CNewsMgr() = default;
        ~CNewsMgr() override = default;
        static CNewsMgr* GetInstance();
        static bool IsLowMemory();

        const CNode& GetConfig();
        bool GetPathesByExtension(std::vector<fs::path>& retFilePathes, const std::string& relDirPath = ".", const std::string& ext = ".html");
        bool GetDirFileNames(strvector_t& retFileNames, const std::string& relDirPath = ".");

        bool DoWork  (std::string path, ETask task);
        bool runCategories (std::string path);
        bool runThreads    (std::string path);
        bool runTop        (std::string path);

    protected:

    private:
        void SplitPathes(std::vector<fs::path>& main, std::vector<std::vector<fs::path>>& retPathes, size_t count);
        size_t GetThreadCount();

        static const std::string ConfigPath;
        static CNewsMgr* m_pInstance;
        static bool LowMemoryMode;

        CNode m_Config;
    };

} //namespace tg

#endif // TGNEWS_MGR_H
