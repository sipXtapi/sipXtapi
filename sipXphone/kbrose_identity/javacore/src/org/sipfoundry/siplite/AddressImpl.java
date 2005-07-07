/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/AddressImpl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.siplite ;

import java.util.Vector;

import javax.siplite.InvalidAddressException ;
import javax.siplite.InvalidArgumentException;
import javax.siplite.Address;
import javax.siplite.SIPParameter;

import org.sipfoundry.util.SipParser ;

/**
 * The Address is used to indicate the originator of a request, the
 * destination of a request and to specify any redirections of the request.
 * The Address interface encapsulates the username, the host, the port, extra
 * parameters and the display name by providing accessor methods for all.
 * Address also allows the scheme to be set i.e. "sip:" or "tel:". If a scheme
 * is not set, the default scheme, "sip:", will be used.
 *
 * An Address is created using the Factory. The Factory creates
 * an address with the default port of 5060. This can be changed if
 * neccessary by the
 * <code> setPort </code> method<br>
 *<br>
 * An example of an address is : <br>
 * <br>
 *     <code> A Jones &lt;ajones@somewhere.com:5060&gt; </code><br>
 *<br>
 * where<blockquote> username = ajones<br>
 * domain = somewhere.com<br>
 * display name = A Jones<br>
 * port = 5060</blockquote>
 *
 * Pingtel has added the following extensions:
 *     addURLParameter<br>
 *     addFieldParameter<br>
 *     addHeaderParameter<br>
 *<br>
 *     getURLParameterNames<br>
 *     getFieldParameterNames<br>
 *     getHeaderParameterNames<br>
 *<br>
 *     getURLParameterValue<br>
 *     getFieldParameterValue<br>
 *     getHeaderParameterValue<br>
 *<br>
 *     getURLParameterValues<br>
 *     getFieldParameterValues<br>
 *     getHeaderParameterValues<br>
 *<br>
 **/
