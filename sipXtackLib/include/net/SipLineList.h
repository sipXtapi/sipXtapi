//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIPLINELIST_H__3822B7DD_69A6_44FC_B936_B75FACC65DC2__INCLUDED_)
#define AFX_SIPLINELIST_H__3822B7DD_69A6_44FC_B936_B75FACC65DC2__INCLUDED_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsLockingList.h>
#include <net/SipLine.h>
#include <os/OsRWMutex.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipLineList
{
public:
        UtlBoolean getDeviceLine(SipLine *line);
        UtlBoolean linesInArray(int maxSize , int* returnSize, SipLine* Line[]);
   UtlBoolean linesInArray(int maxSize, int* returnSize, SipLine Line[]);

        UtlBoolean getFirstLine(SipLine *Line);
        SipLine* getLine(const Url& identityUrl);
   SipLine* getLine(const UtlString& lineId);
   SipLine* getLine(const UtlString& userId, int& numOfMatches);

        int getListSize() ;

        UtlBoolean isDuplicate( SipLine* line);
        UtlBoolean isDuplicate( const Url& lineIdentityUrl );

        UtlBoolean remove(SipLine* line);
        UtlBoolean remove(const Url& lineIdentityUrl);
   void add(SipLine* newLine);

   SipLine* findLine(const char* lineId,
                     const char* realm,
                     const Url& toFromUrl,
                     const char* userId,
                     const Url& defaultLine) ;

        SipLineList();
        virtual ~SipLineList();
        void dumpLines();

protected:
        OsLockingList m_LineList;
};

#endif // !defined(AFX_SIPLINELIST_H__3822B7DD_69A6_44FC_B936_B75FACC65DC2__INCLUDED_)
