/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/RestartDevicesFacade.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.entity;

import javax.ejb.EJBException;
import javax.ejb.MessageDrivenBean;
import javax.ejb.MessageDrivenContext;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.naming.InitialContext;

import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.BaseEJB;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocate;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocateHome;
import com.pingtel.pds.pgs.user.UserGroupAdvocate;
import com.pingtel.pds.pgs.user.UserGroupAdvocateHome;

/**
 * RestartDevicesFacade is the MessageDrivenBean implementation class.
 * RestartDevicesFacade is an aysnchronous session facade for sending
 * restart requests to Devices in either a DeviceGroup or Devices
 * owned by Users in a UserGroup.
 *
 * @author IB
 * @see #onMessage
 */
public class RestartDevicesFacade extends BaseEJB
        implements MessageDrivenBean, MessageListener {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private MessageDrivenContext mCTX = null;

    private UserGroupAdvocate mUserGroupAdvocate = null;
    private DeviceGroupAdvocate mDeviceGroupAdvocate = null;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////

    public RestartDevicesFacade() { }


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     *  Standard EJB implementation
     */
    public void setMessageDrivenContext( MessageDrivenContext ctx )
            throws EJBException {

        this.mCTX = ctx;

        try {
            InitialContext initial = new InitialContext( );

            UserGroupAdvocateHome userGroupAdvocateHome =
                        (UserGroupAdvocateHome) initial.lookup( "UserGroupAdvocate");

            mUserGroupAdvocate = userGroupAdvocateHome.create();

            DeviceGroupAdvocateHome deviceGroupAdvocateHome =
                        (DeviceGroupAdvocateHome) initial.lookup( "DeviceGroupAdvocate");

            mDeviceGroupAdvocate = deviceGroupAdvocateHome.create();
        }
        catch ( Exception e ) {
            logFatal ( e.toString() );
            throw new EJBException ( "Error in RestaredDevicesFacade:setMessageDrivenContext " + e.toString() );
        }
    }

    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() {
        mCTX = null;
    }

    /**
     * onMessage is executed on callback from the EJ Bean container whenever
     * a JMS message is sent to the queue on which ProjectionFacade is bound.
     * Messages send to RestardDevicesFacade are used to send restart
     * requests to Devices in an aysnchronous manner.
     *
     * The message has to include the following bits of information as mapMessage
     * fields:
     *          "entitytype" - the type of group for which this restard is
     * intended.
     *          "entityid" - the PK of the particular group you want to restart.
     *
     * @param message JMS Message taken from the queue
     */
    public void onMessage( Message message ) {

        MapMessage mapMessage = (MapMessage) message;

        try {
            String entityType = mapMessage.getString( "entitytype" );
            String entityID = mapMessage.getString( "entityid" );

            if ( entityType.equals( "usergroup") ) {
                logDebug ( "RestartDevicesFacade: restarting devices for user group: " + entityID );
                mUserGroupAdvocate.restartDevices( entityID );
            }
            else if ( entityType.equals( "devicegroup") ) {
                logDebug ( "RestartDevicesFacade: restarting devices for device group: " + entityID );
                mDeviceGroupAdvocate.restartDevices( entityID );
            }
        }
        catch (RemoteException e) {
            logFatal ( e.getMessage() );
        }
        catch (JMSException e) {
            logFatal ( e.getMessage() );
        }
        catch (PDSException e) {
            logError ( e.getMessage() );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
