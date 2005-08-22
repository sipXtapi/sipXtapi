/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.List;

import org.apache.commons.lang.enums.Enum;

public interface SipxProcessContext {
    public static class Process extends Enum {
        public static final Process REGISTRAR = new Process("SIPRegistrar");
        public static final Process AUTH_PROXY = new Process("SIPAuthProxy");
        public static final Process STATUS = new Process("SIPStatus");
        public static final Process PROXY = new Process("SIPProxy");
        public static final Process MEDIA_SERVER = new Process("MediaServer");
        public static final Process PARK_SERVER = new Process("ParkServer");

        public Process(String name) {
            super(name);
        }

        public static List getAll() {
            return getEnumList(SipxProcessContext.Process.class);
        }
    };
    
    public static class Command extends Enum {
        public static final Command START = new Command("start");
        public static final Command STOP = new Command("stop");
        public static final Command RESTART = new Command("restart");
        public static final Command STATUS = new Command("status");

        public Command(String name) {
            super(name);
        }

        public static List getAll() {
            return getEnumList(SipxProcessContext.Command.class);
        }
    }
    
    /**
     * Return an array containing a ServiceStatus entry for each process on the first
     * server machine.  This is a first step towards providing status for all
     * server machines.
     */ 
    public ServiceStatus[] getStatus();

    /** 
     * Apply the specified command to the named services.
     * This method handles only commands that don't need output, which excludes the
     * "status" command.
     */
    public void manageServices(String[] serviceNames, Command command);

    /** 
     * Apply the specified command to the named service.
     * This method handles only commands that don't need output, which excludes the
     * "status" command.
     */
    public void manageService(String serviceName, Command command);
    
    /** 
     * Apply the specified command to the process/service.
     * This method handles only commands that don't need output, which excludes the
     * "status" command.
     */
    public void manageService(Process process, Command command);    
}
