//
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"
#include "odbc/OdbcWrapper.h"

// APPLICATION INCLUDES
#include "CallStateEventWriter.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static const char* ModuleName = 
   "CallStateEventWriter";

static const char* ErrUnknownLogType =
   "Unknown log type";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventWriter::CallStateEventWriter(CseLogType logType,
                                           const char* logName, 
                                           const char* logLocation,
                                           const char* logUserName,
                                           const char* logDriver,
                                           const char* logPassword)
                        : mLogType(logType),
                          mLogName(logName),
                          mLogLocation(logLocation),
                          mLogUserName(logUserName),
                          mLogDriver(logDriver),
                          mLogPassword(logPassword),
                          mEventFile(NULL),
                          mHandle(NULL),
                          mbWriteable(false)
{
   switch (mLogType)
   {
   case CseLogFile:
      OsSysLog::add(FAC_CDR, PRI_DEBUG,
                    "%s::constructor Log type file", ModuleName);
      break;
   case CseLogDatabase:
      OsSysLog::add(FAC_CDR, PRI_DEBUG,
                    "%s::constructor Log type database", ModuleName);      
      break;
   default:
     OsSysLog::add(FAC_CDR, PRI_ERR,
                   "%s::constructor %s", ModuleName, ErrUnknownLogType);         
   }
}

/// Destructor
CallStateEventWriter::~CallStateEventWriter()
{
   closeLog();
}

bool CallStateEventWriter::openLog()
{
   bool bRet = false;
   
   if (!mbWriteable)
   {
      switch (mLogType)
      {
      case CseLogFile:
         {
         OsPath callStateLogPath(mLogName);
         mEventFile = new OsFile(callStateLogPath);
   
         OsStatus callStateLogStatus = mEventFile->open(OsFile::CREATE|OsFile::APPEND);
         if (OS_SUCCESS == callStateLogStatus)
         {
            OsSysLog::add(FAC_CDR, PRI_DEBUG,
                          "%s::openLog opened %s", ModuleName, mLogName.data());       
            mbWriteable = true;
            bRet = true;
         }
         else
         {
            OsSysLog::add(FAC_CDR, PRI_ERR,
                          "%s::openLog failed (%d) to open Call State Event Log '%s'",
                          ModuleName, callStateLogStatus, mLogName.data());         
            if (mEventFile)
            {
               delete mEventFile;
            }              
            mEventFile = NULL;
         }
         break;
         }
      case CseLogDatabase:
         if ((mHandle=odbcConnect(mLogName, mLogLocation, mLogUserName, mLogDriver, mLogPassword)) != NULL)
         {
            OsSysLog::add(FAC_CDR, PRI_DEBUG,
                          "%s::openLog connected to database %s", ModuleName, mLogName.data()); 
            mbWriteable = true;
            bRet = true;
         }
         else
         {
            OsSysLog::add(FAC_CDR, PRI_ERR,
                          "%s::openLog connection to database %s failed", 
                          ModuleName, mLogName.data());             
         }
         break;
      default:
        OsSysLog::add(FAC_CDR, PRI_ERR,
                      "%s::openLog %s", ModuleName, ErrUnknownLogType);
      }
   }
   else
   {
     OsSysLog::add(FAC_CDR, PRI_ERR,
                   "%s::openLog log %s already open", ModuleName, mLogName);
   }
   return bRet;
}

bool CallStateEventWriter::closeLog()
{
   bool bRet = false;
   
   switch (mLogType)
   {
   case CseLogFile:
      if (mEventFile)
      {
         mEventFile->flush(); // try to get buffered records to the file...
         mEventFile->close();
         delete mEventFile;
         mEventFile = NULL;
      }
      mbWriteable = false;
      bRet = true;
         
      OsSysLog::add(FAC_CDR, PRI_DEBUG,
                    "%s::closeLog", ModuleName);
      break;
   case CseLogDatabase:
      if (mHandle)
      {
         odbcDisconnect(mHandle);
         mHandle = NULL;
      }
      mbWriteable = false;
      bRet = true;
         
      OsSysLog::add(FAC_CDR, PRI_DEBUG,
                    "%s::closeLog", ModuleName);      
      break;
   default:
     OsSysLog::add(FAC_CDR, PRI_ERR,
                   "%s::closeLog %s", ModuleName, ErrUnknownLogType);         
   }
   return bRet;
}

bool CallStateEventWriter::writeLog(const char* event)
{
   bool bRet = false;

   if (mbWriteable)
   {   
      switch (mLogType)
      {
      case CseLogFile:
         {
            if (mEventFile)
            {
               // write it to the log file
               unsigned long written;
               mEventFile->write(event, strlen(event), written);         
               OsSysLog::add(FAC_CDR, PRI_DEBUG,
                             "%s::writeLog", ModuleName);
               bRet = true;
            }
         break;
         }
      case CseLogDatabase:
         if (mHandle)
         {
            odbcExecute(mHandle, event);
            bRet = true;
         }
         OsSysLog::add(FAC_CDR, PRI_DEBUG,
                       "%s::writeLog", ModuleName);      
         break;
      default:
        OsSysLog::add(FAC_CDR, PRI_ERR,
                      "%s::writeLog %s", ModuleName, ErrUnknownLogType);         
      }
   }
   else
   {
      OsSysLog::add(FAC_CDR, PRI_ERR,
                    "%s::writeLog log %s not writeable", ModuleName, mLogName);            
   }
   return bRet;
}

void CallStateEventWriter::flush()
{
   if (mLogType == CseLogFile && mEventFile)
   {
      mEventFile->flush();
   }
}
