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


package org.sipfoundry.sipxphone.awt.form ;

import java.awt.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
/**
 *
 *
 */
public abstract class PTabForm extends PAbstractForm
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** title bar control */
    protected PTitlebar   m_titlebar ;
    /** tab control */
    protected PTabControl m_tabControl ;
    /** where we actually place our data*/
    protected PBorderedContainer m_displayPane ;

    protected Component  m_components[] = new Component[3] ;
    protected String     m_hint[] = new String[3] ;


//////////////////////////////////////////////////////////////////////////////
// Constructions
////
    /**
     *
     */
    public PTabForm(Application application)
    {
        super(application) ;

        createComponents() ;
        layoutComponents() ;

        addFormListener(new icFormAdapter()) ;
        addButtonListener(new icButtonListener()) ;
    }

    public PTabForm(Application application, String strFormName)
    {
        super(application, strFormName) ;

        createComponents() ;
        layoutComponents() ;

        addFormListener(new icFormAdapter()) ;
        addButtonListener(new icButtonListener()) ;
    }

    public PTabForm(Application application, PForm formParent)
    {
        super(application, formParent) ;

        createComponents() ;
        layoutComponents() ;

        addFormListener(new icFormAdapter()) ;
        addButtonListener(new icButtonListener()) ;
    }

    public PTabForm(Application application, PForm formParent, String strFormName)
    {
        super(application, formParent, strFormName) ;

        createComponents() ;
        layoutComponents() ;

        addFormListener(new icFormAdapter()) ;
        addButtonListener(new icButtonListener()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
    }


    /**
     * called when the form has lost focus.
     *
     * @param formGainingFocus form that is gaining focus over this form
     */
    public void onFocusLost(PForm formGainingFocus)
    {
    }

    /**
     *
     */
    public void setLabel(int iTab, PLabel label)
    {
        m_tabControl.setLabel(label, iTab) ;
    }


    public void setTab(int iTab)
    {
        int iOldTab = getTab() ;

        // Remove old tab...
        if ((iOldTab != iTab) && (iOldTab != -1) && (m_components[iOldTab] != null)) {
            m_displayPane.remove(m_components[iOldTab]) ;
        }

        m_tabControl.setTab(iTab) ;
        m_displayPane.add(m_components[iTab]) ;
        m_displayPane.validate() ;
        m_displayPane.repaint() ;
    }


    public int getTab()
    {
        return m_tabControl.getTab() ;
    }


    /**
     *
     */
    public void setTitle(String strTitle)
    {
        m_titlebar.setTitle(strTitle) ;
    }


    /**
     *
     */
    public void setIcon(Image imgIcon)
    {
        m_titlebar.setIcon(imgIcon) ;
    }


    public void addTabListener(PTabListener listener)
    {
        m_tabControl.addTabListener(listener) ;
    }


    public void removeTabListener(PTabListener listener)
    {
        m_tabControl.removeTabListener(listener) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    protected PTabControl getTabControl()
    {
        return m_tabControl ;
    }


    /**
     *
     */
    protected void createComponents()
    {
        // Create Titlebar
        m_titlebar = new PTitlebar() ;

        // Create Tab Control
        m_tabControl = new PTabControl() ;
        m_tabControl.setLabel(new PLabel(""), PTabControl.LEFT) ;
        m_tabControl.setLabel(new PLabel(""), PTabControl.MIDDLE) ;
        m_tabControl.setLabel(new PLabel(""), PTabControl.RIGHT) ;

        m_displayPane = new PBorderedContainer() ;
    }


    /**
     *
     */
    protected void layoutComponents()
    {
        setLayout(null) ;

        m_titlebar.setBounds(0, 0, 160, 26) ;
        add(m_titlebar) ;

        m_displayPane.setBounds(0, 26, 160, 108) ;
        add(m_displayPane) ;

        m_tabControl.setBounds(0, 134, 160, 26) ;
        add(m_tabControl) ;
    }


    /**
     *
     */
    protected void setContent(int iTab, Component component)
    {
        // m_tabControl
        m_components[iTab] = component ;

        if (m_tabControl.getTab() == iTab) {
            setTab(iTab) ;
        }
    }

    /**
     * Adds a hint for the specified tab.  (will be applied to the BX button.
     */
    protected void setHint(int iTab, String hint)
    {
        m_hint[iTab] = hint;
    }

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    private class icFormAdapter extends PFormListenerAdapter
    {
        public void focusLost(PFormEvent event)
        {
            onFocusLost(event.getFormGainingFocus()) ;
        }

        public void focusGained(PFormEvent event)
        {
            onFocusGained(event.getFormLosingFocus()) ;
        }
    }



    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {

        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_B1:
                    setTab(PTabControl.LEFT) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B2:
                    setTab(PTabControl.MIDDLE) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B3:
                    setTab(PTabControl.RIGHT) ;
                    event.consume() ;
                    break ;
            }
        }


        /**
         * The specified button is being held down
         */
        public void buttonRepeat(PButtonEvent event)
        {
           switch (event.getButtonID()) {
                case PButtonEvent.BID_B1:
                    if (m_hint[0] != null)
                        Shell.getInstance().displayHint(m_hint[0]);
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B2:
                    if (m_hint[1] != null)
                        Shell.getInstance().displayHint(m_hint[1]);
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B3:
                    if (m_hint[2] != null)
                        Shell.getInstance().displayHint(m_hint[2]);
                    event.consume() ;
                    break ;
            }
        }
    }
}
