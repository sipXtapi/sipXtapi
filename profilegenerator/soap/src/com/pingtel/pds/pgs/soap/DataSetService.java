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

package com.pingtel.pds.pgs.soap;

import com.pingtel.pds.common.PDSException;

import java.util.HashMap;

/**
 * DataSetService is an adapter class used to rebuild all of the export
 * data sets.
 *
 * This should be used after you have modified the state of Config Server
 * to pass on this new state to the other SIPxhchange servers.
 *
 * It is not called automatically after events which are known to modify
 * the state of the Config Server such as adding or deleting a User so
 * as to avoid needless regeneration.  You may be adding 1000 users so
 * there is not need to regenate the Data Sets for each User, just once
 * at the end when they have all be added.
 */
public class DataSetService extends SoapService {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String DATASET_TYPE_MSG_KEY = "datasettype";


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private JMSHelper mJmsHelper;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    public DataSetService () {
        mJmsHelper = new JMSHelper();
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * rebuildDataSets does just that, recreates all of the export
     * data sets and sends them to the other SIPxchange servers.
     *
     * @throws PDSException is thrown for all application level errors.
     */
    public void rebuildDataSets() throws PDSException {
        try {
            HashMap messageValues = new HashMap();
            messageValues.put(DATASET_TYPE_MSG_KEY, "credentials");
            mJmsHelper.sendJMSMessage(mJmsHelper.DATASET_QUEUE_NAME, messageValues);

            messageValues.put(DATASET_TYPE_MSG_KEY, "aliases");
            mJmsHelper.sendJMSMessage(mJmsHelper.DATASET_QUEUE_NAME, messageValues);

            messageValues.put(DATASET_TYPE_MSG_KEY, "authexceptions");
            mJmsHelper.sendJMSMessage(mJmsHelper.DATASET_QUEUE_NAME, messageValues);

            messageValues.put(DATASET_TYPE_MSG_KEY, "extensions");
            mJmsHelper.sendJMSMessage(mJmsHelper.DATASET_QUEUE_NAME, messageValues);

            messageValues.put(DATASET_TYPE_MSG_KEY, "permissions");
            mJmsHelper.sendJMSMessage(mJmsHelper.DATASET_QUEUE_NAME, messageValues);
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
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
