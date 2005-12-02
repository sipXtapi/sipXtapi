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
import java.util.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * The task form is shown when the user presses the 'more' key.  This form
 * displays 3 tabs of content: app list, help menu, app menu.
 *
 * @author Robert J. Andreasen. Jr.
 */
public class TaskForm extends PTabForm implements PTabListener
{
    private PForm               m_formCurrent ;
    private TaskApplicationList m_pnlAppList ;
    private TaskContextHelp     m_pnlContextHelp ;
    private TaskContextMenu     m_pnlContextMenu ;
    private boolean             m_bHideMenuTab ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default form construction
     */
    public TaskForm(Application application)
    {
        super(application, "Task Manager") ;

        addTabListener(this) ;
        addFormListener(new icFormListener()) ;

        // Create major subpanels
        m_pnlAppList = new TaskApplicationList(this) ;
        m_pnlContextHelp = new TaskContextHelp(this) ;
        m_pnlContextMenu = new TaskContextMenu(this) ;

        // Initialize Tabs
        setContent(PTabControl.LEFT, m_pnlContextMenu) ;
        setHint(PTabControl.LEFT,getString("hint/coreapp/task/menu_tab")) ;
        PLabel lblMenu = new PLabel(getString("lblTaskMenu"), PLabel.ALIGN_CENTER) ;
        lblMenu.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_TAB_ENTRY)) ;
        setLabel(PTabControl.LEFT, lblMenu ) ;

        setContent(PTabControl.MIDDLE, m_pnlContextHelp) ;
        setHint(PTabControl.MIDDLE,getString("hint/coreapp/task/help_tab")) ;
        PLabel lblHelp = new PLabel(getString("lblTaskHelp"), PLabel.ALIGN_CENTER) ;
        lblHelp.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_TAB_ENTRY)) ;
        setLabel(PTabControl.MIDDLE, lblHelp) ;

        setContent(PTabControl.RIGHT, m_pnlAppList) ;
        setHint(PTabControl.RIGHT,getString("hint/coreapp/task/apps_tab")) ;
        PLabel lblApps = new PLabel(getString("lblTaskApps"), PLabel.ALIGN_CENTER) ;
        lblApps.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_TAB_ENTRY)) ;
        setLabel(PTabControl.RIGHT, lblApps) ;

        // Kick start the world
        setTab(PTabControl.RIGHT) ;
        onTab(PTabControl.RIGHT) ;



        m_bHideMenuTab = isMenuTabHidden() ;
        if (m_bHideMenuTab)
        {
            hideMenuTab() ;
        }
        
        m_formOverlay = new PTaskFormOverlay();
        m_formOverlay.initialize();
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the active form and populate the various tabs
     */
    public void setForm(PForm form)
    {
        m_formCurrent = form ;

        m_pnlAppList.populateApplications() ;
        m_pnlContextHelp.populateHelp(form) ;
        m_pnlContextMenu.populateMenu(form) ;
    }


    /**
     * Gets the active form
     */
    public PForm getForm()
    {
        return m_formCurrent ;
    }


    /**
     * Super's implementation for setting the tab for a form.
     */
    public void setTab(int iTab)
    {
        if ((iTab != PTabControl.LEFT) || !m_bHideMenuTab)
        {
            super.setTab(iTab) ;
        }
        else
        {
            closeForm() ;
        }
    }


    /**
     * Invoked when a tab is changed, this method sets the title/icon
     */
    public void onTab(int iTab)
    {
        AppResourceManager resMgr = AppResourceManager.getInstance() ;

        switch (iTab) {
            case PTabControl.LEFT:
                if (m_formCurrent != null) {
                    String strFormName = m_formCurrent.getFormName() ;
                    if (strFormName == null)
                        strFormName = "Menu" ;

                    setTitle(strFormName) ;
                }
                setIcon(resMgr.getImage("imgMenuIcon")) ;
                break ;
            case PTabControl.MIDDLE:
                if (m_formCurrent != null) {
                    String strHelpTitle = m_formCurrent.getHelpTitle() ;
                    if (strHelpTitle == null)
                        strHelpTitle = "Help" ;

                    setTitle(strHelpTitle) ;
                }
                setIcon(resMgr.getImage("imgHelpIcon")) ;
                break ;
            case PTabControl.RIGHT:
                setTitle("Applications") ;
                setIcon(resMgr.getImage("imgPingerIcon")) ;
                break ;
        }
    }


    /**
     * Called by the framework when a tab is changed.
     * <br>
     * NOTE: Exposed as an implementation side effect
     */
    public void tabChanged(PTabEvent event)
    {
        onTab(event.getTab()) ;
    }


    /**
     *
     */
    public class icFormListener extends PFormListenerAdapter
    {
        public void focusGained(PFormEvent event)
        {
            onTab(getTab()) ;
        }


        public void focusLost(PFormEvent event)
        {
            m_pnlAppList.cleanup() ;
            m_pnlContextHelp.cleanup() ;
            m_pnlContextMenu.cleanup() ;

            closeForm() ;
        }
    }


    protected boolean isMenuTabHidden()
    {
        boolean bMenuTabHidden = false ;

        String strSetting ;

        strSetting = PingerConfig.getInstance().getValue(
                PingerConfig.PHONESET_SHOW_MENU_TAB) ;
        if ((strSetting != null) && strSetting.equalsIgnoreCase("DISABLE"))
        {
            bMenuTabHidden = true ;
        }

        return bMenuTabHidden ;
    }


    protected void hideMenuTab()
    {
        PTabControl ctrl = getTabControl() ;

        ctrl.setActiveTabImage(PTabControl.LEFT, null) ;
        ctrl.setLabel(new PLabel(getString("lblGenericClose")), PTabControl.LEFT);
        setHint(PTabControl.LEFT,getString("hint/core/system/closeform")) ;
        ctrl.setActiveTabImage(PTabControl.MIDDLE, getImage("imgTabFrameMiddleSelectedNoB1")) ;
        ctrl.setActiveTabImage(PTabControl.RIGHT, getImage("imgTabFrameRightSelectedNoB1")) ;
    }
}
