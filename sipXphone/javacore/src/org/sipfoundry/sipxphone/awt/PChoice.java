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

package org.sipfoundry.sipxphone.awt ;

import java.awt.* ;
import java.util.Vector ;
import java.util.Enumeration;


import org.sipfoundry.util.GUIUtils ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.SimpleListForm ;
import org.sipfoundry.sipxphone.awt.form.PForm ;
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.SystemDefaults ;
import org.sipfoundry.sipxphone.sys.Settings ;

/**
 * The Choice control represents a non-editable list of options. The current
 * choice displays in the control; the end user selects the control
 * to bring up a separate control where a different choice can be selected.
 *
 * Currently, instead of a pop-up control (such as those used by the Windows
 * operating systems) a separate list of items displays. Users can select
 * an item from this list by pressing "OK" or "double-clicking" (pressing twice)
 * on the item.
 * @author Robert J. Andreasen, Jr.
 */
public class PChoice extends PComponent implements PFocusTraversable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** vector of list listeners */
    private Vector m_vListListeners ;


    /** hint displayed when a user presses and holds down the associated
        screen button */
    private String m_strHint ;

    /** button id bound to this check box */
    private int m_iAssociatedButtonId ;

     /** is the associated button currently held down? */
    private boolean m_bButtonDown ;

    /** image representing drop down control tag/triangle */
    private static Image s_imgDropDown ;

    /** lnternal data model for PChoice */
    private PDefaultListModel m_choiceListModel ;

    /** display list */
    private SimpleListForm             m_choiceSimpleListForm ;

    /** dispatches commands to various onXXX handlers */
    private icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

    /** The last selected PChoice index to comeback to the same selection
     * if the user hits "CANCEL" in the actual list
     */
    private int m_iLastSelectedIndex = -1;

    /** vector for adding and removing focus listeners */
    private Vector m_vFocusListeners;

    /** boolean to denote if this component has focus */
    private boolean m_bHasFocus = false;

    /** boolean to denote if the list inside this component is in focus
     * Its used to check to see if the list is to be shown or not
     * depending on if its already shown.
     */
    private boolean m_bListInFocus = false;

    /** each text field can have user label */
    private String m_strLabel ;
    /** the user label must be aligned relatively to the text field */
    private int m_iLabelAlignment ;
    /** font used to when displaying text (with a label) */
    private Font m_displayLabelFont  = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL) ;
    /** font used to when displaying text (without a label) */
    private Font m_displayFont       = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;

    /** height of the textfield where the timeout value is shown */
    private int m_iTextFieldHeight ;
    /** width of the textfield where the timeout value is shown */
    private int m_iTextFieldWidth  ;
    /** X offset of the textfield where the timeout value is shown */
    private int m_iTextFieldXOffset ;
    /** Y offset of the textfield where the timeout value is shown */
    private int m_iTextFieldYOffset  ;

     /** X shift value for the text field. */
    private int m_iAlignXOffset = 0 ;
    /** Y shift value for the text field. */
    private int m_iAlignYOffset = 0 ;
    /** Default length of text field in characters */
    private int m_iTextFieldLength ;
    /** Should we automatically calculate the text field length? */
    private boolean m_bAutoTextFieldLength ;
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * default constructor is made "protected"
     */
    protected  PChoice()
    {

    }


    /**
     * This constructor sets the hint and title to empty strings.
     */
    public PChoice(PForm formParent)
    {
        this(formParent, "", "");
    }

    /**
     * Constructor for PChoice
     * @param formParent Parent form that is using this control
     * @param strHint  hint for this control
     * @param strTitle title for the list in the control.
     */
    public PChoice(PForm formParent, String strHint, String strTitle)
    {
        setAssociatedButton(-1) ;
        m_bButtonDown = false ;
        m_strHint = strHint ;

        addButtonListener(new icButtonWatcher()) ;

        // Only load check box images if needed
        if (s_imgDropDown == null) {
            s_imgDropDown = getImage("imgChoiceDropDownArrow") ;
        }

        m_choiceSimpleListForm      =
            new SimpleListForm(formParent, strTitle) ;
        m_choiceSimpleListForm.addActionListener(m_dispatcher) ;
        m_choiceListModel = new PDefaultListModel();
        m_choiceListModel.addListDataListener(new icListDataListener());
      }


    /**
     * @deprecated do not expose
     */
    public void doLayout()
    {
        calculateDimensions(null) ;
        super.doLayout() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////



    /////METHODS DELEGATING TO m_choiceListModel

    /*-------------------begin of accessor methods--------------*/
    /** gets an object at index */
    public Object  getItem(int index){
        return m_choiceListModel.getElementAt(index) ;
    }

    /** get the number of items */
    public int getItemCount(){
        return m_choiceListModel.getSize() ;
    }

    /** gets the selected item */
    public synchronized Object getSelectedItem(){
        Object obj = null;
        int index = getSelectedIndex();
        if( index != -1 ){
            if( m_choiceListModel.getSize() > 0 )
                obj =  m_choiceListModel.getElementAt(getSelectedIndex());
        }
        return obj;
    }
    /*---------------end of accessor methods------------------*/

    //begin of setter methods
    //both list model need to be updated

    /** add an item to PChoice */
    public synchronized void addItem(Object item){
        m_choiceListModel.addElement(item) ;
        m_choiceSimpleListForm.addElement( item );
    }

    /** inserts the item at the given index*/
    public synchronized void insertItemAt(Object item, int index){
        m_choiceListModel.insertElementAt(item, index) ;
        m_choiceSimpleListForm.insertElementAt( item, index );
    }


    /** removes the item from the list */
    public synchronized void removeItem(Object item){
        m_choiceListModel.removeElement(item) ;
        m_choiceSimpleListForm.removeElement(item);
    }


    /** removes the item at the given position */
    public synchronized void removeItemAt(int position){
        m_choiceListModel.removeElementAt(position) ;
        m_choiceSimpleListForm.removeElementAt(position);
    }


    /** removes all the elements */
    public synchronized void removeAll(){
        m_choiceListModel.removeAllElements() ;
        m_choiceSimpleListForm.removeAllElements();

    }

    /////METHODS DELEGATING TO m_choiceSimpleListForm

    /** sets the item renderer for list of choices */
    public void setItemRenderer(PItemRenderer renderer){
        m_choiceSimpleListForm.setItemRenderer(renderer) ;
    }

    public synchronized void setSelectedIndex(int iIndex ){
        m_choiceSimpleListForm.setSelectedIndex( iIndex );
    }


    /** gets the selected index */
    public int getSelectedIndex(){
        return m_choiceSimpleListForm.getSelectedIndex();
    }

    /** sets the help text for the list */
    public void setHelpText(String strHelpText){
        m_choiceSimpleListForm.setHelpText( strHelpText );
    }



    /**
     * Set the hint text to display when a user presses and holds down the
     * screen display button associated with this check box. The hint text
     * is centered and wrapped in the hint frame automatically.  Developers
     * can explictly force a line break by adding a &quot;\n&quot;
     * into their text.
     *
     * @param strHint New hint text or null to remove the hint text.
     */
    public void setHint(String strHint)
    {
        m_strHint = strHint ;
    }


    /**
     * Get the hint that displays when a user presses and holds down the
     * screen display button that is associated with the check box.
     *
     * @return Hint text.
     */
    public String getHint()
    {
        return m_strHint ;
    }


    /**
     * Add the specified item listener to this choice.
     * @param listener New list listener.
     */
    public void addListListener(PListListener listener)
    {
        if (m_vListListeners == null)
            m_vListListeners = new Vector() ;

        if (!m_vListListeners.contains(listener)) {
            m_vListListeners.addElement(listener) ;
        }
        m_choiceSimpleListForm.addListListener(listener);
    }


    /**
     * Remove the specified list listener from this choice.
     * @param listener list listener that is no longer
     * interested in receiving state change notifications.
     */
    public void removeListListener(PListListener listener)
    {
        m_vListListeners.removeElement(listener) ;
        m_choiceSimpleListForm.removeListListener(listener);
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Set the button id that is associated or leased to this component.
     *
     * @param iButtonId Button that the component should listen for or -1 to
     *        disassociate this checkbox from all screen display buttons.
     */
    public void setAssociatedButton(int iButtonId)
    {
        m_iAssociatedButtonId = iButtonId ;
    }


    /**
     * The actual art of drawing
     *
     * @deprecated This should not be exposed
     */
    public void paint(Graphics g)
    {
        final int DROPDOWN_WIDTH = 14 ;
        Rectangle rectBounds ;
        Rectangle rectTextArea ;
        Rectangle rectDropDownArea ;

        if (m_iTextFieldHeight == 0)
        {
            calculateDimensions(g) ;
        }

        // Determine desired height
        int iDesiredHeight = 14 ;

        // Calculate desired bounding rectangle

        Dimension dim = getSize() ;

        rectBounds = new Rectangle(0, 0, dim.width-1, dim.height) ;
        if (((m_iLabelAlignment & PLabel.ALIGN_NORTH) > 0) ||
                  ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) > 0)) {
            g.setFont(m_displayLabelFont) ;
        } else {
            g.setFont(m_displayFont) ;
        }

        // Display the Label
        if (m_strLabel != null)
        {
            int xOffset = GUIUtils.calcXOffset(m_strLabel, g, rectBounds, m_iLabelAlignment) + 1;
            int yOffset = GUIUtils.calcYOffset(m_strLabel, g, rectBounds, m_iLabelAlignment) ;

            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;

            if ((m_iLabelAlignment & PLabel.ALIGN_NORTH) == PLabel.ALIGN_NORTH){
                g.drawString(m_strLabel, xOffset, yOffset+2) ;
            }else{
                g.drawString(m_strLabel, xOffset, yOffset) ;
            }
        }

        int iDiff = (rectBounds.height - iDesiredHeight) / 2 ;

        if (iDiff > 0)
        {
            rectBounds.y += iDiff ;
            rectBounds.height -= (iDiff*2) ;
        }

        rectBounds.height = m_iTextFieldHeight;
        rectBounds.x = m_iTextFieldXOffset;
        rectBounds.y = m_iTextFieldYOffset;

        // Calculate Text Area (blows up if width < DROPDOWN_WIDTH)
        rectTextArea = new Rectangle(rectBounds) ;
        if (rectTextArea.width >= (DROPDOWN_WIDTH+1))
            rectTextArea.width -= (DROPDOWN_WIDTH+1) ;
        else
            rectTextArea.width = (DROPDOWN_WIDTH+1);

         // Calculate Drop Down Area
        rectDropDownArea = new Rectangle(rectBounds) ;
        rectDropDownArea.x =
            rectBounds.x + (rectBounds.width - DROPDOWN_WIDTH-1) ;
        rectDropDownArea.width = DROPDOWN_WIDTH-1;

         // Draw Text Stuff
        GUIUtils.draw3DFrame(g,
                rectTextArea,
                SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;

        Object obj = getSelectedItem();
        String strSelection = null;
        if( obj != null )
            strSelection = getSelectedItem().toString() ;

        if (strSelection != null)
        {
            g.setFont(SystemDefaults.getFont
                      (SystemDefaults.FONTID_DEFAULT_SMALL)) ;
            Color colorOld = g.getColor() ;
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
            rectTextArea.grow(-2, -1) ;

            int iSelX = GUIUtils.calcXOffset(strSelection,
                    g, rectTextArea, GUIUtils.ALIGN_WEST) ;
            int iSelY = GUIUtils.calcYOffset(strSelection,
                    g, rectTextArea, GUIUtils.ALIGN_CENTER) ;

            Rectangle rectClipOld = g.getClipBounds() ;
            g.setClip(rectTextArea) ;
            g.drawString(strSelection, iSelX, iSelY) ;
            g.setClip(rectClipOld) ;
            g.setColor(colorOld) ;
        }

        // Draw Drop down Stuff
        GUIUtils.draw3DFrame(g, rectDropDownArea,
             SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;

        int iImgX = GUIUtils.calcXImageOffset(s_imgDropDown, rectDropDownArea,
                GUIUtils.ALIGN_CENTER) ;
        int iImgY = GUIUtils.calcYImageOffset(s_imgDropDown, rectDropDownArea,
                GUIUtils.ALIGN_CENTER) ;

        g.drawImage(s_imgDropDown, iImgX, iImgY, this) ;
    }


    /**
     * Set a label for this choice control.  The label will be displayed using
     * the specified alignment relative to the choice box.  For example, an
     * alignment of (NORTH | WEST) will place the text above the choice box
     * in the upper left.
     *
     * @param strText the text that will be displayed in this control
     * @param iAlignment where/how the text should be aligned
     *
     * @see #getLabelAlignment
     * @see #getLabel
     */
    public void setLabel(String strLabel, int iAlignment)
    {
        m_strLabel = strLabel ;
        m_iLabelAlignment = iAlignment ;

        if ((iAlignment != (PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST)) && (iAlignment != PLabel.ALIGN_WEST)) {
            System.out.println("WARNING: PTextField::setLabel *currently* only supports NORTHWEST and WEST alignments") ;
            m_iLabelAlignment = (PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        }

        calculateDimensions(null) ;
        repaint() ;
    }

    /**
     * get the user label set for this choice control.  The user label is
     * displayed relative to the choice control by some alignment factor.
     *
     * @return label alignment id
     *
     * @see #setLabel
     * @see #getLabelAlignment
     */
    public String getLabel()
    {
        return m_strLabel ;
    }

     /**
     * Get the label alignment for this choice control's label.  The alignment
     * specifies how the label is aligned relatively to the choice control.
     *
     * @return the alignment id for the choice control's label
     *
     * @see #setLabel
     * @see #getLabel
     */
    public int getLabelAlignment()
    {
        return m_iLabelAlignment ;
    }

    ///////////////////////////////////////
    ////implementating methods of PFocusTraversable


    /**
     * Set the focus state of this a traversable component.  A component
     * is required to have focus before receiving keyboard and/or button
     * events.
     *
     * @param bSet boolean true to set focus otherwise false to give it up.
     */
    public void setFocus(boolean bSet)
    {

        if (m_bHasFocus != bSet) {

            PFocusEvent event = new PFocusEvent(this) ;
            m_bHasFocus = bSet ;
            if( m_vFocusListeners != null ){
                Enumeration enumListeners = m_vFocusListeners.elements() ;
                while (enumListeners.hasMoreElements()) {
                    PFocusListener listener = (PFocusListener) enumListeners.nextElement() ;
                    if (listener != null) {
                        if (bSet) {
                            listener.focusGained(event) ;
                        } else {
                            listener.focusLost(event) ;
                        }
                    }
                }
            }
        }
    }


    /**
     * Add a focus listener to this element. Focus listeners receive
     * notification whenever this component gains or loses input focus.
     * If the specified listener has already been added, the add should
     * be ignored.
     *
     * @param listener The PFocusListener that will receive focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public void addFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners == null)
            m_vFocusListeners = new Vector() ;

        if (!m_vFocusListeners.contains(listener)) {
            m_vFocusListeners.addElement(listener) ;
        }
    }


    /**
     * Remove a focus listener from this element.  If the specified focus
     * listener is not listening, the request should be ignored.
     *
     * @param listener The PFocusListenre that should stop receiving focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public void removeFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners != null)
            m_vFocusListeners.removeElement(listener) ;
    }

    /** utility method to print debug messages */
    protected static void debug(String str){
      System.out.println("-------------------"+str);
    }

   /**
    * if the list is not in focus,
    * shows the list when the button associated with this
    * control is pressed. Inside the list, if CANCEL
    * is pressed, the selected item is set to the one
    * that was previously set.
    */
    protected void showList(){
       if( getItemCount() > 0 ){
            m_bListInFocus = true;
            m_iLastSelectedIndex = getSelectedIndex();
            int iShowModal;
            if( (iShowModal = m_choiceSimpleListForm.showModal())
                      == SimpleListForm.CANCEL ){
                m_bListInFocus = false;
                if( m_iLastSelectedIndex != -1 ){
                    m_choiceSimpleListForm.setSelectedIndex( m_iLastSelectedIndex );
                }
            }else if( iShowModal == SimpleListForm.OK ){
              m_bListInFocus = false;
            }
        }
    }

     /**
     * calculates the dimensions for drawing this control.
     */
    protected void calculateDimensions(Graphics g)
    {
        Dimension dimSize = getSize() ;
        boolean   bCreatedG = false ;

        if (g == null)
        {
            g = getGraphics() ;
            bCreatedG = true ;
        }


        if (g != null)
        {
            /*
             * This text control automatically switches between fonts based
             * on the TextField/Label.  If a label is displayed above or below
             * the textfield then a smaller font is used.
             */

            if (((m_iLabelAlignment & PLabel.ALIGN_NORTH) > 0) || ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) > 0)) {
                g.setFont(m_displayLabelFont) ;
            } else {
                g.setFont(m_displayFont) ;
            }

            FontMetrics fm = g.getFontMetrics() ;
            if (fm != null) {
                int iCharWidth = fm.charWidth('O') ;

                // Calc width
                if ((m_iTextFieldLength == -1) || m_bAutoTextFieldLength) {
                    // Calculate max
                    m_iTextFieldLength = (dimSize.width-8) / iCharWidth ;
                    m_iTextFieldWidth = m_iTextFieldLength * iCharWidth + 4 ;
                } else {
                    m_iTextFieldWidth = (m_iTextFieldLength * iCharWidth) ;
                    // Use passed, but truncated if too large
                    while (m_iTextFieldWidth > (dimSize.width-8)) {
                        m_iTextFieldLength-- ;
                        m_iTextFieldWidth = (m_iTextFieldLength * iCharWidth) + 4 ;
                    }
                }

                // Calc Height
                m_iTextFieldHeight = fm.getAscent() + 2 ;

                m_iAlignXOffset = 0 ;
                m_iAlignYOffset = 0 ;

                // Now calculate drawing offsets
                m_iTextFieldXOffset = 2 ;

                if ((m_iLabelAlignment == PLabel.ALIGN_WEST)  && (m_strLabel != null)) {
                    int iLabelWidth = fm.stringWidth(m_strLabel) ;

                    m_iTextFieldXOffset += iLabelWidth ;
                    m_iTextFieldWidth -= iLabelWidth ;

                }

                // Adjust for label alignment
                if ((m_iLabelAlignment & PLabel.ALIGN_NORTH) == PLabel.ALIGN_NORTH) {
                    // Label is on top, align text field south
                    m_iTextFieldYOffset = (dimSize.height - m_iTextFieldHeight) + 1 ;

                } else if ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) == PLabel.ALIGN_SOUTH) {
                    // Label is on bottom, align text field north
                    m_iTextFieldYOffset = 1 ;
                } else {
                    // Label is centered, center text field
                    m_iTextFieldYOffset = ((dimSize.height - m_iTextFieldHeight) / 2)+ 1 ;
                }
            }

            if (bCreatedG)
            {
                g.dispose() ;
            }
        }
    }





