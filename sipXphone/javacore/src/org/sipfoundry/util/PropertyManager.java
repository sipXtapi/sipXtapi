/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util ;

import java.io.* ;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.Repository;
import org.sipfoundry.sipxphone.Application;

/**
 * Todo:
 * The PropertyManager needs to cache the Configurable and Roamable as well
 * as the Retainable properties locally in the case of power failure.
 *
 * Need to add an addListener interface for changes to the properties.
 * Need to add a method for an app to get a property, if available.
 * If it's not available, then the app can create a new bean.
 *
 * Do we need to define an OwnedBean that you can then add listeners to?
 */

public class PropertyManager
{

    /**
     * This is the suffix used for the OwnedBean name to denote the
     * version number. Eventually, we will want to overload
     * methods to allow the application programmer to get at different
     * versions of their serialized beans. Right now, we are just using
     * the (1) suffix to make the name unique.
     */
    private static String v1 = "(1)";

    /**
     * m_apps is used to keep track of all of the application's OwnedBeans.
     * The key is the app name, the value is the Hashtable of beans that are owned
     * by the app.
     */
    private Hashtable m_apps;

    /**
     * m_wholeBeanListeners is used to keep track of what listeners are
     * listening for what beans. The key is class name of the bean, the
     * value is a vector of listeners for the bean.
     */
    private Hashtable m_wholeBeanListeners;

    /**
     * Default constructor for singleton.
     */
    protected PropertyManager()
    {
        // Create the member variables to store stuff
        m_apps = new Hashtable();
        m_wholeBeanListeners = new Hashtable();
    }

    /**
     * Used to store the singleton for this class
     */
    protected static PropertyManager m_reference = null;

    /**
     * Method used to get reference to the singleton for this class
     */
    public static PropertyManager getInstance()
    {
        if (m_reference == null)
            m_reference = new PropertyManager();

        return m_reference;
    }

    /**
     * This method is used by an application to tell the PropertyManager
     * that the OwnedBean has changed and that it should be saved.
     * Any listeners that are listening for this OwnedBean will get
     * notified of any changes as a side effect.
     *
     * @param ownedBean The OwnedBean that changed.
     */
    public synchronized void ownedBeanChanged(Object ownedBean)
        throws IOException
    {
        String beanClassName = ownedBean.getClass().getName() + v1;
        Repository r = Repository.getInstance();

        // Save the new bean in the repository
        r.store(beanClassName, ownedBean);

        // Now go through the list of listeners for this bean, if there
        // are any, and notify them of the change.

        Vector listenerList = (Vector) m_wholeBeanListeners.get((Object) beanClassName);
        if (listenerList != null)
        {
            Enumeration listenerEnum = listenerList.elements();

            PropertyChangeEvent changeEvent = new PropertyChangeEvent(ownedBean, "", null, ownedBean);

            while (listenerEnum.hasMoreElements())
            {
                PropertyChangeListener listener = (PropertyChangeListener) listenerEnum.nextElement();
                listener.propertyChange(changeEvent);
            }
        }
    }


    /**
     */
    public synchronized void clearOwnedBean(Object ownedBean)
        throws IOException
    {
        String beanClassName = ownedBean.getClass().getName() + v1;
        Repository r = Repository.getInstance();

        // Save the new bean in the repository
        r.remove(beanClassName) ;

        // Now go through the list of listeners for this bean, if there
        // are any, and notify them of the change.

        Vector listenerList = (Vector) m_wholeBeanListeners.get((Object) beanClassName);
        if (listenerList != null)
        {
            Enumeration listenerEnum = listenerList.elements();

            PropertyChangeEvent changeEvent = new PropertyChangeEvent(ownedBean, "", null, ownedBean);

            while (listenerEnum.hasMoreElements())
            {
                PropertyChangeListener listener = (PropertyChangeListener) listenerEnum.nextElement();
                listener.propertyChange(changeEvent);
            }
        }
    }


    /**
     * This method is used by the ApplicationManager to deserialize
     * an application and initialize all of the member variables.
     * The ApplicationManager should call the init() and then go().
     * If the class does not exist, then this method should throw an
     * exception.
     * It should be noted that it is assumed that there is a one-to-one mapping
     * between the class name and an instance of an object of that class in the
     * repository.
     *
     * @param className This is the name of the class to use to deserialize and initialize
     * the application object.
     * @return The Application object.
     */
    public Application deserializeApplication(String className)
    {
        return null;
    }




