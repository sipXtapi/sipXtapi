//
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "odbc/OdbcWrapper.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATICS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

bool odbcCheckExistence(const char* dbname)
{
   bool ret = false;
   SQLHENV env;
   SQLRETURN sqlRet;
   SQLUSMALLINT direction;
   SQLSMALLINT dsn_ret;
   SQLSMALLINT desc_ret;
   char dsn[256];
   char desc[256];
  
   SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
   SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0);
   
   direction = SQL_FETCH_FIRST;
   while (SQL_SUCCEEDED(sqlRet=SQLDataSources(env, direction,
                                              (SQLCHAR*)dsn, sizeof(dsn), &dsn_ret,
                                              (SQLCHAR*)desc, sizeof(desc), &desc_ret)))
   {
      direction = SQL_FETCH_NEXT;
      if (strcasecmp(dbname, dsn) == 0)
      {
         ret = true;
      }
   }
   
   return ret;
}

OdbcHandle odbcConnect(const char* dbname,
                       const char* username,
                       const char* password,
                       const char* driver)
{
   OdbcHandle handle = NULL;
   
   return handle;
}
                        
void odbcDisconnect(const OdbcHandle handle)
{
}

bool odbcExecute(const OdbcHandle handle,
                 const char sqlStatement)
{
   bool ret = false;
   
   return ret;
}                  
              
int odbcResultColumns(const OdbcHandle handle)
{
   int ret = 0;
   
   return ret;
}

bool odbcGetNextRow(const OdbcHandle handle)
{
   bool ret = false;
   
   return ret;
}
              
bool odbcGetColumnStringData(const OdbcHandle handle,
                             int columnIndex,
                             char* data,
                             int dataSize)
{
   bool ret = false;
   
   return ret;
}
