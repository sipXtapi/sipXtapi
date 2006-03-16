/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.IOException;

import org.apache.log4j.FileAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.helpers.LogLog;

import sun.misc.Signal;
import sun.misc.SignalHandler;

/**
 * logrotate on linux systems simply moves the log from under an application.
 * Loggers continue to write to phantom log file, thereby losing log entries
 * and consuming phantom disk space until application is reset and file handle
 * is reclaimed by OS w/file references goes to zero.
 *   
 * Luckily logrotate can send signals to an application after it has rotated
 * the log and so we take advantage of that here.
 */
public class SystemLogRotateAppender extends FileAppender {

    private String m_signal = "USR2";

    public SystemLogRotateAppender() {
        super();
        addSignalHandler();
    }

    public SystemLogRotateAppender(Layout layout, String filename, boolean append,
            boolean bufferedIO, int bufferSize, String signal) throws IOException {
        super(layout, filename, append, bufferedIO, bufferSize);
        m_signal = signal;
        addSignalHandler();
    }

    /**
     * Default signal is USR2 or 12, see kill -l
     * I tried HUP and USR1 on Sun's JVM 1.4 and 1.5 JVMs and IBM's 1.5 JVM and the 
     * default action is to bring down JVM.
     * 
     * NOTE: JVM arg "-Xrs" seems to disable this from working
     * 
     * @param signal See POSIX signal codes, Default is USR2 
     */
    public SystemLogRotateAppender(Layout layout, String filename, boolean append,
            boolean bufferedIO, int bufferSize) throws IOException {
        super(layout, filename, append, bufferedIO, bufferSize);
        addSignalHandler();
    }

    public SystemLogRotateAppender(Layout layout, String filename, boolean append)
        throws IOException {
        super(layout, filename, append);
        addSignalHandler();
    }

    public SystemLogRotateAppender(Layout layout, String filename) throws IOException {
        super(layout, filename);
        addSignalHandler();
    }

    private void addSignalHandler() {
        LogLog.warn("Registered log rotate log on signal:" + m_signal);
        Signal.handle(new Signal(m_signal), new RefreshLogOnHupSignal());
    }

    void rotate() {
        activateOptions();
    }

    class RefreshLogOnHupSignal implements SignalHandler {
        public void handle(Signal arg0) {
            // in production env., good chance this log is never seen because logrotate
            // has already moved file
            LogLog.warn("Caught signal:" + m_signal + ", about to rotate log...");
            rotate();

            // in production env., this should be in log
            LogLog.warn("...log rotated");
        }
    }
}
