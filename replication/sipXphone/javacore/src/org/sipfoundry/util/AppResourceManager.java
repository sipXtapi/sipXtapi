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

package org.sipfoundry.util ;

import java.util.* ;
import java.io.* ;
import java.awt.* ;
import java.net.* ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.Application;
import org.sipfoundry.sipxphone.sys.util.PingerInfo;
import org.sipfoundry.sipxphone.sys.SystemDefaults;
/**
 * Application Resource Manager - A ResourceBundle style Resource Manager
 * that creates a stack-like mechanism for resolving resources.  At the lowest
 * level of the heirarchy, a system string resource bundle and images bundle
 * are queried to look up strings and images.  If the keys are not found, the
 * Resource Manager returns back the "key name" or a default image, respectfully.
 * <br><br>
 * Applications can place other resource files ahead of the system resource in
 * the stack by invoking the addStringResourceFile and addImageResourceFile
 * methods on the AppResourceManager.  The AppResourceManager will search the
 * list by evaluating the last added item to the first added item (LIFO).
 * Developers should use removeStringResourceFile and removeImageResourceFile
 * when contexts move our of scope.
 * <br><br>
 * In cases of multiple locales, the Application Resource Manager will search
 * the current local followed by the default locale (en_US) before
 * traversing the resource bundle stack.
 * <br><br>
 * A Locale is constructed using the language code, country code and
 * the variant if available. If the locale is different from the default one,
 * when we set a property file for an application, we look for a file
 * with the suffixes on the basis of locale.
 * <br><br>
 * For example, if new locale has to be created for Japan,
 * PHONESET_LOCALE_LANGUAGE will be "ja" and
 * PHONESET_LOCALE_COUNTRY will be "JP".
 * In this case, when looking for a localized value of a
 * key for SpeedDial, it searches the value from the resources of a file in the
 * order of <br>
 * "SpeeddialForm.properties_ja_JP", <br>
 * "SpeeddialForm.properties_ja" <br>
 * "SpeeddialForm.properties" <br>
 *
 * For a case like Hotel Commonwealth, the language code and the country
 * code will be default ones but the variant will be something like "commonwealth".
 * In this case, when looking for a localized value of a
 * key for SpeedDial, it searches the value from the resources of a file in the
 * order of <br>
 * "SpeeddialForm.properties_en_US_commonwealth", <br>
 * "SpeeddialForm.properties_en_US"  <br>
 * "SpeeddialForm.properties_en" <br>
 * "SpeeddialForm.properties"  <br>
 * <br><br>
 *
 * @author Robert J. Andreasen, Jr.
 *
 * ::TODO:: Add Missing Image detection (getImage doens't fail if image isn't found)
 */
