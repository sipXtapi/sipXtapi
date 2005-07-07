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

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.util.* ;


/**
 * The TaskContextMenu is the context/application specific menu list.  This is
 * displayed in one of the tabs within the task manager.
 *
 * @author Robert J. Andreasen. Jr.
 */
public class TaskContextMenu extends PMultiColumnList implements PListListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the parent form */
    private PForm  m_formParent ;
    private PLabel m_lblNoMenu ; 


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create the tab form given the specified parent frame
     */        
    public TaskContextMenu(PForm formParent)
    {
        m_formParent = formParent ;
        addListListener(this) ;
        m_lblNoMenu = null ;
        
        // Initialized the left and right list renderers
        setItemRenderer(COL1, new icLeftMenuRenderer()) ;
        setItemRenderer(COL2, new icRightMenuRenderer()) ;        
    }
           
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Populate the menu given the specified 'active form'
     */         
    public void populateMenu(PForm activeForm)
    {                
        if (m_lblNoMenu != null) {
            remove(m_lblNoMenu) ;
            m_lblNoMenu = null ;
        }
        
        removeAllItems() ;
        
        if (activeForm != null) {
            PActionItem[] itemsLeft = activeForm.getLeftMenu() ;
            PActionItem[] itemsRight = activeForm.getRightMenu() ;
            
            if (((itemsLeft == null) || (itemsLeft.length == 0)) &&
                    ((itemsRight == null) || (itemsRight.length == 0))) {
                Dimension dimSize = getSize() ;
                        
                m_lblNoMenu = new PLabel("No menu choices are available") ;
                add(m_lblNoMenu) ;
                m_lblNoMenu.setBounds(0, 0, dimSize.width, dimSize.height) ;
                m_lblNoMenu.doLayout() ;                                        
            } else {                                                                            
                int iLength ;
                int i ;
                    
                // Populate left side
                if (itemsLeft != null) {                    
                    iLength = itemsLeft.length ;
                    for (i=0; i<iLength; i++) {
                        if (itemsLeft[i] != null)
                            addElement(COL1, itemsLeft[i], itemsLeft[i].getHint()) ;
                    }
                }

                // Populate right side
                if (itemsRight != null) {
                    iLength = itemsRight.length ;
                    for (i=0; i<iLength; i++) {
                        if (itemsRight[i] != null)
                            addElement(COL2, itemsRight[i], itemsRight[i].getHint()) ;
                    }
                }
            }
        }
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
    
        
        
    /**
     * Invoked by the framework when an item within the list is selected.
     * <br>
     * NOTE: Exposed as a side effect of the ListListener
     */         
    public void selectChanged(PListEvent event)
    {
        if (event.getSelectedRow() != -1) {
            PActionItem item = (PActionItem) getElement(event.getSelectedColumn(), event.getSelectedRow()) ;
                
            if ((item != null) && item.isEnabled()) {
                PActionListener listener = item.getActionListener() ;
                String          strCommand = item.getActionCommand() ;
                    
                // We have a listener- stuff the event down it's throat.
                if (listener != null) {
                    ((PAbstractForm) m_formParent).closeForm() ;
                    PActionEvent actionEvent = new PActionEvent(this, strCommand) ;
                    
                    // Service the event on another context, otherwise we 
                    // could have deadlock if this menu action brings up 
                    // another form which tries to bring up the task manager.
                    new icMenuHandlerContext(listener, actionEvent).start() ;
                } else {
                    MessageBox alert = new MessageBox(m_formParent.getApplication(), MessageBox.TYPE_ERROR) ;                        
                    alert.setMessage("Cannot dispatch command with a null listener") ;
                    alert.showModal() ;
                }
            }
        }
    }                
     
        
//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    /** 
     * Thread context that delievers the menu event to the assigned listener
     */
    private class icMenuHandlerContext extends Thread
    {
        PActionListener m_listener ;
        PActionEvent    m_event ;
        
        public icMenuHandlerContext(PActionListener listener, PActionEvent event)
        {
            super("MenuHandler") ;
            
            m_listener = listener ;
            m_event = event ;                        
        }
        
        public void run()
        {
            m_listener.actionEvent(m_event) ;            
        }        
    }

    /**
     * Renderer that simply returns a label with left justified text
     */
    private class icLeftMenuRenderer implements PItemRenderer
    {
        public Component getComponent(Object objSource, Object objValue, boolean bSelected)
        {
            PLabel label ; 
            if (objValue != null) {
                PActionItem item = (PActionItem) objValue  ;
                
                label = item.getLabel().makeClone() ;
                label.setAlignment(PLabel.ALIGN_WEST) ;                    
                if (item.isEnabled()) {
                    label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
                } else {
                    label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_DISABLED)) ;
                }
            } else {
                label = new PLabel("") ;
            }
                
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_MENU_ENTRY)) ;
                
            return label ;
        }
    }        
    
    /**
     * Renderer that simply returns a label with right justified text
     */
    private class icRightMenuRenderer implements PItemRenderer
    {
        public Component getComponent(Object objSource, Object objValue, boolean bSelected)
        {
            PLabel label = null ;
            if (objValue != null) {
                PActionItem item = (PActionItem) objValue  ;
                
                label = item.getLabel().makeClone() ;
                label.setAlignment(PLabel.ALIGN_EAST) ;                    
                if (item.isEnabled()) {
                    label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;                    
                } else {
                    label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_DISABLED)) ;
                }
                
            } else {
                label = new PLabel("") ;
            }
                
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_MENU_ENTRY)) ;
                
            return label ;
        }
    }        
}