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

import java.util.List;

public interface VoicemailManager {
    
    public List<String> getFolders();

    public List<Voicemail> getVoicemail(String userid, String folder);
    
    public String getMailstoreDirectory();
}