public class AppResourceManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** system string resource property file */
    public static final String SYSTEM_STRING_RESFILE     = "pinger.properties" ;
    /** system image resource property file */
    public static final String SYSTEM_IMAGE_RESFILE      = "images.properties" ;

    /** default optimized size of our application resource bundle 'stack' */
    protected static final int    DEFAULT_STACK_SIZE       = 5 ;
    /** Default image if desire image is not found */
    protected static final String DEFAULT_MISSING_IMAGE    = "images/missing.gif" ;

    //using an array here to make things efficient
    //as hashtable and vectors may have big overhead
    //when looking for a resource.

    /**
     * constant to denote type of system string resource.
     * It is also an index to look for in the resources array
     * to find system string resources.
     */
    protected static final int  SYSTEM_STRING_RESOURCE  = 0;
    /**
     * constant to denote type of system image resource.
     * It is also an index to look for in the resources array
     * to find system image resources.
     */
    protected static final int  SYSTEM_IMAGE_RESOURCE   = 1;
    /**
     * constant to denote type of application string resource.
     * It is also an index to look for in the resources array
     * to find application string resources.
     */
    protected static final int  APP_STRING_RESOURCE     = 2;
    /**
     * constant to denote type of application image resource.
     * It is also an index to look for in the resources array
     * to find application image resources.
     */
    protected static final int  APP_IMAGE_RESOURCE      = 3;

    /** total number of different resource types. */
    protected static final int  NUMBER_OF_RESOURCE_TYPES = 4;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** singleton instance of our AppResourceManager */
    static protected AppResourceManager         m_resourceManager = null;

    /**
     * contains vectors of application and system resourceIDs.
     * The index of respective resource type is defined by constants
     * SYSTEM_IMAGE_RESOURCE, SYSTEM_STRING_RESOURCE, APP_IMAGE_RESOURCE and
     * APP_STRING_RESOURCE.
     */
    protected  Vector[]      m_vResourceIDs;

    /**
     * contains hashtables of application and system resourcebundles.
     * The index of respective resource type is defined by constants
     * SYSTEM_IMAGE_RESOURCE, SYSTEM_STRING_RESOURCE, APP_IMAGE_RESOURCE and
     * APP_STRING_RESOURCE.
     */
    protected Hashtable[]   m_htResourceBundles;

    /**
     * hastable of image resourcsIDs.
     */
    protected Vector m_htImgResourceIDs;

    /** Maintains a cache of rendered images */
    protected        Hashtable                  m_htRenderedImages ;

    /** locale of the system */
    protected String m_strLocale = PingerInfo.getInstance().getLocale().toString();

    /** is the locale of the system the default locale? */
    protected boolean m_bDefaultLocale = false;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Protected no argument constructor.  Use getInstance() to obtain a
     * reference to this class.
     */
    protected AppResourceManager()
    {
        // Initialize the application resource stack
        m_vResourceIDs =  new Vector[NUMBER_OF_RESOURCE_TYPES];
        m_htResourceBundles = new Hashtable[NUMBER_OF_RESOURCE_TYPES];
        for( int i = 0; i<NUMBER_OF_RESOURCE_TYPES; i++ )
        {
            m_vResourceIDs[i] = new Vector(DEFAULT_STACK_SIZE);
            m_htResourceBundles[i] = new Hashtable(DEFAULT_STACK_SIZE);
        }

        m_htRenderedImages = new Hashtable(100) ;

        if( PingerInfo.getInstance().getLocale().getLanguage().equals(SystemDefaults.LOCALE_LANGUAGE) &&
            PingerInfo.getInstance().getLocale().getCountry().equals(SystemDefaults.LOCALE_COUNTRY) &&
            TextUtils.isNullOrSpace( PingerInfo.getInstance().getLocale().getVariant() ) )
        {
            m_bDefaultLocale = true;
        }

        // Load System String Resources
        Vector vecFiles = getFilesToLookup( SYSTEM_STRING_RESFILE, m_strLocale);
        Enumeration enum = vecFiles.elements();
        while( enum.hasMoreElements() )
        {
            String strPropertyFile = (String)enum.nextElement();

            try {
                InputStream stream = ClassLoader.getSystemResourceAsStream(strPropertyFile);
                if( stream != null )
                {
                    ResourceBundle resourceBundle = new PropertyResourceBundle(stream) ;
                    m_vResourceIDs[SYSTEM_STRING_RESOURCE].insertElementAt(strPropertyFile, 0) ;
                    m_htResourceBundles[SYSTEM_STRING_RESOURCE].put(strPropertyFile, resourceBundle) ;
                }
            } catch (IOException e) {
                System.out.println("AppResourceManager: Unable to open " + strPropertyFile) ;
            }
        }



        // Load System Image Resources
        vecFiles = getFilesToLookup( SYSTEM_IMAGE_RESFILE, m_strLocale);
        enum = vecFiles.elements();
        while( enum.hasMoreElements() )
        {
            String strPropertyFile = (String)enum.nextElement();
            try {
                InputStream stream = ClassLoader.getSystemResourceAsStream(strPropertyFile);
                if( stream != null )
                {
                    ResourceBundle resourceBundle = new PropertyResourceBundle(stream) ;
                    m_vResourceIDs[SYSTEM_IMAGE_RESOURCE].insertElementAt(strPropertyFile, 0) ;
                    m_htResourceBundles[SYSTEM_IMAGE_RESOURCE].put(strPropertyFile, resourceBundle) ;
                }
            } catch (IOException e) {
                System.out.println("AppResourceManager: Unable to open " + strPropertyFile) ;
            }
        }
    }


    /**
     * get an instance to the AppResourceManager.
     *
     * @return AppResourceManager instance
     */
    static public AppResourceManager getInstance()
    {
        // Create an instance if we do not already have one
        if (m_resourceManager == null) {
          m_resourceManager = new AppResourceManager() ;
        }

        return m_resourceManager ;
    }


