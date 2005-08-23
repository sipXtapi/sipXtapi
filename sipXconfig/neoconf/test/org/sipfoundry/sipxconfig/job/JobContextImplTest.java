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
package org.sipfoundry.sipxconfig.job;

import java.io.Serializable;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

public class JobContextImplTest extends TestCase {

    public void testSchedule() {
        JobContext context = new JobContextImpl();
        assertEquals(0, context.getJobs().size());

        Serializable jobId = context.schedule("abc");
        assertEquals(1, context.getJobs().size());

        Job job = (Job) context.getJobs().get(0);
        assertEquals("abc", job.getName());

        assertEquals(JobStatus.SCHEDULED, job.getStatus());

        context.start(jobId);
        job = (Job) context.getJobs().get(0);
        assertEquals(JobStatus.IN_PROGRESS, job.getStatus());

        Date during = new Date();

        context.success(jobId);
        job = (Job) context.getJobs().get(0);
        assertEquals(JobStatus.COMPLETED, job.getStatus());

        assertFalse(job.getStart().after(during));
        assertFalse(job.getStop().before(during));

        assertEquals(0, job.getErrorMsg().length());
    }

    public void testRemoveCompleted() {
        JobContext context = new JobContextImpl();
        Serializable[] jobIds = new Serializable[4];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = context.schedule("test" + i);
            context.start(jobIds[i]);
        }

        context.success(jobIds[2]);
        context.failure(jobIds[3], null, null);

        int removed = context.removeCompleted();
        
        assertEquals(1, removed);
        
        List jobs = context.getJobs();
        assertEquals(3, jobs.size());

        for (Iterator i = jobs.iterator(); i.hasNext();) {
            Job job = (Job) i.next();
            assertTrue(job.getName().startsWith("test"));
            assertFalse(job.getName().endsWith("2"));
        }
    }

    public void testClear() {
        JobContext context = new JobContextImpl();
        Serializable[] jobIds = new Serializable[4];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = context.schedule("test" + i);
            context.start(jobIds[i]);
        }

        context.success(jobIds[2]);
        context.failure(jobIds[3], null, null);

        List jobs = context.getJobs();
        context.clear();
        // clear does not affect retrieved jobs

        assertEquals(4, jobs.size());
        jobs = context.getJobs();
        // but the list should be empty next time we retrieve it
        assertEquals(0, jobs.size());
    }

    public void testNullId() {
        // it should be OK to call most of context functions with null id
        JobContext context = new JobContextImpl();
        context.start(null);
        context.success(null);
        context.failure(null, null, null);
    }

    public void testFull() {
        JobContext context = new JobContextImpl();
        Serializable first = null;
        for (int i = 0; i < JobContextImpl.MAX_JOBS; i++) {
            Serializable id = context.schedule("job" + i);
            assertNotNull(id);
            context.start(id);
            if(i == 0) {
                first = id;
            }
        }

        assertEquals(JobContextImpl.MAX_JOBS, context.getJobs().size());
        assertTrue(context.getJobs().contains(first));
        
        // it should be OK now
        assertNotNull(context.schedule("extra job"));
        
        // but the queue do not grow
        assertEquals(JobContextImpl.MAX_JOBS, context.getJobs().size());
        assertFalse(context.getJobs().contains(first));
        
        // but it still should be OK to call success (or failure)
        context.success(first);        
    }
}
