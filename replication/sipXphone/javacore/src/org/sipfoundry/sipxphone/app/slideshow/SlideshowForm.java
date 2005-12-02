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

 
package org.sipfoundry.sipxphone.app.slideshow ;

import java.awt.* ; 
import java.util.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;


/**
 *
 */
public class SlideshowForm extends PAbstractForm
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
//// 
    /** label that we will display everything in */
    protected PLabel m_label ;
    
    /** the list of a images in the slide show*/
    Image       m_imgList[] ;
    
    /** the current image id (one we are on/viewing) */
    int         m_iCurrIndex ;    
    
                            
//////////////////////////////////////////////////////////////////////////////
// Constructions
//// 
    /**
     *
     */
    public SlideshowForm(Application application)
    {
        super(application, "Slide Show") ;
        
        createComponents() ;
        layoutComponents() ;
                                   
        addButtonListener(new icButtonListener()) ;
        addFormListener(new icFormAdapter()) ;
        
        processProperityFile() ;
    }
    

    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of 
     */    
    public void onFocusGained(PForm formLosingFocus)
    {
        displayImage(m_imgList[0]) ;
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
    private void createComponents()
    {
        m_label = new PLabel() ;
    }
    
    
    /**
     *
     */
    public PBottomButtonBar getBottomButtonBar()
    {
        return null ; 
    }
    

    /**
     *
     */
    private void layoutComponents()
    {        
        setLayout(null) ;
        m_label.setBounds(0,0,160,160) ;
        add(m_label) ;
    }                       
    
    public void onNext() 
    {
        m_iCurrIndex = (m_iCurrIndex+1) % m_imgList.length ;
        displayImage(m_imgList[m_iCurrIndex]) ;
    }
    
    
    public void onPrevious()
    {
        if (m_iCurrIndex == 0)
            m_iCurrIndex = m_imgList.length - 1 ;
        else
            m_iCurrIndex = (m_iCurrIndex-1) % m_imgList.length ;   
        displayImage(m_imgList[m_iCurrIndex]) ;
    }
        
    public void displayImage(Image image)
    {
        if (image != null) {
            m_label.setImage(image) ;
            m_label.repaint() ;
        }
    }
    
    
    /** 
     * snag the slideshow information out of the passed property file
     */
    protected void processProperityFile()
    {
        int iNumImages = 0 ;
                        
        try {
            PropertyResourceBundle bundle = new PropertyResourceBundle(ClassLoader.getSystemResourceAsStream("slideshow.properties")) ;
            
            iNumImages = Integer.parseInt(bundle.getString("images")) ;
            
            m_imgList = new Image[iNumImages] ;        
            for (int i=0;i<iNumImages;i++) {            
                m_imgList[i] = Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(bundle.getString("image" + Integer.toString(i+1)))) ;
            }
        } catch (Exception e) {
            iNumImages = 0 ;
        }        
    }
    
                    
    
//////////////////////////////////////////////////////////////////////////////
// Inner classes
////    

    public class icButtonListener implements PButtonListener
    {
        public void buttonUp(PButtonEvent event) 
        {
            switch (event.getButtonID())
            {
                case PButtonEvent.BID_B1:
                    onPrevious() ;
                    break ;
                case PButtonEvent.BID_B2:
                    closeForm() ;
                    break ;
                case PButtonEvent.BID_B3:
                    onNext() ;                
                    break ;                
            }
            
            event.consume() ;
        }
        
        public void buttonDown(PButtonEvent event)
        {
            event.consume() ;
        }
        
        public void buttonRepeat(PButtonEvent event)
        {
            
        }                
    }
    
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
}
