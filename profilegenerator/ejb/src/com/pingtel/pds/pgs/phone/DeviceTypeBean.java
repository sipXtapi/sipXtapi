/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/DeviceTypeBean.java#4 $
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
 *  Description of the Class
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public class DeviceTypeBean extends JDBCAwareEJB implements EntityBean, DeviceTypeBusiness {

    /**
     *  Description of the Field
     */
    public Integer id;
    /**
     *  Description of the Field
     */
    public Integer manufacturerID;
    /**
     *  Description of the Field
     */
    public String model;


    /**
     *  Gets the iD attribute of the DeviceTypeBean object
     *
     *@return    The iD value
     */
    public Integer getID() {
        return this.id;
    }


    /**
     *  Gets the manufacturerID attribute of the DeviceTypeBean object
     *
     *@return    The manufacturerID value
     */
    public Integer getManufacturerID() {
        return this.manufacturerID;
    }


    /**
     *  Sets the manufacturerID attribute of the DeviceTypeBean object
     *
     *@param  manufacturerID  The new manufacturerID value
     */
    public void setManufacturerID(Integer manufacturerID) {
        this.manufacturerID = manufacturerID;
    }


    /**
     *  Gets the model attribute of the DeviceTypeBean object
     *
     *@return    The model value
     */
    public String getModel() {
        return this.model;
    }


    /**
     *  Sets the model attribute of the DeviceTypeBean object
     *
     *@param  model  The new model value
     */
    public void setModel(String model) {
        this.model = model;
    }


    public String getExternalID () {
        return "model: " + this.model;
    }


    /**
     *  Description of the Method
     *
     *@param  manufacturerID       Description of the Parameter
     *@param  model                Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     */
    public Integer ejbCreate(Integer manufacturerID, String model)
             throws CreateException {

        this.id = new Integer(getNextSequenceValue("PT_SEQ"));
        this.manufacturerID = manufacturerID;
        this.model = model;

        return null;
    }


    /**
     *  Description of the Method
     *
     *@param  manufacturerID  Description of the Parameter
     *@param  model           Description of the Parameter
     */
    public void ejbPostCreate(Integer manufacturerID, String model) { }


    /**
     *  Description of the Method
     */
    public void ejbLoad() { }


    /**
     *  Description of the Method
     */
    public void ejbStore() { }


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
     *  Sets the entityContext attribute of the DeviceTypeBean object
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
