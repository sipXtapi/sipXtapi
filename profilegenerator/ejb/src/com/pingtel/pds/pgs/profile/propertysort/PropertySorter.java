/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/propertysort/PropertySorter.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
 
package com.pingtel.pds.pgs.profile.propertysort;

import java.util.TreeMap;
import java.util.Collection;

import org.jdom.Element;

abstract public class PropertySorter {

    protected TreeMap m_propertyMap = new TreeMap();

    public void addProperty ( Element property, String id ) {
        m_propertyMap.put( id, property );
    }

    public abstract Collection getValues ();

}