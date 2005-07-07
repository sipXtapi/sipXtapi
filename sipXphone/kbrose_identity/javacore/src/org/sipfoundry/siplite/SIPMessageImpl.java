/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPMessageImpl.java#2 $
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

import java.util.Vector ;
import java.util.Enumeration;
import java.util.StringTokenizer ;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.Serializable;

import javax.siplite.SIPMessage ;
import javax.siplite.Constants;
import javax.siplite.SIPHeader;
import javax.siplite.SIPValue;
import javax.siplite.SIPParameter;
import javax.siplite.InvalidArgumentException;


/**
 * The SIPMessage interface provides the means for adding additional information
 * in the form of headers to the basic SIP Message. The interface also
 * provides the means for retrieving the headers.
 *
 */
public class SIPMessageImpl implements SIPMessage
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
   protected static final String NEWLINE = "\r\n";
   protected static final String NAMEVALUE_SEPARATOR = ": ";
   protected static final String SIP_PROTOCOL = "SIP/2.0" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
   protected String m_strStartLine = null;
   protected Vector m_vSipHeaders = new Vector();
   protected byte body[] = null;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default constructor
     */
    public SIPMessageImpl()
    {
    }


    /**
     * Constructor accepting a InputStream as a source of the Sip Message.
     * This object will be populated from the text data read from the designated
     * stream.
     *
     * @param inputstream InputStream containing a text of a Sip Message.
     * @throws IOException thrown if any IO error is encountered while reading
     *         the input stream.
     * @throws InvalidArgumentException thrown if the inputsteam has invalid
     *         data
     */
    public SIPMessageImpl(InputStream inputstream)
        throws IOException, InvalidArgumentException
    {
        serializeFromInputStream(inputstream) ;
    }


    /**
     * Constructor accepting a string as the source of the Sip Message.
     *
     * @param strMessage Text of a Sip Message
     * @throws InvalidArgumentException thrown if the inputsteam has invalid data
     */
    public SIPMessageImpl(String strMessage) throws InvalidArgumentException
    {
        InputStream iStream = new ByteArrayInputStream(strMessage.getBytes()) ;
        try
        {
            serializeFromInputStream(iStream) ;
        }
        catch (IOException e)
        {
            org.sipfoundry.util.SysLog.log(e) ;
            throw new InvalidArgumentException(e.getMessage());
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Add a header to the basic SIP Message. This method provides a means of
     * adding additional headers to the basic SIP Message
     * @param header header to add to the message
     * @throws InvalidArgumentException if the header already exists
     * and only one is permitted for the message type.
     */
    public void addHeader(SIPHeader header)
               throws InvalidArgumentException
    {
        if( header != null )
        {
            if( ! SIPPermissions.getInstance().isHeaderPermitted( header.getType() ) ){
                throw new InvalidArgumentException("header " +  header.getType()
                                         + " is not permitted to be added "+
                                        " in the SIP message ");
            }
            m_vSipHeaders.addElement(header);
        }
        else
        {
            throw new InvalidArgumentException( "null SIPHeader cannot be added");
        }
    }


    /**
     * Retrieve an array of all the headers in this message. This method returns a
     * SIPHeader[] for the headers only. Multiple headers appear only once in the array.
     * @return The headers in a SIPHeader[]
     **/
    public SIPHeader[] getHeaders(java.lang.String type){
        Vector vec = new Vector();
        for( Enumeration enum = m_vSipHeaders.elements(); enum.hasMoreElements(); )
        {
            SIPHeader header = (SIPHeader)(enum.nextElement());
            if( header.getType().equals(type) )
            {
                vec.addElement(header);
            }
        }
        SIPHeader[] sipHeaders = new SIPHeader[vec.size()] ;
        vec.copyInto(sipHeaders) ;
        return sipHeaders;
    }

    /**
     * Retrieve an array of the headers in this message of type.
     * @param type headers of type to be returned
     * @return The headers in a SIPHeader[]
     */
    public SIPHeader[] getHeaders()
    {
        SIPHeader[] sipHeaders = new SIPHeader[m_vSipHeaders.size()] ;
        m_vSipHeaders.copyInto(sipHeaders) ;
        return sipHeaders;
    }


    /**
     * Retrieve the payload of the message.
     **/
    public byte[] getPayload()
    {
        return(body) ;
    }


    /**
     * Retrieve the method of the message.
     * @return message method
     **/
    public String getMethod()
    {
        String strCSeq = getHeaderValue("Cseq");
		if (strCSeq == null) return "";
        StringTokenizer tokenizer = new StringTokenizer(strCSeq) ;
        tokenizer.nextToken() ;

        return tokenizer.nextToken() ;
    }


    /**
     * Retrieve the status code of the message.
     * @return message status code
     **/
    public int getStatusCode()
    {
        int iRet = -1;
        if( isResponse() ){
            StringTokenizer tokenizer = new StringTokenizer(m_strStartLine);
            tokenizer.nextToken() ;
            String strCode = tokenizer.nextToken() ;
            iRet = Integer.parseInt(strCode) ;
        }
        return iRet;
    }


    /**
     * Retrieve the status string of the message.
     * @return message status string
     **/
    public String getStatusString()
    {
        String strStatus = null;
        if( isResponse() ){
            StringTokenizer tokenizer = new StringTokenizer(m_strStartLine);
            tokenizer.nextToken() ;
            tokenizer.nextToken() ;
            strStatus = tokenizer.nextToken("\n") ;
        }
        return strStatus ;
    }


    /**
     * Retreives request URI of the message.
     * @return requestURI of a request message, null if the message is a
     * response.
     **/
    public String getRequestURI()
    {
        String requestURI = null;

        if(!isResponse())
            requestURI = m_strStartLine;

        return requestURI;
    }


    /**
     * Renderers a String representation of the SipMessage that is suitable for
     * transmission.
     * <br><br>
     * Note: the Content-length will be set automaticly base upon the size of
     * the body.
     *
     * @return Rendered text of the SipMessage
     */
    public String toString()
    {
        String name;
        String value;
        int stringLength = 1000;  // Guess at buffer length
        if(body != null)
        {
           stringLength += body.length;
        }
        StringBuffer messageContent = new StringBuffer(stringLength);

        // Add the header
        messageContent.append(m_strStartLine + NEWLINE);

        // Set the content length if there is a body
        if(body != null && body.length > 0)
        {
           Integer length = new Integer(body.length);
           replaceHeader(new SIPHeaderImpl("Content-Length", length.toString() ), true);
           //m_vSipHeaders.addElement(new SIPHeaderImpl("Content-Length", length.toString()));
        }

        // Add the name/value pairs
        for (int headerIndex = 0; headerIndex < m_vSipHeaders.size(); headerIndex++)
        {
           SIPHeader sipHeader = (SIPHeader)m_vSipHeaders.elementAt(headerIndex);
           messageContent.append(sipHeader + NEWLINE);
        }

        // End of header and name/value pairs
        messageContent.append(NEWLINE);

        // Add the body/file
        if(body != null && body.length > 0)
        {
            //messageContent.append((char[])body);
            //messageContent.append(body);
            char aChar;
            for(int byteIndex = 0; byteIndex < body.length; byteIndex++)
            {
                aChar = (char) body[byteIndex];
                messageContent.append(aChar);
            }

        }
        return(messageContent.toString());
    }

    /**
     * returns the value of first header of this name, if any.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strHeaderName name of the header whose value you are looking for.
     * If there are more than one header defined with the same name, it gets the
     * first one.
     */
    public String getHeaderValue( String strHeaderName )
    {
        String strRet = null;
        SIPHeader[] headers = getHeaders(strHeaderName);
        if( headers.length > 0 ){
            SIPHeader header = headers[0];
            if( header != null )
                strRet = ((SIPHeaderImpl)(header)).getValuesAsString();
        }
        return strRet;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementatoin Methods
////


    /**
     * Get the first line of the header for the message.
     */
    protected String getStartLine()
    {
        return(m_strStartLine);
    }


    /**
     * Sets the first line for the message.
     *
     * @param startLine The text of the first line of the message.
     */
    protected void setStartLine(String startLine)
    {
        m_strStartLine = startLine ;
    }


    /**
     * Determine if the sip message contains the named header field
     *
     * @param strHeaderType The name of the header field whose existence
     *        will be checked.
     *
     * @return true of the named header field is defined in this method,
     *         otherwise false.
     */
    protected boolean containsHeader(String strHeaderType)
    {
        boolean bRet = false;
        for( Enumeration enum = m_vSipHeaders.elements(); enum.hasMoreElements(); ){
            SIPHeader header = (SIPHeader)(enum.nextElement());
            if( header.getType().equals(strHeaderType) ){
                bRet = true;
                break;
            }
        }
        return bRet;
    }


    /**
     * This is for INTERNAL USE only. If bForce is true, this method does not check if a
     * header is permitted to be added.
     * Set the value of the named name value pair.
     * It the named header field does not exist it creates one.
     */
     protected void addHeader(SIPHeader header, boolean bForce)
           throws InvalidArgumentException{
        if( header != null ){
            if( bForce ){
                 m_vSipHeaders.addElement(header);
            }else{
                if( SIPPermissions.getInstance().isHeaderPermitted( header.getType() ) ){
                    m_vSipHeaders.addElement(header);
                }
            }
        }else{
            throw new InvalidArgumentException( "null SIPHeader cannot be added");
        }
    }

    protected void replaceHeader(SIPHeader header, boolean bForce){
        if( header != null ){
            if( bForce ){
                 for( Enumeration enum = m_vSipHeaders.elements(); enum.hasMoreElements(); ){
                    SIPHeader headerInEnum = (SIPHeader)(enum.nextElement());
                    if( headerInEnum.getType().equals(header.getType()) ){
                        int index = m_vSipHeaders.indexOf(headerInEnum) ;
                        if( index != -1 ){
                          m_vSipHeaders.removeElementAt(index);
                        }
                    }
                 }
                 m_vSipHeaders.addElement(header);
            }else{
                if( SIPPermissions.getInstance().isHeaderPermitted( header.getType() ) ){
                    m_vSipHeaders.addElement(header);
                }
            }
        }
    }


   /**
    * Get the length of the payload(body/file) part of the message. Note: this does
    * not read the Content-length field.  It directly checks the size of the
    * body.
    *
    * @return The size of payload(body/file) part of the message.
    */
    protected int getPayloadLength()
    {
        int iLength = 0 ;

        if (body != null)
            iLength = body.length ;

        return iLength ;
    }



    /**
     * Set the payload(body or file part) of the message.
     *
     * @param bodyBytes The payload(body/file) part of the message
     */
    protected void setPayload(byte bodyBytes[])
    {
        body = bodyBytes;
    }


    private static String readLine(InputStream stream) throws IOException
    {
        StringBuffer buffer = new StringBuffer(120);
        byte oneByte;


        // While not at the end and not a new line keep reading.
        for(oneByte = (byte) stream.read(); ! isTerminatorChar(oneByte);
            oneByte = (byte) stream.read())
        {
            buffer.append((char)oneByte);
        }


        // Check for a new line after the CR or vis versa
        stream.mark(2);
        byte nextByte = (byte) stream.read();
        if(! isTerminatorChar(nextByte) || oneByte == nextByte)
        {
            // back up because this is not a terminator or there are two new
            // lines which need to be treated as seporate lines.
            stream.reset();
        }

        return(buffer.toString());
    }


    private static boolean isTerminatorChar(byte aByte)
    {
        boolean returnCode = false;
        if(aByte == '\n' || aByte == '\r' || aByte == -1)
        {
            returnCode = true;
        }
        return(returnCode);
    }


   /**
     * Gives you an array of name and value from a string where name and
     * value are separated by an ":" sign.
     * @param strNameValue  a string consisting of name and value
     *                     separated by ":" For example Cseq: 1 INVITE
     * @return String[] an array of two strings where the first one is name
     *      and the second one is value.

     */
    private  SIPHeader parseHeader(String strHeaderLine)
        throws InvalidArgumentException
    {
        //here goes the logic for parsing

        int headerNameEnd = strHeaderLine.indexOf(":");
        if(headerNameEnd < 0)
        {
            throw new InvalidArgumentException
                ("header "+ strHeaderLine +" doesn't have a colon to separate name and value ");
        }
        String strHeaderName = strHeaderLine.substring(0, headerNameEnd);
        String strHeaderValueWithParams = strHeaderLine.substring(headerNameEnd + 1);
        return new SIPHeaderImpl( strHeaderName, strHeaderValueWithParams );
    }


     /**
     * Is this message a response?
     */
    private boolean isResponse(){
         boolean bRet = false;
         String startLine = getStartLine();
         if(( startLine != null ) && ( startLine.startsWith(SIP_PROTOCOL) ))
            bRet = true;

        return bRet;
    }


   /**
    *  reads the inputstream for headers and the body.
    *  If the inputstream is a request, the first line
    *  should be of the form Method request-URI SIP-Version.
    *  Otherwise, if it is a response, the first line should
    *  be of the form SIP-Version Status-Code Reason-Phrase.
    *
    *  @throws InvalidArgumentException if the first line is invalid or
    *  if the headers do not come in teh format param:value
    */
   private void serializeFromInputStream(InputStream iStream)
      throws IOException, InvalidArgumentException
   {
        String sipVersionString = "SIP/2.0" ;
        //names = new Vector();
        //values = new Vector();
        String nvArray[] = new String[2];
        m_strStartLine = readLine(iStream);

        /** the first line should be either of the form
        *Method request-URI SIP-Version or
        *SIP-Version Status-Code Reason-Phrase
        */
        if(!(( m_strStartLine != null ) && ( m_strStartLine.trim().startsWith(sipVersionString) ||
                                   m_strStartLine.trim().endsWith  (sipVersionString) ))){
            throw new InvalidArgumentException("InputStream doesn't have a valid request/response header");
        }
        String strTempValue = "";
        for (String nvLine = readLine(iStream); !nvLine.equals("");
                                                nvLine = readLine(iStream)){

            // If this is a contiuation of the previous line
            if(nvLine.charAt(0) == ' ' ||
                nvLine.charAt(0) == '\t'){
                strTempValue += nvLine;
            }else
            {
                strTempValue += nvLine;
                addHeader( parseHeader(strTempValue) );
                strTempValue = "";
            }
        }

        String lengthString = getHeaderValue("Content-Length");
        if (lengthString != null)
            lengthString = lengthString.trim() ;
        int length = 0;
        if(lengthString != null && lengthString.compareTo("") != 0){
            length = Integer.parseInt(lengthString);
        }else{
            length = iStream.available();
        }

        body = new byte[length];
        try
        {
            iStream.read(body);
        }
        catch(IOException e)
        {
            body = null;
            length = 0;
        }
        if(lengthString != null && lengthString.compareTo("") != 0)
        {
            replaceHeader(new SIPHeaderImpl("Content-Length", Integer.toString(length)), true);
        }
   }

   /**
    * testing if two SIPMessages are equals.
    * According to this implementation,
    * the string representation of the SIPmessage to
    * compare must match the string representation of this
    * SIPMessage for them to be equal.
    */
   public boolean equals( Object obj){
        boolean bRet = false;
        SIPMessage paramMessgae = (SIPMessage) obj;
        if( obj.toString().equals(this.toString()))
            bRet = true;
        return bRet;
   }

}
