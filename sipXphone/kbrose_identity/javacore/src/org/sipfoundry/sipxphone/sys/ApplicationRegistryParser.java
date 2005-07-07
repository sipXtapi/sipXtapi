/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/ApplicationRegistryParser.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys ;

import java.io.* ;
import java.net.* ;
import java.util.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.util.* ;

/**
 * This class understands the "app-config" format and knows how to read and
 * write it.  The format is fairly simple and follows this form:
 *
 * <APP_TYPE>, <URL><crlf>
 *
 * The <APP_TYPE> can be anyone of the following:
 *   USER - For a user install application
 *   CORE - For a core application
 *   PHONEBOOK - For the phone book application
 *   CALLLOG - For the call log application
 *   SPEEDDIAL - For the speeddial application
 *   DIALBYURL - For the Dial By URL application
 *   VOLUMEADJUST - For the Volume and Contrast adjustment application
 *   PREFERENCE - For the core preferences application
 *   CONFERENCE - For the conference application
 *   TRANSFER - For the transfer application
 *
 * Blank lines are comments ('#') are allowed.
 *
 * In order to preserve whitespace, invalid lines, and comments, a internal
 * data format (icAppListLine) is used to store all of the lines along with
 * the parsed representation.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ApplicationRegistryParser
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String APPLICATION_LIST = "app-config" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private StringBuffer m_errorLog ;           // Log of last parse errors
    private boolean      m_bErrors ;            // Did we have any errors in
                                                // the last parse attempt

    private Vector       m_vProcessedAppList ;  // Our post-processed
                                                // application list.


    private AppListItem  m_cacheListItems[] ;   // Cache of the post-processed
                                                // app list
    private boolean      m_bDirty ;             // Do we have any writes pending?

    private boolean      m_bEmptyFile ;         // Was the source file empty?

    private static       ApplicationRegistryParser m_instance ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the Application Registry Parser
     * This will parse the defailt app-config file.
     */
    protected ApplicationRegistryParser()
    {
        m_vProcessedAppList = new Vector() ;
        clearErrors() ;
        generateInternalAppList(null) ;
        m_bDirty = false ;
    }

    /**
     * Constructs the Application Registry Parser
     * This will parse the lines from the reader constructed
     * out of the inputStream passed.
     */
    protected ApplicationRegistryParser(InputStream inputStream)
    {
        m_vProcessedAppList = new Vector() ;
        clearErrors() ;
        generateInternalAppList(inputStream) ;
        m_bDirty = false ;
    }


    /**
     * Get the singleton instance of this class.
     * This will return an instance of ApplicationRegistryParser
     * that is constructed by parsing default app-config file.
     */
    public static ApplicationRegistryParser getInstance()
    {
        if (m_instance == null)
            m_instance = new ApplicationRegistryParser() ;

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get all of our items from the app list
     */
    public AppListItem[] getAppList()
    {
        if (m_cacheListItems == null) {
            synchronized (m_vProcessedAppList) {
                if (!((m_vProcessedAppList.size() == 0) && m_bEmptyFile)) {
                    // Look through the list and count the number of entries
                    int iEntries = 0 ;
                    int iListItems = m_vProcessedAppList.size() ;
                    for (int i=0; i<iListItems; i++) {
                        icAppListLine appListLine = (icAppListLine) m_vProcessedAppList.elementAt(i) ;
                        if (appListLine.getAppItem() != null) {
                            iEntries++ ;
                        }
                    }

                    // Create the app list and dump our items into it.
                    m_cacheListItems = new AppListItem[iEntries] ;
                    iEntries = 0 ;
                    for (int i=0; i<iListItems; i++) {
                        icAppListLine appListLine = (icAppListLine) m_vProcessedAppList.elementAt(i) ;
                        if (appListLine.getAppItem() != null) {
                            m_cacheListItems[iEntries++] = appListLine.getAppItem() ;
                        }
                    }
                }
            }
        }
        return m_cacheListItems ;
    }


    /**
     * Remove an application from the app list
     */
    public void removeAppListItem(AppListItem item)
    {
        boolean bChanged = false ;

        synchronized (m_vProcessedAppList) {
            for (int i=0; i< m_vProcessedAppList.size(); i++) {
                icAppListLine appListLine = (icAppListLine) m_vProcessedAppList.elementAt(i) ;
                if ((appListLine.getAppItem() != null) && appListLine.getAppItem().equals(item)) {
                    m_vProcessedAppList.removeElementAt(i) ;
                    i-- ;
                    bChanged = true ;
                }
            }
        }

        // If we've changed the app list, clear the cache
        if (bChanged) {
            m_cacheListItems = null ;
            m_bDirty = true ;
        }
    }

    /**
     * Is the specified item an existing member of the application list?
     */
    public boolean isAppListMember(AppListItem item)
    {
        boolean bMember = false ;

        synchronized (m_vProcessedAppList) {
            for (int i=0; i< m_vProcessedAppList.size(); i++) {
                icAppListLine appListLine = (icAppListLine) m_vProcessedAppList.elementAt(i) ;
                if ((appListLine.getAppItem() != null) && appListLine.getAppItem().equals(item)) {
                    bMember = true ;
                    break ;
                }
            }
        }
        return bMember ;
    }


    /**
     * Add an application to the app list
     */
    public void addAppListItem(AppListItem item)
    {
        if (item != null) {
            if (!isAppListMember(item)) {
                synchronized (m_vProcessedAppList) {
                    icAppListLine listLine = new icAppListLine(generateAppListLine(item), item) ;
                    m_vProcessedAppList.addElement(listLine) ;
                    m_bDirty = true ;
                    m_cacheListItems = null ;
                }
            }
        } else {
            throw new IllegalArgumentException() ;
        }
    }


    /**
     * Writes the ApplicationRegistry out to the file system.  This will only
     * write if changes are detected.
     */
    public void flush()
    {
        if (m_bDirty) {
            m_bDirty = false ;

            m_bEmptyFile = false ;

            try {
                File fileSource = new File(getAppListFileSpec()) ;
                BufferedWriter writer = new BufferedWriter(new FileWriter(fileSource)) ;

                synchronized (m_vProcessedAppList) {
                    for (int i=0; i< m_vProcessedAppList.size(); i++) {
                        icAppListLine appListLine = (icAppListLine) m_vProcessedAppList.elementAt(i) ;

                        // Write each line
                        if (appListLine.m_strRawLine != null)
                            writer.write(appListLine.m_strRawLine) ;
                        writer.newLine() ;
                    }
                }

                writer.close() ;
            } catch (IOException ioe) {
                SysLog.log(ioe) ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Add a parse error which can be looked at later
     */
    private void addError(String strError)
    {
        m_bErrors = true ;

        m_errorLog.append(strError) ;
        m_errorLog.append("\n\n") ;
    }


    /**
     * Clear any parsing errors
     */
    private void clearErrors()
    {
        m_bErrors = false ;
        m_errorLog = new StringBuffer() ;
    }


    private String generateAppListLine(AppListItem item)
    {
        String strRC = null ;
        strRC = mapStringIdentifier(item.iAppType) + ", " + item.strClassNameOrURL ;
        return strRC ;
    }


    /**
     * Get the app type for the designated identifier.  As implemented, this
     * is a fairly expensive operation.
     */
    private int mapType(String strIdentifer)
    {
        int iType = ApplicationRegistry.UNKNOWN ;

        if (strIdentifer != null) {

            // We know the min and max app ids and have the mapping from type
            // to string, so let use a less-efficient but more maintainable
            // implementation of comparing our string against the known
            // identifiers for each type.
            for (int i=ApplicationRegistry.APP_TYPE_MIN; i<=ApplicationRegistry.APP_TYPE_MAX; i++) {
                String strMap = mapStringIdentifier(i) ;
                if ((strMap != null) && strMap.equalsIgnoreCase(strIdentifer)) {
                    // We have match: store it and kick out.
                    iType = i ;
                    break ;
                }
            }
        }
        return iType ;
    }


    /**
     * Get the string identifier for the designated application type.
     */
    private String mapStringIdentifier(int iType)
    {
        String strIdentifer = "UNKNOWN" ;

        switch (iType) {
            case ApplicationRegistry.CORE_APP:
                strIdentifer = "CORE" ;
                break ;
            case ApplicationRegistry.USER_APP:
                strIdentifer = "USER" ;
                break ;
//            case ApplicationRegistry.PHONEBOOK_APP:
//                strIdentifer = "PHONEBOOK" ;
///                break ;
            case ApplicationRegistry.CALL_LOG_APP:
                strIdentifer = "CALLLOG" ;
                break ;
            case ApplicationRegistry.SPEED_DIAL_APP:
                strIdentifer = "SPEEDDIAL" ;
                break ;
            case ApplicationRegistry.DIAL_BY_URL_APP:
                strIdentifer = "DIALBYURL" ;
                break ;
            case ApplicationRegistry.VOLUME_ADJUST_APP:
                strIdentifer = "VOLUMEADJUST" ;
                break ;
            case ApplicationRegistry.PREFERENCES_APP:
                strIdentifer = "PREFERENCES" ;
                break ;
            case ApplicationRegistry.CONFERENCE_APP:
                strIdentifer = "CONFERENCE" ;
                break ;
            case ApplicationRegistry.TRANSFER_APP:
                strIdentifer = "TRANSFER" ;
                break ;
        }
        return strIdentifer ;
    }


    /**
     * Parse an single application load list line into an AppListItem.
     */
    private AppListItem parseAppListLine(String strLine)
    {
        AppListItem item = null ;
        String      strID = null ;
        String      strURL = null ;

        /*
         * 1. Make sure that we have some data to work with
         */
        if ((strLine != null)) {
            // Clean the string and make sure the line isn't commented out.
            if (strLine.trim().startsWith("#") || (strLine.trim().length() == 0)) {
                // Ignore Blank and Commented out lines
                strLine = null ;
            }
        }

        /*
         * 2. Parse our the strID and strURL
         */
        if (strLine != null) {
            StringTokenizer toker = new StringTokenizer(strLine.trim(), ",") ;

            // Get the Identifer
            if (toker.hasMoreTokens()) {
                strID = toker.nextToken() ;
                if (strID != null)
                    strID = strID.trim() ;
            }

            // Get URL
            if (toker.hasMoreTokens())
            {
                strURL = toker.nextToken("") ;
                if (strURL != null)
                {
                    // ARGH: Working around a BUG in JDK 1.4.1_03 - It seems
                    // that the "," is being left as part of the next token even
                    // though the docs clearly state that the position is advanced
                    // AFTER the delimiter.
                    strURL = strURL.trim() ;
                    if (strURL.startsWith(","))
                    {
                        strURL = strURL.substring(1) ;
                        if (strURL != null)
                            strURL = strURL.trim() ;
                    }
                } 
            }
         }

        /*
         * 3. Validate Data
         */
        // Parse/Validate ID
        int iType = mapType(strID) ;
        if (iType == ApplicationRegistry.UNKNOWN) {
            String strError = "Invalid Application Identifer: " + strID ;
            addError(strError) ;
        }

        // Validate URL
        if (strURL != null  && (strURL.startsWith("http") || strURL.startsWith("file:"))) {
            try {
                URL url = new URL(strURL) ;
            } catch (MalformedURLException mue) {
                String strError = "Invalid Application URL: " + strURL ;
                addError(strError) ;
                strURL = null ;
            }
        }


        /**
         * 3. Create AppDataItem, if everything looks good.
         */
        if (iType != ApplicationRegistry.UNKNOWN)  {
            item = new AppListItem(iType, strURL) ;
        }

        return item ;
    }


    /**
     * Generate the internal list of applications.
     * If the inputStream is not null, make a bufferedReader out of
     * that and read the lines from the reader and generate app list.
     * Else, read the default file app-config in rootFileSystem+"/"+app-config .
     */
    private void generateInternalAppList(InputStream inputStream)
    {
        m_vProcessedAppList = new Vector(32) ;

        try {
            BufferedReader reader = null;
            if( inputStream == null )
            {
                File fileSource = new File(getAppListFileSpec()) ;
                if (fileSource.exists() && fileSource.canRead())
                {
                    reader = new BufferedReader(new FileReader(fileSource)) ;
                }
            }else
            {
                reader = new BufferedReader(new InputStreamReader(inputStream));
            }
            if( reader != null )
            {
                for (   String strLine = reader.readLine();
                        strLine != null;
                        strLine = reader.readLine()) {

                    // Add the next item to our list
                    AppListItem item = parseAppListLine(strLine) ;
                    m_vProcessedAppList.addElement(new icAppListLine(strLine, item)) ;
                }
                reader.close() ;
            } else {
                m_bEmptyFile = true ;
            }
        } catch (IOException ioe) {
            SysLog.log(ioe) ;
        }
    }


    /**
     * What is the default path to the app-config file?
     */
    private String getAppListFileSpec()
    {
        return PingerInfo.getInstance().getFlashFileSystemLocation() + File.separator + APPLICATION_LIST ;
    }



//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    private class icAppListLine
    {
        public String m_strRawLine ;
        public AppListItem m_item ;

        public icAppListLine(String strRawLine, AppListItem item)
        {
            m_strRawLine = strRawLine ;
            m_item = item ;
        }


        public String getRawLine()
        {
            return m_strRawLine ;
        }


        public AppListItem getAppItem()
        {
            return m_item ;
        }
    }
}
