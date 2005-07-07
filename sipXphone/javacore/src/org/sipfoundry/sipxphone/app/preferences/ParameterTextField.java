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


package org.sipfoundry.sipxphone.app.preferences ; 

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

 
public class ParameterTextField extends PTextField
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////  
    public final static String ACTION_BACKSPACE = "action_backspace" ;
    public final static String ACTION_FORWARD   = "action_forward" ;
    public final static String ACTION_BACKWARD  = "action_backward" ;
    public final static String ACTION_CLEAR     = "action_clear" ;
    public final static String ACTION_SHIFT     = "action_shift" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
        
    /** reference to command bar */
    protected PCommandBar m_bbCommand ;
        
//////////////////////////////////////////////////////////////////////////////
// Construction
////   
    public ParameterTextField(PCommandBar bbCommand) 
    {
        super() ;
        
        m_bbCommand = bbCommand ;
        m_bbCommand.addActionListener(new icTextFieldCommandDispatcher()) ;
            
        setAlphanumericMode(false) ;
    }
 
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    
        
    /**
    *
    */
    protected void onCaretForward()
    {
        if (hasFocus())
            moveCaretForward() ;
    }
        
        
    /**
    *
    */
    protected void onCaretBackward()
    {
        if (hasFocus())
            moveCaretBackward() ;
    }
        
        
    /**
    *
    
    
    */
    protected void onBackspace()
    {
        if (hasFocus())
            backspace() ;
    }   
        
    /**
    *
    */
    protected void onClear()
    {
        if (hasFocus())
            setText("") ;
    }

    /**
    *
    */
    protected void onShift()
    {
        this.setCapsLock(!this.getCapsLock());
    }

    /**
    *
    */
    protected void onButtonDown(PButtonEvent event)
    {   
        // Only do the * = . when in numeric mode...
        if ((!getAlphanumericMode()) && (event.getButtonID() == PButtonEvent.BID_STAR)) {
            insertCharacter('.') ;
            event.consume() ;
        }
        else {
            super.onButtonDown(event) ;
            event.consume() ;
        }
    }
    
    public String getText()
    {
        String strValue = super.getText() ;
        if (strValue == null) {
            strValue = "" ;
        }
        
        return strValue ;        
    }
    
    
    
    /**
    *
    */
    protected void onButtonUp(PButtonEvent event)
    {
       switch (event.getButtonID()) {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
            case PButtonEvent.BID_MUTE:
            case PButtonEvent.BID_POUND:    
                super.onButtonUp(event) ;  
                break ;
            case PButtonEvent.BID_STAR: 
                if (!getAlphanumericMode())
                    event.consume(); 
                else
                    super.onButtonUp(event) ;  
                break ;
        }                 
        
    }
    
    

//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////    
        
    public class icTextFieldCommandDispatcher implements PActionListener
    {
            
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_BACKSPACE)) {
                onBackspace() ;                
            }
            else if (event.getActionCommand().equals(ACTION_FORWARD)) {
                onCaretForward() ;                                
            }
            else if (event.getActionCommand().equals(ACTION_BACKWARD)) {
                onCaretBackward() ;                                
            }
            else if (event.getActionCommand().equals(ACTION_CLEAR)) {
                onClear() ;   
            }
            else if (event.getActionCommand().equals(ACTION_SHIFT)) {
                onShift() ;   
            }
        }        
    }          
}
