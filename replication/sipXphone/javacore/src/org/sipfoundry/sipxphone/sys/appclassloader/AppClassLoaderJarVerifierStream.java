package org.sipfoundry.sipxphone.sys.appclassloader;


import java.io.*;
import java.util.*;
import java.util.zip.*;

import org.sipfoundry.sipxphone.sys.appclassloader.*;
import org.sipfoundry.util.SysLog;
/**
 *
 * @version 	1.23 97/06/21
 * @author	Benjamin Renaud
 * @author	Roland Schemers
 */
public class AppClassLoaderJarVerifierStream extends ZipInputStream {

    /* Are we debugging ? */
    private static final boolean debug = false;

    /* The current JAR entry */
    private ZipEntry currentEntry;

    /**
     */
    public AppClassLoaderJarVerifierStream(InputStream is) throws IOException {
	super(is);
    }

    /**
     * This method scans to see which entry we're parsing and
     * keeps various state information depending on what type of
     * file is being parsed. Files it treats specially are: <ul>
     *
     * <li>Manifest files. At any point, this stream can be queried
     * for a manifet. If it is present, a Manifest object will be
     * returned.
     *
     * <li>Block Signature file. Like with the manifest, the stream
     * can be queried at any time for all block parsed thus far.
     *
     * </ul>
     */
    public synchronized ZipEntry getNextEntry() throws IOException {

	/* Get the next entry */
	if (currentEntry != null) {
	    closeEntry();
	}
	currentEntry = super.getNextEntry();

	if (currentEntry == null) {
	    return null;
	}

	return currentEntry;
    }

    private void processData() throws ZipException, IOException {
    }

    public synchronized void closeEntry() throws ZipException, IOException {
	super.closeEntry();
	processData();
    }

    /**
     */
    public int read() throws IOException {
	int b = super.read();

	return b;
    }

    public int read(byte[] b, int off, int len) throws IOException {
	int n = super.read(b, off, len);

	return n;
    }

    static void debug(String s) {
	if (debug) {
	    System.err.println("sun.tools.jar.JarVerifierStream:::" + s);
	}
    }

    static void debug(String s, Throwable t) {
	if (debug) {
	    SysLog.log(t);
	    debug(s);
	}
    }

    // Stub function to match signature of actuall function in
    // JarVerifierStream.java.optional. Needed by AppletResourceLoader
    public Object[] getIdentities(String name) {
        return null;
    }

}
