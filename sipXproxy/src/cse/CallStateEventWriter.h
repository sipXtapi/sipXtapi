//
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventWriter_h_
#define _CallStateEventWriter_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "utl/UtlString.h"
#include "odbc/OdbcWrapper.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This CallStateEventWriter writes CSE events out to either a file or a 
 * database to the specification doc/cdr/call-state-events.html
 */
class CallStateEventWriter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
  
   // Log types, either file or database
   enum CseLogType {
      CseLogFile,
      CseLogDatabase
   };  

/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   /*! \param logType (in) - specifies either file or database
    * \param logName - file name or database name, only parameter needed for file
    * \param logLocation - database host
    * \param logUserName - database user
    * \param logDriver - database driver name
    * \param logPassword - database password 
    * \returns  pointer to an event writer object
    */   
   CallStateEventWriter(CseLogType logType,
                        const char* logName, 
                        const char* logLocation=NULL,
                        const char* logUserName=NULL,
                        const char* logDriver=NULL,
                        const char* logPassword=NULL);

   /// Destructor
   virtual ~CallStateEventWriter();
 
   /// Write event to the CSE log (file or database)
   /*! \param event (in) - call state event
    * \returns  true if event could be written
    */
   bool writeLog(const char* event);
   
   /// Open the log that was specified in the constructor
   bool openLog();
   
   /// Close log that was specified in the constructor
   bool closeLog();
   
   /// Get log type, either CseLogFile or CseLogDatabase
   CseLogType getLogType() {return mLogType;}
   
   /// Flush log - only functional for file
   void flush();
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   CseLogType        mLogType;
   
   UtlString         mLogName;
   UtlString         mLogLocation;
   UtlString         mLogUserName;
   UtlString         mLogDriver;
   UtlString         mLogPassword;
   
   OsFile*           mEventFile;   
   OdbcHandle        mHandle;
   
   /// no copy constructor or assignment operator
   CallStateEventWriter(const CallStateEventWriter& rCallStateEventWriter);
   CallStateEventWriter operator=(const CallStateEventWriter& rCallStateEventWriter);   
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventWriter_h_

