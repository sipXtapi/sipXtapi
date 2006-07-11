/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLogListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.calllog;

/**
 * CallLogListener that listens to event like contentsChanged of Call Log.
 */
public interface CallLogListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
      /**
     * Sent after a call-leg is added to the log.
     */
    //public void callLegAdded(CallLogEvent e) ;


    /**
     * Sent after a call-leg is removed from the log.
     */
    //public void callLegRemoved(CallLogEvent e) ;

    /**
     * Sent when the contents of the list has changed in a way that's to complex
     * to characterize with the previous methods.
     */
    public void contentsChanged(CallLogEvent e) ;
}
