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
 * CreateApplicationBean is a 'value' bean, used in the webui to report
 * errors and persist form values.
 *
 * @author ibutcher
 *
 */
public class CreateApplicationBean {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String name;
    private String url;
    private String description;
    private String errorMessage;

//////////////////////////////////////////////////////////////////////////
// Construction
////
    public CreateApplicationBean() {}


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
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
