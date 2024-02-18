#pragma once

#include <afxtempl.h>
#include "ReEvaluationSettings.h"

namespace ReEvaluation
{
class CReEvaluation_Script
{
public:
    CReEvaluation_Script(void);
    ~CReEvaluation_Script(void);

    typedef struct Job
    {
        // The evaluation log file that should be processed
        CString evaluationLog;

        // The file containing the settings for the re-evaluation
        CString settingsFile;
    } Job;

    // ----------------------------------------------------
    // --------------- PUBLIC DATA ------------------------
    // ----------------------------------------------------

    /** The jobs that are to be done in this script */
    CList <Job, Job&> m_jobs;

    /** The maximum number of threads that we should spawn */
    long m_maxThreadNum;

    // ----------------------------------------------------
    // --------------- PUBLIC METHODS ---------------------
    // ----------------------------------------------------

    /** Empties this script */
    void Clear();

    /** Writes this script to the given file
        @return 0 on success */
    int WriteToFile(const CString& fileName);

    /** Reads in a script from a file
        @return 0 on success */
    int ReadFromFile(const CString& fileName);

    /** Runs this script
        @return 0 on success */
    int Run(CWnd* wnd = nullptr);

};
}