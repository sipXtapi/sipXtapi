package com.pingtel.pds.common.rmi;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

/**
 * Created by IntelliJ IDEA.
 * User: ibutcher
 * Date: Mar 12, 2003
 * Time: 3:30:45 PM
 * To change this template use Options | File Templates.
 */
public class TestRMIInterfaceImpl extends UnicastRemoteObject implements TestRMIInterface {

    public static void main ( String [] args ) {
        TestRMIInterfaceImpl t = null;
        try {
            t = new TestRMIInterfaceImpl();
        } catch (RemoteException e) {
            e.printStackTrace();  //To change body of catch statement use Options | File Templates.
        }


        Thread connection = new Thread (
                new SmartServerConnection ( "rmi://localhost:2001",
                                            "test",
                                            t,
                                            "echo",
                                            null,
                                            null,
                                            //new Class[] { String.class },
                                            //new Object[] {"hello"},
                                            null ) );

        connection.start();


        while ( true) {
            System.out.println ("spinning");
            try {
                Thread.sleep(10000);
            } catch (InterruptedException e) {
                e.printStackTrace();  //To change body of catch statement use Options | File Templates.
            }
        }
    }

    public TestRMIInterfaceImpl () throws RemoteException {
        super ();
    }

    public String echo() throws RemoteException {
        return "empty echo";
    }

    public String echo(String message) throws RemoteException {
        return message;
    }

    public String echo(String message, Integer count) throws RemoteException {
        return message + " " + count;
    }
}
