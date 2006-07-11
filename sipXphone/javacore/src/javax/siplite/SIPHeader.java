package javax.siplite; 


/**
* The SIPHeader interface provides the means for forming headers to be added
* to the basic SIP Message. The interface also provides the means for 
* retrieving the values associated with the headers,
* if this information is required.
*  
*/
public interface SIPHeader
{
    
	/**
	* Retrieve the type of the header. 
	* @return header type
	**/
        java.lang.String getType();
	
        /**
	* Sets the type of the header. 
	* @param type header type
        * @throws AttributeSetException if the type of this header has 
        * already been set.
	**/
        void setType(java.lang.String type) 
            throws AttributeSetException;
        
        /**
        * retreive this header's values 
        * @return values held by the header
        **/
        javax.siplite.SIPValue[] getValues();
        
        /** 
         * Set the values for the header
         * @param values new values for the header
	 * @throws InvalidArgumentException if multi values are 
         * specified but only one is permitted for the header type.
         **/
        void setValues(javax.siplite.SIPValue[] values) 
		throws InvalidArgumentException;


} // end of interface SIPHeader
