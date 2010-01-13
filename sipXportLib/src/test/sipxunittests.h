// 
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2010 SIPez LLC All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////


#ifndef _sipxunittests_h_
#define _sipxunittests_h_


#if !defined(NO_CPPUNIT) && defined(ANDROID)
#define NO_CPPUNIT 1
#endif

#if defined(NO_CPPUNIT)

#define SIPX_UNIT_BASE_CLASS SipxPortUnitTestClass

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

