package javax.siplite; 

/**
* The SIPValue interface provides the means for adding values to SIP headers and
* associating parameters with a value.
* The interface also provides the means for retrieving the value 
* and any parameters, if this information is required.
*  
*/
public interface SIPValue
{
        /**
        * retrieve the value string
        * return string value
        **/
        java.lang.String getValue();
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
        void setValue(java.lang.Object value) 
            throws IllegalArgumentException;


        /**
        * retreive parameters for a SIPValue
        * @return array of SIPParameter
        **/
        javax.siplite.SIPParameter[] getParameters();
        /**
        * set the parameters for a SIPValue
        * @param parameters held by this SIPValue
        **/
        void setParameters(javax.siplite.SIPParameter[] parameters);
    

} // end of interface SIPValue
