/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/organization/OrganizationBean.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.organization;

import javax.ejb.CreateException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

/**
 *  Description of the Class
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public class OrganizationBean extends JDBCAwareEJB implements EntityBean, OrganizationBusiness {

    /**
     *  Description of the Field
     */
    public Integer id;
    /**
     *  Description of the Field
     */
    public String name;
    /**
     *  Description of the Field
     */
    public Integer parentID;


    public String dnsDomain;


    public int stereotype;

    // Accessors for immutable fields
    /**
     *  Gets the iD attribute of the OrganizationBean object
     *
     *@return    The iD value
     */
    public Integer getID() {
        return this.id;
    }


    // Accessors/mutators for fields
    /**
     *  Gets the name attribute of the OrganizationBean object
     *
     *@return    The name value
     */
    public String getName() {
        return this.name;
    }


    /**
     *  Sets the name attribute of the OrganizationBean object
     *
     *@param  name  The new name value
     */
    public void setName(String name) {
        this.name = name;
    }


    /**
     *  Sets the parentID attribute of the OrganizationBean object
     *
     *@param  parentID  The new parentID value
     */
    public void setParentID(Integer parentID) {
        this.parentID = parentID;
    }


    /**
     *  Gets the parentID attribute of the OrganizationBean object
     *
     *@return    The parentID value
     */
    public Integer getParentID() {
        return parentID;
    }


    public String getDNSDomain () {
        return this.dnsDomain;
    }


    public void setDNSDomain ( String dnsDomain ) {
        this.dnsDomain = dnsDomain;
    }


    public int getStereotype () {
        return this.stereotype;
    }



    public String getExternalID () {
        return "name: " + this.name;
    }

    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  parentID              Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  CreateException   Description of the Exception
     */
    public Integer ejbCreate(   String name,
                                Integer parentID,
                                int stereotype,
                                String dnsDomain )
             throws CreateException {

        this.id = new Integer(this.getNextSequenceValue("ORG_SEQ"));
        this.name = name;
        this.parentID = parentID;
        this.stereotype = stereotype;
        this.dnsDomain = dnsDomain;

        return null;
    }


    /**
     *  Description of the Method
     *
     *@param  name      Description of the Parameter
     *@param  parentID  Description of the Parameter
     */
    public void ejbPostCreate(  String name,
                                Integer parentID,
                                int stereotype,
                                String dnsDomain) { }


    /**
     *  Description of the Method
     */
    public void ejbLoad() { }


    /**
     *  Description of the Method
     */
    public void ejbStore() {

    }


    /**
     *  Description of the Method
     */
    public void ejbRemove() { }


    /**
     *  Description of the Method
     */
    public void ejbActivate() { }


    /**
     *  Description of the Method
     */
    public void ejbPassivate() { }


    /**
     *  Sets the entityContext attribute of the OrganizationBean object
     *
     *@param  ctx  The new entityContext value
     */
    public void setEntityContext(EntityContext ctx) {
        this.ctx = ctx;
    }


    /**
     *  Description of the Method
     */
    public void unsetEntityContext() {
        this.ctx = null;
    }


    private EntityContext ctx;
}
