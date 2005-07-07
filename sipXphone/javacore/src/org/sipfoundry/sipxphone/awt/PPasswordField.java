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
import java.awt.event.* ;
import java.util.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.sys.* ;


public class PPasswordField extends PComponent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////        
    /** table holding rotation sequence for alphanumeric mode */
    protected static final char DEF_ALPHANUMERIC_KEYS[][] =
        {
            {'0', '@', '/', '.', ':', '-', '?', '!', ',', '\"', '\'', '&', '$', '(', ')', '%', '_', '>','<', ' '},
            {'1'},
            {'2','a','b','c'},
            {'3','d','e','f'},
            {'4','g','h','i'},
            {'5','j','k','l'},
            {'6','m','n','o'},
            {'7','p','q','r','s'},
            {'8','t','u','v'},
            {'9','w','x','y', 'z'},
            {'*'},
            {'#'},
        } ;
    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** font used to when displaying text */
    protected Font      m_displayFont       = new Font("sansserif", Font.PLAIN, 10) ;
    /** string current within the editor */
    protected String    m_strText           = "" ;
    /** the current caret position */
    protected int       m_iCaretPosition    = 0 ;

    /** are we in alpha numeric mode or simple numeric mode? */
    protected boolean   m_bAlphaNumericMode = false ;
    
    /** last keycode entered in alphanumeric mode */
    protected int       m_iLastKey          = -1 ;
    /** last alpha index number (used for rotation) */
    protected int       m_iLastAlphaIndex   = 0 ;
    
    /** timeout handler so that we advance the caret after xxx ms */
    protected icAlphanumericTimeout m_timeoutHandler = new icAlphanumericTimeout() ;
    

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     *
     */
    public PPasswordField()
    {
        this("") ;        
    }
    
        
    /**
     *
     */
    public PPasswordField(String strText)
    {
        m_strText = strText ;
        m_iCaretPosition = strText.length() ;        
        
        addButtonListener(new icButtonListener()) ;
        setBackgroundImage(getImage("imgEditFrame")) ;
    }
        
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * set text / initial text of the component
     */
    public void setText(String strText)
    {
        m_strText = strText ;
        m_iCaretPosition = strText.length() ;
        repaint() ;
    }        
     
        
    /**
     * get the text as entered
     */
    public String getText()
    {
        return m_strText ;
    }    
    
        
    /**
     * sets the position of the caret within the editor
     * 
     * @param iPosition the new position of the caret
     */
    public void setCaretPosition(int iPosition) 
    {
        int iOldPosition = m_iCaretPosition ;
        
        m_iCaretPosition = iPosition ;
        
        // Clip along lower bound
        if (m_iCaretPosition < 0)
            m_iCaretPosition = 0 ;
            
        // Clip along upper bound
        if (m_iCaretPosition > m_strText.length())
            m_iCaretPosition = m_strText.length() ;
            
        // Only redraw if needed
        if (m_iCaretPosition != iOldPosition) {
            repaint() ;
            Timer.getInstance().removeTimers(m_timeoutHandler) ;
        }
    }
        
    
    /**
     * @return the current index position of the caret
     */
    public int getCaretPosition()
    {
        return m_iCaretPosition ;        
    }
    
    
    /**
     * delete the character at the current caret position
     */
    public void delete()
    {
        // Make sure we are within bounds
        if ((m_iCaretPosition >= 0) && (m_iCaretPosition < m_strText.length())) {
            m_strText = m_strText.substring(0, m_iCaretPosition) + m_strText.substring(m_iCaretPosition+1) ;
            m_iLastKey = -1 ;
        }
        
    }
    
    /**
     * backspace one character at the current caret position
     */
    public void backspace()
    {
        // Make sure we are within bounds
        if ((m_iCaretPosition > 0) && (m_iCaretPosition <= m_strText.length())) {
            m_strText = m_strText.substring(0, m_iCaretPosition-1) + m_strText.substring(m_iCaretPosition) ;
            moveCaretBackward() ;
        }

    }
    

    
    /** 
     * Move the caret position one knock forward
     */
    public void moveCaretForward()
    {
        setCaretPosition(getCaretPosition()+1) ;
        m_iLastKey = -1 ;
    }
    

    /** 
     * Move the caret position one knock backwards
     */
    public void moveCaretBackward()
    {
        setCaretPosition(getCaretPosition()-1) ;
        m_iLastKey = -1 ;
    }
    
    
    /** 
     * Display a graphic view of this object
     */
    public void paint(Graphics g)
    {        
        // Draw any background images we have
        if (m_imageBackground != null) {
            g.drawImage(m_imageBackground, 0, 0, this) ;
        }        
        
        Dimension dim = getSize() ;
        
        g.setFont(m_displayFont) ;
        FontMetrics fm = g.getFontMetrics(m_displayFont) ;            
        if (fm != null) {                                                
            // Draw the current string candidate
            String strText = "" ;
            for (int i = 0; i<m_strText.length(); i = i+1)
                strText = strText + "*" ;
            g.drawString(strText, 5, 10) ;
            //g.drawString(m_strText, 5, 10) ;
            
            
            // Draw the caret: let's make a distinction between numeric mode 
            // and alphanumeric mode by the shape of the cursor.
            if (m_bAlphaNumericMode) {
                // Alpha Numeric Mode will use a horizontal caret
                int iCaretStartLocation = 0 ;
                int iCaretLength = 0 ;
                
                // Start Position is easy
                iCaretStartLocation = fm.stringWidth(m_strText.substring(0, m_iCaretPosition)) ;                
                // End Position is a bit harder if we are at the last character
                if (m_iCaretPosition < m_strText.length()) {
                    iCaretLength = fm.stringWidth(m_strText.substring(m_iCaretPosition, m_iCaretPosition+1)) ;    
                } else {
                    iCaretLength = fm.charWidth('0') ;    // use a random character since we don't have a average width handy
                }
                
                g.drawLine(iCaretStartLocation+4, 12, iCaretStartLocation+4+iCaretLength, 12) ;                            
            } else {
                // Numeric Mode / Insertion Mode will use a vertical caret
                int iCaretLocation = fm.stringWidth(strText.substring(0, m_iCaretPosition)) ;            
                g.drawLine(iCaretLocation+4, 12, iCaretLocation+4, 2) ;
            }            
        }
    }
    
    
    public void shift()
    {
    }    


    /**
     * Set Alphanumeric mode
     *
     * @param bSet true to set alphanumeric mode, false to disable it.
     */
    public void setAlphanumericMode(boolean bSet)
    {
        // Move caret forward if turning off alphanumeric mode
        if (m_bAlphaNumericMode && !bSet)
            moveCaretForward() ;
            
        m_bAlphaNumericMode = bSet ;
        m_iLastKey = -1 ;
        repaint() ;
    }
    
    
    /**
     * @return true if we are in alphanumeric mode or false for numeric mode
     */
    public boolean getAlphanumericMode()
    {
        return m_bAlphaNumericMode ;
    }
               

