// $Id: //depot/OPENDEV/sipXphone/src/pinger/PingerInfoX.cpp#2 $
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

// APPLICATION INCLUDES
#include "pinger/PingerInfo.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

void PingerInfo::getBuildDate(UtlString& buildDate)
{
   buildDate.remove(0) ;

   UtlString date(__DATE__) ;
   buildDate.append(date.data()) ;

   buildDate.append(" ") ;

   UtlString time(__TIME__) ;
   buildDate.append(time.data()) ;
}
