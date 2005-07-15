package org.sipfoundry.sipxconfig.xmlrpc;

/**
 * Interface that we need to proxy
 */
interface TestFunctions {
    String multiplyTest(String test, int times);
    int calculateTest(String[] names);
}