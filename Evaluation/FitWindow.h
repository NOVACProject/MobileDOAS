#pragma once

#include <SpectralEvaluation/Evaluation/ReferenceFile.h>

#ifndef _FITWINDOW_H
#define _FITWINDOW_H

// The maximum number of references that we can use in fitting
#define MAX_N_REFERENCES 10

namespace Evaluation
{

    /** The fit-type defines how the fit should be done;
        FIT_HP_DIV - The smooth features of the spectra are
                     removed by a binomial high-pass filtering
                     using 500 iterations of the spectra.
                     The reference-files that are used should be
                     high-pass filtered before starting the
                     measurements. Each spectrum is divided by the
                     sky-spectrum.

        FIT_HP_SUB - Same as FIT_HP_DIV except for that the (log of the)
                     sky-spectrum is included in the DOAS-fit, thereby
                     allowing for shifts between the sky spectrum
                     and the measured spectra.

        FIT_POLY -   The smooth features of the spectra are removed
                     by including a polynomial into the DOAS fit.
                     No filtering shold be done on the references
                     before running the measurements.
    */
    enum FIT_TYPE { FIT_HP_DIV, FIT_HP_SUB, FIT_POLY };


    /** The class <b>CFitWindow</b> is used to store the settings
        for the DOAS - fits that we should perform on the collected
        spectra. This includes properties such as the range of pixels
        to use in the fit, the reference-files that we should fit
        but also other properties such as which pixels should
        be used to calculate the offset. */
    // TODO: Replace this class with the CFitWindow from SpectralEvaluation
    class CFitWindow
    {
    public:
        CFitWindow(void);
        ~CFitWindow(void);

        /** Resets the data to their default values */
        void Clear();

        /** The lower edge of the fit window (in pixels) */
        int fitLow;

        /** The upper edge of the fit window (in pixels) */
        int fitHigh;

        /** The channel which is used in this fit window */
        int channel;

        /** The reference files to use */
        novac::CReferenceFile  ref[MAX_N_REFERENCES];

        /** The number of references to use */
        int nRef;

        /** The order of the polynomial that will also be fitted */
        int polyOrder;

        /** The length of the spectra */
        int specLength;

        /** The name of the fit window */
        CString name;

        /** The type of fit */
        FIT_TYPE fitType;

        /** True if the spectra are read out in an interlaced way */
        BOOL interlaced;

        /** The offset should be removed between 'offsetFrom' and 'offsetTo' */
        int offsetFrom;
        int offsetTo;

        /** Assignment operator, for convenience... */
        CFitWindow& operator=(const CFitWindow& w2);
    };
}

#endif