/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/ManufacturerBean.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.phone;

import javax.ejb.CreateException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

/**
 * LogicalPhoneBean is the EJB implementation of Manufacturer
 *
 */
public class ManufacturerBean extends JDBCAwareEJB implements EntityBean, ManufacturerBusiness {

    public Integer id;
    public String name;


    public Integer getID() {
        return this.id;
    }

    public String getName() {
        return this.name;
    }

    public void setName ( String name ) {
        this.name = name;
    }

    public Integer ejbCreate ( String name )
        throws CreateException {

        this.id = new Integer ( getNextSequenceValue( "MANU_SEQ" ) );
        this.name = name;
        return null;
    }

    public String getExternalID () {
        return "name: " + this.name;
    }

    public void ejbPostCreate ( String name ){}

    public void ejbLoad () {}

    public void ejbStore  () {}

    public void ejbRemove () {}

    public void ejbActivate () {}

    public void ejbPassivate () {}

    public void setEntityContext ( EntityContext ctx ) {
        this.m_ctx = ctx;
    }
    public void unsetEntityContext ( ) {
        this.m_ctx = null;
    }

    private EntityContext m_ctx;

}