//////////////////////////////////////////////////////////////////////////////
// Implementation
//// 
    /**
     * insert a character at the caret position (and advanced the caret position one notch)
     */
    protected void insertCharacter(char ch)
    {
        m_strText = m_strText.substring(0, m_iCaretPosition) + ch +  m_strText.substring(m_iCaretPosition) ;
        if (!m_bAlphaNumericMode)
            moveCaretForward() ;
        else
            repaint() ;
    }
    
    
    /** 
     * Replace the next character (or insert and set caret just before it)
     */
    protected void replaceCharacter(char ch)
    {        
        // Should we add a character to the end?        
        if (m_iCaretPosition >= m_strText.length()) {
            m_strText += ch ;
        }
        else {
            byte b[] = m_strText.getBytes() ;
            b[m_iCaretPosition] = (byte) ch ;
            m_strText = new String(b) ; 
        }
        repaint() ;
    }
    
    
   
        
    /**
     * 
     */
    protected  void newAlphanumeric(int iRow, boolean bAdvanceCaret)
    {
        // Reset to index 1 (if possible) - first letter instead of digit
        m_iLastAlphaIndex = (1) % DEF_ALPHANUMERIC_KEYS[iRow].length ;
        
        if (bAdvanceCaret) {
            moveCaretForward() ;
        }        
        insertCharacter(DEF_ALPHANUMERIC_KEYS[iRow][m_iLastAlphaIndex]) ;
        // Timer.getInstance().resetTimer(Settings.getInt("ALPHANUMERIC_TIMEOUT_LENGTH", 1000), m_timeoutHandler, null) ;
    }    
    
    /**
     *
     */
    protected void rotateAlphanumeric(int iRow)
    {
    }
    
    
    /**
     * handles the alphanumeric mode you entered a key and it has been xxx seconds, so
     * time out.
     */
    public class icAlphanumericTimeout implements PActionListener
    {
        public void actionEvent(PActionEvent event) {
            moveCaretForward() ;                        
        }                
    }
    
