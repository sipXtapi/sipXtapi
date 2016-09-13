// 
//
// Copyright (C) 2010-2016 SIPez LLC. All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////


#ifndef _sipxunittests_h_
#define _sipxunittests_h_


//#if !defined(NO_CPPUNIT) && defined(ANDROID)
#define NO_CPPUNIT 1
//#endif

#if defined(NO_CPPUNIT)

#define SIPX_UNIT_BASE_CLASS SipxPortUnitTestClass

#include <os/OsIntTypes.h>
#include <sipxportunit/SipxPortUnitTest.h>
#include <utl/UtlString.h>

typedef UtlString string;

#else

#define SIPX_UNIT_BASE_CLASS CppUnit::TestCase

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

using namespace std;

#endif


#endif

