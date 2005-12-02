/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipAgent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sip ;

import org.sipfoundry.sip.event.* ;

import java.io.IOException ;
import java.util.Hashtable ;
import java.util.Enumeration ;


/**
 * This class provides a convenience framework for defining a Sip Agent for
 * handling various Sip Requests.  Developers using this framework can leverage
 * most of the SipUserAgent filtering constraints and some additional header
 * filters.
 *
 * @author Robert J. Andreasen, Jr.
 */
abstract public class SipAgent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** The method we should filter against */
    protected String m_strMethod ;
    /** The SipUserAgent listener instance */
    protected icSipMessageListener m_listener ;
    /** List of additional header contains filter restrictions */
    protected Hashtable m_htContainFilter ;
    /** List of additional header matches filter restrictions */
    protected Hashtable m_htMatchFilter ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Constructor requiring a non-null method id.  This agent will only
     * process Sip message of this method type.
     *
     * @param strMethod The type of SIP message that this agent can process.
     */
    public SipAgent(String strMethod)
    {
        m_listener = null ;
        m_htContainFilter = null ;
        m_htMatchFilter = null ;

        m_strMethod = strMethod ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Once the Sip Agent is started, the handleMessage method will be invoked
     * whenever a new SIP request arrives that mets the various filter
     * constraints.
     *
     * @exception IllegalStateException Thrown if the Sip Agent is already
     *            started.
     */
    public synchronized void start()
    {
        if (m_listener == null)
        {
            SipUserAgent agent = SipUserAgent.getInstance() ;

            // Build filter criteria
            SipMessageFilterCriteria criteria = new SipMessageFilterCriteria(
                    SipMessageFilterCriteria.REQUEST) ;
            criteria.restrictMethod(m_strMethod);

            // Create Listener
            m_listener = new icSipMessageListener() ;

            // Add Listener
            agent.addIncomingMessageListener(criteria, m_listener);
        }
        else
        {
            throw new IllegalStateException("agent is already running") ;
        }
    }


    /**
     * Stops the Sip Agent; once stopped, the handleMessage will not longer be
     * invoked with Sip Requests.
     *
     * @exception IllegalStateException Thrown if the Sip Agent is not started.
     */
    public synchronized void stop()
    {
        if (m_listener != null)
        {
            SipUserAgent agent = SipUserAgent.getInstance() ;
            agent.removeIncomingMessageListener(m_listener);
            m_listener = null ;
        }
        else
        {
            throw new IllegalStateException("agent is not running") ;
        }
    }


    /**
     * Restricts the Sip Requests processed by this agent to requests that
     * contain a specific header and value.  For Request to be processed, the
     * header value must match the strValue exactly (ignoring case).  This
     * method must be invoked before the agent is started.
     *
     * @param strHeader A header value within the Sip Message
     * @param strValue The value for the specified header
     *
     * @exception IllegalArgumentException Thrown if the designated header or
     *            value is null.
     * @exception IllegalStateException Thrown if the agent has already been
     *            started.
     */
    public synchronized void addMatchHeaderFilter(String strHeader,
                                                  String strValue)
    {
        // Validate Parameters
        if (strHeader == null)
            throw new IllegalArgumentException("header cannot be null") ;
        if (strValue == null)
            throw new IllegalArgumentException("value cannot be null") ;

        // Validate State
        if (m_listener != null) {
            throw new IllegalStateException
                    ("cannot set restrictions after starting agent") ;
        }

        // Lazily create match filter DB
        if (m_htMatchFilter == null)
            m_htMatchFilter = new Hashtable() ;

        // Add to DB
        m_htMatchFilter.put(strHeader, strValue) ;
    }


    /**
     * Restricts the Sip Requests processed by this agent to requests that
     * contain a specific header and value.  For Request to be processed, the
     * header value must match the strValue or be a subset of strValue.  This
     * method must be invoked before the agent is started.
     *
     * @param strHeader A header value within the Sip Message
     * @param strValue The value for the specified header
     *
     * @exception IllegalArgumentException Thrown if the designated header or
     *            value is null.
     * @exception IllegalStateException Thrown if the agent has already been
     *            started.
     */
    public synchronized void addContainsHeaderFilter(String strHeader,
                                                     String strValue)
    {
        // Validate Parameters
        if (strHeader == null)
            throw new IllegalArgumentException("header cannot be null") ;
        if (strValue == null)
            throw new IllegalArgumentException("value cannot be null") ;

        // Validate State
        if (m_listener != null) {
            throw new IllegalStateException
                    ("cannot set restrictions after starting agent") ;
        }

        // Lazily create contain filter DB
        if (m_htContainFilter == null)
            m_htContainFilter = new Hashtable() ;

        // Add to DB
        m_htContainFilter.put(strHeader, strValue) ;
    }


    /**
     * Sends a response for the supplied request.
     *
     * @param iCode The response code for the message.
     * @param strDescription Description of the Response
     * @param request the Request message should be be responded to.
     *
     * @exception SipMessageFormatException Thrown if the request is malformed.
     * @exception IOException Thrown if an error occures while physically
     *            trying to send the message.
     */
    public void sendResponse( int iCode,
                              String strDescription,
                              SipRequest request)
        throws SipMessageFormatException, IOException
    {
        SipUserAgent agent = SipUserAgent.getInstance() ;

        if (agent != null)
        {
            SipResponse response =
                    request.buildResponse(iCode, strDescription) ;
            agent.postMessage(response) ;
        }
    }


    /**
     * Sends a 200/OK response for the supplied request.
     *
     * @param request the Request message should be be responded to.
     *
     * @exception SipMessageFormatException Thrown if the request is malformed.
     * @exception IOException Thrown if an error occures while physically
     *            trying to send the message.
     */
    public void SendOKResponse(SipRequest request)
        throws SipMessageFormatException, IOException
    {
        sendResponse(200, "OK", request) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * This method must be implemented by Agent developers to handle SIP
     * messages (generally requests)
     */
    protected abstract void handleMessage(SipMessage message) ;


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner clasess
////
    /**
     * This class implements the actual SipUserAgent Sip Message Listener,
     * provides additional filtering, and invoked handleMessage if appropriate.
     */
    public class icSipMessageListener implements NewSipMessageListener
    {
        /**
         * NewSipMessageListener interface that is invoked whenever a new
         * SIP Message arrives that matches our filter constraints.
         */
        public void newMessage(NewSipMessageEvent event)
        {
            SipMessage msg = event.getMessage() ;

            if (satisfiesMatchesConstraints(msg) &&
                    satisfiesContainsConstraints(msg))
            {
                handleMessage(msg) ;
            }
        }


        /**
         * Checks to see if all of the matches constraints are satisified for
         * the supplied SipMessage.
         *
         * @return true if satisified otherwise false
         */
        protected boolean satisfiesMatchesConstraints(SipMessage msg)
        {
            boolean bMatches = true ;

            // Provide Additional Filter: Matches
            if (m_htMatchFilter != null)
            {
                Enumeration enumKeys = m_htMatchFilter.keys() ;
                while (enumKeys.hasMoreElements())
                {
                    String strKey = (String) enumKeys.nextElement() ;
                    if (strKey != null)
                    {
                        String strMatchValue = (String) m_htMatchFilter.get(strKey) ;
                        String strHeaderValue = msg.getHeaderFieldValue(strKey) ;

                        // If this does not match, note and kickout
                        if (!strMatchValue.equalsIgnoreCase(strHeaderValue))
                        {
                            bMatches = false ;
                            break ;
                        }
                    }
                }
            }

            return bMatches ;
        }


        /**
         * Checks to see if all of the contains constraints are satisified for
         * the supplied SipMessage.
         *
         * @return true if satisified otherwise false
         */
        protected boolean satisfiesContainsConstraints(SipMessage msg)
        {
            boolean bContains = true ;

            // Provide Additional Filter: Contains
            if (m_htContainFilter != null)
            {
                Enumeration enumKeys = m_htContainFilter.keys() ;
                while (enumKeys.hasMoreElements())
                {
                    String strKey = (String) enumKeys.nextElement() ;
                    if (strKey != null)
                    {
                        String strMatchValue = (String) m_htContainFilter.get(strKey) ;
                        String strHeaderValue = msg.getHeaderFieldValue(strKey) ;

                        // Safely Convert to lowercase
                        if (strMatchValue != null)
                            strMatchValue = strMatchValue.toLowerCase() ;
                        if (strHeaderValue != null)
                            strHeaderValue = strHeaderValue.toLowerCase() ;

                        // If this does not match, note and kickout
                        if (strMatchValue.indexOf(strHeaderValue) == -1)
                        {
                            bContains = false ;
                            break ;
                        }
                    }
                }
            }
            return bContains ;
        }

    }
}
