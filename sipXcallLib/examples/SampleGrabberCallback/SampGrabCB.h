// SampGrabCB.h : Declaration of the CSampGrabCB

#pragma once
#include "resource.h"       // main symbols


// ISampGrabCB
[
	object,
	uuid("50C3C252-E279-4767-AF54-4B3D5F922C19"),
	dual,	helpstring("ISampGrabCB Interface"),
	pointer_default(unique)
]
__interface ISampGrabCB : IDispatch
{
    [id(1), helpstring("method SetWindow")] HRESULT SetWindow([in] LONG hWnd);
};


// _ISampGrabCBEvents
[
	dispinterface,
	uuid("D2975F1B-7D42-4B1A-AA47-23D6437364A3"),
	helpstring("_ISampGrabCBEvents Interface")
]
__interface _ISampGrabCBEvents
{
};


// CSampGrabCB

[
    coclass,
    threading("apartment"),
    support_error_info("ISampGrabCB"),
    event_source("com"),
    vi_progid("SampleGrabberCallback.SampGrabCB"),
    progid("SampleGrabberCallback.SampGrabCB.1"),
    version(1.0),
    uuid("31576FBD-7081-4D6D-9A08-9B94C213E653"),
    helpstring("SampGrabCB Class")
]
class ATL_NO_VTABLE CSampGrabCB : 
    public ISampGrabCB,
    public ISampleGrabberCB
{
public:
    CSampGrabCB() :
        m_hWnd(NULL)
    {
    }

    __event __interface _ISampGrabCBEvents;

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease() 
    {
    }

public:


    // ISampleGrabberCB Methods
public:
    STDMETHOD(SampleCB)(double SampleTime, IMediaSample * pSample)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(BufferCB)(double SampleTime, unsigned char * pBuffer, long BufferLen)
    {
        int x = 42;
        return E_NOTIMPL;
    }
    STDMETHOD(SetWindow)(LONG hWnd);
private:
    HWND m_hWnd;    
};

