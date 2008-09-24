// SampleGrabberCallback.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{702E3FAF-847D-4A5E-A280-1BAEBE6C681E}", 
		 name = "SampleGrabberCallback", 
		 helpstring = "SampleGrabberCallback 1.0 Type Library",
		 resource_name = "IDR_SAMPLEGRABBERCALLBACK") ]
class CSampleGrabberCallbackModule
{
public:
// Override CAtlDllModuleT members
};
		 
