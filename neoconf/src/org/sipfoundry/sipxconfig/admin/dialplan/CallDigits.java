/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.enum.Enum;


public class CallDigits extends Enum {
    public static final CallDigits NO_DIGITS = new CallDigits("nodigits");
    public static final CallDigits VARIABLE_DIGITS = new CallDigits("vdigits");
    public static final CallDigits FIXED_DIGITS = new CallDigits("digits");

    public CallDigits(String name) {
        super(name);
    }
}
