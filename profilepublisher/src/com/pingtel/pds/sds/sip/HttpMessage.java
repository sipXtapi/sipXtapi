/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/sip/HttpMessage.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.sip;

import java.util.Vector;
import java.util.StringTokenizer;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.Serializable;

import org.apache.log4j.Category;
//import com.pingtel.pds.common.PDSCategory;


/** This class is used to build, hold and obtain content from texted
 * based streamed messages.  The format of the message is a generalized
 * form of HTTP messages.  The first line of the message may be free
 * form text.  Lines are terminated by the newline character '\n'.
 * Following the the first line are zero or more lines of name value
 * pairs seporated by the colin and space characters ": ".
 * The name value pair section is terminated by a double newline.
 * Optionally a message body/file follows the terminators for the
 * name value pair section.  This is indicated by the Content-length
 * name value pair.
 *
 * @author Daniel Petrie
 */
public class HttpMessage extends Object implements Serializable {
    // public static String NewLine = "\n\r";

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    public static String NewLine = "\r\n";
    public static String NameValueSeporator = ": ";

    private String header = null;
    private Vector names = null;
    private Vector values = null;
    private byte body[] = null;


    /**
     * Default public no argument constructor required for serialization
     */
    public HttpMessage() {
        super() ;

        names = new Vector() ;
        values = new Vector() ;
    }


    /** Construct a new message from the given stream.
    */
    public HttpMessage(InputStream iStream) throws IOException
    {
        serializeFromInputStream(iStream) ;
    }


    /**
     * Constructor accepting a string as the source of the http message
     */
    public HttpMessage(String strMessage) {
        InputStream iStream = new ByteArrayInputStream(strMessage.getBytes()) ;
        try {
            serializeFromInputStream(iStream) ;
        } catch( IOException e ) {
            // I believe this exception is impossible
            e.printStackTrace() ;
        }
    }



    /** Construct a new message given the first line of the
     * header.
     * @param headerString - the text of the first line of the message
     *                          header.  This should not contain newline
     *                                  or CR characters ('\n' or '\r').
     */
    //public HttpMessage(String headerString)
    //{
    //header = headerString;
    //nameValues = new Hashtable(10);
    //}

    //public HttpMessage(int httpReturnCode, String returnMessage)
    //{
    //setHeader(httpReturnCode, returnMessage);
    //nameValues = new Hashtable(10);
    //}

    /** Get the first line of the header for the message.
     */
    public String getHeader() {
        return(header);
    }


    /** Gets the request method
     */
    public String getRequestMethod() {
        String method = (new StringTokenizer(header)).nextToken();
        return(method);
    }

    /** Gets the request URI
     */
    public String getRequestUri() {
        StringTokenizer tokenizer = new StringTokenizer(header);
        tokenizer.nextToken();
        String uri = tokenizer.nextToken();
        return(uri);
    }

    /** Gets the request method
     */
    public String getRequestProtocol() {
        StringTokenizer tokenizer = new StringTokenizer(header);
        tokenizer.nextToken();
        tokenizer.nextToken();
        String protocol = tokenizer.nextToken();
        return(protocol);
    }

    /** Get the reponse protocol
     */
    public String getResponseProtocol() {
        return(getRequestMethod());
    }

    /** Gets the response status code
    public int getResponseStatusCode();
    {
        return(Integer.parseInt(getRequestUri(());
    }

    /** Gets the response status text
    public String getResponseStatusText()
    {
        return(getRequestProtocol());
    }

   /** Sets the first line of the request header
    * @param method - the request method name
    * @param uri - the request uri
    * @param protocol - the request protocol ("HTTP/1/0" or "SIP/2.0")
    */
    public void setRequestHeader(String method, String uri, String protocol) {
        setHeader(method + " " + uri + " " + protocol);
    }

