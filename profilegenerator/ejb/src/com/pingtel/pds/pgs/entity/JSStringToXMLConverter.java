/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/JSStringToXMLConverter.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.entity;

import org.jdom.*;
import java.util.*;
import org.jdom.output.*;
import org.apache.log4j.Category;

import com.pingtel.pds.common.Base64;
import com.pingtel.pds.common.XMLSupport;

public class JSStringToXMLConverter {

    private static XMLOutputter x = new XMLOutputter();

    private Category logger = Category.getInstance( "pgs" );

    public static void main ( String [] args ) {

        String [] strings = new String [] {
            "N10192.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.REALM. ^^^ abc",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.USERID.10141 ^^^ 11",
            "N10192.USER_LINE.43.USER_LINE~EMPTY.REGISTRATION. ^^^ PROVISION",
            "N10192.USER_LINE.43.USER_LINE~EMPTY.URL.i_true. ^^^ y%20y%3Cyyy@abc%3E",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.USER_LINE~EMPTY.ALLOW_FORWARDING. ^^^ ENABLE",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.PASSTOKEN.10141 ^^^ 1",
            "N10192.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.USERID. ^^^ 8",
            "null.USER_DEFAULT_OUTBOUND_LINE.42.EMPTY.USER_DEFAULT_OUTBOUND_LINE. ^^^",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.USER_LINE~EMPTY.URL.i_true. ^^^ x%20x%3Cxxx@1234%3E",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.USERID.1014 ^^^",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.REALM.10141 ^^^",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.USER_LINE~EMPTY.REGISTRATION. ^^^ REGISTER",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.PASSTOKEN.1014 ^^^ 4",
            "eCB4PHh4eEAxMjM0Pg--.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.REALM.1014 ^^^ 5555",
            "N10192.USER_LINE.43.CREDENTIAL~USER_LINE~EMPTY.PASSTOKEN. ^^^",
            "N10192.USER_LINE.43.USER_LINE~EMPTY.ALLOW_FORWARDING. ^^^ DISABLE" };

        ArrayList al = new ArrayList ();
        for ( int i = 0; i < strings.length; ++i )
            al.add( strings [ i ]);

        JSStringToXMLConverter j = new JSStringToXMLConverter ();
        j.convert( al );

    }

     public String convert ( Collection jsStrings ) {

        Element root = new Element ( "PROFILE" );

        for ( Iterator iString = jsStrings.iterator(); iString.hasNext(); ) {
            String line = (String) iString.next();

            StringTokenizer nameValueTok = new StringTokenizer ( line, "^^^" );
            String name = nameValueTok.nextToken().trim();
            String value = null;

            if ( line.length() == (line.indexOf( "^^^" ) + 3 ) ) {
                value = "";
                //continue;
            }
            else {
                value = XMLSupport.decodeUtf8( nameValueTok.nextToken().trim() );
            }

            String reorderedName = reorderName ( name );
            mergeXML ( root, reorderedName, value );

        } // for jsStrings

        logger.debug( "about to call postProcessElement" );
        postProcessElement ( root );

        String returnValue = x.outputString( root );
        //System.out.println( x.outputString( root ) );
        return returnValue;
    }


