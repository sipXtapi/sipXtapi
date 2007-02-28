/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.Writer;

public interface ProfileGenerator {
    void generate(ProfileContext context, String templateFileName, Writer out);

    void generate(ProfileContext context, String templateFileName, String outputFileName);

    void generate(ProfileContext context, String templateFileName, ProfileFilter filter, String outputFileName);
}
