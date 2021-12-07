#pragma once
#include "ReEvaluator.h"
#include "../Common/XMLFileReader.h"

namespace FileHandler {

    class CReEvalSettingsFileHandler : protected FileHandler::CXMLFileReader
    {
    public:
        CReEvalSettingsFileHandler(void);
        ~CReEvalSettingsFileHandler(void);

        /** Parses a file with settings for the re-evaluation.
            @return 0 on success
         */
        int ParseFile(ReEvaluation::CReEvaluationSettings& settings, const CString& fileName);

        /** Writes a file with settings for the re-evaluation
            @return 0 on success
        */
        static int WriteFile(const ReEvaluation::CReEvaluationSettings& settings, const CString& fileName);

    protected:
        /** Starts the parsing */
        int Parse(ReEvaluation::CReEvaluationSettings& settings);

        int Parse_IgnoreDark(ReEvaluation::CReEvaluationSettings& settings);
        int Parse_IgnoreSaturated(ReEvaluation::CReEvaluationSettings& settings);
        int ParseOffset(ReEvaluation::CReEvaluationSettings& settings);
        int ParseSky(ReEvaluation::CReEvaluationSettings& settings);
        int ParseSkyColumn(ReEvaluation::CReEvaluationSettings& settings);
        int ParseSkyIntensity(ReEvaluation::CReEvaluationSettings& settings);
        int ParseDark(ReEvaluation::CReEvaluationSettings& settings);
        int ParseFitWindow(ReEvaluation::CReEvaluationSettings& settings);

        /** Parses a reference-file section */
        int	ParseReference(novac::CReferenceFile& reference);

        /** Parses a shift or squeeze section */
        int Parse_ShiftOrSqueeze(const CString& label, novac::SHIFT_TYPE& option, double& lowValue /**, double &highValue*/);

    };

}