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


package org.sipfoundry.sipxphone.awt ;

import java.awt.* ;
import java.util.Vector;
import java.util.Hashtable;
import java.util.StringTokenizer;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;

/**
 * Default renderer for displaying a SIP URL in a user friendly way.  This
 * code should 'talk' to whatever resources possible to complete this task.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DefaultSIPAddressRenderer extends PDefaultItemRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    private static final String ANONYMOUS =  "Anonymous";

    /** callerid large font used to display display name */
    public static final Font FONT_CALLERID_LARGE =
        SystemDefaults.getFont(SystemDefaults.FONTID_CALLERID_LARGE) ;

    /** callerid small font used to display address  */
    public static final Font FONT_CALLERID_SMALL =
        SystemDefaults.getFont(SystemDefaults.FONTID_CALLERID_SMALL) ;

      /** default large font used to display display name */
    public static final Font FONT_DEFAULT_LARGE =
        SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE) ;

    /** default small font used to display address */
    public static final Font FONT_DEFAULT_SMALL =
        SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL) ;

    protected static final int DEFAULT_HASHTABLE_SIZE = 5;

    /** hashtable to hold the keys and subsitution values used for showing caller
     *  id information.
     */
    private Hashtable m_hashSubstitutions ;

    /** user identity that needs to be parsed to display callerid info.*/
    private String m_strIdentity;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public DefaultSIPAddressRenderer()
    {
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * renderer your data onto the passed graphics context
     * overridden from PDefaultItemRenderer.
     */
    public void drawItemText(Graphics g)
    {
        Object objData = super.getData();
        if (objData != null)
        {
            String strSIPURL = (String) objData.toString() ;
            String strFirstLine = getDisplayText(strSIPURL, 1);

            Dimension dimSize = getSize() ;
            int xOffset ;
            int yOffset ;
            int iLargeHeight = 0 ;

            g.setColor(m_colorText) ;

            /*
             * Display Name
             */
            g.setFont(FONT_CALLERID_LARGE) ;

            xOffset = calcXOffset(g, strFirstLine, FONT_CALLERID_LARGE, WEST) ;
            yOffset = calcYOffset(g, strFirstLine, FONT_CALLERID_LARGE, NORTH) ;
            FontMetrics fm = g.getFontMetrics(g.getFont()) ;
            //make sure it fits in the width provided.
            String strDisplayFirstLine = TextUtils.truncateToWidth(strFirstLine, fm, dimSize.width) ;
            g.drawString(strDisplayFirstLine, xOffset, yOffset) ;

            String strSecondLine = getDisplayText(strSIPURL, 2);
            g.setFont(FONT_CALLERID_SMALL) ;

            xOffset = calcXOffset(g, strSecondLine, FONT_CALLERID_SMALL, WEST) ;
            yOffset = calcYOffset(g, strSecondLine, FONT_CALLERID_SMALL, SOUTH) ;

            iLargeHeight = fm.getHeight() ;
            if (yOffset > iLargeHeight)
            {
                fm = g.getFontMetrics(g.getFont()) ;
                // make sure it fits in the width provided.
                String strDisplaySecondLine =
                    TextUtils.truncateToWidth(strSecondLine, fm, getSize().width) ;
                g.drawString(strDisplaySecondLine, xOffset, yOffset) ;
            }
        }
    }


    /**
     * create a copy/clone of this renderer .
     * This class's superclass implements Cloneable and hence this method here.
     * @deprecated DO NOT EXPOSE.
     */
    public PDefaultItemRenderer createInstance()
    {
        DefaultSIPAddressRenderer renderer = null ;
        try {
            renderer = (DefaultSIPAddressRenderer) clone() ;
            renderer.setAlignment(getAlignment()) ;
            renderer.handleNotToBeClonedData() ;
        } catch (CloneNotSupportedException cnse) {
            System.out.println(cnse) ;
        }
        return renderer ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * handles data that shouldn't be cloned as its superclass implements
     * Cloneable .For example, we have a hashtable instance that needs to
     * be different for different renderer objects. This method takes care
     * of creating new instances of those variables and/or making them null
     * and/or clearing them.
     */
    protected void handleNotToBeClonedData()
    {
        m_hashSubstitutions = new Hashtable(DEFAULT_HASHTABLE_SIZE);
        m_strIdentity = null;
    }


    /**
     *  Gets the callerid information to be displayed on the line specified
     *  using the pattern specified for that particular line number.
     *
     *  More info:<br>
     *  The parameter PHONESET_REMOTE_ID_FORMAT controls the default information
     *  displayed to the local users for remote caller-id and called-id.
     *  This parameter is also consulted when displaying held call information,
     *  conference party information, and call log information.
     *
     *  The values for the caller-id are case insensitive.  The default
     *  value is:
     *      {name}\n{sipurl}
     *
     *  A "\n" is used to mark the difference between the first line of text
     *  (larger font) and the second line of text (smaller font).  Only a
     *  single "\n" is expected within the string.  If more are found, they
     *  are ignored.
     *  In the conference, held call and call log forms, only the first line
     *  of text is displayed (data before first "\n").
     *
     *  NOTE: If the display name is "anonymous" (case insensitive), then
     *         {sipurl}, {username}, and {hostname} resolve to empty strings.
     *
     *   @param strIdentity identity to parse according to the pattern for the
     *                      line number specified.
     *   @param iTextLineNumber line number is used to find the pattern used for
     *                      displaying callerid information. The only values
     *                      accepted are 1 and 2.
     */
    public String getDisplayText( String strIdentity,  int iTextLineNumber )
    {
        if( iTextLineNumber<1 || iTextLineNumber>2 )
        {
            throw new IllegalArgumentException
                ("TextLineNumber is " + iTextLineNumber +".It can only be 1 or 2 ");
        }
        Vector vCallerIDPatterns = Shell.getXpressaSettings().getCallerIDPatterns();
        //if the identity has no @ symbol or sip: in it,
        //I think the parser treats it as a host.
        //For display purpose, we are treating it as username.
        if(   strIdentity != null
            && strIdentity.indexOf("sip:") == -1
            && strIdentity.indexOf("@")    == -1  )
        {
            strIdentity += "@";
        }
        refreshHashtableOfSubstitutions( strIdentity );
    //System.out.println("************************** hashSubs for [" + strIdentity + "] is " + m_hashSubstitutions );
        if( vCallerIDPatterns.size() < iTextLineNumber )
          return "";

        String strPattern = (String) (vCallerIDPatterns.elementAt(iTextLineNumber-1));
        StringBuffer strTranslationBuf = new StringBuffer();
        StringTokenizer tokenizer = new StringTokenizer(strPattern, "{");
        if( !tokenizer.hasMoreTokens() )
            strTranslationBuf.append( strPattern );
        while( tokenizer.hasMoreTokens() )
        {
            String strToken = tokenizer.nextToken();

            int iIndexOfClosingBrace = strToken.indexOf('}');
            if( iIndexOfClosingBrace == -1 )
            {
                strTranslationBuf.append( strToken );
            }
            else
            {
                String strPatternKey = strToken.substring(0, iIndexOfClosingBrace);
                String lowerCasePatternKey = strPatternKey.toLowerCase();
                if( m_hashSubstitutions.containsKey(lowerCasePatternKey) )
                {
                    strTranslationBuf.append
                        ( m_hashSubstitutions.get(lowerCasePatternKey));
                }
                if( strToken.length() >= iIndexOfClosingBrace+1 )
                    strTranslationBuf.append( strToken.substring(iIndexOfClosingBrace+1) );
            }

        }
        return strTranslationBuf.toString();
    }

    /**
    * populates hashtable for the substitution strings that are supported.
    * If we add support for more substitution strings in the future, we just
    * need to change this method.
    * More info:<br>
    *  The values for the caller-id are case insensitive.
    * Options include:
    * {displayname} : Remote party's display name.  If no display name is
    *                 provided, then "Unknown" is displayed.
    * {sipurl}      : SIP URL in the format {username}@{hostname}
    * {username}    : Remote party's user name.
    * {hostname}    : Remote party's hostname.
    * {name}        : Remote party's displayname or username.  The
                       displayname is used if available, otherwise the
                       username is presented. If the username is not available,
                       "Unknown" is used.
     *
     *
     *
     *  NOTE: If the display name is "anonymous" (case insensitive), then
     *     {sipurl}, {username}, and {hostname} resolve to empty strings.
     *
     */
    protected void refreshHashtableOfSubstitutions( String strIdentity )
    {
        boolean bNoDisplayName = false;

        if( m_hashSubstitutions == null )
            m_hashSubstitutions = new Hashtable(DEFAULT_HASHTABLE_SIZE);

        if(   (m_strIdentity == null)             ||
             !(m_strIdentity.equals(strIdentity)) ||
               m_hashSubstitutions.isEmpty()            )
        {
            m_strIdentity = strIdentity;
            SipParser sipParser = new SipParser( strIdentity );

            String strDisplayName = sipParser.getDisplayName();
            boolean bAnonymous = false;
            if( strDisplayName != null &&
                strDisplayName.equalsIgnoreCase(ANONYMOUS) )
            {
                bAnonymous = true;
            }

            //sip url string. just username@hostname:port
            SipParser builder = new SipParser() ;
            builder.setUser(sipParser.getUser()) ;
            builder.setHost(sipParser.getHost()) ;
            builder.setPort(sipParser.getPort()) ;
            String strSipUrl = builder.render() ;
            strSipUrl.trim() ;

            strDisplayName = stripFrontBack(strDisplayName, '"') ;
            bNoDisplayName = TextUtils.isNullOrSpace(strDisplayName) ;
            if (bNoDisplayName)
                m_hashSubstitutions.put("displayname", "Unknown") ;
            else
                m_hashSubstitutions.put("displayname", strDisplayName) ;

            strSipUrl = stripFrontBack(strSipUrl, '@') ;
            m_hashSubstitutions.put("sipurl",  bAnonymous?"":strSipUrl);
            String strUser = bAnonymous?"":sipParser.getUser();

            m_hashSubstitutions.put("username", strUser);
            m_hashSubstitutions.put("hostname", bAnonymous?"":sipParser.getHost());

            //if no displayname, username is used
            //if no username, "Unknown" is used.
            m_hashSubstitutions.put("name",
                TextUtils.isNullOrSpace(strDisplayName)?
                    (TextUtils.isNullOrSpace(strUser)?"Unknown":strUser):strDisplayName);
        }
    }


    protected String stripFrontBack(String original, char ch)
    {
        String strCh = "" + ch ;

        // Beginning characters
        while (original.startsWith(strCh))
        {
            original = original.substring(1) ;
        }

        // Trailing characters
        while (original.endsWith(strCh))
        {
            original = original.substring(0, original.length()-1) ;
        }
        return original ;
    }
}
