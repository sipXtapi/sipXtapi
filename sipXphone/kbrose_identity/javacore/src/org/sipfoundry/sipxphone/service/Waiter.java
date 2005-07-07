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
 
package org.sipfoundry.sipxphone.service;

import java.net.*;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.event.*;

/**
 * This class is used to perform an operation that may take an
 * indeterminate amount of time.
 * If the operation takes longer than specified, the Waiter will
 * return. A word of caution: be careful of what Monitors you use
 * implicitely and explicitely in the operation. If the Waiter times out,
 * there is still a thread running with the operation hanging.
 * <p>
 * You must implement a Runnable class, probably an inner class, whose
 * run() method performs the desired operation. The operation is performed
 * as a side effect of the constructor of the Waiter. Example code:
 * <pre>
 * class icMyRunnable implements Runnable
 * {
 *     private String m_a;
 *
 *     // Contructor for the inner class
 *     icMyRunnable(String a)
 *     {
 *         m_a = a;
 *     }
 *
 *     public void run()
 *     {
 *         // Do stuff here with m_a
 *     }
 * }
 *
 * String a = "An object the inner class needs to do the work";
 *
 * Waiter myWaiter = new Waiter(new icMyRunnable(a), 1000);
 * if (!myWaiter.isSuccessful()) {
 *     // The operation took too long
 * }
 * </pre>
 */
public class Waiter
{
    private Thread t;
    private boolean m_success = false;

    public Waiter(Runnable runnable, int durationInMsec)
    {
        WaiterControl control = null ;
        boolean       bComplete = false ;


        if (runnable instanceof WaiterControl)
            control = (WaiterControl) runnable ;

        t = new Thread(runnable);
        t.start();


        while (!bComplete)
        {
            try
            {
                if (control != null)
                {
                    control.resetCanAbort(true) ;
                }

                t.join(durationInMsec);

                if (t.isAlive())
                {
                    if ((control == null) || control.canAbort())
                    {
                        m_success = false;
                        bComplete = true ;

                        // Make a feeble attempt to kill the thread. This doesn't
                        // seem to work for URLConnection under PersonalJava.
                        // It does seem to work for sleep under PJ.
                        // It does seem to work under 1.2.2.
                        t.interrupt();
                    }
                    else
                    {
                        System.out.println("-- Not aborting jar file download");
                    }
                } else {
                    m_success = true;
                    bComplete = true ;
                }
            } catch (InterruptedException e) {
                System.out.println("Waiter:InterruptedException " + e);
                m_success = false;
                bComplete = true ;
            }
        }
    }

    /**
     * This method is used to find out if the operation completed
     * successfully. A return value of false means that the timeout
     * occurred before the operation was completed.
     */
    public boolean isSuccessful()
    {
        return m_success;
    }

    // Used for unit test
    public static void main(String argv[])
    {


        System.out.println("Oh Waiter!");
        Waiter myWaiter = new Waiter(new icMyRunnable(), 10*1000);
        System.out.println("Waiter returned");

        if (!myWaiter.isSuccessful())
        {
            System.out.println("Waiter: not successful");
        }
    }


}

   //inner class used by main method for unit testing.
    class icMyRunnable implements Runnable {
            public void run()
            {
                URL url = null;
                try {
                    url = new URL("http://10.1.1.245/foo.jar");
                } catch (Exception e) {
                    System.out.println(e);
                }

                try {
                    System.out.println("Connecting...");
                    URLConnection connection = url.openConnection();
                    System.out.println("connect");
                    connection.connect();
                    System.out.println("Connected");
                } catch (Exception e) {
                    System.out.println("icMyRunnable: exception during connect " + e);
                }
            }
    }
