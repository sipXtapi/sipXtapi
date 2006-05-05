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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.map.LinkedMap;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

/**
 * Metrics about registration distributions
 */
public class RegistrationMetrics {
    private List m_registrations;
    private long m_startTime;
    
    public void setRegistrations(List registrations) {
        m_registrations = registrations;        
    }
    
    public void setStartTime(long startTime) {
        m_startTime = startTime;
    }
    
    public double getLoadBalance() {
        LoadDistribution metric = new LoadDistribution();
        // decided to count expired registrations, shouldn't matter and more history
        // gives a more accurate value.
        CollectionUtils.forAllDo(m_registrations, metric);
        double loadBalance = metric.getLoadBalance();
        return loadBalance;
    }
    
    public int getActiveRegistrationCount() {
        int count = CollectionUtils.countMatches(m_registrations, new ActiveRegistrations(m_startTime));
        return count;        
    }
    
    public Collection getUniqueRegistrations() {
        UniqueRegistrations unique = new UniqueRegistrations();
        // decided to count expired registrations, shouldn't matter and more history
        // gives a more accurate value.
        CollectionUtils.forAllDo(m_registrations, unique);
        return unique.getRegistrations();
    }

    /**
     * Filter out multiple registrations for a single contact
     * 
     * @param registrations
     * @return registration list without duplicated
     */
    static class UniqueRegistrations implements Closure {        
        private LinkedMap m_contact2registration = new LinkedMap();
        
        public Collection getRegistrations() {
            return m_contact2registration.values();
        }

        public void execute(Object input) {
            RegistrationItem ri = (RegistrationItem) input;
            String contact = ri.getContact();
            RegistrationItem riOld = (RegistrationItem) m_contact2registration.get(contact);
            // replace older registrations
            if (riOld == null || ri.compareTo(riOld) > 0) {
                m_contact2registration.put(contact, ri);
            }
        }        
    }
    
    /**
     * Filter out expired registrations
     * 
     * @param registrations
     * @return registration list without duplicated
     */
    static class ActiveRegistrations implements Predicate {
        private long m_startTime;
        ActiveRegistrations(long startTime) {
            m_startTime = startTime;
        }
        
        public boolean evaluate(Object input) {
            RegistrationItem reg = (RegistrationItem) input;
            if (reg.timeToExpireAsSeconds(m_startTime) > 0) {
                return true;
            }        
            return false;
        }
    }
    
    /**
     * Calculate how many registrations originated on each regististrar 
     * 
     * @param registrations
     * @return registration list without duplicated
     */
    static class LoadDistribution implements Closure {
        
        private double m_total;

        // use int[1] to store registation counts because its a mutable and an object
        // Integer and int are not  
        private Map<String, int[]> m_distribution = new HashMap<String, int[]>();
        
        public int getRegistrationCount(String server) {
            return m_distribution.get(server)[0];
        }
        
        public void execute(Object input) {
            RegistrationItem reg = (RegistrationItem) input;
            String primary = reg.getPrimary();
            if (m_distribution.containsKey(primary)) {
                m_distribution.get(primary)[0]++;
            } else {
                m_distribution.put(primary, new int[] { 
                    1 });
            }
            m_total++;
        }
        
        public double getLoadBalance() {
            double loadBalance = 1;
            int[][] metrics = m_distribution.values().toArray(new int[0][0]);
            if (metrics.length > 0) {
                double loadBalanceInverse = 0; 
                for (int i = 0; i < metrics.length; i++) {
                    double ratioSquared = Math.pow(metrics[i][0] / m_total, 2);
                    loadBalanceInverse += ratioSquared;
                }
                loadBalance = 1 / loadBalanceInverse;
            }            
            return loadBalance;
        }
    }    
}
