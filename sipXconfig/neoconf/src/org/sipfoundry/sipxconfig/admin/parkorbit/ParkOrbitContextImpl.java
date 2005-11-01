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
package org.sipfoundry.sipxconfig.admin.parkorbit;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.common.CollectionUtils;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class ParkOrbitContextImpl extends SipxHibernateDaoSupport implements ParkOrbitContext {
    private static final String NAME_PROP_NAME = "name";
    private static final String EXTENSION_PROP_NAME = "extension";
    private static final String VALUE = "value";
    private static final String QUERY_PARK_ORBIT_IDS_WITH_ALIAS = "parkOrbitIdsWithAlias";

    private SipxReplicationContext m_replicationContext;
    private Orbits m_orbitsGenerator;

    public void storeParkOrbit(ParkOrbit parkOrbit) {
        // Check for duplicate names and extensions before saving the park orbit
        String name = parkOrbit.getName();
        final String parkOrbitTypeName = "call park extension";
        DaoUtils.checkDuplicates(getHibernateTemplate(), parkOrbit, NAME_PROP_NAME,
                new NameInUseException(parkOrbitTypeName, name));
        String extension = parkOrbit.getExtension();
        DaoUtils.checkDuplicates(getHibernateTemplate(), parkOrbit, EXTENSION_PROP_NAME,
                new ExtensionInUseException(parkOrbitTypeName, extension));

        getHibernateTemplate().saveOrUpdate(parkOrbit);
    }

    public void removeParkOrbits(Collection ids) {
        if (ids.isEmpty()) {
            return;
        }
        removeAll(ParkOrbit.class, ids);
        activateParkOrbits();
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

    public void setOrbitsGenerator(Orbits orbitsGenerator) {
        m_orbitsGenerator = orbitsGenerator;
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public boolean isAliasInUse(String alias) {
        // Look for the ID of a park orbit with the specified alias as its name or extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_PARK_ORBIT_IDS_WITH_ALIAS, VALUE, alias);
        return CollectionUtils.safeSize(objs) > 0;        
    }

    /**
     * Remove all park orbits - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection orbits = template.loadAll(ParkOrbit.class);
        template.deleteAll(orbits);
    }

    // Park orbits do not generate any aliases - registrar handles this directly.
    // Therefore we return the empty collection here, even though park orbit names
    // and extensions both yield SIP aliases.
    public Collection getAliasMappings() {
        return Collections.EMPTY_LIST;
    }

}
