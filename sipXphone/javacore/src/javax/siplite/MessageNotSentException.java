package javax.siplite;


/**
 * This Exception indicates that the SIP message has not been sent.
 */
public class MessageNotSentException extends Exception {

    /**
    * Constructs a new <code>MessageNotSentException</code>
    */
    public MessageNotSentException() {
    }

    /**
    * Constructs a new <code>MessageNotSentException</code> with the specified
    * error message.
    *
    * @param  error the detail of the error message
    */
    public MessageNotSentException(java.lang.String error) {
        super(error);
    }

}
