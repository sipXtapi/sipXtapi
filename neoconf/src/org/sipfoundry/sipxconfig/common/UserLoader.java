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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Query;
import org.hibernate.Session;

/** Helper class for loading users by query */
public class UserLoader {
    private static final Log LOG = LogFactory.getLog(UserLoader.class);
    
    // names of query parameters
    private static final String PARAM_USER_NAME = "userName";
    private static final String PARAM_USER_NAME_OR_ALIAS = "userNameOrAlias";
    private static final String PARAM_FIRST_NAME = "firstName";
    private static final String PARAM_LAST_NAME = "lastName";

    private static final String SPACE_CHAR = " ";
    
    private Session m_session;
    private StringBuffer m_queryBuf;
    private boolean m_noWhere;    // true if we haven't added the SQL "where" keyword yet
    private List m_paramNames = new ArrayList();
    private List m_paramValues = new ArrayList();

    public UserLoader(Session session) {
        m_session = session;
    }
    
    public List loadUsers(final User userTemplate, boolean matchUserNameOrAlias) {
        init();
        
        // Add constraints
        handleUserNameAliasesConstraint(userTemplate, matchUserNameOrAlias);
        handleFirstNameConstraint(userTemplate);
        handleLastNameConstraint(userTemplate);

        // Sort by last name
        m_queryBuf.append("order by u.lastName asc");
        
        // Create the query and add parameters
        Query query = m_session.createQuery(m_queryBuf.toString());
        Iterator iter1 = m_paramNames.iterator();
        for (Iterator iter2 = m_paramValues.iterator(); iter2.hasNext();) {
            String name = (String) iter1.next();
            String value = (String) iter2.next();
            query.setString(name, value);
        }
        
        // Execute the query.  Eliminate any duplicates in the users list.
        // See http://www.hibernate.org/117.html#A11 -- we can't count on the "distinct"
        // keyword in HQL to remove duplicates, because the query may use an outer join.
        List users = query.list();
        users = removeDuplicateUsers(users);
        
        return users;
    }

    /**
     * Handle userName/aliases constraint
     */
    private void handleUserNameAliasesConstraint(final User userTemplate,
                                                 boolean matchUserNameOrAlias) {
        if (!StringUtils.isEmpty(userTemplate.getUserName())) {
            if (matchUserNameOrAlias) {
                m_queryBuf.append("left outer join u.aliases alias ");
                m_queryBuf.append("where (u.userName like :");
                m_queryBuf.append(PARAM_USER_NAME_OR_ALIAS);
                m_queryBuf.append(" or alias like :");
                m_queryBuf.append(PARAM_USER_NAME_OR_ALIAS);
                m_queryBuf.append(") ");
                
                m_paramNames.add(PARAM_USER_NAME_OR_ALIAS);
            } else {
                m_queryBuf.append("where u.userName like :");
                m_queryBuf.append(PARAM_USER_NAME);
                m_queryBuf.append(SPACE_CHAR);
                
                m_paramNames.add(PARAM_USER_NAME);
            }
            addParamValue_(userTemplate.getUserName());
            m_noWhere = false;  // we added "where" to the query string
        }
    }
    
    /**
     * Handle firstName constraint
     */
    private void handleFirstNameConstraint(final User userTemplate) {
        if (!StringUtils.isEmpty(userTemplate.getFirstName())) {
            startQueryConstraint();
            m_queryBuf.append("u.firstName like :");
            m_queryBuf.append(PARAM_FIRST_NAME);
            m_queryBuf.append(SPACE_CHAR);

            m_paramNames.add(PARAM_FIRST_NAME);
            addParamValue_(userTemplate.getFirstName());
        }
    }
    
    /**
     * Handle lastName constraint
     */
    private void handleLastNameConstraint(final User userTemplate) {
        if (!StringUtils.isEmpty(userTemplate.getLastName())) {
            startQueryConstraint();
            m_queryBuf.append("u.lastName like :");
            m_queryBuf.append(PARAM_LAST_NAME);
            m_queryBuf.append(SPACE_CHAR);

            m_paramNames.add(PARAM_LAST_NAME);
            addParamValue_(userTemplate.getLastName());
        }
    }

    /**
     * Add a param value to the list of values.
     * Append a SQL wildcard "%" to the end of the string.  
     */
    // Put an underscore at the end of the method name to suppress a bogus
    // warning from Checkstyle about this method being unused.
    private void addParamValue_(String value) {
        m_paramValues.add(value + "%");
    }

    /** 
     * For the query contraint, add SQL "where " if this is the first constraint
     * or "and " if this is a subsequent constraint.
     */
    private void startQueryConstraint() {
        if (m_noWhere) {
            m_queryBuf.append("where ");
            m_noWhere = false;
        } else {
            m_queryBuf.append("and ");
        }        
    }

    /** Initialize internal state to get ready for a new query */
    private void init() {
        m_queryBuf = new StringBuffer("from User u ");
        m_noWhere = true;
        m_paramNames.clear();
        m_paramValues.clear();       
    }
    
    /**
     * Remove duplicates from the users list and return the new list.
     */
    private List removeDuplicateUsers(List users) {        
        // Store each user in a map, indexed by userName and look for collisions.
        // userName is guaranteed to be unique.
        List uniqueUsers = new ArrayList(users.size());
        Map usersMap = new HashMap();
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
            if (!usersMap.containsKey(user.getUserName())) {
                usersMap.put(user.getUserName(), null);
                uniqueUsers.add(user);
            } else {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("Skipping duplicate user " + user);
                }
            }
        }
        return uniqueUsers;
    }

}
