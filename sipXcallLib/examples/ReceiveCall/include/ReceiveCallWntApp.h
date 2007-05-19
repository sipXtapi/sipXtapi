//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && defined(VIDEO)

#ifndef ReceiveCallWntApp_h
#define ReceiveCallWntApp_h

class CmdParams
{
public:
    int argc;
    char** argv;

};
int CreateWindows();
HWND createVideoWindow(HWND hParent);
HWND createPreviewWindow(HWND hParent);

#endif

#endif