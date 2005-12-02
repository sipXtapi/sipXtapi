// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAYTASK_H
#define DISPLAYTASK_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "IMDBWorkerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsEvent;
class TiXmlNode;

class DisplayTask : public IMDBWorkerTask
{
public:
    /**
     * Ctor
     * 
     * @param rCommand
     * @param rArgument
     * @param rMsgQ
     */
    DisplayTask ( 
        const UtlString& rArgument, 
        OsMsgQ& rMsgQ,
        OsEvent& rCommandEvent);

    /**
     * Dtor
     */
    virtual ~DisplayTask();
    
    /**
     * The worker method, this is where all the command types execute, they
     * run as separate threads
     * 
     * @param runArg
     * 
     * @return 
     */
    virtual int run( void* runArg );

    /**
     * 
     * @param rTableName
     * 
     * @return 
     */
    int showTableRows ( const UtlString& rTableName ) const;

    /**
     * Populates the IMDB from an XML file
     * 
     * @param rImportFilename
     * 
     * @return 
     */
    OsStatus loadDB( const UtlString& rImportFilename ) const;

    /**
     * 
     * @param rNode
     * @param rKey
     * @param rValue
     * 
     * @return 
     */
    OsStatus getAttributeValue ( 
        const TiXmlNode& rNode, 
        const UtlString& rKey, 
        UtlString& rValue ) const;

    /**
     * 
     * @param nvPairs
     * @param tableName
     */
    void insertRow ( 
        const UtlHashMap& nvPairs, 
        const UtlString& tableName ) const;
};

#endif  // DISPLAYTASK_H

