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


package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.PDSDefinitions;

import javax.servlet.jsp.JspException;
import java.util.HashMap;


public class X_GenerateCredentialsTag extends JMSTagHandler {


    public int doStartTag() throws JspException {

        String installStereotype = getInstallStereoType();
        if(installStereotype.equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)){

            HashMap messageValues = new HashMap();
            messageValues.put( DATASET_TYPE_MSG_KEY, "credentials" );
            SendJMSMessage( DATASET_QUEUE_NAME, messageValues );
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

}