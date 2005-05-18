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


import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * ConfigFileManager is a utility class which solves the problem of a program
 * wanting to read one or more Properties file only once.   This can be solved
 * by putting the logic in a base class but this fails when you have > 1
 * class hierarchy.   I saw this problem with EJ Bean and JSPs which both
 * needed to read properties files (AOP style).
 *
 * A logical extension is to replace 'Properties' with an interface and put
 * create adapter classes (one for properties, one for XML files, etc.).
 *
 * @author ibutcher
 * 
 * FIXME: remove singleton, implement cache on JNDI, remove getProperty in favor of 
 * just returning property object
 * Longer term: we should not be using property files to exchange information between
 * sipX components
 *
 */
public class ConfigFileManager {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static ConfigFileManager mInstance;
    private Map mFileMap;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    private ConfigFileManager() {
        mFileMap = new HashMap();
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * getInstance returns a reference to a ConfigFileManager object
     *
     * @return refence to ConfigFileManager
     */
    public synchronized static ConfigFileManager getInstance() {
        if(mInstance == null) {
            mInstance = new ConfigFileManager();
        }
        return mInstance;
    }

    /**
     * getProperty returns the value for a given key in a particular
     * properties file.  It will return null if the key does not exist in the
     * file.   RuntimeExceptions are thrown if the file does not exist or can
     * not be read.
     *
     * @param fileName fully-qualified Properties file name.
     * @param key key whose value you want from the given Properties file.
     * @return String value for the given key or null if it does not exist
     * in the properties file.
     */
    public String getProperty(String fileName, String key){
        String returnValue = null;
        Properties savedProps = getProperties(fileName);

        if(savedProps != null){
            returnValue = savedProps.getProperty(key);
        }

        return returnValue;
    }

    /**
     * Loads and caches property object
     * Since this is a singleton class all objects will share just once instance of each properties object.
     * @param fileName property file name
     * @return property objects
     */
    public Properties getProperties(String fileName) {
        synchronized(mFileMap){
            if(!mFileMap.containsKey(fileName)){
                File inputFile = checkFileIsAvailable(fileName);

                Properties props = new Properties();

                try {
                    props.load(new FileInputStream(inputFile));
                }
                catch(IOException e) {
                    throw new RuntimeException(e);
                }

                mFileMap.put(fileName, props);
            }
        } //synchronized

        Properties savedProps = (Properties) mFileMap.get(fileName);
        return savedProps;
    }

    
    
//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


    /**
     * checkFileIsAvailable performs various checks to make sure that
     * the file can be read.
     *
     * @param fileName fully qualified name of the file to be read.
     * @return File object which corresponds to the given fileName.
     */
    private File checkFileIsAvailable(String fileName) {
        File inputFile = new File(fileName);
        if(!inputFile.exists()){
            //@todo put in resource bundle
            throw new RuntimeException("File: " + fileName +
                    " does not exist");
        }

        if(!inputFile.canRead()){
            //@todo put in resource bundle
            throw new RuntimeException("Can not read file: " +
                    fileName );
        }
        return inputFile;
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}

