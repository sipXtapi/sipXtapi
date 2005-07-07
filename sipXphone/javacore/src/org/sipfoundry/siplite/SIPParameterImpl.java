/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPParameterImpl.java#2 $
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

import javax.siplite.SIPParameter ;


/**
 * The SIPParameter interface provides the means for adding additional information
 * in the form of name value pairs to the basic SIPValue. The interface also
 * provides the means for retrieving the names and values.
 *
 */
public class SIPParameterImpl implements SIPParameter
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String m_name;
    private String m_value;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SIPParameterImpl()
    {
    }

    public SIPParameterImpl(String name, String value)
    {
        setName(name) ;
        setValue(value) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * retrieve the name portion of the parameter
     * @return name part of the parameter
     **/
    public java.lang.String getName()
    {
        return m_name;
    }

    /**
     * retrieve the value portion of the parameter
     * @return string value of the parameter
     **/
    public java.lang.String getValue()
    {
        return m_value;
    }


    /**
     * Set the name and value pairing of a paramter
     * @param name parameter name, lefthand side of equals
     * @param value parameter value, righthand side of equals
     **/
    public void setParameter(java.lang.String name, java.lang.String value)
    {
        m_name = name;
        m_value = value;
    }


    /**
     * Set the name part of the parameter
     * @param name parameter name
     **/
    public void setName(java.lang.String name)
    {
        m_name = name;
    }


    /**
     * Set the value part of the parameter
     * @param value parameter vale
     **/
    public  void setValue(java.lang.String value)
    {
        m_value = value;
    }


}// SIPParameterImpl
