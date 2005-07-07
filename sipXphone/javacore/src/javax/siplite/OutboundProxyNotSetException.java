package javax.siplite;


/**
 * This Exception indicates that the oubound proxy has not been set.
 */
public class OutboundProxyNotSetException extends Exception {

    /**
    * Constructs a new <code>OutboundProxyNotSetException</code>
    */
    public OutboundProxyNotSetException() {
    }

    /**
    * Constructs a new <code>OutboundProxyNotSetException</code> with the specified
    * error message.
    *
    * @param  error the detail of the error message
    */
    public OutboundProxyNotSetException(java.lang.String error) {
        super(error);
    }

}

