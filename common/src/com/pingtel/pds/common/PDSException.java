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


package com.pingtel.pds.common;


/**
 * PDSException is a slightly smarter version of Exception.   It provides
 * the ability to store two error messages; a business-type error message
 * and a low-level (internal) error message.   This is useful as it allows you
 * to display the correct message to users but still captures system messages
 * for support.
 *
 */
public class PDSException extends Exception {

    private static final String separator = " : ";

    private String internalMessage;
    private String publicMessage;
    private PDSException nested;

    private static boolean inDebugMode;

    static {
        String debugValue = System.getProperty( "pds.debug");
        if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
            inDebugMode = true;
        }
        else {
            inDebugMode = false;
        }
    }


    public PDSException ( String message ) {
        this.publicMessage = message;
    }

    public PDSException ( String message, Exception e ) {
        this.publicMessage = message;
        this.internalMessage = e.toString();
    }

    public PDSException ( String message, PDSException e ) {
        this.publicMessage = message;
        this.nested = e;
    }

    public String getInternalMessage () {
        StringBuffer message = new StringBuffer();

        message.append( this.publicMessage );
        if ( this.internalMessage != null ) {
            message.append( separator );
            message.append( this.internalMessage );
        }

        if ( nested != null ) {
            message.append( "   (" );
            message.append( this.nested.getInternalMessage() );
            message.append( ")" );
        }

        return message.toString();
    }


    public String getMessage() {
        if ( inDebugMode ) {
            return getInternalMessage();
        }
        else {
            StringBuffer message = new StringBuffer();

            message.append( this.publicMessage );
            if ( nested != null ) {
                message.append( "   (" );
                message.append( this.nested.getMessage() );
                message.append( ")" );
            }

            return message.toString();
        }
    }


    public String toString () {
        return getMessage();
    }
}
