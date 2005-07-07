package javax.siplite; 

/**
* The SIPParameter interface provides the means for adding additional information
* in the form of name value pairs to the basic SIPValue. The interface also 
* provides the means for retrieving the names and values.
*  
*/
public interface SIPParameter
{
        /**
        * retrieve the name portion of the parameter
        * @return name part of the parameter
        **/
        java.lang.String getName();
        
        /**
        * retrieve the value portion of the parameter
        * @return string value of the parameter
        **/ 
        java.lang.String getValue();
        
        /**
         * Set the name and value pairing of a paramter
         * @param name parameter name, lefthand side of equals 
         * @param value parameter value, righthand side of equals 
         **/
        void setParameter(java.lang.String name, java.lang.String value);
        /**
         * Set the name part of the parameter
         * @param name parameter name 
         **/
        void setName(java.lang.String name);
        /**
         * Set the value part of the parameter
         * @param value parameter vale
         **/
        void setValue(java.lang.String value);

} // end of interface SIPParameter
