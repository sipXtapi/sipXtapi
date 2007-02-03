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

#ifndef SAVEGREETING_H
#define SAVEGREETING_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Saves the mailbox owners's recorded greeting or name.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class SaveGreetingCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SaveGreetingCGI(const UtlString& mailbox,
                                        const UtlString& greetingType,
                                        const char* data,
                                        int   datasize);

    /**
     * Virtual Destructor
     */
    virtual ~SaveGreetingCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out);

protected:

private:

        /** Fully qualified mailbox id. */
        UtlString m_mailbox;

        /** Type of greeting to be saved.
         *      Possible values: standard, outofoffice, extendedabsence, name.
         *      Used to save the recorded data in appropriate filenames.
         */
        UtlString m_greetingType;

        /**     Recorded data */
        char* m_data ;

        /**     Size of recorded data*/
        int m_datasize;

};

#endif //SAVEGREETING_H
