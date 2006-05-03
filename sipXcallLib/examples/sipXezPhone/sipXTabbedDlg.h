//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef __sipXTabbedDlg_H__
#define __sipXTabbedDlg_H__

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <utl/UtlString.h>
#include "sipXezPhone_wdr.h"

#define ID_TAB_NOTEBOOK_CTRL    2999
#define ID_TAB_IDENTITY_CTRL    3000
#define ID_TAB_REALM_CTRL       3001
#define ID_TAB_USERNAME_CTRL    3002
#define ID_TAB_PASSWORD_CTRL    3003
#define ID_TAB_LOCATION_CTRL    3004
#define ID_TAB_PROXY_CTRL       3005
#define ID_TAB_STUNSERVER_CTRL  3006
#define ID_TAB_STUNPORT_CTRL    3007
#define ID_TAB_TURNSERVER_CTRL  3008
#define ID_TAB_TURNPORT_CTRL    3009
#define ID_TAB_SIPPORT_CTRL     3010
#define ID_TAB_RTPPORT_CTRL     3011
#define ID_TAB_ICE_CTRL         3012
#define ID_TAB_RPORT_CTRL       3013
#define ID_TAB_ANSWER_CTRL      3014
#define ID_TAB_REGISTER_CTRL    3015
#define ID_TAB_BANDWIDTH_CHOICE 3016
#define ID_TAB_CODEC_LIST       3017
#define ID_TAB_CODEC_SELECT     3018
#define ID_TAB_ECHO_CTRL        3019
#define ID_TAB_DTMF_CTRL        3020
#define ID_TAB_IDLE_CTRL        3021
#define ID_TAB_SHORT_CTRL       3022

//----------------------------------------------------------------------------
// sipXTabbedDlg
//----------------------------------------------------------------------------


/*************************************************** 
 * Base class for all pages in the notebook 
 */
class sipXTabbedPage : public wxPanel
{
public:
    sipXTabbedPage(wxWindow *parent, const UtlString& title, wxWindowID id = -1,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize);

    virtual const char* GetTitle()
        {return mTitle.data();}

    virtual void getData() = 0;             // Get page data
    virtual void restoreData() = 0;         // Some pages have to remember original settings
                                            // and restore them on Cancel
    virtual bool validateData() = 0;        // Some pages have to validate data on page change

protected:
    bool        mbInitialized;

private:
    virtual void setData() = 0;

    UtlString   mTitle;
};

/*************************************************** 
 * Base class for all codec pages in the notebook 
 *
 * The sipXTabbedCodecPage handles the basic three
 * controls relating to codecs - the bandwidth selection,
 * the select button for explicit codec selection, and 
 * the codec list box.
 */

class sipXTabbedCodecPage : public sipXTabbedPage
{
public:
    sipXTabbedCodecPage(wxWindow* parent, const UtlString& title);

protected:
    void rebuildCodecList(int sel);

    void OnBandwidthChange(wxCommandEvent &event);
    void OnDblClick(wxCommandEvent &event);
    void OnSelect(wxCommandEvent &event);

    int         mCodecPref;
    int         mOrigCodecPref;
    UtlString   mCodecName;
    UtlString   mOrigCodecName;
    bool        mbCodecByName;

    wxChoice*   mpCodecPref;
    wxListBox*  mpCodecList;
    wxButton*   mpSelectBtn;

    DECLARE_EVENT_TABLE()
};

/************************************************** 
 * Specialized tabbed pages 
 */

class sipXIdentityPage : public sipXTabbedPage
{
public:
    sipXIdentityPage(wxWindow *parent, const UtlString& title);

    void getData();
    void restoreData();
    bool validateData();

private:
    void setData();

    wxTextCtrl* mpIdentity;
    wxTextCtrl* mpRealm;
    wxTextCtrl* mpUser;
    wxTextCtrl* mpPassword;
    wxTextCtrl* mpLocation;
    wxCheckBox* mpAnswer;

    DECLARE_EVENT_TABLE()
};

class sipXNetworkPage : public sipXTabbedPage
{
public:
    sipXNetworkPage(wxWindow *parent, const UtlString& title);

    void getData();
    void restoreData();
    bool validateData();

private:
    void setData();

    wxTextCtrl* mpProxy;
    wxTextCtrl* mpTurnServer;
    wxTextCtrl* mpTurnPort;
    wxTextCtrl* mpStunServer;
    wxTextCtrl* mpStunPort;
    wxTextCtrl* mpSipPort;
    wxTextCtrl* mpRtpPort;
    wxTextCtrl* mpIdle;
    wxCheckBox* mpIce;
    wxCheckBox* mpRport;
    wxCheckBox* mpShortNames;
};

class sipXAudioPage : public sipXTabbedCodecPage
{
public:
    sipXAudioPage(wxWindow *parent, const UtlString& title);

    void getData();                   
    void restoreData();
    bool validateData();

private:
    void setData();

    wxCheckBox* mpEcho;
    wxCheckBox* mpDtmf;
};

class sipXVideoPage : public sipXTabbedCodecPage
{
public:
    sipXVideoPage(wxWindow *parent, const UtlString& title);

    void getData();
    void restoreData();
    bool validateData();

private:
    void setData();
};

class sipXSecurityPage : public sipXTabbedPage
{
public:
    sipXSecurityPage(wxWindow *parent, const UtlString& title);

    void getData();
    void restoreData();
    bool validateData();

private:
    void setData();
};

class sipXTabbedDlg : public wxDialog
{
public:
    sipXTabbedDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE,
        long options = 0);

    virtual ~sipXTabbedDlg();

    void InitDialog();

private:
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );
    void OnRegister( wxCommandEvent &event );
    void OnChanging( wxNotebookEvent &event );
    void OnChanged( wxNotebookEvent &event );

    int mXpos, mYpos;     // Position of dialog
    int mWidth, mHeight;

    long mOptions;        // Notebook options
    int  mPages;          // Number of pages in notebook
    int  mLastPage;       // Index of last selected page

    wxNotebook*       mpNotebook;

    sipXTabbedPage*   mpPage[9];

    wxButton*         mpOkBtn;
    wxButton*         mpCancelBtn;
    wxButton*         mpRegisterBtn;

    DECLARE_EVENT_TABLE()
};

#endif
