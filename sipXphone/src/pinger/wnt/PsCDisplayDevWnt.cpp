// $Id: //depot/OPENDEV/sipXphone/src/pinger/wnt/PsCDisplayDevWnt.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <os/iostream>
#include <pinger/wnt/stdafx.h>
#include <pinger/wnt/PhoneGui.h>
#include <pinger/wnt/PhoneGuiDlg.h>

// APPLICATION INCLUDES
#include "ps/wnt/PsCDisplayDevWnt.h"
#include "os/OsTask.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// Initialize the nRows x nCols character display.
PsCDisplayDevWnt::PsCDisplayDevWnt(const int nRows, const int nCols)
:  PsCDisplayDev(nRows, nCols)
{
   doClrScreen();
}

// Destructor
PsCDisplayDevWnt::~PsCDisplayDevWnt()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Clear one row of the display.
void PsCDisplayDevWnt::doClrRow(const int row)
{
   UtlString      blankRow(' ', mNumCols);
   CPhoneGuiDlg* gui;
   
   gui = CPhoneGuiApp::getGui();
   if (gui)
		gui->displayReplaceText(row, 0, blankRow.data());
}

// Clear the entire display.
void PsCDisplayDevWnt::doClrScreen(void)
{
   int row;

   for (row=0; row < mNumRows; row++)
   {
      doClrRow(row);
   }
}

// Platform-specific implementation to display a cursor at the 
// designated location.
void PsCDisplayDevWnt::doDrawCursor(const int row, const int col)
{
   // no-op for the Win32 implementation
}

// Platform-specific implementation for drawString().
void PsCDisplayDevWnt::doDrawString(const char* s,
                                    const int row, const int col)
{
   CPhoneGuiDlg* gui;
   
   gui = CPhoneGuiApp::getGui();
   gui->displayReplaceText(row, col, s);
}

// Starting at (row, col), erase "cnt" chars from the display.
void PsCDisplayDevWnt::doErase(const int row, const int col, const int cnt)
{
   UtlString      blankStr(' ', cnt);
   CPhoneGuiDlg* gui;
   
   gui = CPhoneGuiApp::getGui();
   gui->displayReplaceText(row, col, blankStr.data());
}

// Scroll the display down one line.
void PsCDisplayDevWnt::doScrollDown(void)
{
   CPhoneGuiDlg* gui;
   int           rowIndex;
   UtlString      screenText;

   gui = CPhoneGuiApp::getGui();
   for (rowIndex = 1; rowIndex < mNumRows; rowIndex++)
   {
		gui->displayGetTextLine(rowIndex,   &screenText);
		gui->displaySetTextLine(rowIndex-1, screenText.data());
   }

   // clear the last row
   doClrRow(mNumRows-1);
}

// Platform-specific implementation for setCursor()
void PsCDisplayDevWnt::doSetCursor(const int row, const int col)
{
   // no-op for the Win32 implementation
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

