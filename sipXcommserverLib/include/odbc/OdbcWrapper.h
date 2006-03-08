// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ODBCWRAPPER_H_
#define _ODBCWRAPPER_H_

// SYSTEM INCLUDES
#include <sql.h>
#include <sqlext.h>

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

struct OdbcControlStruct {
   SQLHENV mEnvironmentHandle;
   SQLHDBC mConnectionHandle;
   SQLHSTMT mStatementHandle;
};   

/// Checks the existence of the data source
/*! \param dbname (in) - name of the data source
 * \returns  true if data source exists otherwise FALSE   
 */
bool odbcCheckExistence(const char* dbname);

#endif // _ODBCWRAPPER_H_
