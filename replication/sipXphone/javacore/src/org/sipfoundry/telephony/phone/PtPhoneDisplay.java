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

package org.sipfoundry.telephony.phone ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.sipxphone.service.* ;

public class PtPhoneDisplay extends PtComponent implements PhoneDisplay
{
//////////////////////////////////////////////////////////////////////////////
// Constants
//// 

//////////////////////////////////////////////////////////////////////////////
// Attributes
//// 
    private String m_strInfo = null ;
   
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use getComponent() from
     * the terminal to gain a reference to a component.
     *
     * @deprecated do not expose
     */
    public PtPhoneDisplay(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Please see the Java Telephony API version 1.3 documentation.  
     */
    public String getDisplay(int x, int y) 
    {
        return null ;    
    }
    
    
    /**
     * Please see the Java Telephony API version 1.3 documentation.  
     */
    public int getDisplayColumns() 
    {
        return 0 ; 
    }

          
    /**
     * Please see the Java Telephony API version 1.3 documentation.  
     */
    public int getDisplayRows() 
    {
        return 0 ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.  
     */
    public void setDisplay(java.lang.String string, int x, int y) 
    {
        
    }
    
    
    /**
     * Set the contrast level of the display.  This value must be between
     * <i>getContrastLow</i> and <i>getContrastHigh</i>.
     *
     * @param level Desired contrast level.
     *
     * @exception IllegalArgumentException Thrown if the passed level is 
     *            invalid.
     */
	public void setContrast(int level)
	    throws IllegalArgumentException
	{
	    if ((level < getContrastLow()) || (level > getContrastHigh())) {
	        throw new IllegalArgumentException("invalid level passed to set contrast") ;	        
	    }
	    
        JNI_setContrast(m_lHandle, level) ;    
	}

	
    /**
     * Get the contrast level of the display.
     *
     * @return LCD contrast level
     */
	public int getContrast()
	{
	    return JNI_getContrast(m_lHandle) ;
	}

	
    /**
     * Get the minimum contrast level supported by this hardware platform.
     */
	public int getContrastLow()
	{
	    return JNI_getContrastLow(m_lHandle) ;
	}
		

    /**
     * Get the maximum contrast level supported by this hardware platform.
     */
	public int getContrastHigh()
	{
	    return JNI_getContrastHigh(m_lHandle) ;
	}


    /**
     * Get the default/nominal contrast level for this hardware platform.
     */
	public int getContrastNominal()
	{
	    return JNI_getContrastNominal(m_lHandle) ;
	}
		

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native final static void JNI_setContrast(long lHandle, int level) ;
    protected native final static int  JNI_getContrast(long lHandle) ;
    protected native final static int  JNI_getContrastLow(long lHandle) ;
    protected native final static int  JNI_getContrastHigh(long lHandle) ;
    protected native final static int  JNI_getContrastNominal(long lHandle) ;
}
