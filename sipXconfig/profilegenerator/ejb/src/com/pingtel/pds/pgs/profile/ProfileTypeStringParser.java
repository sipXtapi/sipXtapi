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
 
package com.pingtel.pds.pgs.profile;

import java.util.StringTokenizer;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;

public class ProfileTypeStringParser implements PDSDefinitions {

    public static boolean [] parse ( String profileTypes ) throws PDSException {

        boolean [] profTypesToCreate = new boolean [ 6 ];

        for (   StringTokenizer ptST = new StringTokenizer ( profileTypes, "," );
                ptST.hasMoreTokens(); ) {

            int profileValue = new Integer ( ptST.nextToken() ).intValue();

            switch ( profileValue ) {
                case PROF_TYPE_USER : {
                    profTypesToCreate [ PROF_TYPE_USER ] = true;
                    break;
                }
                case PROF_TYPE_PHONE : {
                    profTypesToCreate [ PROF_TYPE_PHONE ] = true;
                    break;
                }
                default : {
                    throw new PDSException ( String.valueOf ( profileValue ) );
                }
            } // switch
        }

        return profTypesToCreate;
    }

}