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

import java.rmi.RemoteException;
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.MessageDrivenBean;
import javax.ejb.MessageDrivenContext;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.BaseEJB;
import com.pingtel.pds.pgs.user.UserGroupHome;

/**
 * DataSetBuilderFacade is the bean implementation of the DataSetBuilderFacade bean. It is used as
 * a asynchronous session facade for the DataSetBuilder methods. The full dataset creation methods
 * of DataSetBuilder can take many seconds so we use a MDB to run this in the background so the
 * user doesn't have to wait for the UI to return
 * 
 * @author IB
 */
public class DataSetBuilderFacade extends BaseEJB implements MessageDrivenBean, MessageListener {
    public static final String CHANGEPOINT_ID_KEY = "changepointid";
    public static final String CHANGEPOINT_TYPE_KEY = "changepointtype";

    private DataSetBuilder mDataSetBuilder;

    private UserGroupHome mUserGroupHome;

    public DataSetBuilderFacade() {
        // empty
    }

    /**
     * Standard EJB implementation
     */
    public void setMessageDrivenContext(MessageDrivenContext ctx) throws EJBException {
        String error = "Error in DataSetBuilderFacade:setMessageDrivenContext";
        try {
            InitialContext initial = new InitialContext();

            DataSetBuilderHome dataSetBuilderHome = (DataSetBuilderHome) initial
                    .lookup("DataSetBuilder");

            mDataSetBuilder = dataSetBuilderHome.create();

            mUserGroupHome = (UserGroupHome) initial.lookup("UserGroup");
        } catch (NamingException e) {
            logFatalAndRethrow(error, e);
        } catch (RemoteException e) {
            logFatalAndRethrow(error, e);
        } catch (CreateException e) {
            logFatalAndRethrow(error, e);
        }
    }

    /**
     * Standard EJB implementation
     */
    public void ejbCreate() {
        // empty
    }

    /**
     * Standard EJB implementation
     */
    public void ejbRemove() {
        // empty
    }

    /**
     * Handles messages sent to this bean.
     * 
     * Each invocation generates one data set. Each data set is generated for entire content of
     * the database. (there is no support for incremental dataset generation).
     */
    public void onMessage(Message message) {
        String error = "Error in DataSetBuilderFacade:onMessage";

        try {
            MapMessage mapMessage = (MapMessage) message;
            String datasetType = mapMessage.getString("datasettype");

            if (datasetType.equals("credentials")) {
                mDataSetBuilder.generateCredentials();
            } else if (datasetType.equals("aliases")) {
                mDataSetBuilder.generateAliases();
            } else if (datasetType.equals("permissions")) {
                Collection allRootGroups = mUserGroupHome.findTopGroups();
                mDataSetBuilder.generatePermissions(allRootGroups);
            } else if (datasetType.equals("extensions")) {
                mDataSetBuilder.generateExtensions();
            } else if (datasetType.equals("authexceptions")) {
                Collection allRootGroups = mUserGroupHome.findTopGroups();
                mDataSetBuilder.generateAuthExceptions(allRootGroups);
            }
        } catch (RemoteException e) {
            logFatalAndRethrow(error, e);
        } catch (PDSException e) {
            logFatalAndRethrow(error, e);
        } catch (JMSException e) {
            logFatalAndRethrow(error, e);
        } catch (FinderException e) {
            logFatalAndRethrow(error, e);
        }
    }
}