//////////////////////////////////////////////////////////////////////////////
// Operations
////



    /**
     * removes the resource ID and adds it to the top of the stack.
     * This method is used to set up system resource.
     */
    public synchronized void topStringResourceFile( String strPropertyFile ){
        Application app = null;
        topStringResourceFile( strPropertyFile, app );
    }

    /**
     * removes the resource ID already in the stack( if any )
     * and adds it to the top of the stack.
     * It tries to get the resource from the resource of application
     * and if it fails, tries to get system resource.
     */
    public synchronized void topStringResourceFile( String strPropertyFile,
                                                    Application application){
        if( strPropertyFile != null ){
            if (m_vResourceIDs[APP_STRING_RESOURCE].contains(strPropertyFile)) {
               int iIndex = findLastKeyInstance( strPropertyFile, m_vResourceIDs[APP_STRING_RESOURCE]);
               m_vResourceIDs[APP_STRING_RESOURCE].removeElementAt(iIndex);
               m_vResourceIDs[APP_STRING_RESOURCE].insertElementAt(strPropertyFile, 0) ;
            }else
            {
                addStringResourceFile( strPropertyFile, application );
            }
        }

    }

    /**
     * Add a string resource file to the AppResourceManager stack
     * It tries to get the resource from the resource of application
     * and if it fails, tries to get system resource.
     */
    public synchronized void addStringResourceFile( String strPropertyFile,
                                                    Application application ){

        Vector vecFiles = getFilesToLookup( strPropertyFile, m_strLocale);
        Enumeration enum = vecFiles.elements();
        while( enum.hasMoreElements() )
        {
            String strPropertyFileToLookup = (String)enum.nextElement();
            InputStream inputStream = null;
            if( application != null ){
                inputStream = application.getResourceAsStream(strPropertyFileToLookup);
            }
            if( inputStream ==  null ){
                inputStream =
                    ClassLoader.getSystemResourceAsStream(strPropertyFileToLookup);
            }
            if( inputStream != null )
            {
                addStringResourceFile( strPropertyFileToLookup, inputStream );
            }
        }
    }

   /**
     * Add a string resource file to the AppResourceManager stack
     * The property file should be a part of system resource.
     *
     * @param strPropertyFile file name of the property file
     */
    public synchronized void addStringResourceFile(String strPropertyFile)
    {
        Application app = null;
        addStringResourceFile( strPropertyFile, app);
    }


/**
 * Add a string resource file to the AppResourceManager stack
 *
 * @param strPropertyFile file name of the property file
 * @param InputStream  InputStream to load to construct the resourcebundle
 */
public synchronized void addStringResourceFile
    (String strPropertyFile, InputStream stream)
{
    ResourceBundle resourceBundle = null ;

    // Don't bother loading resource bundle if already loaded
    if (m_vResourceIDs[APP_STRING_RESOURCE].contains(strPropertyFile)) {
        // retrieve existing entry...
        resourceBundle =
           (ResourceBundle)m_htResourceBundles[APP_STRING_RESOURCE].get(strPropertyFile) ;
    }
    else {
        // Go ahead and try to load the file...
        try {
            resourceBundle = new PropertyResourceBundle(stream) ;
            //debug(" succesfully added  " + strPropertyFile );
        } catch (Exception e) {
            System.out.println
                ("AppResourceManager: Unable to open " + strPropertyFile) ;
        }
    }

    if (resourceBundle != null) {
        // Add lookup key to resource id vector

        //commented this line as we want multiple resource IDs
        //to be present even if multiple apps add
        //the same resource bundle. The side effect would be
        //when an application unloads a resource file, it will
        //only remove the resource ID than belonged to the application
        //from the stack.
        //another side effect is that application should remember to
        //unload the resource ID  when they are done so as to
        //minimize memory leak.
        //m_vResourceIDs[APP_STRING_RESOURCE].removeElement(strPropertyFile) ;
        m_vResourceIDs[APP_STRING_RESOURCE].insertElementAt(strPropertyFile, 0) ;

        // Add actual resource bundle to our bundle hash
        m_htResourceBundles[APP_STRING_RESOURCE].put(strPropertyFile, resourceBundle) ;

//            System.out.println
//              ("stack of resourceIDS [\n"+ printVector(m_vResourceIDs[APP_STRING_RESOURCE]) +"]");
//            System.out.println
//              ("actual resourcebundles are [\n" + printHash(m_htResourceBundles[APP_STRING_RESOURCE])+"]");
    }


}



/**
 * Add a image resource file to the AppResourceManager stack
 *
 * @param strPropertyFile file name of the property file
 */