public class AddressImpl implements Address
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Default SIP port */
    public static final int DEFAULT_PORT = 5060 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** underlying sip parser (implementation) */
    transient private SipParser m_parser ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor.
     */
    public AddressImpl()
    {
        m_parser = new SipParser() ;
        setPort(DEFAULT_PORT) ;
    }


    /**
     * Constructor accepting a non-parsed string representing the address.
     */
    public AddressImpl(String strAddress)
    {
        m_parser = new SipParser(strAddress) ;
        m_parser.reset(m_parser.render()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Parses the supplied SIP address to set up all the attributes of
     * an address.  If the SIP Address is malformed then an
     * InvalidArgumentException is thrown
     **/
     public void parseSIPAddress(java.lang.String sipAddress)
            throws InvalidArgumentException{
        m_parser = new SipParser(sipAddress) ;
        m_parser.reset(m_parser.render()) ;
     }

    /**
     * Set the user name of an address. An example of a user name would be
     * ajones
     * @param    userName    The username of an address
     **/
    public void setUserName(String username)
    {
        m_parser.setUser(username) ;
    }

    /**
     * Set the host of an address. An example of a host would be a fully-
     * qualified domain name e.g "somewhere.com" or numeric IP address e.g.
     * 111.111.111.11.
     * @param    host    The host of an address
     **/
    public void setHost(String host)
    {
        m_parser.setHost(host) ;
    }

    /**
     * Set the port of an address. An example of a port would be 5060
     * @param    port    The port of an address
     **/
    public void setPort(int port)
    {
        if ( port <= 0 )
            throw new IllegalArgumentException
                ("Port value cannot be set to less than or equal to zero");
        m_parser.setPort(port) ;
    }


    /**
     * Set the display name of an address. An example of a display name would
     * be A Jones
     * @param    displayName    The displayName of an address
     **/
    public void setDisplayName(String displayName)
    {
        m_parser.setDisplayName(displayName);
    }


    /**
     * Used to add a parameter to the URL. An example parameter could be
     * "user=phone"
     * @param   parameter   The parameter to be added
     * @throws InvalidArgumentException This is thrown when malformed Parameter
     *                                  is given,
     **/
    public void addParameter(String parameterAndValuePair)
      throws InvalidArgumentException
    {
        String[] paramNameAndValue = getNameAndValue( parameterAndValuePair );

        /*
        Quote from SIP RFC2543bis-06:
        Even though an arbitrary number of URI parameters may be
             included in a URI, any given parameter-name MUST NOT appear
             more than once.
        */
        if( ifURLParameterExists( paramNameAndValue[0] ) ){
            throw new InvalidArgumentException
                (" Parameter name " + paramNameAndValue[0]
                 + " is already added to this addess ");

        }
        addURLParameter(paramNameAndValue[0], paramNameAndValue[1]) ;

    }

    /**
     * Used to add a parameter to the URL. An example parameter could be
     * "user=phone"
     * @param   parameter   The parameter to be added
     * @throws InvalidArgumentException This is thrown when malformed Parameter
     *                                  is given,
     **/
    public void addParameter(SIPParameter sipParameter)
    {
        m_parser.addURLParameter
            ( sipParameter.getName(), sipParameter.getValue() );
    }


    /**
     * Used to add a parameter to the URL.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strParameter the parameter to be added
     * @param strValue the value to be added
     *
     * @throws InvalidArgumentException if the parameter or value is malformed
     **/
    public void addURLParameter(String strParameter, String strValue)
        throws InvalidArgumentException
    {
        // Validate params
        if ((strParameter == null) || (strValue == null))
        {
            throw new InvalidArgumentException() ;
        }

        m_parser.addURLParameter(strParameter, strValue) ;
    }


    /**
     * Used to add a header parameter to the address.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strParameter the parameter to be added
     * @param strValue the value to be added
     *
     * @throws InvalidArgumentException if the parameter or value is malformed
     **/
    public void addHeaderParameter(String strParameter, String strValue)
        throws InvalidArgumentException
    {
        // Validate params
        if ((strParameter == null) || (strValue == null))
        {
            throw new InvalidArgumentException() ;
        }

        m_parser.addHeaderParameter(strParameter, strValue) ;
    }


    /**
     * Used to add a field parameter to the address.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strParameter the parameter to be added
     * @param strValue the value to be added
     *
     * @throws InvalidArgumentException if the parameter or value is malformed
     **/
    public void addFieldParameter(String strParameter, String strValue)
        throws InvalidArgumentException
    {
        // Validate params
        if ((strParameter == null) || (strValue == null))
        {
            throw new InvalidArgumentException() ;
        }

        m_parser.addFieldParameter(strParameter, strValue) ;
    }


    /**
     * Sets the scheme of the address. i.e. sip or tel. If the scheme is not
     * supported then an Invalid Argument is thrown.
     * @param   scheme  The scheme (as a string) to use.
     * @throws InvalidArgumentException This is thrown when an unsupported
     *	scheme is given.
     */
    public void setScheme(String scheme)
        throws InvalidArgumentException
    {
        if ((scheme.equalsIgnoreCase("sip")) ||
                (scheme.equalsIgnoreCase("tel")))
        {
            m_parser.setScheme(scheme);
        }
        else
        {
            throw new InvalidArgumentException ("Invalid scheme") ;
        }
    }

    /**
     * Retrieve the user name of an address.
     * @returns    the user name of an address
     **/
    public String getUserName()
    {
        return m_parser.getUser() ;
    }

    /**
     * Retrieve the host of an address
     * @returns    the host of an address
     **/
    public String getHost()
    {
        return m_parser.getHost() ;
    }


    /**
     * Retrieve the port of an address
     * @returns    the port of an address
     **/
    public int getPort()
    {
        return m_parser.getPort() ;
    }


    /**
     * Retrieve the displayName of an address.
     * @returns    the displayName of an address
     **/
    public String getDisplayName()
    {
        return m_parser.getDisplayName() ;
    }


    /**
     * Retrieves the List of parameters of an address.
     * @return  The list of parameters
     */
    public javax.siplite.SIPParameter[] getParameters()
    {
        SIPParameter params[] ;
        String strParams[] = getURLParameters() ;

        if (strParams != null)
        {
            params = new SIPParameter[strParams.length] ;
            for (int i=0; i<strParams.length; i++)
            {
                int iParamNameEnd = strParams[i].indexOf("=") ;
                String strParamName = strParams[i].substring(0, iParamNameEnd) ;
                String strParamValue = strParams[i].substring(iParamNameEnd + 1) ;

                params[i] = new SIPParameterImpl(strParamName, strParamValue) ;
            }
        }
        else
            params = new SIPParameter[0] ;

        return params ;
    }

    /**
     * Retrieves the list of parameters for an address in the form:<br>
     *   name=value1<br>
     * Multiple values for the same parameter name are expanded into multiple
     * entries with the same name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @return  The list of parameters
     */
    public String[] getURLParameters()
    {
        Vector vParameters  = new Vector() ;
        String rc[] ;

        String params[] = m_parser.getURLParameters() ;
        for (int i=0; i<params.length; i++)
        {
            String values[] = m_parser.getURLParameterValues(params[i]) ;
            if (values.length == 0)
            {
                vParameters.addElement(params[i]) ;
            }
            else
            {
                for (int j=0; j<values.length; j++)
                {
                    vParameters.addElement(params[i] + "=" + values[j]) ;
                }
            }
        }

        rc = new String[vParameters.size()] ;
        vParameters.copyInto(rc) ;

        return rc ;
    }

    /**
     * Retrieves the list of parameters for an address in the form:<br>
     *   name=value1<br>
     * Multiple values for the same parameter name are expanded into multiple
     * entries with the same name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @return  The list of field parameters
     */
    public String[] getFieldParameters()
    {
        Vector vParameters  = new Vector() ;
        String rc[] ;

        String params[] = m_parser.getFieldParameters() ;
        for (int i=0; i<params.length; i++)
        {
            String values[] = m_parser.getFieldParameterValues(params[i]) ;
            if (values.length == 0)
            {
                vParameters.addElement(params[i]) ;

            }
            else
            {
                for (int j=0; j<values.length; j++)
                {
                    vParameters.addElement(params[i] + "=" + values[j]) ;
                }
            }
        }

        rc = new String[vParameters.size()] ;
        vParameters.copyInto(rc) ;

        return rc ;
    }

    /**
     * Retrieves the list of parameters for an address in the form:<br>
     *   name=value1<br>
     * Multiple values for the same parameter name are expanded into multiple
     * entries with the same name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @return  The list of header parameters
     */
    public String[] getHeaderParameters()
    {
        Vector vParameters  = new Vector() ;
        String rc[] ;

        String params[] = m_parser.getHeaderParameters() ;
        for (int i=0; i<params.length; i++)
        {
            String values[] = m_parser.getHeaderParameterValues(params[i]) ;
            if (values.length == 0)
            {
                vParameters.addElement(params[i]) ;

            }
            else
            {
                for (int j=0; j<values.length; j++)
                {
                    vParameters.addElement(params[i] + "=" + values[j]) ;
                }
            }
        }

        rc = new String[vParameters.size()] ;
        vParameters.copyInto(rc) ;

        return rc ;
    }


    /**
     * Gets the scheme of the address. i.e. sip or tel.
     * @return  The scheme as a string.
     */
    public String getScheme()
    {
        return m_parser.getScheme() ;
    }


    /**
     * Test to see if two addresses are equal. Two addresses are deemed to
     * be equal if the username, host and port are the same.
     **/
    public boolean equals(Address address)
    {
        /*
         * Test to see if two addresses are equal. Two addresses are deemed to
         * be equal if the username, host and port are the same.
         *
         * According to SIP RFC2543bis-06, under section SIP URI Comparison,
         * its stated that:
         * <br>SIP URIs are compared for equality according to the following rules:
         *
         *   <li> Comparison of the userpart of sip URIs is case-sensitive. This
         *     includes userparts containing passwords or formatted as
         *     telephone-subscribers. Comparison of all other components of
         *     the URI is case-insensitive unless explicitly defined
         *     otherwise.
         *
         *   <li> The ordering of parameters and headers is not significant in
         *     comparing SIP URIs.
         *
         *   <li> Characters other than those in the "reserved" and "unsafe"
         *     sets (see RFC 2396 [13]) are equivalent to their ""%" HEX HEX"
         *     encoding.
         *
         *   <li> An IP address that is the result of a DNS lookup of a host
         *     name does not match that host name.
         *
         *   <li> For two URIs to be equal, the user, password, host, and port
         *     components must match. A URI omitting the optional port
         *     component will match a URI explicitly declaring port 5060. A
         *     URI omitting the user component will not match a URI that
         *     includes one. A URI omitting the password component will not
         *     match a URI that includes one.
         *
         *   <li> URI uri-parameter components are compared as follows
         *
         *     <br>- Any uri-parameter appearing in both URIs must match.
         *     <br>- A user, transport, ttl, or method url-parameter appearing in
         *       only one URI must contain its default value or the URIs do
         *       not match.
         *     <br>  A URI that includes an maddr parameter will not match a URI
         *       that contains no maddr parameter.
         *     <br>- All other url-parameters appearing in only one URI are
         *       ignored when comparing the URIs.
         *
         *   <li> URI header components are never ignored. Any present header
         *     component MUST be present in both URIs and match for the URIs
         *     to match.
         */
        boolean bRC = true ;
        if (address != null)
        {

            //username is case sensitive.
            bRC = ( address.getUserName().equals(getUserName()) );


            if( bRC ){
                bRC =(safeStringCompare(getUserName(), address.getUserName())&&
                      safeStringCompare(getHost(), address.getHost()) &&
                      getPort() == address.getPort() ) ;
            }

            if( bRC )
                bRC = compareAddressForURLParameters( address );

            if( bRC )
                bRC = compareAddressForHeaderParameters( address );

            //field params check.
        }
        return bRC ;
    }


    /**
     * Retrieves the string rendered version of the address
     * <BR><BR>
     * PINGTEL EXTENSION
     */
    public String toString()
    {
        return m_parser.render() ;
    }

    /**
     * Retrieves the URL parameters names (no values) for this address.
     * <BR><BR>
     * PINGTEL EXTENSION
     */
    public String[] getURLParameterNames()
    {
        return m_parser.getURLParameters();
    }

    /**
     * Retrieves the header parameter names (no values) for this address.
     * <BR><BR>
     * PINGTEL EXTENSION
     */
    public String[] getHeaderParameterNames()
    {
        return m_parser.getHeaderParameters();
    }

    /**
     * Retrieves the field parameter names (no values) for this address.
     * <BR><BR>
     * PINGTEL EXTENSION
     */
    public String[] getFieldParameterNames()
    {
        return m_parser.getFieldParameters();
    }

    /**
     * Retrieve the first value for the designated URL parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String getURLParameterValue(String name)
    {
        return m_parser.getURLParameterValue(name);
    }

    /**
     * Retrieve the first value for the designated header parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String getHeaderParameterValue(String name)
    {
        return m_parser.getHeaderParameterValue(name);
    }

    /**
     * Retrieve the first value for the designated field parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String getFieldParameterValue(String name)
    {
        return m_parser.getFieldParameterValue(name);
    }


    /**
     * Retrieve the values for the designated URL parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String[] getURLParameterValues(String name)
    {
        return m_parser.getURLParameterValues(name);
    }

    /**
     * Retrieve the values for the designated header parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String[] getHeaderParameterValues(String name)
    {
        return m_parser.getHeaderParameterValues(name);
    }

    /**
     * Retrieve the values for the designated field parameter name.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param name The parameter name whose first value will be returned
     */
    public String[] getFieldParameterValues(String name)
    {
        return m_parser.getFieldParameterValues(name);
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * Safely determine if the two specified strings are identical.
     */
    private boolean safeStringCompare(String strSource, String strTarget)
    {
        if ((strSource == null) && (strTarget != null))
            return false ;

        if ((strSource != null) && (strTarget == null))
            return false ;

        if ((strSource == null) && (strTarget == null))
            return true ;
        else
            return strSource.equalsIgnoreCase(strTarget) ;
    }

    /**
     * Gives you an array of name and value from a string where name and
     * value are separated by an equal sign.
     * @param strNameValue  a string consisting of name and value
     *                     separated by "=" For example transport=UDP
     * @return String[] an array of two strings where the first one is name
     *      and the second one is value. If there is no value, the second one
     *      will be an empty String.

     */
    private String[] getNameAndValue( String parameter){
        String[] nameValueArray = new String[2];

        String strName;
        String strValue;

        int iEquals = parameter.indexOf("=") ;
        if (iEquals != -1)
        {
            strName = parameter.substring(0, iEquals) ;
            strValue = parameter.substring(iEquals+1) ;
        }
        else
        {
            strName = parameter ;
            strValue = "" ;
        }
        nameValueArray[0]= strName;
        nameValueArray[1]= strValue;
        return nameValueArray;
    }


    /**
     * checks if the url paramter with the name strParameterName exists.
     */
    private boolean ifURLParameterExists( String strParameterName){
        boolean bRC = false;
        String[] paramNames = getURLParameterNames();
        if( paramNames != null ){
            for( int i=0;  i< paramNames.length; i++ ){
                if( paramNames[i].equalsIgnoreCase(strParameterName)){
                    bRC = true;
                    break;
                }
            }
        }
        return bRC;
    }

     /**
     * checks if the header paramter with the name strParameterName exists.
     */
    private boolean ifHeaderParameterExists( String strParameterName ){
        boolean bRC = false;
        String[] paramNames = getHeaderParameterNames();
        if( paramNames != null ){
            for( int i=0;  i< paramNames.length; i++ ){
                if( paramNames[i].equalsIgnoreCase(strParameterName)){
                    bRC = true;
                    break;
                }
            }
        }
        return bRC;
    }

     /**
     * checks if the field paramter with the name strParameterName exists.
     */
    private boolean ifFieldParameterExists( String strParameterName ){
        boolean bRC = false;
        String[] paramNames = getFieldParameterNames();
        if( paramNames != null ){
            for( int i=0;  i< paramNames.length; i++ ){
                if( paramNames[i].equalsIgnoreCase(strParameterName)){
                    bRC = true;
                    break;
                }
            }
        }
        return bRC;
    }

    /**
     *gets the default value of some parameter names
     *as defined by SIP RFC 2543bis-06
     */
    private String getDefaultValue( String paramName ){
        String strRet = "";
        if( paramName.equalsIgnoreCase("user") ){
            strRet = "";
        }else if ( paramName.equalsIgnoreCase("transport") ){
            strRet = "UDP";
        }else if ( paramName.equalsIgnoreCase("ttl") ){
            strRet = "1";
        }else if ( paramName.equalsIgnoreCase("method") ){
            strRet = "INVITE";
        }
        return strRet;
    }


    /**
     *compares this address with the "address" to see
     *if they are "equal" as far as  url-parameters are
     *concerned.
     *
     * Basically confirms to the following from the SIP RFC2543bis-06 spec:
     *  URI uri-parameter components are compared as follows

          - Any uri-parameter appearing in both URIs must match.

          - A user, transport, ttl, or method url-parameter appearing in
            only one URI must contain its default value or the URIs do
            not match.

            A URI that includes an maddr parameter will not match a URI
            that contains no maddr parameter.

          - All other url-parameters appearing in only one URI are
            ignored when comparing the URIs.
    */
    private boolean compareAddressForURLParameters(Address address){

        boolean bRC = true;

        //Any uri-parameter appearing in both URIs must match.
        //one-directional check is enough for checking params
        //that appear in both
        //but two-directional check is necessary for checking
        //params that appear in only one.

        String[] urlParamNames = getURLParameterNames();
        for( int i =0; i< urlParamNames.length; i++ ){
            String paramName = urlParamNames[i];
            String paramValue = getURLParameterValue( paramName );

            if(((AddressImpl)(address)).ifURLParameterExists( paramName )){
                String paramValueCompare =
                    ((AddressImpl)(address)).getURLParameterValue( paramName );

                if( safeStringCompare(paramValue, paramValueCompare) ){
                    //very good, the values matched.
                }else{
                    //values didn't match, so the addresses are not equal
                    bRC = false;
                    break;
                }
            }else{
                // A URI that includes an maddr parameter will not match a URI
                //that contains no maddr parameter.
                if( paramName.equalsIgnoreCase("maddr") ){
                    bRC = false;
                    break;
                }else{
                    if( paramValue.equalsIgnoreCase
                        ( getDefaultValue( paramName ) ) ){
                         //very good, you have the default value
                    }else{
                        bRC = false;
                        break;
                    }
                }
            }
        }


        /*
          Just did pne way param matching in the previous section to find
          our url params appearing only in "this" address.
          now doing the other way param matching to find out url params that
          appear only in the "address to compare".
        */
        urlParamNames = null;
        if( bRC ){
            urlParamNames =
                ((AddressImpl)(address)).getURLParameterNames();
            for( int i =0; i< urlParamNames.length; i++ ){
                String paramName = urlParamNames[i];
                //if only doesn't match
                if( ! ifURLParameterExists( paramName ) ){
                    // A URI that includes an maddr parameter will not
                    // match a URI
                    // that contains no maddr parameter.
                    if( paramName.equalsIgnoreCase("maddr") )
                    {
                        bRC = false;
                        break;
                    }
                    else
                    {
                        String paramValueCompare =
                            ((AddressImpl)(address)).getURLParameterValue( paramName );

                        if(( paramValueCompare != null ) &&
                           ( paramValueCompare.equalsIgnoreCase( getDefaultValue(paramName)) )){
                                //very good, you have the default value
                        }else{
                            bRC = false;
                            break;
                        }
                    }

                }
            }//end for loop
        }

        return bRC;

    }

    /**
     compares this address with the "address" to see
     *if they are "equal" as far as  header-parameters are
     *concerned.
     *
     * Basically confirms to the following from the SIP RFC2543bis-06 spec:

     * URI header components are never ignored. Any present header
     component MUST be present in both URIs and match for the URIs
     to match.
     */
    private boolean compareAddressForHeaderParameters(Address address){
        boolean bRC = true;


        String[] headerParamNames = getHeaderParameterNames();
        for( int i =0; i< headerParamNames.length; i++ ){
            String paramName = headerParamNames[i];
            String paramValue = getURLParameterValue( paramName );

            if(((AddressImpl)(address)).ifHeaderParameterExists( paramName ) ){
                String paramValueCompare =
                   ((AddressImpl)(address)).getHeaderParameterValue(paramName);
                if( safeStringCompare(paramValue, paramValueCompare )){
                    //very good, the values matched.
                }else{
                    //values didn't match, so the addresses are not equal
                    bRC = false;
                    break;
                }
            }else{
                bRC = false;
                break;
            }
        }


        //the other direction for headers.
        //don't need to check the ones that are equal
        //as they are already checked in the one-way check.
        headerParamNames = null;
        if( bRC ){
             headerParamNames =
                ((AddressImpl)(address)).getHeaderParameterNames();
            for( int i =0; i< headerParamNames.length; i++ ){
                String paramName = headerParamNames[i];
                if( !ifHeaderParameterExists( paramName ) ){
                    bRC = false;
                    break;
                }
            }
        }
        return bRC;
    }



}








