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


package org.sipfoundry.sipxphone.featureindicator ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.PForm ;
import org.sipfoundry.sipxphone.awt.form.SimpleListForm ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.Timer;
import org.sipfoundry.util.AppResourceManager ;
import java.util.* ;
import java.awt.* ;


/**
 * <b>DO NOT EXPOSE</b><br><br>
 *
 * This container displays the various feature icon states.  It is designed
 * to listen to the FeatureIndicatorManager and update as notifications are
 * received.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class FeatureIndicatorPanel extends PScrollableComponentContainer implements FeatureIndicatorListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected FeatureIndicatorManager m_manager ;     // manager maintaining master state
    protected Hashtable m_htComponents ;        // map of indicators/components
    protected int m_iButtonDown = -1 ;          // Is a button being held down? (hints)

    protected Vector m_vIconTray ;              // Lists of elements for the icon tray
    protected icIconTrayContainer m_compTray ;  // container for icon tray elements
    protected PForm m_formParent ;              // parent form


    protected boolean m_ShowingTip ;             // Are we showing a tip?
    protected PLabel m_lblTip ;                  // label used for tips
    protected int m_iTipIndex = 0 ;              // current tip index
    protected int m_iNumTips = -1 ;              // max number of tips
    protected icTooltipHandler m_tooltipHandler = new icTooltipHandler() ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the feature indicator panel given a parent form.
     *
     * @param formParent The parent form used to display child forms.
     */
    public FeatureIndicatorPanel(PForm formParent)
    {
        super(4, PButtonEvent.BID_R4) ;

        m_formParent = formParent ;
        m_manager = Shell.getFeatureIndicatorManager() ;
        m_manager.addIndicatorListener(this) ;
        m_htComponents = new Hashtable() ;
        m_ShowingTip = false ;
        m_lblTip = new PLabel("", PLabel.ALIGN_SOUTH) ;
        addButtonListener(new icButtonListener()) ;

        m_vIconTray = new Vector(6) ;

        //force a create when getting the instance. all other calls to getInstance should pass false.
        VoicemailNotifyStateListener notifyStateListener = VoicemailNotifyStateListener.getInstance(true);
        notifyStateListener.initialize() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Rotate the tip displayed in the feature indicator panel.
     */
    public void rotateTip()
    {
        // Lazily load number of tips if not already known
        if (m_iNumTips == -1)
        {
            try
            {
                m_iNumTips = Integer.parseInt(getString("lblNumIdleTips")) ;
            }
            catch (Exception e) { /* burp */ }
        }


        // Cycle if we have enough...
        if (m_iNumTips > 0)
        {
            m_iTipIndex = ((m_iTipIndex + 1) % m_iNumTips) ;

            String strTip = getString("lblIdleTip" + (m_iTipIndex+1)) ;
            m_lblTip.setText(strTip) ;
        }

        surfaceComponents() ;
    }


    /**
     * This method is invoked when a new indicator is installed.
     *
     * @param indicator The indicator that was installed.
     */
    public void indicatorInstall(FeatureIndicator indicator)
    {
        int iState = m_manager.getRequestedIndicatorState(indicator) ;

        switch (iState)
        {
            case FeatureIndicatorManager.VIEWSTYLE_ICON:
                m_vIconTray.addElement(indicator);
                refreshIconTray() ;
                break ;
            case FeatureIndicatorManager.VIEWSTYLE_LINE:
            case FeatureIndicatorManager.VIEWSTYLE_MULTILINE:
                Component component = indicator.getComponent() ;
                if (component != null)
                {
                    if (m_compTray == null)
                        insertComponentAt(component, 0) ;
                    else
                        insertComponentAt(component, 1) ;
                    m_htComponents.put(indicator, component) ;
                }
                surfaceComponents() ;
                break ;
        }
    }


    /**
     * This method is invoked when a indicator is uninstalled.
     *
     * @param indicator The indicator that was removed.
     */
    public void indicatorRemoved(FeatureIndicator indicator)
    {
        int iState = m_manager.getRequestedIndicatorState(indicator) ;
        switch (iState)
        {
            case FeatureIndicatorManager.VIEWSTYLE_ICON:
                m_vIconTray.removeElement(indicator);
                refreshIconTray() ;
                break ;
            case FeatureIndicatorManager.VIEWSTYLE_LINE:
            case FeatureIndicatorManager.VIEWSTYLE_MULTILINE:
                Component component = (Component) m_htComponents.get(indicator) ;
                if (component != null)
                {
                    removeComponent(component) ;
                }

                m_htComponents.remove(indicator) ;
                surfaceComponents() ;
                break ;
        }
    }


    /**
     * This method is invoked when an end user requests a indicator refresh.
     *
     * @param indicator The indicator that is being refreshed.
     */
    public void indicatorRefreshed(FeatureIndicator indicator)
    {
        int iState = m_manager.getRequestedIndicatorState(indicator) ;
        switch (iState)
        {
            case FeatureIndicatorManager.VIEWSTYLE_ICON:
                refreshIconTray() ;
                break ;
            case FeatureIndicatorManager.VIEWSTYLE_LINE:
            case FeatureIndicatorManager.VIEWSTYLE_MULTILINE:
                Component component = (Component) m_htComponents.get(indicator) ;
                if (component != null)
                {
                    int iIndex = indexOf(component) ;
                    if (iIndex != -1)
                    {
                        Component compNew = indicator.getComponent() ;
                        setComponentAt(compNew, iIndex) ;
                        m_htComponents.put(indicator, compNew) ;
                    }
                }
                surfaceComponents() ;
                break ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Method
////

    /**
     * Presently, the expand icon tray displays a form detailing all of the
     * feature icons.
     */
    protected void expandIconTray()
    {
        if (m_vIconTray.size() == 1)
        {
            FeatureIndicator fi = (FeatureIndicator) m_vIconTray.elementAt(0) ;
            fi.buttonPressed() ;
        }
        else if (m_vIconTray.size() > 1)
        {
            icIconTrayExpansionForm form = new icIconTrayExpansionForm(m_formParent) ;

            form.showModal() ;
        }
    }


    /**
     * Refresh the icon tray and remove/add if needed.
     */
    protected void refreshIconTray()
    {
        if (m_vIconTray.size() == 0)
        {
            // Empty, simply remove the icon tray.
            if (m_compTray != null)
            {
                removeComponent(m_compTray) ;
                m_compTray = null ;
            }
        }
        else
        {
            icIconTrayContainer newContainer = new icIconTrayContainer(m_vIconTray) ;
            if (m_compTray != null)
            {
                // Replace
                int iIndex = indexOf(m_compTray) ;
                if (iIndex != -1)
                {
                    setComponentAt(newContainer, iIndex) ;
                    m_compTray = newContainer ;
                }
            }
            else
            {
                // Add
                insertComponentAt(newContainer, 0) ;
                m_compTray = newContainer ;
            }
        }
    }


    /**
     * Get the indicator for a particular component (reverse lookup)
     */
    protected FeatureIndicator getIndicator(Component component)
    {
        FeatureIndicator fi = null ;

        if (component != null)
        {
            Enumeration enumKeys = m_htComponents.keys() ;
            while (enumKeys.hasMoreElements())
            {
                FeatureIndicator indicator = (FeatureIndicator) enumKeys.nextElement() ;
                if (component == m_htComponents.get(indicator))
                {
                    fi = indicator ;
                    break ;
                }
            }
        }

        return fi ;
    }


    /**
     * Surface the components to the top most container.  Here, we add
     * also add the tips if needed.
     */
    protected void surfaceComponents()
    {
        super.surfaceComponents() ;

        if (getLength() <= 1)
        {
            if (!m_ShowingTip)
            {
                m_ShowingTip = true ;
                add(m_lblTip) ;
                Dimension dim = getSize() ;
                int iHeight = dim.height ;
                int iWidth = dim.width ;
                m_lblTip.setBounds(0, iHeight/2, iWidth, iHeight/2) ;
            }
        }
        else if (m_ShowingTip)
        {
            remove (m_lblTip) ;
            m_ShowingTip = false ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


    /**
     * Button Listener listens for buttons, kicks off hints, and dispatches
     * button events to the various feature indicator.
     */
    private class icButtonListener implements PButtonListener
    {
        public icButtonListener() { }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            // Release any tooltip timers
            if (m_iButtonDown != -1) {
                Timer.getInstance().removeTimers(m_tooltipHandler) ;
                m_iButtonDown = -1 ;
            }


            Component component = null ;
            switch (event.getButtonID())
            {
                case PButtonEvent.BID_R1:
                    component = getComponentAt(0) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R2:
                    component = getComponentAt(1) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R3:
                    component = getComponentAt(2) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R4:
                    component = getComponentAt(3) ;
                    event.consume() ;
                    break ;
            }

            if (component != null)
            {
                FeatureIndicator fi = getIndicator(component) ;
                if (fi != null)
                {
                    fi.buttonPressed() ;
                }
                else if (component == m_compTray)
                {
                    expandIconTray() ;
                }
            }
        }


        /** The specified button is being held down */
        public void buttonRepeat(PButtonEvent event)
        {
            Component component = null ;
            switch (event.getButtonID()) {
                case PButtonEvent.BID_R1:
                    component = getComponentAt(0) ;

                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R2:
                    component = getComponentAt(1) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R3:
                    component = getComponentAt(2) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R4:
                    component = getComponentAt(3) ;
                    event.consume() ;
                    break ;

            }

            if (component != null) {
                Enumeration enumKeys = m_htComponents.keys() ;
                while (enumKeys.hasMoreElements()) {
                    FeatureIndicator indicator = (FeatureIndicator) enumKeys.nextElement() ;
                    if (component == m_htComponents.get(indicator)) {
                        Shell.getInstance().displayHint(indicator.getHint()) ;
                        break ;
                    }
                }
            }
        }

        /** The specified button has been press downwards */
        public void buttonDown(PButtonEvent event)
        {
            Component component = null ;
            FeatureIndicator fiHint = null;

            // avoid multiple tooltip requests
            if (m_iButtonDown != -1)
            {
                return ;
            }

            switch (event.getButtonID())
            {
                case PButtonEvent.BID_R1:
                    component = getComponentAt(0) ;
                    fiHint = getIndicator(component) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R2:
                    component = getComponentAt(1) ;
                    fiHint = getIndicator(component) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R3:
                    component = getComponentAt(2) ;
                    fiHint = getIndicator(component) ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_R4:
                    component = getComponentAt(3) ;
                    fiHint = getIndicator(component) ;
                    event.consume() ;
                    break ;
            }

            // If we were given a hint form the component use it.
            if (fiHint != null)
            {
                String strHint = fiHint.getHint() ;
                if (strHint != null)
                {
                    m_iButtonDown = event.getButtonID() ;
                    Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, strHint) ;
                }
            }
            // Otherwise, this may be the icon tray
            else if ((component != null) && (component == m_compTray))
            {
                // If we only have one icon, use that icon's hint
                if (m_vIconTray.size() == 1)
                {
                    FeatureIndicator fi = (FeatureIndicator) m_vIconTray.elementAt(0) ;
                    String strHint = fi.getHint() ;
                    if (strHint != null)
                    {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, strHint) ;
                    }
                }
                // Otherwise use the generic hint
                else
                {
                    m_iButtonDown = event.getButtonID() ;
                    Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, AppResourceManager.getInstance().getString("lblIconTrayPrompt")) ;
                }
            }
        }
    }



    /**
     * Action listener that displays a hint
     */
    private class icTooltipHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            String strHint = (String) event.getObjectParam() ;
            if (strHint != null)
            {
                Shell.getInstance().displayHint(strHint) ;
            }
            m_iButtonDown = -1 ;
        }
    }


    /**
     * GUI container for icon tray
     */
    private class icIconTrayContainer extends PContainer
    {
        protected Vector m_vImages ;

        public icIconTrayContainer(Vector vImages)
        {
            GridBagLayout      gbl = new GridBagLayout() ;
            GridBagConstraints gbc = new GridBagConstraints() ;
            PLabel             lblImage ;

            this.setLayout(gbl) ;

            m_vImages = new Vector(vImages.size()+1) ;

            // Add Blank to force items right
            gbc.weightx = 1.0 ;
            gbc.weighty = 0.0 ;
            gbc.insets = new Insets(0, 0, 0, 0) ;
            gbc.fill = GridBagConstraints.HORIZONTAL ;
            gbc.gridwidth = 1 ;
            lblImage = new PLabel() ;
            this.add(lblImage, gbc) ;

            m_vImages.addElement(lblImage);

            for (int i=0; i<vImages.size(); i++)
            {
                FeatureIndicator fi = (FeatureIndicator) vImages.elementAt(i) ;
                // Add Image
                gbc.weightx = 0.0 ;
                gbc.weighty = 0.0 ;
                gbc.insets = new Insets(0, 8, 0, 0) ;
                gbc.fill = GridBagConstraints.VERTICAL ;
                gbc.gridwidth = 1 ;
                lblImage = new PLabel(fi.getIcon()) ;
                this.add(lblImage, gbc) ;
                m_vImages.addElement(lblImage);
            }
        }

        public void repaint()
        {
            super.repaint() ;

            if (m_vImages != null)
            {
                Enumeration enum = m_vImages.elements() ;
                while (enum.hasMoreElements())
                {
                    PLabel label = (PLabel) enum.nextElement() ;
                    label.repaint() ;
                }
            }
        }


        public void setForeground(Color color)
        {
            if (m_vImages != null)
            {
                Enumeration enum = m_vImages.elements() ;
                while (enum.hasMoreElements())
                {
                    PLabel label = (PLabel) enum.nextElement() ;
                    label.setForeground(color) ;
                }
            }
        }


        public void setBackground(Color color)
        {
            if (m_vImages != null)
            {
                Enumeration enum = m_vImages.elements() ;
                while (enum.hasMoreElements())
                {
                    PLabel label = (PLabel) enum.nextElement() ;
                    label.setBackground(color) ;
                }
            }
        }

        public void setFont(Font font)
        {
            if (m_vImages != null)
            {
                Enumeration enum = m_vImages.elements() ;
                while (enum.hasMoreElements())
                {
                    PLabel label = (PLabel) enum.nextElement() ;
                    label.setFont(font) ;
                }
            }
        }


        public void setOpaque(boolean bOpaque)
        {
            if (m_vImages != null)
            {
                Enumeration enum = m_vImages.elements() ;
                while (enum.hasMoreElements())
                {
                    PLabel label = (PLabel) enum.nextElement() ;
                    label.setOpaque(bOpaque) ;
                }
            }
        }
    }


    /**
     * List Form that displays the expanded view of the tray icons
     */
    protected class icIconTrayExpansionForm extends SimpleListForm implements PActionListener
    {
        Vector m_vIndicators ;

        public icIconTrayExpansionForm(PForm form)
        {
            super(form, "Feature Icons") ;

            setItemRenderer(new PIconTextRenderer()) ;

            initButtons() ;
            populateList() ;

            addActionListener(this) ;
        }


        public void actionEvent(PActionEvent e)
        {
            if (e.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK))
            {
                PIconTextRendererData data = (PIconTextRendererData) getSelectedElement() ;
                if (data != null)
                {
                    FeatureIndicator fi = (FeatureIndicator) data.getObjectData() ;
                    if (fi != null)
                    {
                        fi.buttonPressed() ;
                        populateList() ;
                    }
                }
            }
            else if (e.getActionCommand().equals("ACTION_CANCEL"))
            {
                closeForm(SimpleListForm.CANCEL) ;
            }
        }


        protected void initButtons()
        {
            getBottomButtonBar().clearItem(PBottomButtonBar.B3);

            PActionItem actionCancel = new PActionItem(
                    new PLabel(this.getString("lblGenericClose")),
                    this.getString("hint/core/system/closeform"),
                    this,
                    "ACTION_CANCEL") ;
        }


        protected void populateList()
        {
            PDefaultListModel model = new PDefaultListModel() ;

            m_vIndicators = m_vIconTray ;
            for (int i=0; i<m_vIndicators.size(); i++)
            {
                FeatureIndicator fi = (FeatureIndicator) m_vIndicators.elementAt(i) ;
                model.addElement(new PIconTextRendererData(fi.getShortDescription(),
                        fi.getIcon(), fi), fi.getHint()) ;

            }

            setDefaultListModel(model) ;
        }
    }
}
