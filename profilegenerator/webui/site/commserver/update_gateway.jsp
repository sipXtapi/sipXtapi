<%--
 -
 - Copyright (c) 2003 Pingtel Corp.  (work in progress)
 -
 - This is an unpublished work containing Pingtel Corporation's confidential
 - and proprietary information.  Disclosure, use or reproduction without
 - written authorization of Pingtel Corp. is prohibited.
 -
 - 
 - 
 - Author:
 --%>
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<%@ page import="java.util.*" %>
<jsp:useBean id="updategwbean" class="com.pingtel.commserver.beans.ProcessGatewaysBean" />

<%
    // define Hashtables
    Hashtable gwht = new Hashtable();
    Hashtable gwaltht = new Hashtable();
    Hashtable gwqht = new Hashtable();
    Hashtable gwlabelht = new Hashtable();
    Hashtable gwseqht = new Hashtable();
    Hashtable egwht = new Hashtable();
    Hashtable egwaltht = new Hashtable();
    Hashtable egwqht = new Hashtable();
    Hashtable egwlabelht = new Hashtable();
    Hashtable egwseqht = new Hashtable();

    Enumeration enum = request.getParameterNames();
    String paramName = null;

    // Iterate through parameters, and place them in their appropriate
    // Hashtables.
    while ( enum.hasMoreElements() ) {
        paramName = enum.nextElement().toString();

        if ( paramName.indexOf( "egw" ) != -1 &&
                paramName.indexOf( "_alt" ) != -1 ) {
            egwaltht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "egw" ) != -1 &&
                paramName.indexOf( "_q" ) != -1 ) {
            egwqht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "egw" ) != -1 &&
                paramName.indexOf( "_label" ) != -1 ) {
            egwlabelht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "egw" ) != -1 &&
                paramName.indexOf( "_seq" ) != -1 ) {
            egwseqht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "gw" ) != -1 &&
                paramName.indexOf( "_alt" ) != -1 ){
            gwaltht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "gw" ) != -1 &&
                paramName.indexOf( "_label" ) != -1 ){
            gwlabelht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "gw" ) != -1 &&
                paramName.indexOf( "_seq" ) != -1 ){
            gwseqht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "gw" ) != -1 &&
                paramName.indexOf( "_q" ) != -1 ){
            gwqht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "egw" ) != -1 &&
                paramName.indexOf( "_alt" ) == -1 &&
                paramName.indexOf( "_q" ) == -1 ) {
            egwht.put( paramName, request.getParameter( paramName ) );
        }
        else if ( paramName.indexOf( "gw" ) != -1 &&
                paramName.indexOf( "_alt" ) == -1 &&
                paramName.indexOf( "_q" ) == -1 ) {
            gwht.put( paramName, request.getParameter( paramName ) );
        }
    }

    // Send Hashtables to the ProcessGatewayBean
    updategwbean.setGwht( gwht );
    updategwbean.setGwaltht( gwaltht );
    updategwbean.setGwqht( gwqht );
    updategwbean.setGwlabelht( gwlabelht );
    updategwbean.setGwseqht( gwseqht );
    updategwbean.setEgwht( egwht );
    updategwbean.setEgwaltht( egwaltht );
    updategwbean.setEgwqht( egwqht );
    updategwbean.setEgwlabelht( egwlabelht );
    updategwbean.setEgwseqht( egwseqht );

    // update the destinations.xml file
    updategwbean.updateXML();

    // redirect back to the web form
    response.sendRedirect( "gateway_details.jsp" );

    // commented code is used for testing result comming back
    // from hashtables stored in the Bean
    /*Hashtable results = updategwbean.getGwht();
    String key = null;
    enum = results.keys();
    out.println("<b>This is using the getGwht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }

    results = updategwbean.getGwaltht();
    enum = results.keys();
    out.println("<b>This is using the getGwaltht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }

    results = updategwbean.getGwqht();
    enum = results.keys();
    out.println("<b>This is using the getGwqht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }

    results = updategwbean.getEgwht();
    enum = results.keys();
    out.println("<b>This is using the getEgwht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }

    results = updategwbean.getEgwaltht();
    enum = results.keys();
    out.println("<b>This is using the getEgwaltht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }

    results = updategwbean.getEgwqht();
    enum = results.keys();
    out.println("<b>This is using the getEgwqht() method</b><br>");
    while ( enum.hasMoreElements() ) {
        key = enum.nextElement().toString();
        out.println( key + " : " + results.get( key ) + "<br>" );
    }*/
%>
