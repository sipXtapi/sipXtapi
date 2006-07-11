/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipMessage.java#2 $
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
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.Serializable;
import org.sipfoundry.util.SysLog;

/**
 * This class is used to build, hold and obtain content from text based
 * messages.  The format of the message is a generalized form of HTTP
 * messages.  The first line of the message may be free form text.  Lines
 * are terminated by the newline character '\n'.
 * <br><br>
 * Following the the first line are zero or more lines of name value pairs
 * separated by the colin and space characters ": ".
 * <br><br>
 * The name value pair section is terminated by a double newline. Optionally a
 * message body/file follows the terminators for the name value pair section.
 * This is indicated by the Content-length name value pair.
 *
 * @author Daniel Petrie
 */
public class SipMessage implements Serializable
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
   protected String header = null;
   protected Vector names = null;
   protected Vector values = null;
   protected byte body[] = null;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default public no argument constructor required for serialization
     */
    public SipMessage()
    {
        names = new Vector() ;
        values = new Vector() ;
    }


    /**
     * Constructor accepting a InputStream as a source of the Sip Message.
     * This object will populated from the text data read from the designated
     * stream.
     *
     * @param inputstream InputStream containing a text of a Sip Message.
     */
	public SipMessage(InputStream inputstream)
	    throws IOException
	{
       serializeFromInputStream(inputstream) ;
    }


    /**
     * Constructor accepting a string as the source of the Sip Message.
     *
     * @param strMessage Text of a Sip Message
     */
    public SipMessage(String strMessage)
    {
        InputStream iStream = new ByteArrayInputStream(strMessage.getBytes()) ;
        try {
            serializeFromInputStream(iStream) ;
        } catch (IOException e) {
            SysLog.log(e) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get the first line of the header for the message.
     */
    public String getHeader()
    {
        return(header);
    }


    /**
     * Sets the first line of the header for the message.
     *
     * @param headerString The text of the first line of the header.  This
     *        should not contain new lines (\n) or carriage returns (\r).
     */
    public void setHeader(String strHeader)
    {
        header = strHeader ;
    }


    /**
     * Get the value for the given name value pair.
     *
     * @param strHeaderField The name of the header field whose value
     *        will be returned.
     *
     * @return String value if the header is defined, otherwise null.
     */
    public String getHeaderFieldValue(String strHeaderField)
    {
        String strValue = null ;
		int valueIndex = findHeaderField(0, strHeaderField) ;

		if(valueIndex >= 0) {
			strValue = (String) values.elementAt(valueIndex);
        }

        if (strValue != null)
            strValue = strValue.trim() ;

        return strValue ;
    }


    /**
     * Determine if the sip message contains the named header field
     *
     * @param strHeaderField The name of the header field whose existence
     *        will be checked.
     *
     * @return true of the named header field is defined in this method,
     *         otherwise false.
     */
    public boolean containsHeaderField(String strHeaderField)
    {
        return (findHeaderField(0, strHeaderField) >= 0) ;
    }


   /**
    * Set the value of the named name value pair. This assumes there exists
    * only one instance of the named header field.  If there are more than
	* one it set the first one.  It the named header field does not exist it
	* creates one.
	*
	* @param strName The name of the header field
	* @param strValue The value of the header field
    */
    public void setHeaderFieldValue(String strName, String strValue)
    {
        int valueIndex =  findHeaderField(0, strName) ;

		if(valueIndex >= 0) {
			values.setElementAt(strValue, valueIndex) ;
        } else {
			names.addElement(strName) ;
			values.addElement(strValue) ;
		}
    }


   /**
    * Add a new header field and its value. This method does not replace
    * existing header fields of the given name.  It will create another
	* instance of the header field with the same name.
	*
	* @param strName The name of the header field
	* @param strValue The value of the header field
	*/
   public void addHeaderField(String strName, String strValue)
   {
		names.addElement(strName) ;
		values.addElement(strValue) ;
   }


   /**
    * Get the length of the body/file part of the message. Note: this does
    * not read the Content-length field.  It directly checks the size of the
    * body.
    *
    * @return The size of body/file part of the message.
    */
	public int getBodyLength()
	{
	    int iLength = 0 ;

	    if (body != null)
	        iLength = body.length ;

        return iLength ;
    }


    /**
     * Get the body/file part of the message.
     *
     * @return The body/file part of the message.
     */
    public byte[] getBody()
    {
		return(body) ;
    }


    /**
     * Set the body or file part of the message.
     *
     * @param bodyBytes The body/file part of the message
     */
    public void setBody(byte bodyBytes[])
    {
        body = bodyBytes;
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
		messageContent.append(header + NEWLINE);

		// Set the content length if there is a body
		if(body != null && body.length > 0)
		{
		   Integer length = new Integer(body.length);
		   setHeaderFieldValue("Content-Length", length.toString());
		}

		// Add the name/value pairs
		for (int headerIndex = 0; headerIndex < names.size(); headerIndex++)
		{
		   name = (String) names.elementAt(headerIndex);
		   value = (String) values.elementAt(headerIndex);
		   messageContent.append(name + NAMEVALUE_SEPARATOR + value + NEWLINE);
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


//////////////////////////////////////////////////////////////////////////////
// Implementatoin Methods
////

   /**
    * Returns the name and value of the header field at the given index
    *
    * @param headerIndex - the index into the header field
	*/
	private String[] getHeaderField(int headerIndex)
	{
		String nvArray[] = new String[2];
		nvArray[0] = new String((String)names.elementAt(headerIndex));
		nvArray[1] = new String((String)values.elementAt(headerIndex));
		return(nvArray);
	}


    private int findHeaderField(int startIndex, String headerFieldName)
    {
		String name;
		int foundIndex = -1;
		for (int nameIndex = startIndex; nameIndex < names.size(); nameIndex++)
		{
			name = (String) names.elementAt(nameIndex);
			if(name.equalsIgnoreCase(headerFieldName))
			{
				foundIndex = nameIndex;
				break;
			}
		}
		return(foundIndex);
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

    private static String[] parseNameValue(String nameValue)
    {
	    int nameEnd = nameValue.indexOf(":");
	    if(nameEnd < 0)
	    {
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

		for (String nvLine = readLine(iStream); !nvLine.equals("");
			nvLine = readLine(iStream))

		{

			// If this is a contiuation of the previous line
			if(nvLine.charAt(0) == ' ' ||
				nvLine.charAt(0) == '\t')
			{
				int lastElementIndex = values.size();
				String concatString = values.elementAt(lastElementIndex) +
											nvLine;
				values.setElementAt(concatString, lastElementIndex);
			}

			else
			{
				nvArray = parseNameValue(nvLine);
				//if(nvArray[1] == null) nvArray[1] = "";

				addHeaderField(nvArray[0], nvArray[1]);
			}
		}

		String lengthString = (String) getHeaderFieldValue("Content-Length");
		if (lengthString != null)
		    lengthString = lengthString.trim() ;
		int length = 0;
		if(lengthString != null && lengthString.compareTo("") != 0)
		{
			length = Integer.parseInt(lengthString);
		}
				else
		{
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
			setHeaderFieldValue("Content-Length", Integer.toString(length));
		}
   }
}
