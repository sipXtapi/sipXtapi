/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/X_GenerateAuthExceptionsTag.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.sipxchange.datasets.DataSetBuilderFacade;

import javax.servlet.jsp.JspException;
import java.util.HashMap;

/**
 * X_GenerateAuthExceptionsTag is a tag handler which sends a JMS message
 * to the DataSetBuilderFacade to trigger it to call DataSetBuilder.
 * DataSetBuilder then generates the authexceptions dataset.
 *
 * @author ibutcher
 *
 */
public class X_GenerateAuthExceptionsTag extends DataSetBaseHandler{

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    public int doStartTag() throws JspException {

        String installStereotype = getInstallStereoType();

        if(installStereotype.equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)){

            HashMap messageValues = new HashMap();
            messageValues.put(DATASET_TYPE_MSG_KEY, "authexceptions");
            messageValues.put(  DataSetBuilderFacade.CHANGEPOINT_ID_KEY,
                                changepointid);

            messageValues.put(  DataSetBuilderFacade.CHANGEPOINT_TYPE_KEY,
                                changepointtype);

            SendJMSMessage(DATASET_QUEUE_NAME, messageValues);
        }

        return SKIP_BODY;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        super.clearProperties();
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
