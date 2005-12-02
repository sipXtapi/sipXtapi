package javax.siplite;


/**
 * This Exception indicates that the attribute has already been set and the 
 * attribute in question can no longer be changed.
 */
public class AttributeSetException extends Exception {

    /**
    * Constructs a new <code>AttributeSetException</code>
    */
    public AttributeSetException() {
    }

    /**
    * Constructs a new <code>AttributeSetException</code> with the specified
    * error message.
    *
    * @param  error the detail of the error message
    */
    public AttributeSetException(java.lang.String error) {
        super(error);
    }

}

