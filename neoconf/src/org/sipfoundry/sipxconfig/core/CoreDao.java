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
 * Create/Retrieve/Update/Delete for Object in core
 * package that r/w to a database.
 */
public interface CoreDao {

    public Organization loadRootOrganization();

    
    public User loadUser(int id);
    
    
    public Endpoint loadEndpoint(int id);
    
    public void storeEndpoint(Endpoint endpoint);

    public void deleteEndpoint(Endpoint endpoint);
    
    /* To come...not tests yet, so it doesn't exist yet
    public Line loadLine(User user, int position);

    public void storeLine(Line line);
    */
    
    public void storeEndpointAssignment(EndpointAssignment assignment);
    
    public EndpointAssignment loadEndpointAssignment(int assignmentId);

    public void deleteEndpointAssignment(EndpointAssignment assignment);
}
