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
package org.sipfoundry.sipxconfig.vm;

import org.apache.velocity.VelocityContext;

public class DistributionListsWriter extends XmlWriterImpl<DistributionList[]> {

    public DistributionListsWriter() {
        setTemplate("mailbox/distribution.vm");
    }
    
    @Override
    protected void addContext(VelocityContext context, DistributionList[] object) {
        context.put("lists", object);
    }
}
