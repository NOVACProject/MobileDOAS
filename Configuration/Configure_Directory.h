#pragma once
#include <memory>
#include "MobileConfiguration.h"
#include "../resource.h"

namespace Configuration {

class 	CConfigure_Directory : public CPropertyPage
{
    DECLARE_DYNAMIC(CConfigure_Directory)

public:
    CConfigure_Directory();
    virtual ~CConfigure_Directory();

    enum { IDD = IDD_CONFIGURE_DIRECTORY };

    /** The local handle to the CMobileConfiguration object that we're changing */
    std::shared_ptr<CMobileConfiguration> m_conf;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** Called when the dialog is to be shown */
    virtual BOOL OnInitDialog();

    afx_msg void SaveData();
    BOOL OnSetActive();

public:
    afx_msg void OnBnClickedBrowseDirectory();
    afx_msg void OnBnClickedBrowseDirSky();
    afx_msg void OnBnClickedBrowseDirDark();
    afx_msg void OnBnClickedBrowseDirDarkcur();
    afx_msg void OnBnClickedBrowseDirOffset();
};
}
