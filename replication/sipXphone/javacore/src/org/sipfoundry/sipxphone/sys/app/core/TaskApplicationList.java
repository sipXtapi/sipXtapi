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
import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;


/**
 * The Application task list lists all of the default/user installed
 * applications within the system.  The end user can then select one
 * to active it.
 *
 * @author Robert J. Andreasen. Jr.
 */
public class TaskApplicationList extends PMultiColumnList implements PListListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String DEFAULT_APP_ICON = "imgGenericApp" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the parent form */
    private PForm m_formParent ;

    private ApplicationIconCache m_iconCache = null ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create the tab form given the specified parent frame
     */
    public TaskApplicationList(PForm formParent)
    {
        m_formParent = formParent ;

        addListListener(this) ;
        setItemRenderer(new icAppRenderer()) ;
        ApplicationRegistry.getInstance().addApplicationRegistryListener(new icApplicationRegistryListener()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Populate the list of applications
     */
    public void populateApplications()
    {
        ApplicationManager appMgr = ApplicationManager.getInstance() ;
        m_iconCache = ShellApp.getInstance().getCoreApp().getIconCache() ;

        int iCol = 0;
        removeAllItems() ;

        // Populate our world....
        ApplicationDescriptor desc[] = ApplicationRegistry.getInstance().getApplications() ;
        sortApplicationDescriptors(desc) ;
        for (int i=0; i<desc.length; i++) {
            try {
                String strCodebase = desc[i].getCodebase() ;
                String strTitle = desc[i].getTitle() ;
                String strIcon = desc[i].getLauncherIcon() ;
                String strHint = desc[i].getHint() ;
                if ((strHint == null) || (strHint.length() == 0)) {
                    strHint = null ;
                } else {
                    strHint = strTitle + "|" + strHint ;
                }

                // We cannot continue without an icon and title
                if ((strTitle != null) && (strTitle.trim().length() != 0) &&
                    (strIcon != null)  && (strIcon.trim().length() != 0)) {


                    // Get the Image
                    Image iconImage = null;
                    try {
                        iconImage = getImageIcon(desc[i], true) ;
                    } catch(Exception e) {
                        System.out.println(e) ;
                    }

                    if (iconImage == null) {
                        System.out.println("ERROR: Could not load the icon for: " + strTitle) ;
                        iconImage = AppResourceManager.getInstance().getImage(DEFAULT_APP_ICON) ;
                    }

                    // Make sure the image has been loaded
                    trackImage(iconImage) ;

                    // Add it to our icon list
                    addElement(iCol, new icAppInfo(iCol, iconImage, strTitle, desc[i]), strHint) ;
                    iCol = (iCol + 1) % 2 ;
                }
            } catch (Exception e) {
                System.out.println("Unable to populate task manager with: " + desc[i].getTitle()) ;
                SysLog.log(e) ;
            }
        }
        m_iconCache.save() ;
    }


    /**
     * This method is invoked from the TaskForm whenever the Task form loses
     * focus.  We clear our data to help promote garbage collection and reduce
     * the amount of noise present when trying to track down resource leaks.
     */
    public void cleanup()
    {
        removeAllItems() ;
    }


    protected void dumpApplicationDescriptors(String strLabel, ApplicationDescriptor desc[])
    {
        System.out.println() ;
        System.out.println("Dumping Application Descriptors: " + strLabel) ;
        for (int i=0; i<desc.length; i++) {
            System.out.println(Integer.toString(i) + ": " + desc[i].getTitle()) ;
        }
    }


    /**
     * Sort the list of application descriptors by their title.
     */
    public void sortApplicationDescriptors(ApplicationDescriptor desc[])
    {
//dumpApplicationDescriptors("before sort", desc) ;

        int iLength = desc.length ;
        for (int i=0; i<iLength; i++) {
            for (int j=0; j<iLength; j++) {
                if (i != j) {
                    String strCompareI = desc[i].getTitle() ;
                    String strCompareJ = desc[j].getTitle() ;
                    int    iCompare = 0 ;


                    // Perform null-safe comparison
                    if ((strCompareI == null) && (strCompareJ == null)) {
                        iCompare = 0 ;
                    } else if (strCompareI == null) {
                        iCompare = 1 ;
                    } else if (strCompareJ == null) {
                        iCompare = -1 ;
                    } else {
                        iCompare = strCompareI.toLowerCase().compareTo(strCompareJ.toLowerCase()) ;
                    }

                    if (iCompare < 0) {
                        ApplicationDescriptor swap ;

                        swap = desc[j] ;
                        desc[j] = desc[i] ;
                        desc[i] = swap ;
                    }
                }
            }
        }

    //dumpApplicationDescriptors("after sort", desc) ;
    }

    /**
     * Invoked by the framework when an item within the list is selected.
     * <br>
     * NOTE: Exposes as a side effect of the ListListener
     */
    public void selectChanged(PListEvent event)
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;

        int iRow = event.getSelectedRow() ;
        int iCol = event.getSelectedColumn() ;

        if ((iCol != -1) && (iRow != -1)) {
            icAppInfo info = (icAppInfo) getElement(iCol, iRow) ;

            String strElement = info.m_strTitle ;
            if (strElement != null) {

                // Check to see if the icon has changed...
                Image imgCacheImage = getImageIcon(info.m_appDescriptor, true) ;
                Image imgRealImage = getImageIcon(info.m_appDescriptor, false) ;
                if ((imgRealImage != null) && (imgCacheImage != null)) {
                    trackImage(imgRealImage) ;
                    trackImage(imgCacheImage) ;
                    if (m_iconCache.areImagesDifferent(imgRealImage, imgCacheImage)) {
                        String strCodebase = info.m_appDescriptor.getCodebase() ;
                        String strIcon = info.m_appDescriptor.getLauncherIcon() ;

                        if ((strCodebase != null) && (strIcon != null))
                        {
                            System.out.println("**** ICON CACHE IS STALE FOR: " + strCodebase) ;
                            m_iconCache.clearImage(strCodebase, strIcon) ;
                            m_iconCache.addImage(strCodebase, strIcon, imgRealImage) ;
                        }

                        m_iconCache.save() ;
                    }
                }
                imgCacheImage.flush() ;


                // See if it's a USER app
                if (info.m_appDescriptor != null) {
                    if (ApplicationManager.getInstance().activateApplication(info.m_appDescriptor)) {
                        ((PAbstractForm)m_formParent).closeForm() ;
                    } else {
                        MessageBox alert = new MessageBox(((PAbstractForm)m_formParent).getApplication(), MessageBox.TYPE_ERROR) ;
                        alert.setMessage("\n"+strElement+"\n\nUnable to activate application.") ;
                        alert.showModal() ;
                    }
                }
                else {
                    MessageBox alert = new MessageBox(((PAbstractForm)m_formParent).getApplication(), MessageBox.TYPE_ERROR) ;
                    alert.setMessage("\n"+strElement+"\n\nI'm sorry, but that feature is not yet implemented.") ;
                    alert.showModal() ;

                    clearSelection() ;
                }
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Get the image icon for the specified descriptor
     */
    private Image getImageIcon(ApplicationDescriptor descriptor, boolean bUseCache)
    {
        ApplicationManager  appMgr = ApplicationManager.getInstance() ;
        String              strCodebase = descriptor.getCodebase() ;
        String              strIcon = descriptor.getLauncherIcon() ;
        Image               imgRC = null ;
        try
        {
            // If the code base is null then the icon is local
            if (strCodebase == null) {
                imgRC = Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(strIcon)) ;
            }
            // remote icon, we need to load it.
            else {
                // Get the Icon out of Cache
                if (bUseCache)
                    imgRC = m_iconCache.getImage(strCodebase, strIcon) ;
                if (imgRC == null) {
                    URL iconURL = appMgr.getClassLoader(strCodebase).getResource(strIcon) ;
                    imgRC = Toolkit.getDefaultToolkit().getImage(iconURL);
                    if ((imgRC != null) && bUseCache) {
                        m_iconCache.addImage(strCodebase, strIcon, imgRC) ;
                    }
                }
            }
        }
        catch(Exception e)
        {
            System.out.println("ERROR:Could not load icon, using default");
        }

        // If we have any problems getting the icon, then use the default icon.
        if (imgRC == null) {
            imgRC = AppResourceManager.getInstance().getImage(DEFAULT_APP_ICON) ;
        }
        return imgRC ;
    }


    /**
     * Convenience method that uses a MediaTracker to make sure that an image
     * is fully loaded.
     */
    private void trackImage(Image image)
    {
        // Wait for the icon image to be loaded
        MediaTracker tracker = new MediaTracker(this);
        tracker.addImage(image, 0);
        try {
            tracker.waitForID(0);
        } catch (InterruptedException e) { }
        tracker.removeImage(image, 0) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    private class icApplicationRegistryListener implements ApplicationRegistryListener
    {
        /**
         * Invoked whenever an application is added to the application registry
         */
        public void applicationAdded(ApplicationDescriptor desc)
        {
            if (((PAbstractForm) m_formParent).isInFocus()) {
                populateApplications() ;
                updateRenderers() ;
                repaint() ;
            }
        }



        /**
         * Invoked whenever an application is remove from the application registry
         */
        public void applicationRemoved(ApplicationDescriptor desc)
        {
            if (((PAbstractForm) m_formParent).isInFocus()) {
                populateApplications() ;
                updateRenderers() ;
                repaint() ;
            }
        }
    }


    /**
     * Item renderer for task list, creates an 'icTaskContainer'
     */
    private class icAppRenderer implements PItemRenderer
    {
        public Component getComponent(Object objSource, Object objValue, boolean bSelected)
        {
            Component compRC = null ;
            if (objValue != null) {
                icAppInfo info = (icAppInfo) objValue ;
                compRC = new icTaskContainer(info, info.m_iColumn==COL2) ;
            } else {
                compRC = new PLabel("") ;
            }

            compRC.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;

            return compRC ;
        }


        /**
         * Simple container that displays both an icon and a label
         */
        protected class icTaskContainer extends PContainer
        {
            icAppInfo   m_appInfo = null ;
            PLabel      m_lblText ;
            PLabel      m_lblImage ;
            boolean     m_bIconFirst ;

            public icTaskContainer(icAppInfo appInfo, boolean bIconFirst)
            {
                m_appInfo = appInfo ;
                this.setLayout(null) ;
                this.setOpaque(false) ;

                m_lblText = new PLabel(appInfo.m_strTitle) ;
                this.add(m_lblText) ;
                m_lblImage = new PLabel(appInfo.m_imgAppIcon) ;
                this.add(m_lblImage) ;

                m_bIconFirst = bIconFirst ;
            }

            public void setFont(Font font)
            {
                if (m_lblText != null)
                    m_lblText.setFont(font) ;

                if (m_lblImage != null)
                    m_lblText.setFont(font) ;

                super.setFont(font) ;
            }

            public void setBounds(int x, int y, int w, int h)
            {
                int iIconWidth = m_appInfo.m_imgAppIcon.getWidth(this) ;
                int iIconHeight = m_appInfo.m_imgAppIcon.getHeight(this) ;

                if (m_bIconFirst) {
                    m_lblImage.setBounds(0, 0, iIconWidth, h) ;
                    m_lblText.setBounds(iIconWidth, 0, w-iIconWidth, h) ;
                } else {
                    m_lblImage.setBounds(w-iIconWidth, 0, iIconWidth, h) ;
                    m_lblText.setBounds(0, 0, w-iIconWidth, h) ;
                }

                super.setBounds(x, y, w, h) ;
            }
        }
    }


    /**
     * Simple data object containing the information needed to describe an
     * application.
     */
    private class icAppInfo
    {
        public Image    m_imgAppIcon ;
        public String   m_strTitle ;
        public int      m_iColumn ;
        public ApplicationDescriptor m_appDescriptor;

        public icAppInfo(int iColumn, Image imgAppIcon, String strTitle, ApplicationDescriptor appDescriptor)
        {
            m_iColumn = iColumn ;
            m_imgAppIcon = imgAppIcon ;
            m_strTitle = strTitle ;
            m_appDescriptor = appDescriptor ;
        }
    }
}










