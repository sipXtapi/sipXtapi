package com.pingtel.pds.common.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * Created by IntelliJ IDEA.
 * User: ibutcher
 * Date: Mar 12, 2003
 * Time: 3:28:58 PM
 * To change this template use Options | File Templates.
 */
public interface TestRMIInterface extends Remote {

    public String echo () throws RemoteException;
    public String echo ( String message ) throws RemoteException;
    public String echo ( String message, Integer count ) throws RemoteException;
}
