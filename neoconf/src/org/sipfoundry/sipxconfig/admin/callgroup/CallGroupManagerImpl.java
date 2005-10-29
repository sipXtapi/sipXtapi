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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationManager;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.common.CollectionUtils;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Hibernate implementation of the call group context
 */
public class CallGroupManagerImpl extends SipxHibernateDaoSupport implements CallGroupManager {
    private static final String NAME_PROP_NAME = "name";
    private static final String EXTENSION_PROP_NAME = "extension";
    private static final String VALUE = "value";
    
    private static final String QUERY_CALL_GROUP_IDS_WITH_NAME = "callGroupIdsWithName";
    private static final String QUERY_CALL_GROUP_IDS_WITH_EXTENSION = "callGroupIdsWithExtension";
    private static final String QUERY_PARK_ORBIT_IDS_WITH_EXTENSION = "parkOrbitIdsWithExtension";
    
    private CoreManager m_coreContext;
    private Orbits m_orbitsGenerator;
    private SipxReplicationManager m_replicationContext;

    public CallGroup loadCallGroup(Integer id) {
        return (CallGroup) getHibernateTemplate().load(CallGroup.class, id);
    }

    public void storeCallGroup(CallGroup callGroup) {
        // Check for duplicate names or extensions before saving the call group
        String name = callGroup.getName();
        final String huntGroupTypeName = "hunt group";
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), callGroup,
                QUERY_CALL_GROUP_IDS_WITH_NAME, name, new NameInUseException(huntGroupTypeName, name));
        String extension = callGroup.getExtension();
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), callGroup,
                QUERY_CALL_GROUP_IDS_WITH_EXTENSION,
                extension,
                new CallGroupExtensionInUseException(extension));

        getHibernateTemplate().saveOrUpdate(callGroup);
    }

    public void removeCallGroups(Collection ids) {
        if (ids.isEmpty()) {
            return;
        }
        removeAll(CallGroup.class, ids);
        activateCallGroups();
    }

    public List getCallGroups() {
        return getHibernateTemplate().loadAll(CallGroup.class);
    }

    public void duplicateCallGroups(Collection ids) {
        for (Iterator i = ids.iterator(); i.hasNext();) {
            CallGroup group = loadCallGroup((Integer) i.next());
            
            // Create a copy of the call group with a unique name
            CallGroup groupDup = (CallGroup) duplicateBean(group, QUERY_CALL_GROUP_IDS_WITH_NAME);
            
            // Extensions should be unique, so don't copy the extension from the
            // source call group.  The admin must fill it in explicitly.
            groupDup.setExtension(null);
            
            groupDup.setEnabled(false);
            storeCallGroup(groupDup);
        }
    }

    /**
     * Remove all call groups - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection callGroups = template.loadAll(CallGroup.class);
        template.deleteAll(callGroups);
        Collection orbits = template.loadAll(ParkOrbit.class);
        template.deleteAll(orbits);
    }

    /**
     * Sends notification to profile generator to trigger alias generation
     */
    public void activateCallGroups() {
        m_replicationContext.generate(DataSet.ALIAS);
    }

    /**
     * Generate aliases for all call groups
     * Park orbits do not generate any aliases - registrar handles it directly
     */
    public Collection getAliasMappings() {
        final String dnsDomain = m_coreContext.getDomainName();
        Collection callGroups = getCallGroups();
        List allAliases = new ArrayList(callGroups.size());
        for (Iterator i = callGroups.iterator(); i.hasNext();) {
            CallGroup cg = (CallGroup) i.next();
            allAliases.addAll(cg.generateAliases(dnsDomain));
        }
        return allAliases;
    }    

    public void storeParkOrbit(ParkOrbit parkOrbit) {
        // Check for duplicate names and extensions before saving the park orbit
        String name = parkOrbit.getName();
        final String parkOrbitTypeName = "call park extension";
        DaoUtils.checkDuplicates(getHibernateTemplate(), parkOrbit,
                NAME_PROP_NAME, new NameInUseException(parkOrbitTypeName, name));
        String extension = parkOrbit.getExtension();
        DaoUtils.checkDuplicates(getHibernateTemplate(), parkOrbit,
                EXTENSION_PROP_NAME, new ParkOrbitExtensionInUseException(extension));

        getHibernateTemplate().saveOrUpdate(parkOrbit);
    }

    public void removeParkOrbits(Collection ids) {
        if (ids.isEmpty()) {
            return;
        }
        removeAll(ParkOrbit.class, ids);
        activateParkOrbits();
    }

    void removeAll(Class klass, Collection ids) {
        HibernateTemplate template = getHibernateTemplate();
        Collection entities = new ArrayList(ids.size());
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Object entity = template.load(klass, id);
            entities.add(entity);
        }
        template.deleteAll(entities);
    }

    public ParkOrbit loadParkOrbit(Integer id) {
        return (ParkOrbit) getHibernateTemplate().load(ParkOrbit.class, id);
    }

    public Collection getParkOrbits() {
        return getHibernateTemplate().loadAll(ParkOrbit.class);
    }

    public void activateParkOrbits() {
        m_replicationContext.generate(DataSet.ALIAS);
        try {
            Collection orbits = getParkOrbits();
            BackgroundMusic defaultMusic = getBackgroundMusic();
            m_orbitsGenerator.generate(defaultMusic, orbits);
            m_orbitsGenerator.writeToFile();
        } catch (IOException e) {
            new RuntimeException("Activating call parking configuration failed.", e);
        }
    }

    public String getDefaultMusicOnHold() {
        return getBackgroundMusic().getMusic();
    }

    public void setDefaultMusicOnHold(String music) {
        BackgroundMusic backgroundMusic = getBackgroundMusic();
        backgroundMusic.setMusic(music);
        getHibernateTemplate().saveOrUpdate(backgroundMusic);
    }

    private BackgroundMusic getBackgroundMusic() {
        List musicList = getHibernateTemplate().loadAll(BackgroundMusic.class);
        if (!musicList.isEmpty()) {
            return (BackgroundMusic) musicList.get(0);
        }
        return new BackgroundMusic();
    }

    /**
     * Removes all rings associated with a removed user. This is temporary. In the long term we
     * will introduce hibernate dependency between the users and rings table.
     * 
     * Note: we cannot just blindly removed rings from database, there is a cascade relationship
     * between call groups and rings, hibernate will resave the ring if it's removed from database
     * but not from the call group
     * 
     * This function is called from legacy notification service, there is no need to activate call
     * groups - they will be activated anyway because alias generation follows user deletion.
     * 
     * @param userId id of the user that us being deleted
     * 
     */
    public void removeUser(Integer userId) {
        final HibernateTemplate hibernate = getHibernateTemplate();
        Collection rings = hibernate.findByNamedQueryAndNamedParam("userRingsForUserId",
                "userId", userId);
        for (Iterator i = rings.iterator(); i.hasNext();) {
            UserRing ring = (UserRing) i.next();
            CallGroup callGroup = ring.getCallGroup();
            callGroup.removeRing(ring);
            hibernate.save(callGroup);
        }
    }

    private class NameInUseException extends UserException {
        private static final String ERROR = "There is already a {0} with the name \"{1}\". "
                + "Please choose another name for this {0}.";

        public NameInUseException(String objectType, String name) {
            super(ERROR, objectType, name);
        }
    }

    private class CallGroupExtensionInUseException extends UserException {
        private static final String ERROR =
            "Extension {0} is already in use. Please choose another extension for this hunt group.";

        public CallGroupExtensionInUseException(String extension) {
            super(ERROR, extension);
        }
    }

    private class ParkOrbitExtensionInUseException extends UserException {
        private static final String ERROR =
            "Extension {0} is already in use. Please choose another call park extension.";

        public ParkOrbitExtensionInUseException(String extension) {
            super(ERROR, extension);
        }
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            getHibernateTemplate().update(user);
            removeUser(user.getId());
        }
    }

    // trivial setters
    public void setCoreContext(CoreManager coreContext) {
        m_coreContext = coreContext;
    }

    public void setOrbitsGenerator(Orbits orbitsGenerator) {
        m_orbitsGenerator = orbitsGenerator;
    }

    public void setReplicationContext(SipxReplicationManager replicationContext) {
        m_replicationContext = replicationContext;
    }

    /**
     * Implement AliasOwner.isAliasInUse.
     * CallGroupManagerImpl owns aliases for call groups and park orbits.
     */
    public boolean isAliasInUse(String alias) {
        return isCallGroupAliasInUse(alias) || isParkOrbitAliasInUse(alias);
    }
    
    private boolean isCallGroupAliasInUse(String alias) {
        // Look for the ID of a call group with the specified alias/extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_CALL_GROUP_IDS_WITH_EXTENSION, VALUE, alias);
        return CollectionUtils.safeSize(objs) > 0;
    }
    
    private boolean isParkOrbitAliasInUse(String alias) {
        // Look for the ID of a park orbit with the specified alias/extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_PARK_ORBIT_IDS_WITH_EXTENSION, VALUE, alias);
        return CollectionUtils.safeSize(objs) > 0;        
    }

}
