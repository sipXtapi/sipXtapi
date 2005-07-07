package javax.siplite; 

/**
* The SIPMessage interface provides the means for adding additional information
* in the form of headers to the basic SIP Message. The interface also 
* provides the means for retrieving the headers.
*  
*/
public interface SIPMessage
{
	/**
	* Add a header to the basic SIP Message. This method provides a means of
	* adding additional headers to the basic SIP Message
        * @param header header to add to the message
	* @throws InvalidArgumentException if the header already exists
        * and only one is permitted for the message type.
        **/
	void addHeader(javax.siplite.SIPHeader header)
            throws InvalidArgumentException;

	/**
	* Retrieve an array of all the headers in this message. This method returns a 
        * SIPHeader[] for the headers only. Multiple headers appear only once in the array.
	* @return The headers in a SIPHeader[]
	**/
	javax.siplite.SIPHeader[] getHeaders();

        /**
	* Retrieve an array of the headers in this message of type. 
        * @param type headers of type to be returned
	* @return The headers in a SIPHeader[]
	**/
	javax.siplite.SIPHeader[] getHeaders(java.lang.String type);

	/**
	* Retrieve the payload of the message. 
        * @return byte array payload of the message
	**/
	byte[] getPayload();

	/**
	* Retrieve the method of the message. 
	* @return message method
	**/
	java.lang.String getMethod();


	/**
	* Retrieve the status code of the message. 
	* @return message status code
	**/
	int getStatusCode();


	/**
	* Retrieve the status string of the message. 
	* @return message status string
	**/
	java.lang.String getStatusString();
        
        /**
        * Retreives request URI of the message.
        * @return requestURI of a request message, null if the message is a
        * response.
        **/
        java.lang.String getRequestURI();

} // end of interface SIPMessage
