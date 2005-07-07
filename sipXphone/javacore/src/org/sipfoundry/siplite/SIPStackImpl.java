/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPStackImpl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.siplite ;

import java.util.Vector ;

import javax.siplite.SIPStack ;
import javax.siplite.CallProvider ;
import javax.siplite.Address ;
import javax.siplite.SIPStackStartedException;
import javax.siplite.OutboundProxyNotSetException;
import javax.siplite.InvalidArgumentException;
import javax.siplite.LocalRouteSetNotSetException;

import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.XpressaSettings ;
import org.sipfoundry.sip.SipUserAgent;


/**
*
* This interface is concerned with configuring the stack, the
* transport parameter set by this interface is used to determine the outgoing
* as well as the incoming transport.<br><br>
*
* Please note that an object that implements SIPStack interface:<br>
* - must be referred to as SIPStackImpl<br><br>
*
* The CallProvider object can be created by <br><br>
* <code> getCallProvider() </code><br><br>
*
* Setting the outbound proxy determines the proxy all message are routed via.
*
* When the SIPStack is created, the default values are 5060 for the
* port, "any" for the host and UDP for the transport type. If the default
* values are appropriate, the SIPStack need not be modified as it will
* automatically be set in the CallProvider. If the transport type of TCP is
* required as well as UDP, then the SIPStack can be modified.
* The constants for the transport type are available from the
* Constants class. If the port is changed, this will affect both
* transport types. Also, it is not possible to have multiple ports for one
* transport type.
*
* Pingtel has added the following extensions:<br>
*     addAllowMethod<br>
**/
public class SIPStackImpl implements SIPStack
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private CallProviderImpl m_provider ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor
     */
    public SIPStackImpl()
    {

    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Returns the instance of the CallProvider.
     * @returns the instance of the callProvider implementation
     */
    public CallProvider getCallProvider()
    {
       return CallProviderImpl.getInstance();
    }


    /**
     * Set the local route set if required. The application may wish to route
     * all messages via a sequence of proxies.
     * A proxy is concerned with the routing and redirection of the message.
     * @param	routeSet    route set of proxies
     * @throws	LocalRouteSetNotSetException	This exception is thrown if
     *                                          the local route set could not
     *                                          be set
     **/
    public void setLocalRouteSet(javax.siplite.Address[] routeSet) throws
        LocalRouteSetNotSetException
    {
      //not implementted yet.
    }

    /**
     * Start the stack.   The stack commences listening on the setPort (5060
     * default), for sethost ("any" default), with setTransport (UDP default)
     */
     public boolean startStack()
     {
       return false;
     }

     /**
     * Stop the stack;  Allows the stack to be stopped when no longer required,
     * also certain methods such as setPort cannot be called when the stack
     * is operating. All appending messages will be sent, listening will
     * immediately cease and subsequent calls to Dialog::sendMessage will fail with
     *  MessageNotSendException until the stack is re-started.
     */
    public void stopStack()
    {

    }




    /**
     * Set an outbound proxy if required. The application may wish to route
     * all messages via a proxy. The proxy is concerned with the routing and
     * redirection of the message.
     * @param	outboundProxyAddress    The outbound proxy address
     * @throws	OutboundProxyNotSetException	This exception is thrown if
     *                                          the outbound proxy could not
     *                                          be set
     **/
    public void setOutboundProxy(Address outboundProxyAddress)
        throws OutboundProxyNotSetException
    {
        throw new OutboundProxyNotSetException() ;
    }

    /**
     * Retrieve the port of for the SIPStack
    * @return the port of for the SIPStack
    **/
    public int getPort()
    {
        return Shell.getXpressaSettings().getSIPUDPPort() ;
    }

    /**
    * Retrieve the transport type for the SIPStack, e.g UDP or TCP
    * @return the transport type for the SIPStack
    **/
    public String getTransport()
    {
        return "UDP" ;
    }

    /**
    * Retreive the host for the SIPStack
    * @return the host for the SIPStack
    **/
    public String getHost()
    {
        // TODO: Fix getExternalIPAddress()
        return "localhost" ; // Shell.getXpressaSettings().getExternalIPAddress() ;
    }

    /**
    * Set the port for the SIPStack
    * @param	port 	the port for the SIPStack
    * @throws	InvalidArgumentException	if the port is negative
    * @throws   SIPStackStartedException	if the port cannot be changed once the
    *                                           SIPStack has started (implementation dependant)
    **/
    public void setPort(int port)
                   throws InvalidArgumentException,  SIPStackStartedException{
        throw new SIPStackStartedException() ;
    }

    /**
    * Set the transport type for the SIPStack, e.g. UDP or TCP
    * @param	transport the transport type for the SIPStack
    * @throws	InvalidArgumentException	if the transport type is not
    *						a constant specified in the
    *						Constants class
    * @throws   SIPStackStartedException	if the transport cannot be changed once the
    *                                           SIPStack has started (implementation dependant)
    **/
    public void setTransport(String transport)
                    throws InvalidArgumentException,  SIPStackStartedException{
        throw new SIPStackStartedException() ;
    }

    /**
    * Set the host for the SIPStack
    * @param	host 	the host for the SIPStack
    * @throws	InvalidArgumentException	if the host is null or of zero length
    * @throws   SIPStackStartedException	if the host cannot be changed once the
    *                                           SIPStack has started (implementation dependant)
    **/
    public void setHost(String host)
                    throws InvalidArgumentException,  SIPStackStartedException{
        throw new SIPStackStartedException() ;
    }


    /**
     * Adds the method to the list of methods that are allowed by the
     * underlying SipUserAgent.
     * <br><br>
     * PINGTEL EXTENSION
     *
     * @param strMethod SIP method to be added eg "INFO"
     */
    public void addAllowMethod(String strMethod)
    {
        SipUserAgent.getInstance().addAllowMethod(strMethod);
    }

} // end of class SIPStack
