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
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.phone.Phone;

public class HitachiPhone extends Phone {
    public static final String BEAN_ID = "hitachi";
    
    public HitachiPhone() {
        super(BEAN_ID);
    }

    public HitachiPhone(HitachiModel model) {
        super(model);
    }
}