//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////

    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */    
        public void buttonDown(PButtonEvent event)
        {            
            boolean bJumpedKey = ((m_iLastKey != -1) && (m_iLastKey != event.getButtonID())) ; 
            boolean bNewKey = (m_iLastKey == -1) || (m_iLastKey != event.getButtonID()) ;

            switch (event.getButtonID()) {
                case PButtonEvent.BID_0:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(0, bJumpedKey) ;                        
                        else
                            rotateAlphanumeric(0) ;                                                    
                    }
                    else {    
                        insertCharacter('0') ;
                    }
                        
                    break ;
                case PButtonEvent.BID_1:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(1, bJumpedKey) ;
                        else
                            rotateAlphanumeric(1) ;                                                    
                    }
                    else {    
                        insertCharacter('1') ;
                    }                        
                    break ;
                case PButtonEvent.BID_2:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(2, bJumpedKey) ;
                        else
                            rotateAlphanumeric(2) ;
                    }
                    else {                    
                        insertCharacter('2') ;
                    }
                    break ;
                case PButtonEvent.BID_3:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(3, bJumpedKey) ;
                        else
                            rotateAlphanumeric(3) ;
                    }
                    else {                    
                        insertCharacter('3') ;
                    }
                    break ;
                case PButtonEvent.BID_4:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(4, bJumpedKey) ;
                        else
                            rotateAlphanumeric(4) ;
                    }
                    else {                    
                        insertCharacter('4') ;
                    }
                    break ;
                case PButtonEvent.BID_5:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(5, bJumpedKey) ;
                        else
                            rotateAlphanumeric(5) ;
                    }
                    else {                    
                        insertCharacter('5') ;
                    }
                    break ;
                case PButtonEvent.BID_6:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(6, bJumpedKey) ;
                        else
                            rotateAlphanumeric(6) ;
                    }
                    else {                    
                        insertCharacter('6') ;
                    }
                    break ;
                case PButtonEvent.BID_7: 
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(7, bJumpedKey) ;
                        else
                            rotateAlphanumeric(7) ;
                    }
                    else {                    
                        insertCharacter('7') ;
                    }
                    break ;
                case PButtonEvent.BID_8:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(8, bJumpedKey) ;
                        else
                            rotateAlphanumeric(8) ;
                    }
                    else {                    
                        insertCharacter('8') ;
                    }
                    break ;
                case PButtonEvent.BID_9:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(9, bJumpedKey) ;
                        else
                            rotateAlphanumeric(9) ;
                    }
                    else {                    
                        insertCharacter('9') ;
                    }
                    break ;
                case PButtonEvent.BID_STAR:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(10, bJumpedKey) ;
                        else
                            rotateAlphanumeric(10) ;
                    }
                    else {                    
                        insertCharacter('*') ;
                    }
                    break ;
                case PButtonEvent.BID_POUND:
                    if (m_bAlphaNumericMode) {
                        if (bNewKey)
                            newAlphanumeric(11, bJumpedKey) ;
                        else
                            rotateAlphanumeric(11) ;
                    }
                    else {                    
                        insertCharacter('#') ;
                    }
                    break ;                
            }
            
            // Store 'last key' if we are in alpha numeric mode
            if (m_bAlphaNumericMode) {
                m_iLastKey = event.getButtonID() ;                
            }        
        }        


        /**
         * The specified button has been released
         */        
        public void buttonUp(PButtonEvent event)
        {            
            Timer.getInstance().resetTimer(Settings.getInt("ALPHANUMERIC_TIMEOUT_LENGTH", 1500), m_timeoutHandler, null) ;
        }        
        

        /**
         * The specified button is being held down
         */            
        public void buttonRepeat(PButtonEvent event)
        {            
        }        
    }
}
