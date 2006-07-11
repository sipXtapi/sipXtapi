
import org.sipfoundry.util.PingerConfig;
import java.util.Hashtable;
import org.sipfoundry.util.OrderedHashtable;
import java.lang.reflect.*;

/**
 * Pass most method calls to PingerConfig singleton to a given delegate.
 * Purpose:<ul>
 *
 * <li>This isolates unittests from loading Pinger DLL when a call is
 * made to PingerConfig</li>
 *
 * <li>Allows unitttest to supply there own configuration parameters
 * </li>
 *
 * </ul>
 */
public class PingerConfigMock extends PingerConfig
{
    private PingerConfig m_original;

    private InvocationHandler m_delegate;

    /**
     * Will make <code>PingerConfig.getInstance()</code> return this instance
     * be sure to call <code>restoreOriginal</code>. All method calls will get
     * directed to given delegate.
     */
    public PingerConfigMock(InvocationHandler delegate)
    {
        if (m_reference != null)
        {
            m_original = m_reference;
        }

        m_reference = this;
        m_delegate = delegate;
    }

    /** 
     * Allow <code>PingerConfig.getInstance()</code> non-mocked object.
     */
    public void restoreOriginal()
    {
        m_reference = m_original;
    }

    /** redirected to delegate */
    public String getValue(String value)
    {
        return (String)invoke("getValue", new Class[] {String.class}, 
            new Object[] { value} );
    }

    /** redirected to delegate */
    public OrderedHashtable getConfig()
    {
        return (OrderedHashtable)invoke("getConfig", new Class[0], new Object[0]);
    }

    /** redirected to delegate */
    public Hashtable getGroupValue(String strKey)
    {
        return (Hashtable)invoke("getGroupValue", new Class[] {String.class}, 
            new Object[] {strKey} );
    }

    /** redirected to delegate */
    public void reload()
    {
        invoke("reload", new Class[0], new Object[0]);
    }

    private Object invoke(String method, Class[] args, Object[] params)
    {
        try
        {
            Method m = getClass().getMethod(method, args);
            return m_delegate.invoke(this, m, params);
        }
        catch (NoSuchMethodException nsme)
        {
            throw new RuntimeException("No such method on mock object", nsme);
        }
        catch (SecurityException se)
        {
            throw new RuntimeException("Security restriction on mock object", se);
        }
        catch (Throwable e)
        {
            throw new RuntimeException("Fatal error on mock object", e);
        }
    }

}
