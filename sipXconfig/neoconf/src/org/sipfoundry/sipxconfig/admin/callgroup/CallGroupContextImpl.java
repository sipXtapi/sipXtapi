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

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserDeleteListener;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Hibernate implementation of the call group context
 */
public class CallGroupContextImpl extends HibernateDaoSupport implements CallGroupContext {
    private CoreContext m_coreContext;

    private Orbits m_orbitsGenerator;

    private SipxProcessContext m_processContext;

    public CallGroup loadCallGroup(Integer id) {
        return (CallGroup) getHibernateTemplate().load(CallGroup.class, id);
    }

    public void storeCallGroup(CallGroup callGroup) {
        // only validate callgroups that are being enabled
        if (callGroup.isEnabled()) {
            String name = callGroup.getName();
            DaoUtils.checkDuplicates(getHibernateTemplate(), callGroup,
                    "callGroupIdsEnabledWithName", name, new NameInUseException(name));
            String extension = callGroup.getExtension();
            DaoUtils.checkDuplicates(getHibernateTemplate(), callGroup,
                    "callGroupIdsEnabledWithExtension", extension, new ExtensionInUseException(
                            extension));
        }
        getHibernateTemplate().saveOrUpdate(callGroup);
    }

    public void removeCallGroups(Collection ids) {
        removeAll(CallGroup.class, ids);
    }

    public List getCallGroups() {
        return getHibernateTemplate().loadAll(CallGroup.class);
    }

    public void duplicateCallGroups(Collection ids) {
        for (Iterator i = ids.iterator(); i.hasNext();) {
            CallGroup group = loadCallGroup((Integer) i.next());
            CallGroup groupDup = (CallGroup) group.duplicate();
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
     * Sends notification to progile generator to trigger alias generation
     */
    public void activateCallGroups() {
        m_processContext.generate(DataSet.ALIAS);
    }

    /**
     * Generate aliases for all call groups
     */
    public List getAliases() {
        final String dnsDomain = m_coreContext.getDomainName();
        List callGroups = getCallGroups();
        List allAliases = new ArrayList();
        for (Iterator i = callGroups.iterator(); i.hasNext();) {
            CallGroup cg = (CallGroup) i.next();
            allAliases.addAll(cg.generateAliases(dnsDomain));
        }
        return allAliases;
    }

    public void storeParkOrbit(ParkOrbit parkOrbit) {
        getHibernateTemplate().saveOrUpdate(parkOrbit);
    }

    public void removeParkOrbits(Collection ids) {
        removeAll(ParkOrbit.class, ids);
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

    private class ExtensionInUseException extends UserException {
        private static final String ERROR = "Extension {0} is already used in the system. "
                + "Please choose another extension before enabling this hunt group.";

        public ExtensionInUseException(String extension) {
            super(ERROR, extension);
        }
    }

    private class NameInUseException extends UserException {
        private static final String ERROR = "Name {0} is already used in the system. "
                + "Please choose another name before enabling this hunt group.";

        public NameInUseException(String name) {
            super(ERROR, name);
        }
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            removeUser(user.getId());
        }
    }
    
    // trivial setters
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setOrbitsGenerator(Orbits orbitsGenerator) {
        m_orbitsGenerator = orbitsGenerator;
    }

    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }
}
