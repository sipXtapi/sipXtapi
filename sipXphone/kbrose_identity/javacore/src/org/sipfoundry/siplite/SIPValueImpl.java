/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPValueImpl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
package org.sipfoundry.siplite;

import javax.siplite.*;


/**
 * The SIPValue interface provides the means for adding values to SIP headers and
 * associating parameters with a value.
 * The interface also provides the means for retrieving the value
 * and any parameters, if this information is required.
 *
 */
public class SIPValueImpl implements SIPValue
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Object m_value;
    private SIPParameter[] m_parameters;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * retrieve the value string
     * return string value
     **/
    public java.lang.String getValue()
    {
        String strValue = null;
        if( m_value != null )
            strValue = m_value.toString();
        return strValue;
    }


    /**
     * Sets the SIPValue based on the supplied object,
     * throws an IllegalArgumentException if the object
     * type is not one recognised by the implementation.
     * It is required that all implemenations recognise at least
     * these types: java.lang.String, javax.siplite.Address and
     * java.util.Calendar
     * @param value new value for the SIPValue
     * @throws IllegalArgumentException if the object is not of a type
     * recognised by the implementation
     **/
    public void setValue(java.lang.Object value)
                  throws java.lang.IllegalArgumentException
    {
        m_value = value;
    }


    /**
     * retreive parameters for a SIPValue
     * @return array of SIPParameter
     **/
    public SIPParameter[] getParameters()
    {
        return m_parameters;
    }


    /**
     * set the parameters for a SIPValue
     * @param parameters held by this SIPValue
     **/
    public void setParameters(SIPParameter[] parameters)
    {
        m_parameters = parameters;
    }

}// SIPValueImpl
