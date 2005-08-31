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

import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SipxHibernateDaoSupport extends HibernateDaoSupport {

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }

}
