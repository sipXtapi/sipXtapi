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

package com.pingtel.pds.pgs.beans;

/**
 * CreateExtensionPoolBean is a 'value' bean, used in the webui
 * to report errors and persist form values.
 * @author ibutcher
 * 
 */
public class CreateExtensionPoolBean {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // JavaBean attributes
    private String name;
    private String errorMessage;



//////////////////////////////////////////////////////////////////////////
// Construction
////
    public CreateExtensionPoolBean(){}


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    public void setErrorMessage(String errorMessage) {
        this.errorMessage = errorMessage;
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
