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

import java.text.MessageFormat;
import java.util.ResourceBundle;


/**
 * ErrorMessageBuilder is a singleton which reads our error messages XML file
 * and provides a method (collateErrorMessages) to query and format the values
 * extracted from the XML file.
 */
public class ErrorMessageBuilder {

    private ResourceBundle mPgsStrings;

    public ErrorMessageBuilder(String bundleName ) {
        mPgsStrings = ResourceBundle.getBundle(bundleName);
    }


    public String collateErrorMessages (    String useCase,
                                            String errorMessageNumber,
                                            Object [] detailMsgSubstitutions ) {

        String useCaseMessage = collateErrorMessages ( useCase );
        String detailMessage = collateErrorMessages (  errorMessageNumber,
                                                       detailMsgSubstitutions );

        StringBuffer fullMessage = new StringBuffer(    errorMessageNumber + ": " +
                                                        useCaseMessage + " " +
                                                        detailMessage  );

        return fullMessage.toString();
    }


    public String collateErrorMessages ( String useCase ) {
        return mPgsStrings.getString(useCase);
    }

    public String collateErrorMessages (    String errorMessageNumber,
                                            Object [] detailMsgSubstitutions ) {

        String detailMessage = mPgsStrings.getString( errorMessageNumber );

        StringBuffer fullMessage =
            new StringBuffer ( formatErrorMessage ( detailMessage,
                                                    detailMsgSubstitutions ) );

        return fullMessage.toString();
    }


    private String formatErrorMessage ( String baseMessage,
                                        Object [] arguments ) {

        String finishedMessage = null;

        if ( arguments != null ) {
            for ( int i = 0 ; i < arguments.length; ++i ) {
                if ( arguments [ i ] == null )
                    arguments [ i ] = "NULL";
            }

            finishedMessage = MessageFormat.format( baseMessage, arguments );
        }
        else {
            finishedMessage = baseMessage;
        }
        return finishedMessage;
    }


}