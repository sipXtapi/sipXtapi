//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

#ifndef _MpvoGdi_h_
#define _MpvoGdi_h_

// SYSTEM INCLUDES
#include <windows.h>
#include <commctrl.h>

// APPLICATION INCLUDES
#include "mp/MpVideoBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Video output to Windows GDI
class MpvoGdi
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpvoGdi(HWND hWnd=NULL);

   ~MpvoGdi();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   OsStatus render(MpVideoBufPtr pFrame);

     /// Set window for video display.
   OsStatus setWindow(HWND hwnd);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get window we use to display video.
   HWND getWindow() const {return mHwnd;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


   HWND       mHwnd;        ///< Our video output window.

#pragma pack(push, 1)
   BITMAPINFO mBitmapInfo;  ///< Header for video frame.
   RGBQUAD    r;            ///< Don't touch them if are very sure you know
                            ///< what you're doing! (We write to them somewhere
                            ///< in the code)
   RGBQUAD    g;
   RGBQUAD    b;
#pragma pack(pop)

   OsStatus initOffscreenBuffer();

     /// Update clipping rectangles
   void UpdateRects(bool b_force);
     /**<
     *  This function is called when the window position or size are changed,
     *  and its job is to update the source and destination RECTs used
     *  to display the picture.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   HBITMAP mBitmap;
   void* mpBitmapBuffer;
   int mBmpWidth, mBmpHeight;

   void* getCachedBitmapBuffer(int width, int height);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpvoGdi_h_

#endif // SIPX_VIDEO ]
