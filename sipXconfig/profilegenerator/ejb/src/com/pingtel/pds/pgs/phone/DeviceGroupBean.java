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

package com.pingtel.pds.pgs.phone;

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

/**
 *  DeviceGroupBean is the EJB implementation of DeviceGroup EJBean
 *
 *@author IB
 */
public class DeviceGroupBean extends JDBCAwareEJB
        implements EntityBean, DeviceGroupBusiness {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    /**
     * PK for the DeviceGroup.  This is assigned an auto-generated
     * number.
     */
    public Integer id;

    /**
     * PK of the DeviceGroup which is the parent of this DeviceGroup if there
     * is one.
     */
    public Integer parentID;

    /**
     * name of this DeviceGroup.  This is how the users of the system know the
     * DeviceGroup.
     */
    public String name;

    /**
     * PK of the Organization that this DeviceGroup belongs to
     * @see com.pingtel.pds.pgs.organization.OrganizationBean
     */
    public Integer organizationID;

    /**
     * PK of the RefConfigurationSet that this DeviceGroup uses
     * @see com.pingtel.pds.pgs.profile.RefConfigurationSetBean
     */
    public Integer refConfigSetID;

    // Home references
    private OrganizationHome m_organizationHome;

    // Clearing Entity Bean references;
    private Organization m_organizationEJBObject;

    // Bean context
    private EntityContext mCTX;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * accessor for the ID field.
     * @return the id (PK) of this DeviceGroup bean
     */
    public Integer getID() {
        return this.id;
    }


    /**
     * accessor for the parentID field.
     *
     * @return the parentID of this DeviceGroup bean if it has a parent -
     * otherwise it will return null.
     */
    public Integer getParentID() {
        return this.parentID;
    }


    /**
     * mutator method for the parentID field.
     *
     * @param parentID the PK of the DeviceGroup that should become the
     * parent of this DeviceGroup.
     */
    public void setParentID(Integer parentID) {
        this.parentID = parentID;
    }


    /**
     * accessor for the name field.
     * @return the name of this DeviceGroup bean
     */
    public String getName() {
        return this.name;
    }


    /**
     * mutator method for the name field.
     * @param name the new name value
     */
    public void setName(String name) throws PDSException {
        if ( name != null && name.length() > MAX_NAME_LEN ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3028",
                                        new Object[]{
                                            "name",
                                            new Integer(MAX_NAME_LEN)}));
        }
        this.name = name;
    }


    /**
     * accessor method for the organizationID field.
     *
     * @return the PK of the Organization which this DeviceGroup bean
     * belongs to
     */
    public Integer getOrganizationID() {
        return organizationID;
    }


    /**
     * mutator method for the refConfigSetID field
     *
     * @param refConfigSetID PK of the RefConfigurationSet you are assigning
     * to this DeviceGroup bean.
     * @see com.pingtel.pds.pgs.profile.RefConfigurationSetBean
     */
    public void setRefConfigSetID(Integer refConfigSetID) {
        this.refConfigSetID = refConfigSetID;
    }


    /**
     * accessor method for the refConfigSetID field
     *
     * @return the PK of the refConfigSet which is associated with this
     * DeviceGroup bean.
     * @see com.pingtel.pds.pgs.profile.RefConfigurationSetBean
     */
    public Integer getRefConfigSetID() {
        return refConfigSetID;
    }

    /**
     * the 'externalID' of the DeviceGroup is a user friendly description
     * which is used to fully describe the DeviceGroup is includes its name
     * and the the name of the Organization that it belongs to.
     * @return description of the DeviceGroup
     * @throws PDSException for application level errors
     */
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


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    private Organization getMyOrganization() throws PDSException {
        if ( m_organizationEJBObject == null ) {
            try {
                m_organizationEJBObject =
                        m_organizationHome.findByPrimaryKey( getOrganizationID() );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

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

    /**
     * standard EJ Bean create method
     *
     *@param name name that will be given to the new DeviceGroup
     *@param organizationID PK of the Organization that this DeviceGroup
     * is being created into
     *@param  refConfigSetID PK of the RefConfigurationSet that this
     * DeviceGroup will use
     *@return DeviceGroup EJBObject for the new DeviceGroup
     *@exception CreateException for application level errors
     */
    public Integer ejbCreate(   String name,
                                Integer organizationID,
                                Integer refConfigSetID)
            throws CreateException, PDSException {

        this.id = new Integer(getNextSequenceValue("PHONE_GROUP_SEQ"));
        setName(name);
        this.refConfigSetID = refConfigSetID;
        this.organizationID = organizationID;

        return null;
    }

    /**
     *  Standard CMP implementation
     */
    public void ejbPostCreate(  String name,
                                Integer organizationID,
                                Integer refConfigSetID) { }


    /**
     *  Standard CMP implementation
     */
    public void ejbLoad() { }


    /**
     *  Standard CMP implementation
     */
    public void ejbStore() { }


    /**
     *  Standard CMP implementation
     */
    public void ejbRemove() { }


    /**
     *  Standard CMP implementation
     */
    public void ejbActivate() {
        m_organizationEJBObject = null;
    }


    /**
     *  Standard CMP implementation
     */
    public void ejbPassivate() { }


    /**
     * Standard CMP implementation
     *
     *@param ctx entityContext value passed from the EJ Bean containter
     */
    public void setEntityContext(EntityContext ctx) {
        this.mCTX = ctx;

        try {
            Context initial = new InitialContext();

            m_organizationHome = (OrganizationHome)
                    initial.lookup ("Organization" );

        } catch (NamingException ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }
    }


    /**
     *  Standard CMP implementation
     */
    public void unsetEntityContext() {
        this.mCTX = null;
    }


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
