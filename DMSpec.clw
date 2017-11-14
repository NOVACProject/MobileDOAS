; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CConfiguration
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "DMSpec.h"
LastPage=0

ClassCount=10
Class1=CDMSpecApp
Class2=CDMSpecDoc
Class3=CDMSpecView
Class4=CMainFrame

ResourceCount=12
Resource1=IDD_DIALOG2 (Swedish)
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDD_CONFDLG
Resource4=IDD_DIALOG2
Resource5=IDD_DMSPEC_FORM (English (U.S.))
Resource6=IDD_DMSPEC_FORM
Class6=CFluxCalcDlg
Class7=CLogDlg
Class8=CFLUXSETDLG
Resource7=IDD_ABOUTBOX (English (U.S.))
Resource8=IDD_CONFDLG (English (U.S.))
Class9=CRouteDlg
Resource9=IDR_MAINFRAME (English (U.S.))
Class10=CConfiguration
Resource10=IDD_ROUTEDLG
Resource11=IDD_ABOUTBOX
Resource12=IDD_ROUTEDLG (Swedish)

[CLS:CDMSpecApp]
Type=0
HeaderFile=DMSpec.h
ImplementationFile=DMSpec.cpp
Filter=N

[CLS:CDMSpecDoc]
Type=0
HeaderFile=DMSpecDoc.h
ImplementationFile=DMSpecDoc.cpp
Filter=N

[CLS:CDMSpecView]
Type=0
HeaderFile=DMSpecView.h
ImplementationFile=DMSpecView.cpp
Filter=D
BaseClass=CFormView
VirtualFilter=VWC
LastObject=IDC_SETBUTTON


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T




[CLS:CAboutDlg]
Type=0
HeaderFile=DMSpec.cpp
ImplementationFile=DMSpec.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=IDC_BTNSTART
Command2=ID_CONTROL_STOP
Command3=ID_CONTROL_COUNTFLUX
Command4=ID_CONTROL_START
Command5=ID_APP_EXIT
Command6=ID_CONFIGURATION_OPERATION
Command7=ID_CONFIGURATION_PLOT_CHANGEBACKGROUND
Command8=ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR
Command9=ID_VIEW_TOOLBAR
Command10=ID_VIEW_STATUS_BAR
Command11=ID_APP_ABOUT
CommandCount=11

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_DMSPEC_FORM]
Type=1
Class=CDMSpecView
ControlCount=28
Control1=IDC_STATIC,static,1342308352
Control2=IDC_STATIC,static,1342308352
Control3=IDC_WINDSPEED,edit,1350631552
Control4=IDC_WINDDIRECTION,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BASEEDIT,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308354
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CONCENTRATION,static,1342308352
Control14=IDC_SH,static,1342308352
Control15=IDC_SQ,static,1342308352
Control16=IDC_LAT,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_LON,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_INTTIME,static,1342308352
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,button,1342177287
Control24=IDC_STATIC,static,1342308352
Control25=IDC_GPSTIME,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_SCANNO,static,1342308352
Control28=IDC_BTNSTART,button,1342242816

[DLG:IDD_DMSPEC_FORM (English (U.S.))]
Type=1
Class=CDMSpecView
ControlCount=28
Control1=IDC_STATIC,static,1342308352
Control2=IDC_STATIC,static,1342308352
Control3=IDC_WINDSPEED,edit,1350631552
Control4=IDC_WINDDIRECTION,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_BASEEDIT,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308354
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_CONCENTRATION,static,1342308352
Control14=IDC_SH,static,1342308352
Control15=IDC_SQ,static,1342308352
Control16=IDC_LAT,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_LON,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_INTTIME,static,1342308352
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,button,1342177287
Control24=IDC_STATIC,static,1342308352
Control25=IDC_GPSTIME,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_SCANNO,static,1342308352
Control28=IDC_BTNSTART,button,1342242816

