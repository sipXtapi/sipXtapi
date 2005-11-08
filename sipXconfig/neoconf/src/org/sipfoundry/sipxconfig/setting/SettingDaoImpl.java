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
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends SipxHibernateDaoSupport implements SettingDao {    
    private static final String RESOURCE_PARAM = "resource";
    private static final String NAME_PARAM = "name";
    
    public Group getGroup(Integer groupId) {
        return (Group) getHibernateTemplate().load(Group.class, groupId);
    }

    public void deleteGroup(Group group) {
        getHibernateTemplate().delete(group);
    }
    
    public void storeValueStorage(ValueStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public ValueStorage loadValueStorage(Integer storageId) {
        return (ValueStorage) getHibernateTemplate().load(ValueStorage.class, storageId);    
    }

    public void saveGroup(Group group) {
        checkDuplicates(group);
        assignWeightToNewGroups(group);
        getHibernateTemplate().saveOrUpdate(group);
    }
    
    void assignWeightToNewGroups(Group group) {
        if (group.isNew() && group.getWeight() == null) {
            GroupWeight weight = new GroupWeight();
            getHibernateTemplate().save(weight);
            group.setWeight(weight.getWeight());
            getHibernateTemplate().delete(weight); // delete not strictly nec.
        }        
    }

    
    void checkDuplicates(Group group) {
        String[] params = new String[] {
            RESOURCE_PARAM,
            NAME_PARAM
        };
        Object[] values = new Object[] {
                group.getResource(),
                group.getName()
        };
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam("groupIdsWithNameAndResource", params, values);
        DaoUtils.checkDuplicates(group, objs, new DuplicateGroupException(group.getName()));        
    }
    
    private class DuplicateGroupException extends UserException {
        private static final String ERROR = "A group with name: {0} already exists.";

        public DuplicateGroupException(String name) {
            super(ERROR, name);
        }
    }
    
    public Group getGroupByName(String resource, String name) {
        String[] params = new String[] { 
            RESOURCE_PARAM, 
            NAME_PARAM 
        }; 
        Object[] values = new Object[] { 
            resource, 
            name 
        }; 
        String query = "groupsByResourceAndName";
        List groups = getHibernateTemplate().findByNamedQueryAndNamedParam(query, params, values);
        return (Group) DaoUtils.requireOneOrZero(groups, query);
    }

    public String getGroupsAsString(List groups) {
        String groupsString = StringUtils.join(groups.iterator(), ", ");
        return groupsString;
    }

    public List getGroupsByString(String resource, String groupString) {
        String[] groupNames = groupString.split(",\\w");
        List groups = new ArrayList(groupNames.length);
        for (int i = 0; i < groupNames.length; i++) {
            Group g = getGroupByName(resource, groupNames[i]);
            groups.add(g);
        }
        
        return groups;            
    }

    public Group loadGroup(Integer id) {
        return (Group) getHibernateTemplate().load(Group.class, id);
    }
    
    public List getGroups(String resource) {
        List groups = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResource", 
                RESOURCE_PARAM, resource);
        return groups;
    }

    public Map getGroupMemberCountIndexedByGroupId(Class groupOwner) {
        String query = "select g.id, count(*) from " + groupOwner.getName() 
            + " o join o.groups g group by g.id";
        List l = getHibernateTemplate().find(query);
        Map members = asMap(l);
        
        return members;
    }
    
    private Map asMap(List l) {
        Map m = new HashMap(l.size());
        for (int i = 0; i < l.size(); i++) {
            Object[] row = (Object[]) l.get(i);
            m.put(row[0], row[1]);
        }
        
        return m;        
    }

    /**
     * Internal object, only used to generate group weights in DB neutral way
     */
    static class GroupWeight {
        
        private Integer m_weight = new Integer(-1);

        public Integer getWeight() {
            return m_weight;
        }

        public void setWeight(Integer weight) {
            m_weight = weight;
        }
    }

    public Group getGroupCreateIfNotFound(String resourceId, String groupName) {
        Group g = getGroupByName(resourceId, groupName);
        if (g == null) {
            g = new Group();
            g.setResource(resourceId);
            g.setName(groupName);
            saveGroup(g);
        }
        return g;
    }
}
