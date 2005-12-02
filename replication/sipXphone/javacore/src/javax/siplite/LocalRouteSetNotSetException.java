package javax.siplite;

/**
 * This Exception indicates that the local route set has not been set.
 */
public class LocalRouteSetNotSetException extends Exception {

    /**
    * Constructs a new <code>LocalRouteSetNotSetException</code>
    */
    public LocalRouteSetNotSetException() {
    }

    /**
    * Constructs a new <code>LocalRouteSetNotSetException</code> with the specified
    * error message.
    *
    * @param  error the detail of the error message
    */
    public LocalRouteSetNotSetException(java.lang.String error) {
        super(error);
    }

}

