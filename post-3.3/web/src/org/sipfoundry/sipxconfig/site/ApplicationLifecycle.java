/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

/** 
 * Logout copied from Vlib example, also see
 * http://thread.gmane.org/gmane.comp.java.tapestry.user/31641
 */
public interface ApplicationLifecycle {
    /**
     * Logs the user out of the systems; sets a flag that causes the session to be discarded at the
     * end of the request.
     */
    void logout();

    /**
     * If true, then the session (if it exists) should be discarded at the end of the request.
     */
    boolean getDiscardSession();
}