    /** Sets the first line of the request header
     * @param protocol - the request protocol ("HTTP/1/0" or "SIP/2.0")
     * @param responseStatusCode - response return code
     * @param responseStatusText - response text
     */
    public void setResponseHeader(String protocol, int responseStatusCode,
                                  String responseStatusText) {
        setHeader(protocol + " " + Integer.toString(responseStatusCode) +
                  " " + responseStatusText);
    }

    /** Sets the first line of the header for the message.
     * @param headerString - the text of the first line of the header
     *                                  This should not contain newline or CR characters
     *                                  ('\n' or '\r').
     */
    public void setHeader(String headerString) {
        header = headerString;
    }

    /** Sets the first line of the header for the message.
     */
    public void setResponseHeader(int httpReturnCode, String returnMessage) {
        Integer code = new Integer(httpReturnCode);
        header = new String("HTTP/1.0 " + code.toString() + " " + returnMessage);
    }

    /** Get the value for the given name value pair.
     * @returns string value if set or null if not set
     */
    public String getHeaderFieldValue(String headerFieldName) {
        //return((String)nameValues.get(name));
        String value = null ;
        int valueIndex = findHeaderField(0, headerFieldName);

        if( valueIndex >= 0 ) {
            value = (String) values.elementAt(valueIndex);
        }

        if( value != null )
            value = value.trim() ;

        return(value);
    }

    /** Returns the nam and value of the header field at the given index
     * @param headerIndex - the index into the header field
     */
    public String[] getHeaderField(int headerIndex) {
        String nvArray[] = new String[2];
        nvArray[0] = new String((String)names.elementAt(headerIndex));
        nvArray[1] = new String((String)values.elementAt(headerIndex));
        return(nvArray);
    }

    public int findHeaderField(int startIndex, String headerFieldName) {
        String name;
        int foundIndex = -1;
        for( int nameIndex = startIndex; nameIndex < names.size(); nameIndex++ ) {
            name = (String) names.elementAt(nameIndex);
            if( name.equalsIgnoreCase(headerFieldName) ) {
                foundIndex = nameIndex;
                break;
            }
        }
        return(foundIndex);
    }

    /** Set the value of the named name value pair
     * This assumes there exists only one instance of
     * the named header field.  If there are more than
     * one it set the first one.  It the named header field
     * does not exist it create one.
     */
    public void setHeaderFieldValue(String name, String value) {
        int valueIndex =  findHeaderField(0, name);

        if( valueIndex >= 0 ) {
            values.setElementAt(value, valueIndex);
        } else {
            names.addElement(name);
            values.addElement(value);
        }
    }

    /** Add a new header field and its value.
     * This method does not replace existing header fields
     * of the given name.  It will create another
     * instance of the header field with the same name.
     */
    public void addHeaderField(String name, String value) {
        names.addElement(name);
        values.addElement(value);
    }

    /** Get the length of the body/file part of the message.
     * Note: this does not read the Content-length field.  It directly checks
     * the size of the body.
     */
    public int getBodyLength() {
        return(body.length);
    }

    /** Get the body/file part of the message.
     */
    public byte[] getBody() {
        return(body);
    }

    /** Set the body or file part of the message.
     */
    public void setBody(byte bodyBytes[]) {
        body = bodyBytes;
    }

    /** Set the body or file part of the message.
     */
    public void setBody(String bodyString) {
        body = bodyString.getBytes();
    }

