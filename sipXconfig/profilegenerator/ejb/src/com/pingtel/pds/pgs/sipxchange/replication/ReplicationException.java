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

import java.util.Collection;
import java.util.Iterator;

public class ReplicationException extends Exception
{
    private Collection m_replicationErrors;

    public ReplicationException() {
        // empty
    }

    public ReplicationException(String reason) {
	    super(reason);
    }

    public ReplicationException(Collection replicationErrors) {
        m_replicationErrors = replicationErrors;
    }

    public Collection getReplicationErrors(){
        return m_replicationErrors;
    }

    public String getMessage() {
        StringBuffer messageText = new StringBuffer ();

        for ( Iterator i = m_replicationErrors.iterator(); i.hasNext(); ) {
            ReplicationError err = (ReplicationError) i.next();
            messageText.append( err.toString() );
            if ( i.hasNext() )
                messageText.append( "\n" );
        }

        return messageText.toString();
    }
}