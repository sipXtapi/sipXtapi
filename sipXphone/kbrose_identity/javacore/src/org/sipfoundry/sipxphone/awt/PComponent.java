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

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;

import java.awt.* ;
import java.util.* ; 


/**
 *
 *
 */
public class PComponent extends Component
{   
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** is this component opaque?  (is it not transparent?) */
    protected boolean m_bOpaque = false ;
    /** should this component use double buffering on repaints? */
    protected boolean m_bDoubleBuffer = false ;
    /** an image that is simply tossed into the background */
    protected Image   m_imageBackground = null ;    
    /** offsets from edges (insets) */
    protected Insets  m_insets = new Insets(0,0,0,0) ;
    
    /** every component can have listeners attached to it */
    protected Vector  m_vButtonListeners = null ;
      
    
  
    
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    public PComponent()
    {                
        setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
        setOpaque(false) ;
    }
    
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * set whether or not this image is opaque (not transparent)
     *
     * @param isOpaque true for opaque false for transparent
     */
    public void setOpaque(boolean isOpaque)
    {
        m_bOpaque = isOpaque ;        
    }
    
    
    /**
     * @return true if opaque false of transparent
     */
    public boolean isOpaque()
    {
        return m_bOpaque ;   
    }
    
    
    /**
     * Enable or disable double buffering.  Double buffering will draw to a
     * off screen display context and then blt that display context into
     * the real display context in a single operation.  This is used to avoid
     * flashing.
     *
     * @param bEnable true to enable or false to disable double buffering
     */    
    public void enableDoubleBuffering(boolean bEnable)
    {
        // m_bDoubleBuffer = bEnable ;        
    }
    
        
    /**
     * Is double buffering enabled on this component?
     */
    public boolean isDoubleBufferingEnabled()
    {
        return m_bDoubleBuffer ;
    }

        
    /**
     * set a backgrond image for the component.  This will override and
     * opaque settings.
     */
    public void setBackgroundImage(Image image)
    {
        m_imageBackground = image ;
    }
        

    /**
     * lookup a string from the resource manager.  This is added as a helper
     * and calles AppResourceManager directly.
     */
    public String getString(String strKey)
    {
        return AppResourceManager.getInstance().getString(strKey) ;    
    }
    
    
    /**
     * lookup an image from the resource manager.  This is added as a helper
     * and calles AppResourceManager directly.
     */
    public Image getImage(String strKey)
    {
        return AppResourceManager.getInstance().getImage(strKey) ;    
    }
    
        
//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * @deprecated do not expose
     */
    public void repaint()
    {
        if (m_bDoubleBuffer) {
            Dimension dimSize = getSize() ;
            Color     colorBackground = null ;            
            
            // Kick out if the world doesn't seem just right
            if ((dimSize.width <= 0) || (dimSize.height <= 0)) {
                return ;
            }
            
            Graphics  g = getGraphics() ;
            if (g != null) {                                    
                Image imgOffscreen = createImage(dimSize.width, dimSize.height) ;
                Graphics og = imgOffscreen.getGraphics() ;
                og.setFont(getFont()) ;
                paint(og) ;
                            
                if (m_bOpaque) 
                    colorBackground = getBackground() ;
                        
                g.drawImage(imgOffscreen, 0, 0, colorBackground, null) ;
                
                // Clean Up
                og.dispose() ;
                imgOffscreen.flush() ;                
                g.dispose() ;
            }            
        } else {
            super.repaint() ;   
        }
    }

    
/*   
    public void update(Graphics g) 
    {
        Component comp = getParent() ;
        if (comp != null) {           
            Dimension dimParent = comp.getSize() ;
            if ((dimParent.width <= 0) || (dimParent.height <= 0))
                return ;
                
            Graphics parentG = comp.getGraphics() ;
            Rectangle bounds = getBounds() ;
            Image imgOffscreen = comp.createImage(dimParent.width, dimParent.height) ;
            Graphics og = imgOffscreen.getGraphics() ;            
            og.setClip(bounds.x, bounds.y, bounds.width, bounds.height) ;            
            comp.update(og) ;
            
            g.drawImage(imgOffscreen, bounds.x, bounds.y, bounds.width, bounds.height, null) ;
            
            og.dispose() ;
            parentG.dispose() ;            
        }
    }
*/


    /**
     * called by the framework when it is time to paint a view of this
     * component.  This will draw a background if/needed/set and draw
     * and background images before calling it's super.
     */
    public void paint(Graphics g)
    {        
        // Draw any background images we have
        if (m_imageBackground != null) {
            g.drawImage(m_imageBackground, 0, 0, this) ;
        }
        // if we are opaque, fill the background
        else if (m_bOpaque) {
            Color oldColor = g.getColor() ;
            Dimension dimSize = getSize() ;
            g.setColor(getBackground()) ;            
            g.fillRect(0, 0, dimSize.width, dimSize.height) ;
            g.setColor(oldColor) ;
        }                
    }
    
    
    /**
     * Add a button listener to this component.  Listeners will receive
     * notifications of button presses.
     */
    public void addButtonListener(PButtonListener l)
    {
        if (m_vButtonListeners == null) {
            m_vButtonListeners = new Vector() ;                                               
        }
        m_vButtonListeners.addElement(l) ; 
    }
    

    /**
     * Remove a button listener from this component.  The listener will
     * stop receiving notification of buttons presses.
     */
    public void removeButtonListener(PButtonListener l)
    {
        if (m_vButtonListeners != null) {
            m_vButtonListeners.removeElement(l) ;            
        }        
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * Process an event - A component is generally the leaf node and should
     * either act or ignore the event.
     */
    protected void processPingtelEvent(PButtonEvent event) 
    {        
        if (m_vButtonListeners != null) {
            fireButtonEvent(event) ;
        }
    }
    

    /**
     * fire off a button event to all of our listeners
     */    
    private void fireButtonEvent(PButtonEvent event) 
    {
        int iListeners = m_vButtonListeners.size() ;

        for (int i=0; i<iListeners; i++) {
            PButtonListener listener = (PButtonListener) m_vButtonListeners.elementAt(i) ;
                
            switch (event.getEventType()) {
                case PButtonEvent.KEY_UP:
                case PButtonEvent.BUTTON_UP:
                    listener.buttonUp(event) ;
                    break ;
                case PButtonEvent.KEY_DOWN:
                case PButtonEvent.BUTTON_DOWN:
                    listener.buttonDown(event) ;
                    break ;
                case PButtonEvent.BUTTON_REPEAT:
                    listener.buttonRepeat(event) ;
                    break ;
            }
                
            // Check for consumption
            if (event.isConsumed()) {
                break ;
            }
        }
    }
}