//////////////////////////////////////////////////////////////////////////////
// Nested / Inner class
////

    /**
     * Listner to add for addition or removing of data
     * calls the "paint" method to update the control.
     */
    private class icListDataListener implements PListDataListener
    {
        public void intervalAdded(PListDataEvent e)
        {
            Graphics g = getGraphics();
            if( g!= null )
                paint(g );
        }

        public void intervalRemoved(PListDataEvent e)
        {
            Graphics g = getGraphics();
            if( g!= null )
                paint(g );
        }

        public void contentsChanged(PListDataEvent e)
        {
            Graphics g = getGraphics();
            if( g!= null )
                paint(g );
        }

    }


     /**
     * implementation of PActionListener to listen to
     * double click actions.
     */
    public class icCommandDispatcher implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                m_choiceSimpleListForm.onOk();
                m_bListInFocus = false;
            }
        }
    }


    /**
     * The button watcher is responsible for listening to button event and
     * managing hints and state changes.
     */
    private class icButtonWatcher implements  PButtonListener, PActionListener
    {

        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
      {
            if (m_bButtonDown)
                return ;

            if (event.getButtonID() == m_iAssociatedButtonId) {
                 if (!m_bHasFocus) {
                    setFocus(true) ;
                }

                m_bButtonDown = true ;
                Timer.getInstance().resetTimer
                    (Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000),
                     this, null) ;
                event.consume() ;
            }
        }


        /**
         * The specified button has been released
         */
       public void buttonUp(PButtonEvent event)
        {

            Timer.getInstance().removeTimers(this) ;

            if (m_bButtonDown) {
                m_bButtonDown = false ;
            }

            if (event.getButtonID() == m_iAssociatedButtonId) {
                 if (!m_bHasFocus) {
                    setFocus(true) ;
                }

                showList();
                event.consume() ;
            }
        }



        /** The specified button is being held down */
        public void buttonRepeat(PButtonEvent event) { }


        /**
         * this takes care of showing hint when the button
         * is down.
         */
        public void actionEvent(PActionEvent event)
        {

            if ((m_strHint != null) && (m_strHint.length() > 0)) {
                if (m_bButtonDown) {
                    Shell.getInstance().displayHint(m_strHint) ;
                    m_bButtonDown = false ;
                }
            }
        }

    }


}

















