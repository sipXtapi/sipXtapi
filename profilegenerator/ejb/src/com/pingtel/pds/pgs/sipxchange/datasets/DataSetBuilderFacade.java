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

package com.pingtel.pds.pgs.sipxchange.datasets;

import com.pingtel.pds.pgs.common.ejb.BaseEJB;
import com.pingtel.pds.pgs.sipxchange.datasets.DataSetBuilder;
import com.pingtel.pds.pgs.user.UserGroupHome;

import javax.ejb.EJBException;
import javax.ejb.MessageDrivenBean;
import javax.ejb.MessageDrivenContext;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.naming.InitialContext;
import java.util.Collection;

/**
 * DataSetBuilderFacade is the bean implementation of the DataSetBuilderFacade bean.
 * It is used as a asynchronous session facade for the DataSetBuilder methods.   The
 * full dataset creation methods of DataSetBuilder can take many seconds so we
 * use a MDB to run this in the background so the user doesn't have to wait for the
 * UI to return
 *
 * @author IB
 */
public class DataSetBuilderFacade extends BaseEJB
        implements MessageDrivenBean, MessageListener {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    public static final String CHANGEPOINT_ID_KEY = "changepointid";
    public static final String CHANGEPOINT_TYPE_KEY = "changepointtype";



    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private MessageDrivenContext mCTX;
    private DataSetBuilder mDataSetBuilder;

    private UserGroupHome mUserGroupHome;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public DataSetBuilderFacade() { }


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    /**
     *  Standard EJB implementation
     */
    public void setMessageDrivenContext( MessageDrivenContext ctx )
            throws EJBException {

        mCTX = ctx;

        try {
            InitialContext initial = new InitialContext( );

            DataSetBuilderHome dataSetBuilderHome =
                    (DataSetBuilderHome) initial.lookup ( "DataSetBuilder" );

            mDataSetBuilder = dataSetBuilderHome.create();

            mUserGroupHome = (UserGroupHome) initial.lookup ( "UserGroup" );
        }
        catch ( Exception e ) {
            logFatal ( e.toString() );
            throw new EJBException ( "Error in DataSetBuilderFacade:setMessageDrivenContext " + e.toString() );
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
     *  Standard EJB implementation
     */
    public void onMessage( Message message ) {

        MapMessage mapMessage = (MapMessage) message;

        try {
            String datasetType = mapMessage.getString( "datasettype" );

            // this is only needed until we add the code to correctly locate the
            // 'change points' in the hierarchy.   Right now we just rebuild
            // the world.
            Collection allRootGroups = mUserGroupHome.findTopGroups();

            if ( datasetType.equals( "credentials") ) {
                logDebug( "in onMessage in DataSetBuilderFacade - doing credentials");
                mDataSetBuilder.generateCredentials();
            }
            else if ( datasetType.equals( "aliases") ) {
                logDebug("in onMessage in DataSetBuilderFacade - doing aliases");
                mDataSetBuilder.generateAliases();
            }
            else if ( datasetType.equals( "permissions") ) {
                logDebug("in onMessage in DataSetBuilderFacade - doing permissions");
                mDataSetBuilder.generatePermissions(allRootGroups);
            }
            else if ( datasetType.equals( "extensions") ) {
                logDebug("in onMessage in DataSetBuilderFacade - doing extensions");
                mDataSetBuilder.generateExtensions();
            }
            else if ( datasetType.equals( "authexceptions") ) {
                logDebug("in onMessage in DataSetBuilderFacade - doing authexceptions");

                // @todo remove
                System.out.println("changepointid: " +
                        mapMessage.getString(CHANGEPOINT_ID_KEY));
                System.out.println("changepointtype: " +
                        mapMessage.getString(CHANGEPOINT_TYPE_KEY));

                mDataSetBuilder.generateAuthExceptions(allRootGroups);
            }
        }
        catch ( Exception e) {
            logFatal ( e.toString() );
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
