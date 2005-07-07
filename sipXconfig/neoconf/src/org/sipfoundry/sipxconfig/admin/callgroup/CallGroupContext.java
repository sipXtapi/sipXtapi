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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Collection;
import java.util.List;

public interface CallGroupContext {
    public static final String CONTEXT_BEAN_NAME = "callGroupContext";

    void activateCallGroups();

    CallGroup loadCallGroup(Integer id);

    List getCallGroups();

    void storeCallGroup(CallGroup callGroup);

    void removeCallGroups(Collection ids);

    void duplicateCallGroups(Collection ids);

    void removeUser(Integer userId);

    void clear();

    List getAliases();

    ParkOrbit loadParkOrbit(Integer id);

    void storeParkOrbit(ParkOrbit parkOrbit);

    void removeParkOrbits(Collection ids);

    Collection getParkOrbits();

    void activateParkOrbits();

    String getDefaultMusicOnHold();

    void setDefaultMusicOnHold(String newMusic);
}
