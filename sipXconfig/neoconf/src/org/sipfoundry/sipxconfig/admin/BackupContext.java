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
package org.sipfoundry.sipxconfig.admin;

public interface BackupContext {

    public abstract String[] perform(String backupPath);

    public abstract boolean getConfigs();

    public abstract void setConfigs(boolean configs);

    public abstract boolean getDatabase();

    public abstract void setDatabase(boolean database);

    public abstract boolean getVoicemail();

    public abstract void setVoicemail(boolean voicemail);
}
