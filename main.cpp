//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include <iostream>
#include "tgnews_mgr.h"

void showUsage()
{
    std::cout<<"Unexpected command, available:\n\
    languages source_dir\n\
    news source_dir\n\
    categories source_dir\n\
    threads source_dir\n\
    top source_dir\n\
    server source_dir"<<std::endl;
}

int main(int argc, char *argv[])
{
    std::ios::sync_with_stdio(false);
    std::string path;
    if (argc != 3)
    {
        showUsage();
        return 0;
    }
    else
        path = argv[2];

    if (std::string("languages") == argv[1])
        tg::CNewsMgr::GetInstance()->DoWork(path, tg::ETask::LANGUAGES);
    else if (std::string("news") == argv[1])                                  //#TODO tg::ETask::FromString
        tg::CNewsMgr::GetInstance()->DoWork(path, tg::ETask::NEWS);
    else if (std::string("categories") == argv[1])
        tg::CNewsMgr::GetInstance()->runCategories(path);
    else if (std::string("threads") == argv[1])
        tg::CNewsMgr::GetInstance()->runThreads(path);
    else if (std::string("top") == argv[1])
        tg::CNewsMgr::GetInstance()->runTop(path);
    else if (std::string("server") == argv[1])
        std::cout<<"Not implemented yet"<<std::endl;
    else
        showUsage();

    return 0;
}
