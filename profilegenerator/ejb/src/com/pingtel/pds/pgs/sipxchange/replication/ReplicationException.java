/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package  com.pingtel.pds.pgs.sipxchange.replication;

import java.util.*;
import java.io.*;

import com.pingtel.pds.pgs.sipxchange.*;
/**
 *
 * @author Pradeep Paudyal
 */
public class ReplicationException extends Exception
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Collection m_replicationErrors;

//////////////////////////////////////////////////////////////////////////////
// Construction
////



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public ReplicationException() {
	    super();
    }

    public ReplicationException(String reason) {
	    super(reason);
    }

    public ReplicationException(Collection replicationErrors) {
	    super();
        m_replicationErrors = replicationErrors;
    }

    public Collection getReplicationErrors(){
        return m_replicationErrors;
    }

    public String toString () {
        StringBuffer messageText = new StringBuffer ();

        for ( Iterator i = m_replicationErrors.iterator(); i.hasNext(); ) {
            ReplicationError err = (ReplicationError) i.next();
            messageText.append( err.toString() );
            if ( i.hasNext() )
                messageText.append( "\n" );
        }

        return messageText.toString();
    }

    public String getMessage() {
        return toString();
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}