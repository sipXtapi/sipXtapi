/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/CoreSoftwareBean.java#4 $
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

import java.sql.SQLException;

import javax.ejb.CreateException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;
import javax.ejb.RemoveException;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

public class CoreSoftwareBean extends JDBCAwareEJB implements EntityBean, CoreSoftwareBusiness {

    public Integer id;
    public java.sql.Date installedDate;
    public String description;
    public String version;
    public String name;

    public Integer getID() {
        return this.id;
    }


    public java.sql.Date getInstalledDate() {
        return this.installedDate;
    }

    public String getDescription () {
        return this.description;
    }

    /** Gets the software version identifier */
    public String getVersion () {
        return this.version;
    }

    /** Gets the software version identifier */
    public String getName () {
        return this.name;
    }

    public String getExternalID () {
        return "name: " + this.name + " version: " + this.version;
    }

    /** This signature must match that in the home interface to create the bean */
    public Integer ejbCreate ( String description,
                               String version,
                               String name,
                               String deviceTypeID )
        throws CreateException {

        this.id = new Integer( this.getNextSequenceValue( "CSD_SEQ" ) );
        this.installedDate = new java.sql.Date( new java.util.Date().getTime() );

        this.description = description;
        this.version = version;
        this.name = name;
        // Convention for CMP beans, return null (container does something with it)


        return null;
    }

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbPostCreate ( String description,
                                String version,
                                String name,
                                String deviceTypeID ) throws CreateException {

        try {
            executePreparedUpdate(  "INSERT INTO phone_type_csd_assoc ( CSD_ID, " +
                                    "                                   PT_ID ) " +
                                    "VALUES  ( ?, ? ) ",
                                    new Object [] { this.id, deviceTypeID } );

        }
        catch ( SQLException ex ) {
            m_ctx.setRollbackOnly();

            throw new CreateException ( ex.toString() );
        }

    }

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbLoad () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbStore  () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbRemove () throws RemoveException {
        try {
            executePreparedUpdate(  "DELETE FROM    PHONE_TYPE_CSD_ASSOC " +
                                    "WHERE          csd_id = ? ",
                                    new Object [] { this.id } );

        }
        catch ( SQLException ex ) {
            m_ctx.setRollbackOnly();

            throw new RemoveException ( ex.toString() );
        }
    }

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbActivate () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbPassivate () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void setEntityContext ( EntityContext ctx ) {
        this.m_ctx = ctx;
    }

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void unsetEntityContext ( ) {
        this.m_ctx = null;
    }

    private EntityContext m_ctx;
}