[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=IDC_BTNSTART
Command2=ID_CONTROL_STOP
Command3=ID_CONFIGURATION_OPERATION
Command4=ID_CONTROL_START
Command5=ID_APP_ABOUT
Command6=ID_APP_EXIT
CommandCount=6

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=IDC_BTNSTART
Command2=ID_CONTROL_STOP
Command3=ID_CONTROL_COUNTFLUX
Command4=ID_CONTROL_START
Command5=ID_APP_EXIT
Command6=ID_CONFIGURATION_OPERATION
Command7=ID_CONFIGURATION_PLOT_CHANGEBACKGROUND
Command8=ID_CONFIGURATION_PLOT_CHANGEPLOTCOLOR
Command9=ID_VIEW_TOOLBAR
Command10=ID_VIEW_STATUS_BAR
Command11=ID_APP_ABOUT
CommandCount=11

[ACL:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[CLS:CFluxCalcDlg]
Type=0
HeaderFile=FluxCalcDlg.h
ImplementationFile=FluxCalcDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CFluxCalcDlg

[DLG:IDD_DIALOG2]
Type=1
Class=CFLUXSETDLG
ControlCount=40
Control1=IDC_CALC,button,1342242817
Control2=IDC_FLUXDWS,edit,1350631424
Control3=IDC_FLUXDWD,edit,1350631552
Control4=IDC_FLUXDLAT,edit,1350631552
Control5=IDC_FLUXDLON,edit,1350631552
Control6=IDC_FLUXDOFFSET,edit,1350631552
Control7=IDC_STATIC,static,1342308864
Control8=IDC_STATIC,static,1342308864
Control9=IDC_STATIC,static,1342308864
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308865
Control12=IDC_BTNCHOOSEFILE,button,1342242816
Control13=IDC_FLUX,static,1342308864
Control14=IDC_BTNFOR,button,1342242816
Control15=IDC_BTNBACK,button,1342242816
Control16=IDC_MAXWD,static,1342308352
Control17=IDC_MAXCOL,static,1342308352
Control18=IDC_AVWD,static,1342308352
Control19=IDC_AVCOL,static,1342308352
Control20=IDC_MAXLAT,static,1342308352
Control21=IDC_MAXLON,static,1342308352
Control22=IDC_AVLAT,static,1342308352
Control23=IDC_AVLON,static,1342308352
Control24=IDC_STATIC,button,1342177287
Control25=IDC_STATIC,button,1342177287
Control26=IDC_MAXINDEX,static,1342308352
Control27=IDC_AVINDEX,static,1342308352
Control28=IDC_STATIC,static,1342181390
Control29=IDC_STATIC,static,1342181390
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,static,1342308352
Control32=IDC_FLUXDFROM,edit,1350631552
Control33=IDC_STATIC,static,1342308352
Control34=IDC_FLUXDTO,edit,1350631552
Control35=IDC_PATHEDT,edit,1350631552
Control36=IDC_SLIDERFROM,msctls_trackbar32,1342242817
Control37=IDC_SLIDERTO,msctls_trackbar32,1342242837
Control38=IDC_BTNDEL,button,1342242816
Control39=IDC_SLIDEROFFSET,msctls_trackbar32,1342242822
Control40=IDC_BTNROUTE,button,1342242816

[CLS:CLogDlg]
Type=0
HeaderFile=LogDlg.h
ImplementationFile=LogDlg.cpp
BaseClass=CFileDialog
Filter=D
LastObject=CLogDlg
VirtualFilter=dWC

[CLS:CFLUXSETDLG]
Type=0
HeaderFile=FLUXSETDLG.h
ImplementationFile=FLUXSETDLG.cpp
BaseClass=CDialog
Filter=D
LastObject=CFLUXSETDLG
VirtualFilter=dWC

[DLG:IDD_DIALOG2 (Swedish)]
Type=1
Class=CFLUXSETDLG
ControlCount=40
Control1=IDC_CALC,button,1342242817
Control2=IDC_FLUXDWS,edit,1350631424
Control3=IDC_FLUXDWD,edit,1350631552
Control4=IDC_FLUXDLAT,edit,1350631552
Control5=IDC_FLUXDLON,edit,1350631552
Control6=IDC_FLUXDOFFSET,edit,1350631552
Control7=IDC_STATIC,static,1342308864
Control8=IDC_STATIC,static,1342308864
Control9=IDC_STATIC,static,1342308864
Control10=IDC_STATIC,static,1342308864
Control11=IDC_STATIC,static,1342308865
Control12=IDC_BTNCHOOSEFILE,button,1342242816
Control13=IDC_FLUX,static,1342308864
Control14=IDC_BTNFOR,button,1342242816
Control15=IDC_BTNBACK,button,1342242816
Control16=IDC_MAXWD,static,1342308352
Control17=IDC_MAXCOL,static,1342308352
Control18=IDC_AVWD,static,1342308352
Control19=IDC_AVCOL,static,1342308352
Control20=IDC_MAXLAT,static,1342308352
Control21=IDC_MAXLON,static,1342308352
Control22=IDC_AVLAT,static,1342308352
Control23=IDC_AVLON,static,1342308352
Control24=IDC_STATIC,button,1342177287
Control25=IDC_STATIC,button,1342177287
Control26=IDC_MAXINDEX,static,1342308352
Control27=IDC_AVINDEX,static,1342308352
Control28=IDC_STATIC,static,1342181390
Control29=IDC_STATIC,static,1342181390
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,static,1342308352
Control32=IDC_FLUXDFROM,edit,1350631552
Control33=IDC_STATIC,static,1342308352
Control34=IDC_FLUXDTO,edit,1350631552
Control35=IDC_PATHEDT,edit,1350631552
Control36=IDC_SLIDERFROM,msctls_trackbar32,1342242817
Control37=IDC_SLIDERTO,msctls_trackbar32,1342242837
Control38=IDC_BTNDEL,button,1342242816
Control39=IDC_SLIDEROFFSET,msctls_trackbar32,1342242822
Control40=IDC_BTNROUTE,button,1342242816

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDC_BTNSTART
Command2=ID_CONTROL_STOP
Command3=ID_CONFIGURATION_OPERATION
Command4=ID_CONTROL_START
Command5=ID_APP_ABOUT
Command6=ID_APP_EXIT
CommandCount=6

[DLG:IDD_ROUTEDLG]
Type=1
Class=CRouteDlg
ControlCount=0

[CLS:CRouteDlg]
Type=0
HeaderFile=RouteDlg.h
ImplementationFile=RouteDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CRouteDlg
VirtualFilter=dWC

[DLG:IDD_CONFDLG (English (U.S.))]
Type=1
Class=CConfiguration
ControlCount=37
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SPECCOM,combobox,1344340226
Control5=IDC_STATIC,static,1342308352
Control6=IDC_SPECBAUD,combobox,1344339970
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_USEGPSCHECK,button,1342242819
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_FIXSHIFT,button,1342242819
Control17=IDC_FIXSQUEEZE,button,1342242819
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_CONFCHOOSEREF,button,1342242816
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,button,1342177287
Control25=IDC_STATIC,button,1342177287
Control26=IDC_GPSCOM,combobox,1344340226
Control27=IDC_GPSBAUD,combobox,1344339970
Control28=IDC_FIXEXPTIME,edit,1350631552
Control29=IDC_TIMERES,edit,1350631552
Control30=IDC_SPECCENTER,edit,1350631552
Control31=IDC_PERCENT,edit,1350631552
Control32=IDC_REFFILE,edit,1350631552
Control33=IDC_FITFROM,edit,1350631552
Control34=IDC_FITTO,edit,1350631552
Control35=IDC_POLYNOM,edit,1350631552
Control36=IDC_GASFACTOR,edit,1350631552
Control37=IDC_MAXCOLUMN,edit,1350631552

[CLS:CConfiguration]
Type=0
HeaderFile=Configuration.h
ImplementationFile=Configuration.cpp
BaseClass=CDialog
Filter=D
LastObject=CConfiguration
VirtualFilter=dWC

[DLG:IDD_ROUTEDLG (Swedish)]
Type=1
Class=?
ControlCount=0

[DLG:IDD_CONFDLG]
Type=1
Class=CConfiguration
ControlCount=37
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SPECCOM,combobox,1344340226
Control5=IDC_STATIC,static,1342308352
Control6=IDC_SPECBAUD,combobox,1344339970
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_USEGPSCHECK,button,1342242819
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_FIXSHIFT,button,1342242819
Control17=IDC_FIXSQUEEZE,button,1342242819
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_CONFCHOOSEREF,button,1342242816
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,button,1342177287
Control25=IDC_STATIC,button,1342177287
Control26=IDC_GPSCOM,combobox,1344340226
Control27=IDC_GPSBAUD,combobox,1344339970
Control28=IDC_FIXEXPTIME,edit,1350631552
Control29=IDC_TIMERES,edit,1350631552
Control30=IDC_SPECCENTER,edit,1350631552
Control31=IDC_PERCENT,edit,1350631552
Control32=IDC_REFFILE,edit,1350631552
Control33=IDC_FITFROM,edit,1350631552
Control34=IDC_FITTO,edit,1350631552
Control35=IDC_POLYNOM,edit,1350631552
Control36=IDC_GASFACTOR,edit,1350631552
Control37=IDC_MAXCOLUMN,edit,1350631552

