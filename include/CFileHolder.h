//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#ifndef CFILEHOLDER_H
#define CFILEHOLDER_H

#include <filesystem>

namespace tg{

    namespace fs = std::filesystem;

    class CFileHolder
    {
    public:
        CFileHolder();
        virtual ~CFileHolder();
        bool LoadFile(const fs::path& fileName);
        std::string_view   GetFileView()     const;
        const std::string& GetFileName()     const;
        const std::string& GetRelativePath() const;

    private:
        std::string        m_FileName;
        std::string        m_FilePath;
        const std::string* m_pFile;
    };

} //namespace tg
#endif // CFILEHOLDER_H
