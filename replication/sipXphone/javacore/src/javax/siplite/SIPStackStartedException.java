package javax.siplite;


/**
 * This Exception indicates that the SIPStack has started (automatically done by the SIPLite
 * implementation when its getCallProvider method is called) and the 
 * parameter in question can no longer be changed.
 */
public class SIPStackStartedException extends Exception {

    /**
    * Constructs a new <code>SIPStackStartedException</code>
    */
    public SIPStackStartedException() {
    }

    /**
    * Constructs a new <code>SIPStackStartedException</code> with the specified
    * error message.
    *
    * @param  error the detail of the error message
    */
    public SIPStackStartedException(java.lang.String error) {
        super(error);
    }

}

