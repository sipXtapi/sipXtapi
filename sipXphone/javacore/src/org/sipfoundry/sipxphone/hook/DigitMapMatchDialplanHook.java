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

package org.sipfoundry.sipxphone.hook ;

import java.util.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.util.* ;

import com.oroinc.text.regex.*;

/**
 * <p>NOTE: This should not be exposed</p>
 *
 * DigitMapMatchDialplanHook looks for dial strings that match a specific
 * pattern (defined by RFC2705 section 2.1.5 and 3.4).  These patterns can be
 * listed in thier pinger-config.  For example:
 * <pre>
 *   PHONESET_DIGITMAP.5xxx : sip:{digits}@10.1.1.30
 *   PHONESET_DIGITMAP.9xxx : sip:{digits}@216.91.1.169
 *   PHONESET_DIGITMAP.1555. : sip:{digits}@216.91.1.123
 *   PHONESET_DIGITMAP.[1-7]xxxx : sip:{digits}@216.91.1.171
 *</pre>
 * The portion of the key following the "PHONESET_DIGITMAP." is the digit
 * map and the data following the ":" is the address.  This hook substitutes
 * the dial string in the address wherever the "{digits}" string is found.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DigitMapMatchDialplanHook implements Hook
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our list of potential digit maps */
    private OrderedHashtable m_htDigitMaps ;
    /** our list of pre-compiled digitmaps */
    private Hashtable m_htCompiledDigitMaps ;
    /** perl5 style regex matcher */
    private PatternMatcher m_matcher ;
    /** perl5 style regex compiler */
    private PatternCompiler m_compiler ;
    /** pre-compiled {digits} search string */
    private Pattern m_patternDigits ;
    /** pre-compiled {vdigits} search string */
    private Pattern m_patternVDigits ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a Digit map match dial plan hook
     */
    public DigitMapMatchDialplanHook()
    {
        this(true) ;
    }

    /**
     * Constructs a Digit map match dial plan hook with the option of whether
     * to initialize internal data structures or not.
     */
    public DigitMapMatchDialplanHook(boolean bInitialize)
    {
        m_htDigitMaps = new OrderedHashtable() ;
        m_htCompiledDigitMaps = new Hashtable() ;
        m_matcher = new Perl5Matcher() ;
        m_compiler = new Perl5Compiler() ;

        if (bInitialize)
            initializeDigitMap() ;

        try
        {
            m_patternDigits = m_compiler.compile("\\{[dD][iI][gG][iI][tT][sS]\\}");
            m_patternVDigits = m_compiler.compile("\\{[vV][dD][iI][gG][iI][tT][sS]\\}");
        }
        catch(MalformedPatternException e)
        {
            SysLog.log(e) ;
        }

    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Hook interface method invoked as part of the hook chain.
     */
    public synchronized void hookAction(HookData data)
    {
        String       strLength ;

        if ((data != null) && (data instanceof MatchDialplanHookData)) {
            MatchDialplanHookData hookData = (MatchDialplanHookData) data ;
            String strDialString = hookData.getDialString() ;

            // Traverse all of the digit maps until we find a match
            Enumeration enum = m_htDigitMaps.getOrderedKeys() ;
            while (enum.hasMoreElements())
            {
                String strDigitMap = (String) enum.nextElement() ;
                String strAddress = (String) m_htDigitMaps.get(strDigitMap) ;

                if (doesMatch(strDigitMap, strDialString))
                {
                    String strFullAddress = buildAddress(strDialString, strDigitMap, strAddress) ;
                    if ((strDigitMap.indexOf('T') != -1) || (strDigitMap.indexOf('t') != -1))
                        hookData.setMatchState(MatchDialplanHookData.MATCH_TIMEOUT, strFullAddress) ;
                    else
                        hookData.setMatchState(MatchDialplanHookData.MATCH_SUCCESS, strFullAddress) ;
                    break ;
                }
            }
        }
    }


    /**
     * does the specified dial string match the specified digit map?
     */
    public boolean doesMatch(String strDigitMap, String strDialString)
    {
        boolean bRC = false ;

        Pattern pattern = (Pattern) m_htCompiledDigitMaps.get(strDigitMap) ;
        if (pattern != null)
        {
            if (m_matcher.matches(strDialString, pattern))
            {
                bRC = true ;
            }
        }
        return bRC ;
    }



    /**
     * Given a DigitMap and Dial String, determine the variable digit that were
     * matched but not specified or masked directly in the digitmap definition.
     *
     * For Example, digitmap=9xxx and dialstring=9123 would yield 123.
     */
    public String getMatchedDigits(String strDigitMap, String strDialString)
    {
        boolean bEscape = false ;
        boolean bWildcard = false ;
        StringBuffer results = new StringBuffer() ;
        Pattern patternSub = null ;

        String strPattern = ((Pattern) m_htCompiledDigitMaps.get(strDigitMap)).getPattern() ;
        for (int i=0,j=0; i<strPattern.length(); i++)
        {
            if (bEscape)
            {
                if (strPattern.charAt(i) == ']')
                {
                    bEscape = false ;
                }
            }
            else if (bWildcard)
            {
                if (patternSub != null)
                {
                    if (m_matcher.matches(strDialString.substring(j), patternSub))
                    {
                        bWildcard = false ;
                    }
                    else
                    {
                        if (j<strDialString.length())
                            results.append(strDialString.charAt(j++)) ;
                        i-- ;
                    }
                }
            }
            else
            {
                if (strPattern.charAt(i) == '*')
                {
                    if ((i > 0) && strPattern.charAt(i-1) != '\\')  // ignore escaped *
                    {
                        bWildcard = true ;

                        if ((i+1)<strPattern.length())
                        {
                            try
                            {
                                patternSub = m_compiler.compile(strPattern.substring(i+1));

                            }
                            catch (Exception e)
                            { }
                        }
                        else
                            results.append(strDialString.substring(j)) ;
                    }
                }
                else if (strPattern.charAt(i) == '[')
                {
                    bEscape = true ;
                    if (j<strDialString.length())
                        results.append(strDialString.charAt(j++)) ;
                }
                else
                {
                    j++ ;
                }
            }
        }
        return results.toString() ;
    }


    /**
     * Expands/Extracts a complex digitmap into various smaller digitmaps
     */
    public Vector extractDigitMaps(String strDigitMap)
    {
        Vector vRC = new Vector() ;
        String strToken ;

        StringTokenizer tokenizer = new StringTokenizer(strDigitMap, "|") ;
        while (tokenizer.hasMoreTokens()) {
            strToken = tokenizer.nextToken() ;

            // Clean
            if (strToken != null)
                strToken = simpleSubsitutation(strToken, "\\(", "") ;
            if (strToken != null)
                strToken = simpleSubsitutation(strToken, " ", "") ;
            if (strToken != null)
                strToken = simpleSubsitutation(strToken, "\t", "") ;
            if (strToken != null)
                strToken = simpleSubsitutation(strToken, "\\)", "") ;

            // If not empty...
            if ((strToken != null) && (strToken.length() > 0)) {
                vRC.addElement(strToken) ;
            }
        }

        return vRC ;
    }


    /**
     *
     * convert a MGCP digit string into a perl5 regular expression
     */
    public String buildMatchString(String strDigitMap)
    {
        // Escape Stars
        strDigitMap = simpleSubsitutation(strDigitMap, "\\*", "\\*") ;

        // Expand x to [0123456789aAbBcCdD#\*]
        strDigitMap = simpleSubsitutation(strDigitMap, "x", "[0-9ABCD#\\*]") ;

        // turn . into .*
        strDigitMap = simpleSubsitutation(strDigitMap, "\\.", "[0-9ABCD#\\*]*") ;

        return strDigitMap ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Subsitute strMatch with strSubstituation in strSource.
     */
    private String simpleSubsitutation(String strSource, String strMatch, String strSubstituation)
    {
        Substitution substituation = new Perl5Substitution(strSubstituation) ;

        String strRC = strSource ;
        Pattern pattern ;

        try {
            pattern = m_compiler.compile(strMatch);
            strRC = Util.substitute(m_matcher, pattern, substituation, strSource, Util.SUBSTITUTE_ALL) ;
        } catch(MalformedPatternException e) {
            System.out.println("Bad pattern: " + strMatch) ;
            SysLog.log(e) ;
        }

        return strRC ;
    }


    /**
     * Initialize our digit map by traversing the pinger-config file and
     * looking for a particular prefix.
     */
    private void initializeDigitMap()
    {
        OrderedHashtable configFile = PingerConfig.getInstance().getConfig();

        System.out.println("") ;
        System.out.println("Initializing Digit Map") ;
        System.out.println("") ;

        final String PHONECONFIG_DIGITMAP = "PHONESET_DIGITMAP." ;

        Enumeration configFileKeys = configFile.getOrderedKeys() ;
        while (configFileKeys.hasMoreElements()) {
            String key = (String) configFileKeys.nextElement();

            if (key.startsWith(PHONECONFIG_DIGITMAP)) {
                String strDigitMap = key.substring(PHONECONFIG_DIGITMAP.length());
                String strAddress = (String) configFile.get(key) ;

                if ((strAddress == null) || (strAddress.length() == 0)) {
                    strAddress = "{digits}" ;
                }

                Vector vExtraction = extractDigitMaps(strDigitMap) ;
                for (int i=0; i<vExtraction.size(); i++) {
                    String strMap = (String) vExtraction.elementAt(i) ;

                    String strMatchString = buildMatchString(strMap) ;
                    System.out.println("\t" + strMap + "\t" + strAddress + "\t" + strMatchString) ;
                    // m_htDigitMaps.put(strMatchString, strAddress) ;

                    String strCleanedMatchString = simpleSubsitutation(strMatchString, "T", "") ;
                    try
                    {
                        Pattern pattern = m_compiler.compile(strCleanedMatchString);
                        m_htDigitMaps.put(strDigitMap, strAddress) ;
                        m_htCompiledDigitMaps.put(strDigitMap, pattern) ;
                    }
                    catch(MalformedPatternException e)
                    {
                        System.out.println("Bad pattern: " + strDigitMap) ;
                        SysLog.log(e) ;
                    }
                }
            }
        }
        System.out.println("") ;
    }


    /**
     * Build a sip address from the passed address and dial string
     */
    private String buildAddress(String strDialString, String strDigitMap, String strAddress)
    {
        String strMatchedDigit = getMatchedDigits(strDigitMap, strDialString) ;

        Substitution substituationDigits = new Perl5Substitution(strDialString) ;
        Substitution substituationVDigits = new Perl5Substitution(strMatchedDigit) ;

        String strRC = strDialString ;
        Pattern pattern ;

        // Replace all instances of {digits} with the dial string
        strRC = Util.substitute(m_matcher, m_patternDigits, substituationDigits, strAddress, Util.SUBSTITUTE_ALL) ;
        strRC = Util.substitute(m_matcher, m_patternVDigits, substituationVDigits, strRC, Util.SUBSTITUTE_ALL) ;

        return strRC ;
    }
}
