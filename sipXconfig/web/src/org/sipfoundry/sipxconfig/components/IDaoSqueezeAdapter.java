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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.util.io.SqueezeAdaptor;
import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * Hivemind appears to require all squeezers to implement an interface, 
 * so here it is
 */
public interface IDaoSqueezeAdapter extends SqueezeAdaptor {
    
    public void setDataObjectSource(DataObjectSource dao);

}
