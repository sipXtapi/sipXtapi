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

#include <windows.h>
#include "ReceiveCallWntApp.h"
#include "ReceiveCall.h"
HINSTANCE hinst; 
HWND hMain = NULL;
// Function prototypes. 
 
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int); 
InitApplication(HINSTANCE); 
InitInstance(HINSTANCE, int); 
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); 

HWND ghVideo = NULL;
HWND ghPreview = NULL;

HWND createVideoWindow(HWND hParent)
{
    HWND hWnd = CreateWindow( 
        "ReceiveCallVideoClass",        // name of window class 
        "",            // title-bar string 
        WS_CHILD,      // 
        170,       // default horizontal position 
        0,       // default vertical position 
        320,       // default width 
        240,       // default height 
        hParent,         // no owner window 
        (HMENU) NULL,        // use class menu 
        hinst,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
    ghVideo = hWnd;
    ShowWindow(hWnd, true); 
    UpdateWindow(hWnd); 
    return hWnd;
}

HWND createPreviewWindow(HWND hParent)
{
    HWND hWnd = CreateWindow( 
        "ReceiveCallVideoClass",        // name of window class 
        "",            // title-bar string 
        WS_CHILD,      // 
        0,       // default horizontal position 
        0,       // default vertical position 
        160,       // default width 
        120,       // default height 
        hParent,         // no owner window 
        (HMENU) NULL,        // use class menu 
        hinst,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
    ghPreview = hWnd;
    ShowWindow(hWnd, true); 
    UpdateWindow(hWnd); 
    return hWnd;
}


 
// Application entry point. 
 
int CreateWindows()
{ 
    if (!InitApplication(NULL)) 
        return FALSE; 
 
    if (!InitInstance(NULL, TRUE)) 
        return FALSE; 

    // register the video window class
    WNDCLASSEX wcx; 
 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = 0;                     // redraw if size changes 
    wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = NULL;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = (HBRUSH)GetStockObject( 
        BLACK_BRUSH);                  // black background brush 
    wcx.lpszMenuName =  NULL;          // name of menu resource 
    wcx.lpszClassName = "ReceiveCallVideoClass";  // name of window class 
    wcx.hIconSm = NULL;
    ATOM x = RegisterClassEx(&wcx);
    // create the Preview Window and the Video Window
    HWND hPreview = createPreviewWindow(hMain);
    HWND hVideo = createVideoWindow(hMain);
        
    return 0; 
} 
 
BOOL InitApplication(HINSTANCE hinstance) 
{ 
    WNDCLASSEX wcx; 
 
    // Fill in the window class structure with parameters 
    // that describe the main window. 
 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | 
        CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = hinstance;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = (HBRUSH)GetStockObject( 
        WHITE_BRUSH);                  // white background brush 
    wcx.lpszMenuName =  "MainMenu";    // name of menu resource 
    wcx.lpszClassName = "MainWClass";  // name of window class 
    wcx.hIconSm = (HICON) LoadImage(hinstance, // small class icon 
        MAKEINTRESOURCE(5),
        IMAGE_ICON, 
        GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), 
        LR_DEFAULTCOLOR); 
 
    // Register the window class. 
 
    return RegisterClassEx(&wcx); 
} 
 
BOOL InitInstance(HINSTANCE hinstance, int nCmdShow) 
{ 
    HWND hwnd; 
 
    // Save the application-instance handle. 
 
    hinst = hinstance; 
 
    // Create the main window. 
 
    hwnd = CreateWindow( 
        "MainWClass",        // name of window class 
        "ReceiveCall",            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        500,       // default width 
        360,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        hinstance,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
 
    if (!hwnd) 
        return FALSE; 
 
    // Show the window and send a WM_PAINT message to the window 
    // procedure. 
 
    ShowWindow(hwnd, nCmdShow); 
    UpdateWindow(hwnd); 
    hMain = hwnd;
    return TRUE; 
 
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            return 0l;
        default:
            break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif