#include "StdAfx.h"
#include "XMLFileReader.h"
#include "../Common.h"
#include <SpectralEvaluation/StringUtils.h>

using namespace FileHandler;

CXMLFileReader::CXMLFileReader(void)
{
    this->m_File = nullptr;
    this->nLinesRead = 0;
    this->szToken = nullptr;
}

CXMLFileReader::~CXMLFileReader(void)
{
}

char* CXMLFileReader::NextToken() {
    char separators[] = "<>\t";
    static char* pt = nullptr;
    szToken = nullptr;

    if (nLinesRead == 0) {
        // if this is the first call to this function
        m_File->ReadString(szLine, 4095);
        szToken = (char*)(LPCSTR)szLine;

        pt = strtok(szToken, separators);
        ++nLinesRead;
        return pt;
    }
    else {
        // this is not the first call to this function
        if (pt = strtok(szToken, separators))
            return pt;

        szLine[0] = 0;
        while (0 == strlen(szLine)) {
            if (!m_File->ReadString(szLine, 4095))
                return NULL;
        }

        szToken = (char*)(LPCSTR)szLine;

        pt = strtok(szToken, separators);
        ++nLinesRead;
    }
    return pt;
}

int CXMLFileReader::Parse_StringItem(const CString& label, CString& string) {
    while (szToken = NextToken()) {

        if (Equals(szToken, label))
            return 1;

        string.Format(szToken);
    }

    return 0;
}

int CXMLFileReader::Parse_StringItem(const std::string& label, std::string& string) {
    while (szToken = NextToken()) {

        if (EqualsIgnoringCase(szToken, label))
            return 1;

        string = std::string(szToken);
    }

    return 0;
}

int CXMLFileReader::Parse_LongItem(const CString& label, long& number) {

    while (szToken = NextToken()) {

        if (Equals(szToken, label))
            return 1;

        number = _tstol(szToken);
    }

    return 0;
}
/** General parsing of a single, simple float item */
int CXMLFileReader::Parse_FloatItem(const CString& label, double& number) {
    while (szToken = NextToken()) {

        if (Equals(szToken, label))
            return 1;

        number = _tstof(szToken);
    }

    return 0;
}

/** General parsing of a single, simple integer item */
int CXMLFileReader::Parse_IntItem(const CString& label, int& number) {
    while (szToken = NextToken()) {

        if (Equals(szToken, label))
            return 1;

        number = _tstoi(szToken);
    }

    return 0;
}

