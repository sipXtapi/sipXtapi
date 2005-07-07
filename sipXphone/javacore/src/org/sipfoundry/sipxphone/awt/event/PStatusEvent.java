/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/event/PStatusEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.awt.event;

/**
 *
 * PStatusEvent.java
 *
 * Subclass of PEvent associated with status frame. 
 *
 * Created: Tue Jun 05 13:20:31 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class PStatusEvent extends PEvent{
    public static final int TYPE_STATUS_EVENT = 0;

    public PStatusEvent ( Object objSource, Object objParam){
        super( objSource , TYPE_STATUS_EVENT );
        setObjectParam( objParam );
    }
}// PStatusEvent
