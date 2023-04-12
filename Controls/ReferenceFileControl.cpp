#undef min
#undef max

#include "StdAfx.h"
#include "..\DMSpec.h"
#include "referencefilecontrol.h"
#include <SpectralEvaluation/StringUtils.h>
#include <algorithm>

using namespace Evaluation;
using namespace DlgControls;

CReferenceFileControl::CReferenceFileControl(void)
{
    m_window = nullptr;
    parent = nullptr;
}

CReferenceFileControl::~CReferenceFileControl(void)
{
    m_window = nullptr;
    parent = nullptr;
}

BEGIN_MESSAGE_MAP(CReferenceFileControl, CGridCtrl)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

/* Called when the user has edited one cell */
void CReferenceFileControl::OnEndEditCell(int nRow, int nCol, CString str) {
    CGridCtrl::OnEndEditCell(nRow, nCol, str);

    int index = nRow - 1;
    if (index < 0 || index > MAX_N_REFERENCES)
        return; // TODO - add a message

    // A handle to the reference file
    novac::CReferenceFile& ref = m_window->ref[index];

    // If the name was changed
    if (nCol == 0) {
        ref.m_specieName = std::string((LPCSTR)str);
    }

    // If the path was changed
    if (nCol == 1) {
        if (strlen(str) != 0) {
            FILE* f = fopen(str, "r");
            if (f != 0) {
                ref.m_path = std::string((LPCSTR)str);
                m_window->nRef = std::max(m_window->nRef, index + 1);    // update the number of references, if necessary
                fclose(f);
            }
            else {
                MessageBox("Cannot read reference file", "Error", MB_OK);
                return;
            }
        }
    }

    // If we're editing the shift and squeeze of an unknown reference, quit
    if (ref.m_specieName.size() == 0 && ref.m_path.size() == 0)
        return;

    // If the shift was changed
    if (nCol == 2) {
        ParseShiftOption(ref.m_shiftOption, ref.m_shiftValue, str);
        switch (ref.m_shiftOption) {
        case novac::SHIFT_TYPE::SHIFT_FREE:    SetItemTextFmt(nRow, 2, "free"); break;
        case novac::SHIFT_TYPE::SHIFT_FIX:     SetItemTextFmt(nRow, 2, "fix to %.2lf", ref.m_shiftValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:    SetItemTextFmt(nRow, 2, "link to %s", m_window->ref[(int)ref.m_shiftValue]); break;
        case novac::SHIFT_TYPE::SHIFT_OPTIMAL: SetItemTextFmt(nRow, 2, "find optimal"); break;
        }
    }

    // If the squeeze was changed
    if (nCol == 3) {
        ParseShiftOption(ref.m_squeezeOption, ref.m_squeezeValue, str);
        switch (ref.m_squeezeOption) {
        case novac::SHIFT_TYPE::SHIFT_FREE:    SetItemTextFmt(nRow, 2, "free"); break;
        case novac::SHIFT_TYPE::SHIFT_FIX:     SetItemTextFmt(nRow, 2, "fix to %.2lf", ref.m_squeezeValue); break;
        case novac::SHIFT_TYPE::SHIFT_LINK:    SetItemTextFmt(nRow, 2, "link to %s", m_window->ref[(int)ref.m_squeezeValue]); break;
        case novac::SHIFT_TYPE::SHIFT_OPTIMAL: SetItemTextFmt(nRow, 2, "find optimal"); break;
        }
    }

    // if this is the last line in the grid, add one more line
    if (nRow == GetRowCount() - 1 && GetRowCount() < MAX_N_REFERENCES + 1) {
        if (strlen(str) > 0)
            SetRowCount(GetRowCount() + 1);
    }

    return;
}

void CReferenceFileControl::ParseShiftOption(novac::SHIFT_TYPE& option, double& value, CString& str) {
    char tmpStr[512];
    char txt[512];
    str.MakeLower();
    sprintf(txt, "%s", (LPCTSTR)str);
    char* pt = 0;

    // 1. Shift Fixed 
    if ((pt = strstr(txt, "fix to")) || (pt = strstr(txt, "fixed to")) || (pt = strstr(txt, "set to"))) {
        option = novac::SHIFT_TYPE::SHIFT_FIX;
        if (0 == sscanf(pt, "%511s to %lf", &tmpStr, &value)) {
            value = 0;
        }
        return;
    }

    // 2. Shift Linked
    if ((pt = strstr(txt, "link to")) || (pt = strstr(txt, "linked to"))) {
        option = novac::SHIFT_TYPE::SHIFT_LINK;
        int n = sscanf(pt, "%511s to %lf", &tmpStr, &value);
        if (n == 0) {
            char nameC[512];
            n = sscanf(pt, "%511s to %511s", &tmpStr, &nameC);
            if (n != 0) {
                const std::string name = std::string(nameC);
                for (int k = 0; k < m_window->nRef; ++k) {
                    if (EqualsIgnoringCase(m_window->ref[k].m_specieName, name)) {
                        value = k;
                        return;
                    }
                }
            }
        }
    }

    // 3. Shift free
    if (pt = strstr(txt, "free")) {
        option = novac::SHIFT_TYPE::SHIFT_FREE;
        return;
    }

    // 4. We want to find the optimal shift and squeeze, a speciality for the ReEvaluation
    if (pt = strstr(txt, "find optimal")) {
        option = novac::SHIFT_TYPE::SHIFT_OPTIMAL;
        return;
    }
}

void CReferenceFileControl::OnContextMenu(CWnd* pWnd, CPoint point) {

    if (this->parent == nullptr)
        return;

    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_REEVAL_CONTEXTMENU));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    CCellRange cellRange = GetSelectedCellRange();
    int nRows = cellRange.GetRowSpan();

    if (nRows <= 0) { /* nothing selected*/
        pPopup->EnableMenuItem(ID__REMOVE, MF_DISABLED | MF_GRAYED);
    }
    if (m_window->nRef == 0)
        pPopup->EnableMenuItem(ID__REMOVE, MF_DISABLED | MF_GRAYED);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, parent);
}
