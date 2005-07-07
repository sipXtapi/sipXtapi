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



package com.pingtel.pds.pgs.user;

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

public class UserGroupBean extends JDBCAwareEJB implements EntityBean, UserGroupBusiness {

    // Home references
    private OrganizationHome m_organizationHome;

    // Clearing Entity Bean references
    private Organization m_organizationEJBObject;

    // Bean Context
    private EntityContext m_ctx;

    public Integer id;
    public String name;
    public Integer parentID;
    public Integer refConfigSetID;
    public Integer organizationID;


    // accessors for immutable fields
    public Integer getID () {
        return this.id;
    }

    public Integer getOrganizationID() {
        return organizationID;
    }

    // accessors/mutators for the rest
    public String getName () {
        return this.name;
    }

    public void setName ( String name ) throws PDSException {
        if ( name != null && name.length() > MAX_NAME_LEN ) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3028",
                                        new Object[]{
                                            "name",
                                            new Integer(MAX_NAME_LEN)}));
        }

        this.name = name;
    }

    public Integer getParentID() {
      return this.parentID;
    }

    public void setParentID ( Integer parentID ) {
      this.parentID = parentID;
    }

    public Integer getRefConfigSetID () {
      return this.refConfigSetID;
    }


    public void setRefConfigSetID ( Integer refConfigSetID ) {
      this.refConfigSetID = refConfigSetID;
    }

    public String getExternalID () throws PDSException {
        Organization organization = getMyOrganization();

        try {
            return "name: " + this.name +
                " organization: " + organization.getExternalID();
        }
        catch ( Exception ex ) {
            logFatal( ex.toString(), ex  );
            throw new EJBException ( ex );
        }
    }

    private Organization getMyOrganization() throws PDSException {
        if ( m_organizationEJBObject == null ) {
            try {
                m_organizationEJBObject = m_organizationHome.findByPrimaryKey( getOrganizationID() );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E1018",
                                            new Object[]{ getOrganizationID() }),
                    ex);
            }
            catch ( RemoteException ex ) {
                logFatal ( ex.toString(), ex );

                throw new EJBException( ex.toString());
            }
        }

        return m_organizationEJBObject;
    }


    public Integer ejbCreate (  String name,
                                Integer organizationID,
                                Integer refConfigSetID,
                                Integer parentID )
        throws CreateException, PDSException {

        this.id = new Integer ( getNextSequenceValue( "USER_SEQ" ) );
        setName(name);
        this.name = name;
        this.organizationID = organizationID;
        this.refConfigSetID = refConfigSetID;
        this.parentID = parentID;

        return null;
    }


    public void ejbPostCreate ( String name,
                                Integer organizationID,
                                Integer refConfigSetID,
                                Integer parentID ) {}

    public void ejbLoad () {}

    public void ejbStore  () {}

    public void ejbRemove () {}

    public void ejbActivate () {
        m_organizationEJBObject = null;
    }

    public void ejbPassivate () {}

    public void setEntityContext(EntityContext ctx) {
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

}
