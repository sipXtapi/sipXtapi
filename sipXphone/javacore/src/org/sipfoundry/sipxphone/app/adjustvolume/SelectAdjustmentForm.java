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


package org.sipfoundry.sipxphone.app.adjustvolume ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;

import java.text.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;


public class SelectAdjustmentForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int SEL_NONE        = -1 ;
    public static final int SEL_BASE        = 0 ;
    public static final int SEL_CONTRAST    = 1 ;
    public static final int SEL_HANDSET     = 2 ;
    public static final int SEL_RINGER      = 3 ;
    public static final int SEL_HEADSET     = 4 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** list of adjustable controls */
    protected PList m_listControls ;

    /** list model for our data */
    protected icSelectAdjustmentListModel m_listModel ;

    /** action command dispatcher */
    protected icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor accepting a parent form
     */
    public SelectAdjustmentForm(PForm formParent)
    {
        super(formParent, "Select Adjustment") ;
        setStringResourcesFile(AdjustVolumeForm.STRING_RESOURCES);

        setTitle(getString("lblSelectAdjustmentTitle")) ;

        // Create our event/command dispatcher
        icCommandDispatcher dispatcher = new icCommandDispatcher() ;

        // Create everything else
        initControls() ;
        initMenubar() ;

        setHelpText(getString("adjust_select"), getString("adjust_select_title")) ;

        // physically lay everything out
        layoutComponents() ;
    }



    /**
     * Constructor accepting an application context
     */
    public SelectAdjustmentForm(Application application)
    {
        super(application) ;
        setStringResourcesFile(AdjustVolumeForm.STRING_RESOURCES);

        setTitle(getString("lblSelectAdjustmentTitle")) ;

        // Create our event/command dispatcher
        icCommandDispatcher dispatcher = new icCommandDispatcher() ;

        // Create everything else
        initControls() ;
        initMenubar() ;

        setHelpText(getString("adjust_select"), getString("adjust_select_title")) ;

        // physically lay everything out
        layoutComponents() ;
    }


    /**
     *
     */
    protected void layoutComponents()
    {
        addToDisplayPanel(m_listControls) ;
    }


    /**
     *
     */
    protected void initControls()
    {
        PDefaultItemRenderer renderer = new PDefaultItemRenderer() ;
        m_listControls = new PList() ;
        m_listModel = new icSelectAdjustmentListModel() ;
        m_listControls.setListModel(m_listModel) ;

        m_listControls.addActionListener(m_commandDispatcher) ;

        // Initialize our display renderer
        renderer.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
        renderer.setAlignment(PDefaultItemRenderer.EAST) ;
        m_listControls.setItemRenderer(renderer) ;

        m_listModel.removeAllElements() ;


        // WARNING: This order must match the SEL_ constants
        m_listModel.addElement(getString("lblBaseSpeakerTitle")) ;
        if (!PingerApp.isTestbedWorld())
            m_listModel.addElement(getString("lblLCDContrastTitle")) ;
        m_listModel.addElement(getString("lblHandsetSpeakerTitle")) ;
        m_listModel.addElement(getString("lblRingerTitle")) ;
        m_listModel.addElement(getString("lblHeadsetSpeakerTitle")) ;

        m_listControls.setSelectedIndex(0) ;
    }


    /**
     *
     */
    protected void initMenubar()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionAdjust = new PActionItem( new PLabel("Adjust"),
                getString("hint/adjustments/select/adjust"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_ADJUST) ;
        menuControl.setItem(PBottomButtonBar.B3, actionAdjust) ;
        getLeftMenuComponent().addItem(actionAdjust) ;


        PActionItem actionCancel = new PActionItem( new PLabel("Cancel"),
                getString("hint/core/system/cancelform"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(actionCancel) ;
    }


    /**
     *
     */
    protected void onCancel()
    {
        closeForm(SEL_NONE) ;
    }


    /**
     *
     */
    protected void onAdjust()
    {
        int iIndex = m_listControls.getSelectedIndex();

        //because we removed contrast on the WIN32 platform,
        //when the user selects an item we have to increment 1 so it points
        //to the real value they meant to choose.
        if (PingerApp.isTestbedWorld() && iIndex >= SEL_CONTRAST)
            iIndex++;

        closeForm(iIndex) ;
    }


    /**
     *
     */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL       = "action_cancel" ;
        public final String ACTION_ADJUST       = "action_adjust" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_ADJUST)) {
                onAdjust() ;
            } else  if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                onAdjust() ;
            }
        }
    }


    /**
     * Very wimpy list model
     */
    protected class icSelectAdjustmentListModel extends PDefaultListModel
    {
        /**
         *
         */
        public icSelectAdjustmentListModel()
        {
            super() ;
        }


        /**
         *
         */
        public String getElementPopupTextAt(int iIndex)
        {
            String strRC = null ;

            switch (iIndex) {
            case SEL_RINGER:
                strRC = getString("hint/adjustments/select/ringer_volume") ;
                break;
            case SEL_HEADSET:
                strRC = getString("hint/adjustments/select/headset_volume") ;
                break ;
            case SEL_BASE:
                strRC = getString("hint/adjustments/select/speaker_volume") ;
                break;
            case SEL_CONTRAST:
                strRC = getString("hint/adjustments/select/lcd_contrast") ;
                break;
            case SEL_HANDSET:
                strRC = getString("hint/adjustments/select/handset_volume") ;
                break;
            }

            return strRC ;
        }
    }
}