    /**
     * This method is used to deserialize an OwnedBean.
     * If the class does not exist, then an exception is thrown.
     *
     * For example:
     * MyBean bean = PropertyManager.getInstance().deserializeOwnedBean(new MyBean());
     *
     * This will create a dummy instance of your OwnedBean. The PropertyManager will
     * look in it's list to determine if the bean already exists. If so, the bean
     * is returned. If the bean does not exist, an exception is thrown.
     *
     * @param className
     * @exception java.lang.ClassNotFoundException
     */
    public synchronized Object deserializeOwnedBean(Object ownedBean)
        throws ClassNotFoundException, IOException
    {
        String beanClassName = ownedBean.getClass().getName() + v1;
        Repository r = Repository.getInstance();

        ownedBean = r.get(beanClassName);

        if (ownedBean == null) {
            throw new ClassNotFoundException("Class does not exist");
        }

        return ownedBean;
    }


    /**
     * This method is used to serialize an application. The PropertyManager
     * sets up an OutputObjectStream in which to place the serialized
     * application. The PropertyManager then saves the object away using
     * the class name of the application as the key.
     *
     * @param application foo
     */
    public synchronized void serializeApplication(Object application)
        throws IOException
    {
        Repository r = Repository.getInstance();

        String appClassName = "app_" + application.getClass().getName();

        if (application instanceof Application)
        {
            r.store(appClassName, application);
//            r.fs();
        }
        else
        {
            System.out.println("Tried to serialize an object that was not an Application") ;
        }

        //Todo: I don't know if this is going to work. Do we have to
        //turn the object into an ObjectOutputStream, or something like that?
    }


    /**
     * This method is used by the ApplicationManager to serialize the OwnedBeans
     * associated with an application.
     *
     *
     * @param application foo
     *
     */
    public synchronized void serializeOwnedBeans(Object application) throws Exception
    {

        // I wonder if we should store the mapping between the
        // application and the beans in the repository.
        // This may be handy for debug purposes as for
        // cleaning up the turds left in the repository.

        Repository r = Repository.getInstance();

        String appClassName = application.getClass().getName();

        Hashtable appsOwnedBeans = (Hashtable) m_apps.get((Object) appClassName);

        Enumeration appsEnum = appsOwnedBeans.elements();

        while (appsEnum.hasMoreElements())
        {
            String beanClassName = application.getClass().getName() + v1;
            Object bean = appsEnum.nextElement();

            r.store(beanClassName, bean);
        }
//        r.flush();
    }


    /**
     * This method tells the PropertyManager that the OwnedBean
     * is associated with the application. When the ApplicationManager
     * serializes an Application, it also asks the PropertyManager
     * to serialize any OwnedBeans that belong to the Application.
     *
     * @param application The application that owns the OwnedBean
     * @param ownedBean The OwnedBean.
     */
    public synchronized void registerOwnedBean(Object application, Object ownedBean)
    {
        String appClassName = application.getClass().getName();
        String beanClassName = ownedBean.getClass().getName() + v1;

        Hashtable appsOwnedBeans;

        // Look to see if this app has an OwnedBeanList yet
        if (!m_apps.containsKey(appClassName))
        {
            // No key found for this app. Create an OwnedBeanList for
            // this app
            appsOwnedBeans = new Hashtable();
        }
        else
        {
            // Grab the OwnedBeanList for this app
            appsOwnedBeans = (Hashtable) m_apps.get((Object) appClassName);
        }

        appsOwnedBeans.put((Object) beanClassName, (Object) ownedBean);

        m_apps.put((Object) appClassName, (Object) appsOwnedBeans);
    }

