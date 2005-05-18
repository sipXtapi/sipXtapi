package com.pingtel.pds.common.rmi;

import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;

/**
 * Created by IntelliJ IDEA.
 * User: ibutcher
 * Date: Mar 12, 2003
 * Time: 5:17:39 PM
 * To change this template use Options | File Templates.
 */
public class TestRMIClient {

    public static void main ( String [] args ) {
        try {
            System.setSecurityManager(new RMISecurityManager());
            TestRMIInterface t = (TestRMIInterface) Naming.lookup( "rmi://localhost:2001" + "/" + "test" );

            for( int i = 0; i < 10; ++i ) {
                System.out.println ( t.echo( "Hello world from client" ) );
            }
        } catch (NotBoundException e) {
            e.printStackTrace();  //To change body of catch statement use Options | File Templates.
        } catch (java.net.MalformedURLException e) {
            e.printStackTrace();  //To change body of catch statement use Options | File Templates.
        } catch (RemoteException e) {
            e.printStackTrace();  //To change body of catch statement use Options | File Templates.
        }
    }

}
