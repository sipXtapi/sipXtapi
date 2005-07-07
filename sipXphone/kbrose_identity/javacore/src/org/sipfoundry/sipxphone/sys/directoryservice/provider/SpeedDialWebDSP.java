/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.directoryservice.provider;

import javax.naming.directory.*;
import javax.naming.*;
import java.lang.*;
import java.util.*;

import org.sipfoundry.util.*;
import org.sipfoundry.sipxphone.sys.*;

/**
 * This class is the Directory Service Provider called by the Web UI
 * for the Speed Dial service.
 */
public class SpeedDialWebDSP
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    public static final int MAX_SPEEDDIAL_SIZE = 1000 ;

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////



    public static void addEntry(String  strId, String strLabel, String strPhoneNumber, String strUrl)
    {
        SpeedDialDSP speedDial = SpeedDialDSP.getInstance() ;

        BasicAttributes attrs = new BasicAttributes() ;

        //if( strId != null )
        //  strId = TextUtils.removeLeadingZerosifInt(strId);

        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID, strId);
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL, strLabel);
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER, strPhoneNumber);
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL, strUrl);

        speedDial.addEntry(attrs) ;

    }


    public static void deleteEntry(String strId) throws IllegalArgumentException
    {
        SpeedDialDSP speedDial = SpeedDialDSP.getInstance() ;
        BasicAttributes attrs = speedDial.lookupSpeedDialId(strId) ;
        speedDial.deleteEntry(attrs) ;
    }

    public static void deleteAllEntries() throws IllegalArgumentException
    {
        SpeedDialDSP speedDial = SpeedDialDSP.getInstance() ;
        speedDial.clear();
    }


    public static int getNumberOfEntries(){
        return SpeedDialDSP.getInstance().getEntries().size() ;
    }

    /**
     *Is the speeddial datasource local( from cache.ser )
     *or from config files which get written by config server?
     *@return  1 if true and 0 if false
     */
    public static int doesIDAlreadyExist(String strID){
        int iRet = 0;
        BasicAttributes searchEntry = null;
        try{
            searchEntry =
                SpeedDialDSP.getInstance().lookupSpeedDialId(strID);
        }catch( Exception e ){

        }
        if( searchEntry != null ){
            iRet = 1;
        }
        return iRet;
    }


    /** For now, if the datasource is not local and read from config files,
     * due to the limitation of one-way comunication between config files and
     * config server, the data-source will be read-only.
     * @return  1 if true and 0 if false
     */
    public static int isDataSourceReadOnly(){
        int iRet = 0;
        boolean bReadOnly = SpeedDialDSP.getInstance().isDataSourceReadOnly();
        if( bReadOnly  ){
            iRet = 1;
        }
        return iRet;
    }




    /**
     * This method returns a list of all of the speeddial IDs in the directory.
     */
    public static String[] getSpeeddialEntries(String strType)
    {
        String[] strArraySpeeddial = null; //new String[MAX_SPEEDDIAL_SIZE];
        Vector vSpeedDial = new Vector() ;

        int i = 0 ;

        try {
            SpeedDialDSP speedDial = SpeedDialDSP.getInstance() ;

            vSpeedDial = speedDial.getEntries() ;
            strArraySpeeddial = new String[vSpeedDial.size()];
            Enumeration enum = vSpeedDial.elements() ;
            while (enum.hasMoreElements()) {
                BasicAttributes basic = (BasicAttributes) enum.nextElement() ;
                try {
                    Attribute attr = basic.get(strType) ;
                    strArraySpeeddial[i] = (String) attr.get() ;
                } catch (NullPointerException e) {
                    System.out.println("SpeedDialWebDSP::getSpeeddialEntries exception reading attribute") ;
                } catch (NamingException e) {
                    System.out.println("SpeedDialWebDSP::getSpeeddialEntries exception reading attribute") ;
                }
                i++ ;
            }
        } catch (IllegalArgumentException e) {
            System.out.println("SpeedDialWebDSP::getSpeeddialIdEntries illegal argument") ;
        }

        return strArraySpeeddial ;

    }

     public static  int getNextAvailableID(){
         return SpeedDialDSP.getInstance().getNextAvailableID();
     }


    public static String validateEntry( String strID, String strLabel,
                            String strPhoneNumber, String strURL,
                            boolean bIsPhoneNumber, String strNewLine){
        return SpeedDialDSP.getInstance().validateEntry
            (strID, strLabel,
             strPhoneNumber, strURL,
             bIsPhoneNumber, strNewLine );
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////



//////////////////////////////////////////////////////////////////////////////
// Test Methods
////
	public static void main(String argv[])
	{
       System.load("e:\\dev\\platform\\wnt\\lib\\pingerjni.dll");

	    SpeedDialDSP speedDialDSP = SpeedDialDSP.getInstance();

        BasicAttributes attrs = new BasicAttributes();

        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "01");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Joe Shmoe");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "333344444");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) url);

        speedDialDSP.addEntry(attrs);
	    attrs = new BasicAttributes();

        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "02");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Jane Doe");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "112233");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "Jane Doe<sip:jane@example.com>");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "03");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Yo mamma");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "04");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "John");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "05");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Bugs");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "06");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Daffy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "07");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Road Runner");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "08");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "The Tick");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "09");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Ed Edd and Eddy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "10");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Rocko");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "11");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Ren and Stimpy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

	    attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "12");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Stuff");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        System.out.println("main:: start dump");
        speedDialDSP.dump();


       int iSize = speedDialDSP.getEntries().size() ;
       String[] arrSpeed = new String[iSize] ;

       arrSpeed = SpeedDialWebDSP.getSpeeddialEntries(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID) ;
       for (int i = 0; i < iSize; i++) {
        System.out.println("Speeddial " + arrSpeed[i]) ;
       }
	}


}
