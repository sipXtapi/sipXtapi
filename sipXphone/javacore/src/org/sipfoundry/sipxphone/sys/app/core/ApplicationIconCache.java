/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/ApplicationIconCache.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.app.core ;

import java.awt.* ;
import java.awt.image.* ;
import java.util.* ;
import java.net.* ;
import java.io.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;


/**
 * This class maintains an application Icon Cache for all of the apps
 * installed on the phone.  The cache only actaully only caches images
 * of remote applications (packed into a jar, obtaining via the network),
 * and will short-circuit to AppResourceManager() for all local images.
 *
 * @author Robert J. Andreasen. Jr.
 */
public class ApplicationIconCache
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String DEFAULT_APP_ICON = "imgGenericApp" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Hashtable          m_htIconCache ;
    private ApplicationManager m_appMgr ;
    private boolean            m_bDirty ;

    /** debug toggling */
    private boolean m_bDebug = false ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default constructor.
     */
    public ApplicationIconCache()
    {
        m_htIconCache = new Hashtable() ;
        m_appMgr = ApplicationManager.getInstance() ;
        reload() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Clear the cache and reload contents from persistent storage
     */
    public void reload()
    {
        try {
            PropertyManager manager = PropertyManager.getInstance() ;
            if (manager != null) {
                m_htIconCache = (Hashtable) manager.getObject("iconCache") ;
            }
        }
        catch (IllegalArgumentException iae) {
        	SysLog.log(iae.getMessage());
        }
        catch (Exception e) {
            SysLog.log(e) ;
        }
        m_bDirty = false ;
    }


    /**
     * Save the cache contents to persistent storage.  Note, there is logic to
     * stop redundant writes.  This will note write if their are not changes.
     */
    public void save()
    {
        if (m_bDirty) {
            try {
                PropertyManager manager = PropertyManager.getInstance() ;
                if (manager != null) {
                    manager.putObject("iconCache", m_htIconCache) ;
                }
            } catch (IOException ioe) {
                SysLog.log(ioe) ;
            }
        }
        m_bDirty = false ;
    }


    /**
     * Remove the image specified by the passed codebase and name
     * from the icon cache.
     */
    public void clearImage(String strCodebase, String strImageName)
    {
        // Validate Params...
        if (strImageName == null)
            throw new IllegalArgumentException("image name is required!") ;

        // You do not need to do anything if no code base is supplied
        if (strCodebase != null) {
            String strKey = generateKey(strCodebase, strImageName) ;
            m_htIconCache.remove(strCodebase) ;
            m_bDirty = true ;
        }
    }


    /**
     * Determine if any the two specified images are different
     */
    public boolean areImagesDifferent(Image img1, Image img2)
    {
        boolean     bRC = false ;
        int[]       objImg1 = new int[0] ;
        int         iImg1Width ;
        int         iImg1Heigth ;
        ColorModel  img1ColorModel = null ;
        int[]       objImg2 = new int[0] ;
        int         iImg2Width ;
        int         iImg2Heigth ;
        ColorModel  img2ColorModel = null ;

        if ((img1 != null) && (img2 != null)) {

            // Take apart image1
            {
                iImg1Width = img1.getWidth(null) ;
                iImg1Heigth = img1.getHeight(null) ;

                // Grab the pixel and create/add our descriptor
                PixelGrabber grabber = new PixelGrabber(img1, 0, 0, iImg1Width, iImg1Heigth, true) ;
                try {
                    if (grabber.grabPixels(0)) {
                        objImg1 = (int[]) grabber.getPixels() ;
                        img1ColorModel = grabber.getColorModel() ;
                    }
                } catch (InterruptedException ie) {
                    SysLog.log(ie);
                    return true ;
                }
            }

            // Take apart image2
            {
                iImg2Width = img2.getWidth(null) ;
                iImg2Heigth = img2.getHeight(null) ;

                // Grab the each pixel and create/add our descriptor
                PixelGrabber grabber = new PixelGrabber(img2, 0, 0, iImg2Width, iImg2Heigth, true) ;
                try {
                    if (grabber.grabPixels(0)) {
                        objImg2 = (int[]) grabber.getPixels() ;
                        img2ColorModel = grabber.getColorModel() ;
                    }
                } catch (InterruptedException ie) {
                    SysLog.log(ie);
                    return true ;
                }
            }

            // Check Width
            if (iImg1Width != iImg2Width) {
                bRC = true ;
            }
            // Check Height
            else if (iImg1Width != iImg2Width) {
                bRC = true ;
            } else {
                // Check Pixels and Color
                for (int i=0; i<objImg1.length; i++) {
                    if (objImg1[i] != objImg1[2]) {
                        bRC = true ;
                        break ;
                    } else if (img1ColorModel.getRGB(objImg1[i]) != img2ColorModel.getRGB(objImg2[i])) {
                        bRC = true ;
                        break ;
                    }
                }
            }
        } else if ((img1 == null) && (img2 != null)) {
            // If one is null and the other isn't...
            bRC = true ;
        } else if ((img1 != null) && (img2 == null)) {
            // If one is null and the other isn't...
            bRC = true ;
        }

        return bRC ;
    }


    /**
     * Add an Image to the application icon cache.  Note: Entries will a null
     * codebase will not be added.
     *
     * This routine assumes that the image is already loaded.  Otherwise, we
     * would need an image consumer to force an image load/realization.
     */
    public void addImage(String strCodebase, String strImageName, Image imgIcon)
    {
        // Validate Params...
        if (strImageName == null)
            throw new IllegalArgumentException("image name is required!") ;
        if (imgIcon == null)
            throw new IllegalArgumentException("image is required!") ;


        // Only add Images if they have a codebase,  otherwise they are local
        // and do not require caching.  Also, null strImage and null image
        // params are simply invalid.
        if (strCodebase != null) {
            m_bDirty = true ;

            Object objPixels = null ;
            String strKey = generateKey(strCodebase, strImageName) ;
            int    iWidth = imgIcon.getWidth(null) ;
            int    iHeight = imgIcon.getHeight(null) ;

            // Only continue if we can get the width/height.  We are assuming
            // that the image has already been realized.
            if ((iWidth != -1) && (iHeight != -1)) {

                // Grab the pixel and create/add our descriptor
                PixelGrabber grabber = new PixelGrabber(imgIcon, 0, 0, iWidth, iHeight, true) ;
                try {
                    if (grabber.grabPixels(0)) {
                        objPixels = grabber.getPixels() ;
                    }
                } catch (InterruptedException ie) {
                    SysLog.log(ie);
                }

                // If all is well, add it to the cache!
                if (objPixels != null) {
                    ApplicationIconCacheItem item = new ApplicationIconCacheItem(objPixels, iWidth, iHeight, imgIcon) ;
                    m_htIconCache.put(strKey, item) ;

                    if (m_bDebug) {
                        System.out.println("ApplicationIconCache: added: ") ;
                        System.out.println("  Codebase: " + strCodebase) ;
                        System.out.println("     Image: " + strImageName) ;
                        System.out.println("     Width: " + iWidth) ;
                        System.out.println("    Height: " + iHeight) ;
                    }
                }
            }
        }
    }


    /**
     * Get an image from our icon cache
     */
    public Image getImage(String strCodebase, String strImageName)
    {
        Image imageRC = null ;

        // Validate params
        if (strImageName == null)
            throw new IllegalArgumentException("image name is required!") ;

        // A codebase indicates a remote icon.  If no codebase is found, then
        // simply pull the image out of the AppResourceManager
        if (strCodebase != null) {
            String strKey = generateKey(strCodebase, strImageName) ;

            ApplicationIconCacheItem item = (ApplicationIconCacheItem) m_htIconCache.get(strKey) ;
            if (item != null) {
                imageRC = item.getRenderedImage() ;
                if (imageRC != null) {
                    // HOT HIT: Rendered Image found
                    if (m_bDebug) {
                        System.out.println("ApplicationIconCache: HOT Hit: ") ;
                        System.out.println("  Codebase: " + strCodebase) ;
                        System.out.println("     Image: " + strImageName) ;
                        System.out.println("     Width: " + item.m_iWidth) ;
                        System.out.println("    Height: " + item.m_iHeight) ;
                        System.out.println("     Image: " + imageRC) ;
                    }
                } else {
                    // COLD HIT: Unrendereed Image found
                    MemoryImageSource source = new MemoryImageSource(item.m_iWidth, item.m_iHeight, (int[]) item.m_objData, 0, item.m_iWidth) ;
                    imageRC = Toolkit.getDefaultToolkit().createImage(source) ;
                    item.m_imgRendered = imageRC ;

                    if (m_bDebug) {
                        System.out.println("ApplicationIconCache: COLD Hit: ") ;
                        System.out.println("  Codebase: " + strCodebase) ;
                        System.out.println("     Image: " + strImageName) ;
                        System.out.println("     Width: " + item.m_iWidth) ;
                        System.out.println("    Height: " + item.m_iHeight) ;
                        System.out.println("     Image: " + imageRC) ;
                    }
                }
            }
        } else {
            // Get it from the AppResourceManager
            imageRC = AppResourceManager.getInstance().getImage(strImageName) ;
        }

        return imageRC ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected String generateKey(String strCodebase, String strIcon)
    {
        return strCodebase + ":" + strIcon ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested/Inner Classes
////
}
