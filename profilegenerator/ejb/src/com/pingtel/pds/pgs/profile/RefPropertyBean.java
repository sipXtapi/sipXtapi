/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.profile;

import java.util.ArrayList;
import java.util.Iterator;
import java.sql.SQLException;

import javax.ejb.CreateException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;
import javax.ejb.EJBException;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

public class RefPropertyBean extends JDBCAwareEJB implements EntityBean, PDSDefinitions, RefPropertyBusiness {

    // ejb 'state' variables
    public Integer id;
    public String name;
    public int profileType;
    public String content;
    public String code;

    public Integer getID () {
        return this.id;
    }

    public int getProfileType () {
        return this.profileType;
    }

    public String getName () {
        return this.name;
    }

    public String getContent ()  {
        return content;
    }

    public void setContent ( String content ) {
        this.content = content;
    }

    public String getExternalID () {
        return "name: " + this.name;
    }

    public String getCode() {
        return this.code;
    }


    public Integer ejbCreate (  String name,
                                int profileType,
                                String code,
                                String content )
        throws CreateException {

        this.id = new Integer ( getNextSequenceValue ( "REF_PROP_SEQ") );
        this.name = name;
        this.profileType = profileType;
        this.content = content;
        this.code = code;

        return null;
    }

    public void ejbPostCreate ( String name,
                                int profileType,
                                String code,
                                String content ) {}

    public void ejbLoad () {}

    public void ejbStore  () {}

    public void ejbRemove () {}

    public void ejbActivate () {}
    public void ejbPassivate () {}

    public void setEntityContext ( EntityContext ctx ) {
        this.m_ctx = ctx;
    }

    public void unsetEntityContext () {
        this.m_ctx = null;
    }

    private EntityContext m_ctx;
}
