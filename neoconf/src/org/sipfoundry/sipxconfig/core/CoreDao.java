/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

/**
 * Comments
 */
public interface CoreDao {

    public Endpoint loadEndpoint(int id);
    
    public Organization loadOrganization(int id);
    
    public User loadUser(int id);
    
    public Line loadLine(User user, int position);
    
    public void storeUser(User user);

    public void storeEndpoint(Endpoint endpoint);

    public void storeLine(Line line);
}
