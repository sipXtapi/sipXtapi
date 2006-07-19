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
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.List;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface IntercomManager extends DataObjectSource {
    
    public Intercom newIntercom();

    public void saveIntercom(Intercom intercom);

    public List loadIntercoms();

    public void clear();
    
}
