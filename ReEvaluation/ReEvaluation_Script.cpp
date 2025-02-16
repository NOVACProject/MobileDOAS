#include "stdafx.h"
#include "reevaluation_script.h"
#include "ReEvalScriptFileHandler.h"
#include "ReEvalSettingsFileHandler.h"
#include "ReEval_DoEvaluationDlg.h"

using namespace ReEvaluation;

CReEvaluation_Script::CReEvaluation_Script(void)
{
    Clear();
}

CReEvaluation_Script::~CReEvaluation_Script(void)
{
}

void CReEvaluation_Script::Clear()
{
    m_jobs.RemoveAll();
    m_maxThreadNum = 1;
}

/** Writes this script to the given file */
int CReEvaluation_Script::WriteToFile(const CString& fileName)
{
    FileHandler::CReEvalScriptFileHandler writer;
    return writer.WriteToFile(fileName, *this);
}

/** Reads in a script from a file */
int CReEvaluation_Script::ReadFromFile(const CString& fileName)
{
    FileHandler::CReEvalScriptFileHandler reader;
    return reader.ReadFromFile(fileName, *this);
}

/** Runs this script */
int CReEvaluation_Script::Run(CWnd* wnd)
{
    ReEvaluation::CReEvaluator* reeval = new ReEvaluation::CReEvaluator();
    FileHandler::CReEvalSettingsFileHandler settingsReader;
    CReEval_DoEvaluationDlg* pView = (CReEval_DoEvaluationDlg*)wnd;

    // Loop through each of the jobs in the list
    POSITION pos = m_jobs.GetHeadPosition();
    while (pos != nullptr)
    {
        Job& j = m_jobs.GetNext(pos);

        // Setup the re-evaluator
        if (settingsReader.ParseFile(reeval->m_settings, j.settingsFile))
        {
            // problems!!!
            continue;
        }
        reeval->m_evalLogFileName.Format(j.evaluationLog);

        // Read in the evaluation-log file and make sure that it's ok
        if (reeval->ReadEvaluationLog())
        {
            MessageBox(NULL, "Could not read evaluation log file. Continuing with the next", "Error", MB_OK);
            continue;
        }
        // Extract the directory
        int p = j.evaluationLog.ReverseFind('\\');
        if (p != -1)
            reeval->m_specFileDir.Format(j.evaluationLog.Left(p));
        else
            reeval->m_specFileDir.Format(j.evaluationLog);

        // run the evaluation
        if (wnd != nullptr)
        {
            pView->m_reeval = reeval;
            pView->OnBnClickedDoEvaluation();

            // make a small pause
            Sleep(5000);

            // wait for the processing thread to finish...
            while (reeval->fRun)
            {
                Sleep(500);
            }
        }
        else
        {
            // Set the output window
            reeval->m_mainView = wnd;

            // run the evaluation
            reeval->fRun = true;
            reeval->DoEvaluation();
        }
    }

    return 0;
}
