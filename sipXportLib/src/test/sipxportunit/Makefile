#
#
# Copyright (C) 2010 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2010 SIPez LLC All rights reserved.
# Licensed to SIPfoundry under a Contributor Agreement.
#
# $$
# Author: Daniel Petrie
#         dpetrie AT SIPez DOT com
#////////////////////////////////////////////////////////////////////////////



unitexec : SipxPortUnitTest*.cpp main.cpp foo.cpp bar.cpp
	g++ -I. -g -rdynamic SipxPortUnitTest*.cpp main.cpp foo.cpp bar.cpp -o unitexec
