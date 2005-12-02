/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPHeaderImpl.java#2 $
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

import javax.siplite.* ;
import java.util.StringTokenizer;


/**
 * The SIPHeader interface provides the means for forming headers to be added
 * to the basic SIP Message. The interface also provides the means for
 * retrieving the values associated with the headers,
 * if this information is required.
 *
 */
public class SIPHeaderImpl implements SIPHeader
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String m_type;
    private SIPValue[] m_values;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SIPHeaderImpl()
    {
    }

    public SIPHeaderImpl(String type, String value)
    {
        m_type = type;
        parseValue( type, value );
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


    /**
     * Retrieve the type of the header.
     * @return header type
     **/
    public java.lang.String getType(){
        return m_type;
    }


    /**
     * Sets the type of the header.
     * @param type header type
     * @throws AttributeSetException if the type of this header has
     * already been set.
     **/
    public void setType(java.lang.String type)
        throws AttributeSetException
    {
        m_type = type;
    }


    /**
     * retreive this header's values
     * @return values held by the header
     **/
    public SIPValue[] getValues()
    {
        return m_values;
    }


    /**
     * Set the values for the header
     * @param values new values for the header
     * @throws InvalidArgumentException if multi values are
     * specified but only one is permitted for the header type.
     */
    public void setValues(SIPValue[] values)
        throws InvalidArgumentException
    {
        m_values = values;
    }


    /**
     * Retrieves a string representation of the header
     */
    public String toString()
    {
        return (m_type+": "+getValuesAsString()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected String getValuesAsString()
    {
        String strRet = null;
        SIPValue[] sipValues = getValues();
        if( sipValues != null ){
            for( int i = 0; i<sipValues.length; i++){
                SIPValue sipValue = sipValues[i];
                if( sipValue != null ){
                    String strValueWithoutParams = sipValue.getValue();
                    SIPParameter[] parameters = sipValue.getParameters();
                    StringBuffer strParamsBuffer = new StringBuffer();
                    if( parameters != null ) {
                        for( int j = 0; j<parameters.length; j++ ){
                            SIPParameter sipParam =  parameters[j];
                            if( sipParam != null ){
                                strParamsBuffer.append( sipParam.getName() );
                                strParamsBuffer.append( "=" );
                                strParamsBuffer.append( sipParam.getValue() );
                                if( j != (parameters.length-1) ){
                                   strParamsBuffer.append(";");
                                }
                            }
                        }
                    }
                    String strValue = strValueWithoutParams;
                    if( (strValueWithoutParams != null ) && (strParamsBuffer.length() > 0) ){
                        strValue += ";"+strParamsBuffer.toString();
                    }
                    if( strValue != null ){
                        strRet = strValue;
                        if( (parameters != null) && (i != (parameters.length-1)) ){
                                strRet += (",");
                        }
                    }
                }

            }//end for

        }//end if
        return strRet;
    }

    /*
     * Section 20 of bis08 says:
     * The Contact, From, and To header fields contain a URI. If the URI
     * contains a comma, question mark or semicolon, the URI MUST be
     * enclosed in angle brackets (< and >). Any URI parameters are
     * contained within these brackets. If the URI is not enclosed in angle
     * brackets, any semicolon-delimited parameters are header-parameters,
     * not URI parameters.
     */
    private  void parseValue( String strHeaderName,  String strValueAndParams){
        SIPValue sipValue = new SIPValueImpl();
        //now parse the allvalue to value and parameters.
        //Contact: "Mr. Watson" <sip:watson@worcester.bell-telephone.com>
        //;q=0.7; expires=3600
        //Contact: is already extracted. Now we need to parse the rest of the string.
        //TO DO:
        //THIS PARSER DOESN"T PARSE "," for multiple values for now.
        String strValueWithoutParams = null;

        if( strHeaderName.equalsIgnoreCase("Contact") ||
            strHeaderName.equalsIgnoreCase("From") ||
            strHeaderName.equalsIgnoreCase("To")  ){

            int iUriEndIndex = strValueAndParams.indexOf('>');
            if( iUriEndIndex > 0 ){
                strValueWithoutParams = strValueAndParams.substring(0, iUriEndIndex+1);
                strValueAndParams = strValueAndParams.substring(iUriEndIndex + 1);
            }
        }
        StringTokenizer tokenizer = new StringTokenizer(strValueAndParams, ";");
        int numParams = tokenizer.countTokens()-1;
        if( (strValueWithoutParams == null) && (tokenizer.hasMoreTokens()) ){
            strValueWithoutParams = tokenizer.nextToken().trim();
        }
        if( numParams > 0 ){
            SIPParameter[] sipParameters = new SIPParameter[numParams+1];
            int i = 0;
            while( tokenizer.hasMoreTokens() ){
                String strParameter = tokenizer.nextToken();
                if( strParameter.trim().length() > 0){
                    SIPParameter sipParameter = new SIPParameterImpl();
                    int iParamNameEnd = strParameter.indexOf("=");
                    if( iParamNameEnd != -1 )
                    {
                        String strParamName = strParameter.substring(0, iParamNameEnd);
                        String strParamValue = strParameter.substring(iParamNameEnd + 1);
                        sipParameter.setParameter(strParamName, strParamValue);
                        sipParameters[i++] = sipParameter;
                    }
                }
            }
            sipValue.setParameters(sipParameters);
        }
        sipValue.setValue(strValueWithoutParams);
        m_values = new SIPValue[1];
        m_values[0] = sipValue;
    }

}// SIPHeaderImpl
