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

import java.rmi.RemoteException;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;
import javax.ejb.FinderException;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import com.pingtel.pds.common.PDSException;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;

public class RefConfigurationSetBean extends JDBCAwareEJB implements EntityBean, RefConfigurationSetBusiness {

    // Home interfaces
    private OrganizationHome m_organizationHome;

    // Clearing Entity Bean refs.
    private Organization m_organizationEJBObject;

    // Bean context
    private EntityContext m_ctx;


    // State/attribute fields
    public Integer id;
    public String name;
    public Integer organizationID;


    public Integer getID () {
        return this.id;
    }

    public Integer getOrganizationID () {
        return this.organizationID;
    }

    public String getName () {
        return this.name;
    }


    public String getExternalID () {

        try {
            return "name: " + this.name;
        }
        catch ( Exception ex ) {
            logFatal( ex.toString(), ex  );
            throw new EJBException ( ex );
        }
    }


    public Integer ejbCreate ( String name, Integer organizationID )
        throws CreateException {

        this.id = new Integer( getNextSequenceValue( "RCS_SEQ" ) );
        this.name = name;
        this.organizationID = organizationID;

        return null;
    }


    public void ejbPostCreate ( String id, Integer organizationID ) {}

    public void ejbLoad () {}

    public void ejbStore  () {}

    public void ejbRemove () {}

    public void ejbActivate () {
        m_organizationEJBObject = null;
    }

    public void ejbPassivate () {}

    public void setEntityContext ( EntityContext ctx ) {
        this.m_ctx = ctx;

        try {
            Context initial = new InitialContext();

            m_organizationHome = (OrganizationHome) initial.lookup ("Organization" );
        } catch (NamingException ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }
    }

    public void unsetEntityContext ( ) {
        this.m_ctx = null;
    }


    private Organization getMyOrganization() throws PDSException {
        if ( m_organizationEJBObject == null ) {
            try {
                m_organizationEJBObject =
                    m_organizationHome.findByPrimaryKey( getOrganizationID() );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1018",
                                                new Object[]{ getOrganizationID() }),
                        ex);
            }
            catch ( RemoteException ex ) {
                logFatal ( ex.toString(), ex  );

                throw new EJBException( ex.toString());
            }
        }

        return m_organizationEJBObject;
    }

}
