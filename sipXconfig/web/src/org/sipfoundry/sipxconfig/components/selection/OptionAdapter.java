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
package org.sipfoundry.sipxconfig.components.selection;

public interface OptionAdapter {
    Object getValue(Object option, int index);

    String getLabel(Object option, int index);

    String squeezeOption(Object option, int index);
}
