/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipResponse.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
 
package org.sipfoundry.sip ;

import java.util.Vector;
import java.util.StringTokenizer;
import java.io.InputStream;
import java.io.IOException;
import java.io.Serializable;


/**
 * This class extends a generic SipMessage class to provide accessor and
 * mutator methods that are specific to sip responses.
 *
 * @author Daniel Petrie
 */
public class SipResponse extends SipMessage implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default public no argument constructor required for serialization
     */
    public SipResponse()
    {
        super() ;
    }


    /**
     * Constructor accepting a InputStream as a source of the Sip Message.
     * This object will populated from the text data read from the designated
     * stream.
     *
     * @param inputstream InputStream containing a text of a Sip Message.
     */
	public SipResponse(InputStream inputstream)
	    throws IOException
	{
	    super(inputstream) ;
    }


    /**
     * Constructor accepting a string as the source of the Sip Message.
     *
     * @param strMessage Text of a Sip Message
     */
    public SipResponse(String strMessage)
    {
        super(strMessage) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the Response status code
     */
    public int getStatusCode()
    {
        StringTokenizer tokenizer = new StringTokenizer(header);
		tokenizer.nextToken() ;

		String strCode = tokenizer.nextToken() ;

		return Integer.parseInt(strCode) ;
    }


    /**
     * Get the Response status text
     */
    public String getStatusText()
    {
        StringTokenizer tokenizer = new StringTokenizer(header);
		tokenizer.nextToken() ;
		tokenizer.nextToken() ;

		String strStatus = tokenizer.nextToken("\n") ;

		return strStatus ;
    }


    /**
     * gets the realm from the response.
     * WWW-Authenticate comes in the form of:
     *
     *  WWW-Authenticate: Digest realm="atlanta.com",
        domain="sip:boxesbybob.com",
        nonce="f84f1cec41e6cbe5aea9c8e88d359",
        opaque="", stale=FALSE, algorithm=MD5
     */
    public String getRealm(){
        String strRet = null;
        String wwwAuthenticate = getHeaderFieldValue("Www-Authenticate");
        StringTokenizer tokenizer = new StringTokenizer(wwwAuthenticate, ",");
        String realmKeyValue = null;
        while( tokenizer.hasMoreTokens() ){
            String token = tokenizer.nextToken() ;
            if( token.toUpperCase().indexOf("REALM") != -1 ){
                realmKeyValue = token;
                break;
            }
        }
        if( realmKeyValue != null ){
            tokenizer = new StringTokenizer(realmKeyValue, "=");
            if(  tokenizer.hasMoreTokens() ){
                tokenizer.nextToken();
                if( tokenizer.hasMoreTokens() ){
                    String realmWithQuotes = tokenizer.nextToken().trim();
                    if( realmWithQuotes.length() >= 2 )
                        strRet = realmWithQuotes.substring(1, realmWithQuotes.length()-1);
                }
            }
        }
        return strRet ;
    }


   /**
    * Sets the first line of the Response header
    *
	* @param responseStatusCode response return code
	* @param responseStatusText response text
	*/
	public void setResponseHeader(  int    responseStatusCode,
								    String responseStatusText)
	{
		setHeader(SIP_PROTOCOL + " " + Integer.toString(responseStatusCode) +
            " " + responseStatusText) ;
	}
}
