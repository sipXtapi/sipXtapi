/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/jsptags/ejb/LightweightDeviceTypeComparator.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags.ejb;

import java.util.Comparator;
import com.pingtel.pds.common.PDSDefinitions;

class LightweightDeviceTypeComparator implements Comparator {

        public int compare ( Object o1, Object o2 ) {
            LightweightDeviceType first = (LightweightDeviceType) o1;
            LightweightDeviceType second = (LightweightDeviceType) o2;

            int returnValue = 0;

            if ( first.getManufacturer().equals( second.getManufacturer() ) &&
                first.getModel().equals( second.getModel() ) ) {
            }
            else if ( first.getManufacturer().equalsIgnoreCase( "Pingtel" ) &&
                        !(second.getManufacturer().equalsIgnoreCase( "Pingtel" )) ) {
                returnValue = -1;
            }
            else if ( first.getManufacturer().equalsIgnoreCase( "Pingtel" ) &&
                        second.getManufacturer().equalsIgnoreCase( "Pingtel" ) ) {
                if ( first.getModel().equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) &&
                        !second.getModel().equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) ) {
                    returnValue = -1;
                }
                else if ( second.getModel().equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) &&
                    !first.getModel().equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_XPRESSA )) {
                    returnValue = +1;
                }
                else {
                    returnValue = 0;
                }
            }
            else if ( second.getManufacturer().equalsIgnoreCase( "Pingtel" ) &&
                !first.getManufacturer().equalsIgnoreCase( "Pingtel" ) ) {
                return +1;
            }


            return returnValue;
        }

        public boolean equals ( Object o ) {
            if ( !(o instanceof LightweightDeviceTypeComparator) ) {
                return true;
            }
            else {
                return false;
            }
        }
    } // LightweightDeviceTypeComparator