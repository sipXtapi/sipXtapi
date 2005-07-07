/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/SoundFeaturesForm.java#2 $
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

import java.lang.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sip.* ;
import org.sipfoundry.sip.event.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;


/**
 * The Line Manager form displays status for each of our lines.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SoundFeaturesForm extends SimpleListForm
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private icActionDispatcher m_dispatcher ;   // action dispatcher
    PLabel GIPSActive = null;
    PLabel GIPSInActive = null;
    boolean bGIPSEnabled = true; //aums eit's on unless told otherwise
    boolean m_infoBoxOpen = false;

    MessageBox m_infoMsgBox = null;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor requiring both an application context and a read-only flag.
     */
    public SoundFeaturesForm(Application app)
    {
        super(app, "Audio Features") ;

        m_dispatcher = new icActionDispatcher() ;

        addActionListener(m_dispatcher) ;

        setHelpText(getString("audio"), getString("audio_title")) ;
        setItemRenderer(new SoundFeaturesRenderer()) ;
        initBars() ;
        loadProductImages();
        addFeatures();

        //info box used when user hits for more info
        m_infoMsgBox = new MessageBox(getApplication(),MessageBox.TYPE_INFORMATIONAL);

    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////





    /**
     * Invoked up form opening (NOTE: this is not focus change).
     */
    public void onFormOpening()
    {
    }


    /**
     * Invoked up form closing (NOTE: this is not focus change).
     */
    public void onFormClosing()
    {
    }



//////////////////////////////////////////////////////////////////////////////
// Implemenation
////

    /**
     * Initialize the bottom button bar and command bar.
     */
    protected void initBars()
    {
        // Init the command bar
//        enableCommandBar(true) ;

//        PCommandBar commandBar = getCommandBar() ;
        PBottomButtonBar bottomBar = getBottomButtonBar() ;
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;


        // Clear bottom button bar and menus
        bottomBar.clearItem(PBottomButtonBar.B1);
        bottomBar.clearItem(PBottomButtonBar.B2);
        bottomBar.clearItem(PBottomButtonBar.B3);
        leftMenu.removeAllItems() ;
        rightMenu.removeAllItems() ;



        // "Cancel" Action
        PActionItem actionExit = new PActionItem(new PLabel(getString("lblGenericClose")),
                getString("hint/core/system/closeform"),
                m_dispatcher,
                m_dispatcher.ACTION_EXIT) ;

        bottomBar.setItem(PBottomButtonBar.B2, actionExit);

        // "Info" Action
        PActionItem actionInfo = new PActionItem(new PLabel("Info"),
                getString("hint/preferences/soundfeatures/info"),
                m_dispatcher,
                m_dispatcher.ACTION_INFO) ;

        bottomBar.setItem(PBottomButtonBar.B3, actionInfo);
        leftMenu.addItem(actionInfo) ;

        rightMenu.addItem(actionExit) ;

    }

    public void loadProductImages()
    {
        GIPSActive = new PLabel(getImage("imgGIPSActive")) ;
        GIPSInActive = new PLabel(getImage("imgGIPSInActive")) ;
    }

    public void addFeatures()
    {
        removeAllElements(); //clear out all elements

        if (bGIPSEnabled == true)
            addElement(GIPSActive);
        else
            addElement(GIPSInActive);
        invalidate();
    }

    public void enableGIPS(boolean state)
    {
        System.out.println("Called enableGIPS with " + state);
        bGIPSEnabled = state;
        addFeatures();
    }

    public void closeForm(int exitCode)
    {
        super.closeForm(exitCode);

        if (m_infoBoxOpen)
            m_infoMsgBox.closeForm(0);

    }

    protected void onInfo(int index)
    {
        switch(index)
        {
            case 0:
                m_infoMsgBox.setTitle("Global IP Sound");
                m_infoMsgBox.setMessage("Powered with SoundWare(tm)\n"+
                        "SoundWare is a trademark of Global IP Sound.\n"+
                        "For more information, visit www.globalipsound.com "+
                        "or send an email to pingtel@globalipsound.com");
                m_infoBoxOpen = true;
                m_infoMsgBox.showModal();
                m_infoBoxOpen = false;
                break;

        }
    }

    protected void setActionEnable(String strAction, boolean bEnable)
    {
        enableMenusByAction(bEnable, strAction) ;

        PCommandBar commandBar = getCommandBar() ;
        if (commandBar != null)
        {
            commandBar.enableByAction(strAction, bEnable) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested/Inner Classes
////


    /**
     * Helper class that listens for action commands and dispatches to various
     * onXXX handlers.
     */
    private class icActionDispatcher implements PActionListener
    {
        public final String ACTION_INFO = "ACTION_INFO" ;
        public final String ACTION_EXIT = "ACTION_EXIT" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_INFO)||
             (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)))
            {
                int selectedSoundOption = getSelectedIndex() ;
                onInfo(selectedSoundOption) ;
            }
            else if (event.getActionCommand().equals(ACTION_EXIT))
            {
                closeForm(OK) ;
            }
        }
    }
}