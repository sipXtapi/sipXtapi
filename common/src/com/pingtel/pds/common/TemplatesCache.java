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
import java.io.FileNotFoundException;
import java.util.Date;
import java.util.HashMap;

import javax.xml.transform.Templates;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamSource;

/**
 * TemplatesCache is a store for JAXP Transformation templates.   The
 * process of compiling XLST stylesheets is pretty expensive so this class
 * caches pre-compiled Templates and returns new Transformer instances on
 * request.
 */
public class TemplatesCache {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // I would have loved to encapsulte this in the CacheEntry class but
    // you can't have static fields in inner classes
    private static TransformerFactory mTransformerFactory = null;

    // singleton instance
    private static TemplatesCache mInstance = new TemplatesCache();

    // storage datastructure for the templates
    private HashMap mCache = new HashMap();

//////////////////////////////////////////////////////////////////////////
// Construction
////
    private TemplatesCache() {
        try {
            mTransformerFactory = TransformerFactory.newInstance();

            String styleSheetPath =
                PathLocatorUtil.getInstance().getPath(
                    PathLocatorUtil.XSLT_FOLDER,
                    PathLocatorUtil.PGS );

            File directory = new File ( styleSheetPath );

            String [] sheets = directory.list();

            for ( int i = 0; i < sheets.length; ++i ) {
                if (sheets [i].endsWith( ".xslt" ) || sheets [i].endsWith(".XSLT")){
                    newTransformer( styleSheetPath + sheets [i] );
                }
            }
        }
        catch ( FileNotFoundException ex ) {
            throw new RuntimeException ( ex.toString() );
        }
        catch ( TransformerConfigurationException ex ) {
            throw new RuntimeException ( ex.toString() );
        }
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * newTransformer returns a new Transformer for the given styelsheet.
     *
     * @param       styleSheetFileName is the full file and pathname for the
     *              stylesheet that you want to get a Transformer mInstance for.
     * @return      A Transformer object which represents the styleSheetFileName
     *              argument.
     * @exception   TransformerConfigurationException is thrown if an error
     *              occurrs during the compilation of the stylesheet.
    */
    public Transformer newTransformer ( String styleSheetFileName )
        throws TransformerConfigurationException, FileNotFoundException {

        CacheEntry entry = null;
        File file = new File ( styleSheetFileName );
        Date lastModified = new Date ( file.lastModified() );

        if ( mCache.containsKey( styleSheetFileName ) ) {
            entry = (CacheEntry) mCache.get( styleSheetFileName );

            if ( lastModified.after( entry.getTimestamp() ) ) {
                entry = new CacheEntry ( lastModified, styleSheetFileName );
                mCache.put(  styleSheetFileName,  entry );
            }

        } else {
                entry = new CacheEntry ( lastModified, styleSheetFileName );
                mCache.put( styleSheetFileName, entry );
        }

        return entry.getTemplates().newTransformer();
    }

    /**
     * flush clears all the cached templates.
     */
    public synchronized void flush() {
        mCache.clear();
    }

    /**
     * singleton accessor
     * @return instance of TemplatesCache.
     */
    public static TemplatesCache getInstance() {
        return mInstance;
    }



//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    /**
     * CacheEntry represents a what is stored for each sytlesheet in the
     * TemplatesCache datastructure.   Currently it includes the compliled
     * Templates for the stylesheet and a timestamp.
     */
    private class CacheEntry {

        private Date mTimestamp;
        private Templates mTemplates;

        public Date getTimestamp () {
            return mTimestamp;
        }

        public Templates getTemplates() {
            return mTemplates;
        }

        public CacheEntry ( Date timestamp, String styleSheetFileName )
            throws FileNotFoundException, TransformerConfigurationException {

            FileInputStream inputStream =
                new FileInputStream ( styleSheetFileName );

            ///////////////////////////////////////////////////////////////////////
            //
            // Warning: TransformerFactory implementations are not guarenteed to
            // be thread safe.
            //
            ///////////////////////////////////////////////////////////////////////
            synchronized ( mTransformerFactory ) {
                mTemplates = mTransformerFactory.newTemplates(
                    new StreamSource ( inputStream ) );
            }

            this.mTimestamp = timestamp;
        }
    } // class


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}