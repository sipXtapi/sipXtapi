/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/common/MasterDetailsMap.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */



package com.pingtel.pds.pgs.common;

import java.util.Collection;
import java.util.HashMap;

/**
 * MasterDetailsMap allows you to store Objects against a combination of two
 * keys.   This is useful when you have a situation like the following.   You
 * want to store the rabies vaccination date for owners pets.   More than one
 * owner may have a cat called Tibbles so you would use the owner's name as
 * the masterID and then the pet's name as the secondary key.
 *
 * NOTE: this class is _NOT_ _NOT_thread safe.  It is only used in a thread
 * safe way in the PGS.   If you are going to use it with multiple threads
 * then synchronize access to it.
 *
 * @author IB
 */
public class MasterDetailsMap {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private HashMap masterMap = new HashMap(); // container for details


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * storeDetail puts a value into the MasterDetailsMap.   It uses two keys,
     * the master and detail.   This allows you to store objects in a two
     * dimensional way.
     * @param masterID primary key to use when deciding where in the
     * map
     * @param detailID secondary key
     * @param value Object that you wish to store
     */
    public void storeDetail (   Object masterID,
                                Object detailID,
                                Object value ) {

        HashMap detailMap = null;

        if ( masterMap.containsKey( masterID ) ) {
            detailMap = (HashMap) masterMap.get( masterID );
        }
        else {
            detailMap = new HashMap();
        }

        detailMap.put( detailID, value );
        masterMap.put( masterID, detailMap );
    }


    /**
     * retrieves an object which has been stored for the given master and
     * details IDs.
     *
     * @param masterID primary key to use for retieving the object
     * @param detailID secondary key to use for retieving the object
     * @return previously stored objected if one exists or otherwise null
     */
    public Object getDetail ( Object masterID, Object detailID ) {

        if ( masterMap.containsKey( masterID ) ) {
            HashMap detailMap = (HashMap) masterMap.get( masterID );
            return detailMap.get( detailID );
        }
        else {
            return null;
        }
    }

    /**
     * retieves all objects stored for a given masterID regardless
     * of their detailID
     *
     * @param masterID key to use for retieving the object
     * @return Collection of Objects.
     */
    public Collection getDetails ( Object masterID ) {
        if ( masterMap.containsKey( masterID ) ) {
            HashMap detailMap = (HashMap) masterMap.get( masterID );
            return detailMap.values();
        }
        else {
            return null;
        }
    }


    /**
     * deletes all Objects stored for the given masterID regardless of their
     * detailID
     *
     * @param masterID key to use for removing the objects
     */
    public void remove ( Object masterID ) {
        if ( masterMap.containsKey( masterID ) ) {
            Object o = masterMap.get( masterID );
            masterMap.remove( o );
        }
    }

    /**
     * deletes all Objects stored for the given masterID and detailID
     *
     * @param masterID primary key to use for removing the objects
     * @param detailID secondary key to use for removing the objects
     */
    public void remove ( Object masterID, Object detailID ) {
        if ( masterMap.containsKey( masterID ) ) {
            HashMap detailMap = (HashMap) masterMap.get( masterID );
            Object o = detailMap.get( detailID );
            detailMap.remove( o );
        }
    }

    /**
     * tests to see if any object is stored in the MasterDetailsMap for
     * the given master and details ID
     *
     * @param masterID primary key used to determine if an object exists
     * in the MasterDetailsMap
     * @param detailID secondary key used to determine if an object exists
     * in the MasterDetailsMap
     * @return true if an object exists otherwise false
     */
    public boolean contains ( Object masterID, Object detailID ) {

        boolean returnValue = false;

        if ( masterMap.containsKey( masterID ) ) {
            HashMap detailMap = (HashMap) masterMap.get( masterID );
            if ( detailMap.containsKey( detailID) )
                returnValue = true;
        }

        return returnValue;
    }

    /**
     * empties the MasterDetailMap of all values.
     */
    public void clear() {
        masterMap.clear();
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