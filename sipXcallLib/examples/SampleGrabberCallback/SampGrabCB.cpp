// SampGrabCB.cpp : Implementation of CSampGrabCB

#include "stdafx.h"
#include "SampGrabCB.h"
#include ".\sampgrabcb.h"


// CSampGrabCB


STDMETHODIMP CSampGrabCB::SetWindow(LONG hWnd)
{
    m_hWnd = (HWND) hWnd;

    return S_OK;
}
