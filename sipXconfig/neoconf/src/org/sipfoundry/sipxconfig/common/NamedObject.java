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
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.collections.Transformer;

public interface NamedObject {
    public String getName();

    public void setName(String name);

    public static class ToName implements Transformer {
        public Object transform(Object input) {
            return ((NamedObject) input).getName();
        }
    }
}