    /**
     * This method is used to look at the contents of the PropertyManager
     */
    public String toString()
    {
        String dump = "Dump:\n";

        Enumeration appsEnum = m_apps.keys();

        // Loop through each application key
        while (appsEnum.hasMoreElements())
        {
            String appKey = (String) appsEnum.nextElement();

            dump = dump + "App: " + appKey + "\n";

            Hashtable appsOwnedBeans = (Hashtable) m_apps.get((Object) appKey);

            Enumeration ownedBeansEnum = appsOwnedBeans.keys();

            // Loop through each OwnedBean for this app
            while (ownedBeansEnum.hasMoreElements())
            {
                String beanClass = (String) ownedBeansEnum.nextElement();

                Object bean = appsOwnedBeans.get((Object) beanClass);

                dump = dump + "    bean: " + beanClass + "\n";
            }
        }

        return dump;
    }

    /**
     * This method is typically used in the constructor
     * of an app to determine if there is already an OwnedBean
     * of this class already. If it exists, the app will ask for
     * the contents. If it does not exist, then it means that
     * this is the first time that the app has run and it creates a new
     * OwnedBean based on some programmed defaults.
     */
    public synchronized Object getOwnedBean(Object ownedBean)
        throws ClassNotFoundException, IOException
    {
        Repository r = Repository.getInstance();

        String beanClassName = ownedBean.getClass().getName() + v1;

        if (r.containsKey(beanClassName))
        {
            return r.get(beanClassName);
        }
        else
        {
            throw new ClassNotFoundException("OwnedBean class not found");
        }
    }

    public synchronized boolean exists(Object ownedBean)
    {
        Repository r = Repository.getInstance();

        String beanClassName = ownedBean.getClass().getName() + v1;

        if (r.containsKey(beanClassName))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * This method is used to add a listener for changes to an OwnedBean.
     * The listener will be notified of any change to the bean through
     * the PropertyChangeEvent. The listener will get a whole new copy
     * of the bean through the event.
     * To get the new bean, use the getNewValue() method on the
     * PropertyChangeEvent.
     * Be careful not to call ownedBeanChanged() in your listener,
     * as this will result in an infinite loop.
     */
    public void addWholeBeanChangeListener(Object ownedBean, PropertyChangeListener listener)
    {
        String beanClassName = ownedBean.getClass().getName() + v1;

        Vector listenerList = (Vector) m_wholeBeanListeners.get((Object) beanClassName);

        // Create a listener list for this bean if there isn't one already
        if (listenerList == null)
        {
            listenerList = new Vector();
            m_wholeBeanListeners.put((Object)beanClassName, (Object) listenerList);
        }

        // Add this listener to the list, if it's not there already
        if (!listenerList.contains(listener))
        {
            listenerList.addElement(listener);
        }
    }

    /**
     * This method is used to remove a listener for changes to an OwnedBean.
     */
    public void removeWholeBeanChangeListener(Object ownedBean, PropertyChangeListener listener, boolean all)
    {
        String beanClassName = ownedBean.getClass().getName() + v1;

        if (all)
        {
            m_wholeBeanListeners.remove((Object) beanClassName);
        }
        else
        {
            Vector listenerList = (Vector) m_wholeBeanListeners.get((Object) beanClassName);

            if (listenerList != null)
            {
                if (!listenerList.contains(listener))
                {
                    listenerList.removeElement(listener);
                }
            }
        }
    }


    /**
     * Clears the specified key from the repository by removing it.
     *
     * @param strKey The key of the object you would like to clear
     */
    public synchronized void removeObject(String strKey)
    {
        Repository r = Repository.getInstance();

        r.remove(strKey) ;
//        r.flush() ;
    }


    /**
     * This method is used to deserialize a saved object.
     * If the key does not exist, then an exception is thrown.
     *
     * @param key
     * @exception java.lang.IllegalArgumentException
     */
    public synchronized Object getObject(String key)
        throws IllegalArgumentException, IOException
    {
        Object o = null;

        Repository r = Repository.getInstance();

        o = r.get(key);

        if (o == null) {
            throw new IllegalArgumentException("Key " + key + " not found");
        }

        return o;
    }

    /**
     * This method is used to save an object.
     *
     * @param key The key to save the object under
     * @param object The object to save
     */
    public synchronized void putObject(String key, Object object)
        throws IllegalArgumentException, IOException

    {
        Repository r = Repository.getInstance();

        if (key == null) {
            throw new IllegalArgumentException("Key cannot be null");
        }

        r.store(key, object);
//        r.flush();
    }
}