public synchronized void addImageResourceFile(String strPropertyFile)
{
    ResourceBundle resourceBundle = null ;

    Vector vecFiles = getFilesToLookup( strPropertyFile, m_strLocale);
    Enumeration enum = vecFiles.elements();
    while( enum.hasMoreElements() )
    {
        String strPropertyFileToLookup = (String)enum.nextElement();
        // Don't bother loading resource bundle if already loaded
        if ( m_vResourceIDs[APP_IMAGE_RESOURCE].contains(strPropertyFileToLookup)) {
            // retrieve existing entry...
            resourceBundle = (ResourceBundle)
                m_htResourceBundles[APP_IMAGE_RESOURCE].get(strPropertyFileToLookup) ;
        }
        else {
            // Go ahead and try to load the file...
            try {
                InputStream stream = ClassLoader.getSystemResourceAsStream(strPropertyFileToLookup);
                if( stream != null )
                {
                    resourceBundle = new PropertyResourceBundle(stream) ;
                }
            } catch (IOException e) {
                System.out.println
                    ("AppResourceManager: Unable to open " + strPropertyFileToLookup) ;
            }
        }

        if (resourceBundle != null) {
            // Add lookup key to resource id vector
            m_vResourceIDs[APP_IMAGE_RESOURCE].removeElement(strPropertyFileToLookup) ;
            m_vResourceIDs[APP_IMAGE_RESOURCE].insertElementAt(strPropertyFileToLookup, 0) ;

            // Add actual resource bundle to our bundle hash
            m_htResourceBundles[APP_IMAGE_RESOURCE].put(strPropertyFileToLookup, resourceBundle) ;
        }
    }

}

    private void debug(String str ){
        System.out.println("\n\n-----------------APP RESOURCE MANAGER---------------\n"
                            +str+"\n------------------------------------------\n\n");
    }

    /**
     * Remove a String resource file from the AppResourceManager stack
     *
     * @param strPropertyFile file name of the property file
     */
    public synchronized void removeStringResourceFile(String strPropertyFile)
    {
        int iIndex =
            findLastKeyInstance(strPropertyFile, m_vResourceIDs[APP_STRING_RESOURCE]) ;

        if (iIndex != -1) {
            m_vResourceIDs[APP_STRING_RESOURCE].removeElementAt(iIndex) ;
            // Only release if we don't have any more instances loaded
            if (findLastKeyInstance
                (strPropertyFile, m_vResourceIDs[APP_STRING_RESOURCE]) == -1) {
                m_htResourceBundles[APP_STRING_RESOURCE].remove(strPropertyFile) ;
            }
        }
        else {
            System.out.println
                ("AppResourceManager: Unable to release " + strPropertyFile) ;
        }
    }


    /**
     * Remove a Image resource file from the AppResourceManager stack
     *
     * @param strPropertyFile file name of the property file
     */
    public synchronized void removeImageResourceFile(String strPropertyFile)
    {
        int iIndex =
            findLastKeyInstance(strPropertyFile, m_vResourceIDs[APP_IMAGE_RESOURCE]) ;

        if (iIndex != -1) {
            m_vResourceIDs[APP_IMAGE_RESOURCE].removeElementAt(iIndex) ;

            // Only release if we don't have any more instances loaded
            if (findLastKeyInstance
                (strPropertyFile, m_vResourceIDs[APP_IMAGE_RESOURCE]) == -1) {

                m_htResourceBundles[APP_IMAGE_RESOURCE].remove(strPropertyFile) ;
            }
        }
        else {
            System.out.println
                ("AppResourceManager: Unable to release " + strPropertyFile) ;
        }
    }

    /**
     * Lookup a string from our stack of resource bundles.
     *
     * @param strKey the key of the string to lookup
     * @return string pointed to by strKey or strKey itself if not found
     *
     */
     public synchronized String getString(String strKey)
    {
        String strRC = getString( strKey, APP_STRING_RESOURCE );
        // if not found then check the system resource bundle
        if( strRC == null ){
            strRC = getSystemString(strKey) ;
        }
        return strRC ;
    }


    /**
     * Identical to getString, except it ignores the application stack and
     * only tries to find a string from the system resource file.
     */
    public synchronized String getSystemString(String strKey)
    {
        String strRC = getString( strKey, SYSTEM_STRING_RESOURCE );
        if( strRC == null )
            strRC = strKey ;
        return strRC;
    }



    /**
    * Lookup an image from our stack of resource bundles.
    *
    * @param strKey the key of the image to lookup
    * @return Image pointed to by strKey or a default image if not found
    *
    */
    public Image getImage(String strKey)
    {
        Image imageRC = (Image) m_htRenderedImages.get(strKey) ;
        if( imageRC == null )
            imageRC = getImage( strKey, APP_IMAGE_RESOURCE );
        if( imageRC == null )
            imageRC = getSystemImage(strKey) ;
        return imageRC;
    }


    /**
     * Identical to getImage, except it ignores the application stack and
     * only tries to find a Image from the system resource file.
     *
     */
    public synchronized Image getSystemImage(String strKey)
    {
        Image imageRC = getImage( strKey, SYSTEM_IMAGE_RESOURCE );

        if (imageRC == null) {
            URL url = ClassLoader.getSystemResource(DEFAULT_MISSING_IMAGE) ;
            if (url != null) {
                imageRC = Toolkit.getDefaultToolkit().getImage(url) ;
            } else {
                System.out.println("AppResourceManager: Missing missing image") ;
            }
        }

        return imageRC ;
    }





