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
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public interface ModelSource {
    Collection<PhoneModel> getModels();
}
