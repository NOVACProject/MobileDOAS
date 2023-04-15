#pragma once

#include <string>

namespace mobiledoas
{
    /** ReferenceFitResult is a class to store the evaluated parameters of a
        reference file  from evaluating a spectrum */
    struct ReferenceFitResult
    {
    public:
        /** The resulting column */
        double m_column = 0.0;

        /** The error in the resulting column */
        double m_columnError = 0.0;

        /** The shift that was applied to the reference in the evaluation */
        double m_shift = 0.0;

        /** The uncertainty in the applied shift */
        double m_shiftError = 0.0;

        /** The squeeze that was applied to the reference in the evaluation */
        double m_squeeze = 0.0;

        /** The uncertainty in the applied squeeze */
        double m_squeezeError = 0.0;

        /** The name of the specie that the reference identifies */
        std::string m_specieName = "";
    };

}