/**
 * Utility method that will go ahead and preload all the images in the
 * passed property file.
 *
 */
public synchronized void preloadImages(Component component, String strPropertyFile)
{
    ResourceBundle resourceBundle = null ;
    Vector vecFiles = getFilesToLookup( strPropertyFile, m_strLocale);
    Enumeration enum = vecFiles.elements();
    int iCounter = 0;

    MediaTracker            tracker ;
    int                     iID = 0 ;
    boolean bPreloaded = false;

    while( enum.hasMoreElements() )
    {
        resourceBundle = null;
        iCounter++;
        String strPropertyFileToLookup = (String)enum.nextElement();
        for( int i = 0; i<2; i++ )
        {
            resourceBundle = null;
            int iResourceId = APP_IMAGE_RESOURCE;
            if( i == 1)
                iResourceId = SYSTEM_IMAGE_RESOURCE;

            // Don't bother loading resource bundle if already loaded
            if ( m_vResourceIDs[iResourceId].contains(strPropertyFileToLookup)) {
                // retrieve existing entry...
                resourceBundle = (ResourceBundle)
                    m_htResourceBundles[iResourceId].get(strPropertyFileToLookup) ;
            }
            else {
                // Go ahead and try to load the file...
                try {
                    InputStream stream =
                        ClassLoader.getSystemResourceAsStream(strPropertyFileToLookup);
                    if( stream != null ){
                        resourceBundle = new PropertyResourceBundle(stream) ;
                    }

                } catch (IOException e) {
                    if( iCounter == 1)
                    {
                        System.out.println
                            ("AppResourceManager: Unable to preload " + strPropertyFileToLookup) ;
                    }//otherwise no need to report error for locale specific files.
                }
            }

            if (resourceBundle != null)
            {
                Enumeration enumKeys = resourceBundle.getKeys() ;

                tracker = new MediaTracker(component) ;

                System.out.print("Preloading Images defined in : "+strPropertyFileToLookup) ;

                // Add all of our images to the tracker...
                while (enumKeys.hasMoreElements()) {
                    Object objElement = enumKeys.nextElement() ;
                    if (objElement != null) {
                        String strKey = objElement.toString() ;
                        String strImageFile = resourceBundle.getString(strKey) ;
                        if (strImageFile != null) {
                            URL url = ClassLoader.getSystemResource(strImageFile) ;
                            if (url != null) {
                                Image image = Toolkit.getDefaultToolkit().getImage(url) ;
                                if (image != null)
                                {
                                    // System.out.print("\t" + Integer.toString(iID) + ". " + strKey + " " + strImageFile + ": ") ;
                                    tracker.addImage(image, iID) ;

                                    try {
                                        tracker.waitForID(iID) ;
                                        if (tracker.isErrorID(iID)) {
                                            System.out.println("") ;
                                            System.out.println("\tError preloading image: " + Integer.toString(iID) + ". " + strKey + " " + strImageFile) ;
                                        } else {
                                            m_htRenderedImages.put(strKey, image) ;
                                            System.out.print(".") ;
                                        }
                                        iID++ ;
                                    } catch (InterruptedException ie) {
                                        SysLog.log(ie);

                                    }
                                }//end if( image != null )
                            }
                        }
                    }
                }//end while
                bPreloaded = true;
                break;
            }

        }
    }

}


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    protected synchronized String getString(String strKey,  int iResourceId)
    {
        int             i ;                 // index variable
        ResourceBundle  resourceBundle ;    // reference of a resource bundle (from hash table)
        String          strHashKey ;        // property file name
        String          strRC = null ;      // our return string
        boolean         bFound = false ;    // Did we find the string?

        // Traverse the Application Stack
        Vector vec = m_vResourceIDs[ iResourceId ];
        Hashtable hash = m_htResourceBundles[ iResourceId ];
        int iSize = vec.size();
        for (i=0; i<iSize; i++) {
        // Get the key
            strHashKey = (String) vec.elementAt(i) ;
            if (strHashKey != null) {
                // Try to snag the actual resource bundle
                resourceBundle = (ResourceBundle)hash.get(strHashKey) ;
                if (resourceBundle != null) {
                    try {
                        strRC = (String) resourceBundle.getString(strKey) ;
                        bFound = true ;
                        break ;
                    } catch (MissingResourceException mre) {
                        // we really don't care... for now, keep searching...
                    }
                }
            }
        }
        return strRC ;
    }


    protected synchronized Image getImage(String strKey, int iResourceID)
    {
        Image           imageRC = null ;    // image to return
        String strImageName = getString( strKey, SYSTEM_IMAGE_RESOURCE );
        if (strImageName != null)
        {
            strImageName = strImageName.trim();
            imageRC = Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(strImageName)) ;
            if (imageRC != null)
            {
                m_htRenderedImages.put(strKey, imageRC) ;
            }else{
                 System.out.println("AppResourceManager: Couldn't find image "+ strImageName );
            }
        }
        return imageRC ;
    }

    /**
     * gets the locale specific files to lookup given a property file name.
     * For example, if the locale is ja_JP, if strPropertFile is
     * SpeeddialForm.properties, it adds
     *
     * "SpeeddialForm.properties" <br>
     * "SpeeddialForm.properties_ja" <br>
     * "SpeeddialForm.properties_ja_JP" <br> to the vector.
     */
    protected Vector getFilesToLookup(String strPropertyFile, String strLocale)
    {
        Vector vec = new Vector();
        vec.addElement( strPropertyFile );
        if( !m_bDefaultLocale )
        {
            StringTokenizer tokenizer = new StringTokenizer( strLocale, "_");
            StringBuffer strBuffer = new StringBuffer( strPropertyFile );
            while( tokenizer.hasMoreTokens() )
            {
                String strToken = tokenizer.nextToken();
                strBuffer.append('_');
                strBuffer.append( strToken );
                strPropertyFile = strBuffer.toString();
                vec.addElement( strPropertyFile );
            }
        }
        return vec;
    }


    /**
     * find the last reference of the passed key in the application stack
     *
     * @param strKey key to search for
     * @param vector to search
     *
     * @return int index of key in list or -1 if not found
     */
    protected int findLastKeyInstance(String strKey, Vector vector)
    {
        String strResourceID ;
        int    iRC = -1 ;
        int iSize = vector.size();
        for (int i=iSize; i>0; i--) {
            strResourceID = (String) vector.elementAt(i-1) ;
            if (strResourceID != null) {
                if (strResourceID.equals(strKey)) {
                    iRC = i-1 ;
                    break ;
                }
            }
        }
        return iRC ;
    }


    /**
     * prints the elements in vector nicely separated by new lines.
     */
    private String printVector(Vector v ){
        StringBuffer buffer = new StringBuffer();
        Enumeration enum = v.elements();
        while( enum.hasMoreElements() ){
            buffer.append( (String)(enum.nextElement()) );
            buffer.append("\n");
        }
        return buffer.toString();
    }

    /**
     *prints the keys in the hashtable nicely separated by new lines.
     */
    private String printHash(Hashtable hash ){
        StringBuffer buffer = new StringBuffer();
        Enumeration enum = hash.keys();
        while( enum.hasMoreElements() ){
            buffer.append( (String)(enum.nextElement()) );
            buffer.append("\n");
        }
        return buffer.toString() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Testing Main
////

    /**
     * Santity check: make sure we can load a simple string
     */
    public static void main(String argv[])
    {
        AppResourceManager arm = AppResourceManager.getInstance() ;

        System.out.println(arm.getString("lblTest")) ;
    }
}





