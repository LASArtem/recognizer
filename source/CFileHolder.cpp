//******************************
//  This code developed for Telegram Data clustering Contest 2020 by Alexey. N. Gorobets.
//  Lilolik.work@gmail.com
//******************************

#include "CFileHolder.h"
#include "tgtypes.h"
#include <fstream>

tg::CFileHolder::CFileHolder() :
    m_FileName(),
    m_FilePath(),
    m_pFile   (nullptr)
{
    //empty
}

tg::CFileHolder::~CFileHolder()
{
    delete m_pFile;
}

bool tg::CFileHolder::LoadFile(const fs::path& fileName)
{
    std::ifstream file(fileName, std::ios_base::in);
	if (!file.is_open())
	{
	    DEBUG_LOG("Warn: Can't open file: \""<<fileName.relative_path().string()<<"\"");
        return false;
	}

    DEBUG_ASSERT(nullptr == m_pFile && "Double Loading is forbidden");
	std::string* pFile = new std::string;
	pFile->assign((std::istreambuf_iterator<char>(file)),    //#TODO const char* + megachunk with NULL delimeter from CParser
                   std::istreambuf_iterator<char>());

    m_pFile    = pFile;
	m_FileName = fileName.filename().string();
	m_FilePath = fileName.relative_path().string();
	file.close();

    return true;
}

std::string_view tg::CFileHolder::GetFileView() const
{
    DEBUG_ASSERT(m_pFile && "CFileHolder does not initialized");
    return std::string_view(*m_pFile);
}

const std::string& tg::CFileHolder::GetFileName() const
{
    return m_FileName;
}

const std::string& tg::CFileHolder::GetRelativePath() const
{
    return m_FilePath;
}

