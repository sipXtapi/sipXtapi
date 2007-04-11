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

import java.io.OutputStream;

/**
 * Genereric location for profiles. Provides a writer that should be closed once the generation is
 * done.
 */
public interface ProfileLocation {
    OutputStream getOutput(String profileName);

    void removeProfile(String profileName);
}