    /** Conver the message into a string which can be sent via a stream.
     * Note: the Content-length will be set automaticly base upon the size of
     * the body.
     */
    public String toString() {
        String name;
        String value;
        int stringLength = 1000;  // Guess at buffer length
        if( body != null ) {
            stringLength += body.length;
        }
        StringBuffer messageContent = new StringBuffer(stringLength);

        // Add the header
        messageContent.append(header + NewLine);

        // Set the content length if there is a body
        if( body != null && body.length > 0 ) {
            Integer length = new Integer(body.length);
            setHeaderFieldValue("Content-Length", length.toString());
        }

        // Add the name/value pairs
        for( int headerIndex = 0; headerIndex < names.size(); headerIndex++ ) {
            name = (String) names.elementAt(headerIndex);
            value = (String) values.elementAt(headerIndex);
            //PDSCategory.getInstance().logError(
            //    this, "toString",
            //    value);
            messageContent.append(name + NameValueSeporator + value + NewLine);
        }

        // End of header and name/value pairs
        messageContent.append(NewLine);

        // Add the body/file
        if( body != null && body.length > 0 ) {
            //messageContent.append((char[])body);
            //messageContent.append(body);
            char aChar;
            for( int byteIndex = 0; byteIndex < body.length; byteIndex++ ) {
                aChar = (char) body[byteIndex];
                messageContent.append(aChar);
            }

        }
        return(messageContent.toString());
    }

    private static String readLine(InputStream stream) throws IOException
    {
        StringBuffer buffer = new StringBuffer(120);
        byte oneByte;


        // While not at the end and not a new line keep reading.
        for( oneByte = (byte) stream.read(); ! isTerminatorChar(oneByte);
           oneByte = (byte) stream.read() ) {
            buffer.append((char)oneByte);
        }


        // Check for a new line after the CR or vis versa
        stream.mark(2);
        byte nextByte = (byte) stream.read();
        if( ! isTerminatorChar(nextByte) || oneByte == nextByte ) {
            // back up because this is not a terminator or there are two new lines
            // which need to be treated as seporate lines.
            stream.reset();
        }

        return(buffer.toString());
    }

    private static boolean isTerminatorChar(byte aByte) {
        boolean returnCode = false;
        if( aByte == '\n' || aByte == '\r' ) {
            returnCode = true;
        }
        return(returnCode);
    }

    private static String[] parseNameValue(String nameValue) {
        int nameEnd = nameValue.indexOf(":");
        if( nameEnd < 0 ) {
            nameEnd = nameValue.length();
        }

        String name = nameValue.substring(0, nameEnd);
        String value = nameValue.substring(nameEnd + 1);
        String returnStrings[] = new String[2];
        name.trim();
        value.trim();
        returnStrings[0] = name;
        returnStrings[1] = value;
        return(returnStrings);
    }


    private void serializeFromInputStream(InputStream iStream)
    throws IOException
    {
        names = new Vector();
        values = new Vector();
        String nvArray[] = new String[2];
        header = readLine(iStream);

        for( String nvLine = readLine(iStream); !nvLine.equals("");
           nvLine = readLine(iStream) ) {

            // If this is a contiuation of the previous line
            if( nvLine.charAt(0) == ' ' ||
                nvLine.charAt(0) == '\t' ) {
                int lastElementIndex = values.size();
                String concatString = values.elementAt(lastElementIndex) +
                                      nvLine;
                values.setElementAt(concatString, lastElementIndex);
            }

            else {
                nvArray = parseNameValue(nvLine);
                //if(nvArray[1] == null) nvArray[1] = "";

                addHeaderField(nvArray[0], nvArray[1]);
            }
        }

        String lengthString = (String) getHeaderFieldValue("Content-Length");
        if( lengthString != null )
            lengthString = lengthString.trim() ;


        m_logger.debug ( "serializeFromInputStream Content-Length " + lengthString );

        int length = 0;
        if( lengthString != null && lengthString.compareTo("") != 0 ) {
            length = Integer.parseInt(lengthString);
        } else {
            length = iStream.available();
        }

        body = new byte[length];
        try {
            iStream.read(body);
            /*byte aByte;
            char aChar;
            for(int bodyIndex = 0; bodyIndex < length; bodyIndex++)
            {
                aByte = (byte) iStream.read();
                aChar = (char) aByte;
                body[bodyIndex] = (byte) aChar;
            }*/
        } catch( IOException e ) {
            body = null;
            length = 0;
        }
        if( lengthString != null && lengthString.compareTo("") != 0 ) {
            setHeaderFieldValue("Content-Length", Integer.toString(length));
        }
    }
}