    private Element postProcessElement ( Element root ) {
        // migrate ref property ids

        logger.debug( "postProcessElement input" );
        //System.out.println( x.outputString( root ) );

        migrateRefPropertyIDs ( root );
        logger.debug( "done migrateRefPropertyIDs" );
        //System.out.println( x.outputString( root ) );

        HashMap counter = new HashMap();

        normalizeContainers ( root, counter );
        logger.debug( "done normalizeContainers" );
        //System.out.println( x.outputString( root ) );

        refactorContainers ( root );
        logger.debug( "done refactorContainers" );
        //System.out.println( x.outputString( root ) );

        migrateIDs ( root );
        logger.debug( "done migrateIDs" );
        //System.out.println ( x.outputString( root ) );

        for ( Iterator i = XMLSupport.detachableIterator(root.getChildren().iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();

            Collection grands = child.getChildren();
            for ( Iterator iGrand = XMLSupport.detachableIterator(grands.iterator()); iGrand.hasNext(); ) {
                Element grand = (Element) iGrand.next();
                root.addContent( grand.detach() );
            }
            root.removeContent( child );
        }

        logger.debug( "removed intermediate N elements" );

        return root;
    }



    private void normalizeContainers ( Element root, HashMap counter ) {
        getCounts ( root, counter );

        Collection values = counter.entrySet();
        for ( Iterator i = values.iterator(); i.hasNext(); ) {
            Map.Entry entry = (Map.Entry) i.next();
            String nodeName = (String) entry.getKey();
            if ( !nodeName.startsWith( "c_" ) )
                continue;

            Integer nodeCount = (Integer) entry.getValue();
            if ( nodeCount.intValue() > 1 ) {
                removeCommon ( root, nodeName );
            }
        }

    }


    private void getCounts ( Element root, HashMap counter ) {
        Collection children = root.getChildren();
        for ( Iterator i = XMLSupport.detachableIterator(children.iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();

            if ( counter.containsKey( child.getName() ) ) {
                 int count = ((Integer) counter.get( child.getName() )).intValue();
                 counter.put( child.getName(), new Integer ( ++count) );
            }
            else {
                counter.put( child.getName(), new Integer ( 1 ) );
            }
            getCounts ( child, counter );
        }
    }

    private void removeCommon ( Element root, String nodeName ) {
        Collection children = root.getChildren();

        for ( Iterator i = XMLSupport.detachableIterator(children.iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();
            String name = child.getName();
            if ( name.equals( nodeName ) ) {
                Element parent = (Element)child.getParent();
                Collection migrants = child.getChildren();
                for ( Iterator iMigrant = XMLSupport.detachableIterator(migrants.iterator()); iMigrant.hasNext(); ) {
                    Element migrant = (Element) iMigrant.next();
                    parent.addContent( migrant.detach());
                }
                parent.removeChild( nodeName );
            } else {
                removeCommon ( child, nodeName );
            }
        }
    }


    private void migrateRefPropertyIDs ( Element root ) {
        Collection children = root.getChildren();

        for ( Iterator i = XMLSupport.detachableIterator(children.iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();
            String name = child.getName();
            if ( name.startsWith( "rp_" ) ) {
                name = name.substring( 3, name.length() );
                Element parent = (Element)child.getParent();
                parent.setAttribute( "ref_property_id", name );
                Collection grandChildren = child.getChildren();
                for ( Iterator iGrand = XMLSupport.detachableIterator(grandChildren.iterator()); iGrand.hasNext(); ) {
                    Element grand = (Element) iGrand.next();
                    parent.addContent( grand.detach() );
                    //iGrand.remove();
                }
                parent.removeContent( child );
                break;
            } else {
                migrateRefPropertyIDs ( child );
            }
        }
    }

    private void migrateIDs ( Element root ) {
        Collection children = root.getChildren();

        for ( Iterator i = XMLSupport.detachableIterator(children.iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();
            String name = child.getName();
            if ( name.equalsIgnoreCase( "i_true" ) ) {
                Element parent = (Element)child.getParent();
                Element grand = (Element)parent.getParent();
                Element greatGrand = (Element)grand.getParent();
                String idValue = child.getText();
                //greatGrand.setAttribute( "id", String.valueOf( idValue.hashCode() ) );

                byte [] replace = idValue.getBytes();
                StringBuffer target = new StringBuffer();
                for ( int count = 0; count < replace.length; ++count )
                    target.append( Integer.toHexString( (int) replace[count] ) );

                //String encoded64 = Base64.encodeString( idValue );
                //greatGrand.setAttribute( "id", encoded64 );


                greatGrand.setAttribute( "id", target.toString() );
                parent.removeChild( child.getName() );
                parent.addContent( new CDATA ( idValue ) );
            }

            migrateIDs ( child );
        }
    }


    private void refactorContainers ( Element root ) {
        Collection children = root.getChildren();

        for ( Iterator i = XMLSupport.detachableIterator(children.iterator()); i.hasNext(); ) {
            Element child = (Element) i.next();

            String name = child.getName();
            if ( name.startsWith( "c_" ) && ( child.getParent().getParent() != null ) ) {
                Element parent = (Element)child.getParent();
                Element grandParent = (Element)parent.getParent();
                Element sibling = new Element ( parent.getName() );
                grandParent.addContent( sibling );

                for ( Iterator iMigrate = XMLSupport.detachableIterator(child.getChildren().iterator()); 
                        iMigrate.hasNext(); ) {

                    Element migrator = (Element) iMigrate.next();
                    sibling.addContent( migrator.detach() );
                    //iMigrate.remove();
                }

                parent.removeContent( child );

                if ( parent.getChildren().size() == 0 ) {
                    grandParent.removeContent( root );
                }
            }

            refactorContainers ( child );
        }

    }




    private String reorderName ( String name ) {

        StringBuffer reorderedName = new StringBuffer();
        StringTokenizer nameTok = new StringTokenizer ( name, "." );
        ArrayList tokenList = new ArrayList ();
        int counter = 0;

        while ( nameTok.hasMoreTokens() ) {
            ++counter;
            String token = nameTok.nextToken();

            if ( token.indexOf( "~" ) != -1 ) {
                StringTokenizer pathTok = new StringTokenizer ( token, "~" );
                ArrayList pathAL = new ArrayList ();
                while ( pathTok.hasMoreTokens() )
                    pathAL.add( pathTok.nextToken());

                Collections.reverse( pathAL );

                for ( Iterator i = pathAL.iterator(); i.hasNext(); ) {

                    String pathElement = (String) i.next();
                    if ( !pathElement.equals( "EMPTY" ) )
                        tokenList.add( pathElement );
                }
            }
            else if ( token.equalsIgnoreCase( "EMPTY" ) ) {
                continue;
            }
            else {
                if ( counter == 3 )
                    token = "rp_" + token;

                tokenList.add( token );
            }
        }

        String lastToken = (String) tokenList.get( tokenList.size() -1 );
        if ( isNumeric ( lastToken ) ) {
            String nextToLast = (String) tokenList.get( tokenList.size() -2 );
            tokenList.remove( tokenList.size() -2 );
            tokenList.add( nextToLast ) ;
        }

        for ( Iterator i = tokenList.iterator(); i.hasNext(); )
            reorderedName.append( ( (String) i.next() ) ).append( '.' );

        return reorderedName.toString();
    }



    private void mergeXML ( Element root, String property, String value ) {

        Element current = root;
        StringTokenizer propertyTokens = new StringTokenizer ( property, "." );
        while ( propertyTokens.hasMoreTokens() ) {
            String token = propertyTokens.nextToken();
            Element child = current.getChild( produceValidElementName ( token ) );
            if ( child == null ) {
                Element newElement = new Element ( produceValidElementName ( token ) );
                if ( !propertyTokens.hasMoreTokens() )
                    newElement.addContent( new CDATA ( value ) );

                current.addContent( newElement );
                current = newElement;
            } else {
                current = child;
            }
        }
    }



    private String produceValidElementName ( String name ) {
        if ( isNumeric ( name ) )
            name = "c_" + name;

        return name;
    }

    private boolean isNumeric ( String token ) {
        if (    token.startsWith( "0") || token.startsWith( "1") ||
                token.startsWith( "2") || token.startsWith( "3") ||
                token.startsWith( "4") || token.startsWith( "5") ||
                token.startsWith( "6") || token.startsWith( "7") ||
                token.startsWith( "8") || token.startsWith( "9") ) {
            return true;
        } else {
            return false;
        }
    }



}
