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
package org.sipfoundry.sipxconfig.common;

import java.lang.reflect.InvocationTargetException;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.collections.Transformer;

/**
 * BeanWithId - simplify implementation of the model layer
 * 
 * Hibernate advises against using object identifiers in equals and hashCode methods
 */
public class BeanWithId implements Cloneable, PrimaryKeySource {
    private static final Integer UNSAVED_ID = new Integer(-1);

    private static int s_id = 1;

    private Integer m_id;

    public BeanWithId() {
        this(UNSAVED_ID);
    }

    public BeanWithId(Integer id) {
        setId(id);
    }

    void setId(Integer id) {
        m_id = id;
    }

    public Integer getId() {
        return m_id;
    }
    
    /**
     * Checks if the object has been saved to the database
     * Works because hibernate changes id when the object is saved
     * @return true if the object has never been saved
     */
    public boolean isNew() {
        return UNSAVED_ID.equals(getId());
    }

    public boolean equals(Object o) {
        if (!(o instanceof BeanWithId)) {
            return false;
        }
        BeanWithId other = (BeanWithId) o;
        return m_id.equals(other.m_id);
    }

    public int hashCode() {
        return m_id.hashCode();
    }

    public void update(BeanWithId object) {
        try {
            Integer saveId = getId();
            BeanUtils.copyProperties(this, object);
            setId(saveId);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        }
    }

    public BeanWithId duplicate() {
        try {
            BeanWithId clone = (BeanWithId) clone();
            clone.setId(UNSAVED_ID);
            return clone;
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Assigns a unique id to a newly created object.
     * 
     * For testing only.  Most objects are created with id -1 and hibernate sets a proper id.
     * We want to be able to set the id to a unique value in tests.
     * 
     * @return the same object - to allow for chaining calls
     */
    public BeanWithId setUniqueId() {
        setId(new Integer(s_id++));
        return this;
    }

    public static final class BeanToId implements Transformer {
        public Object transform(Object item) {
            BeanWithId bean = (BeanWithId) item;
            return bean.getId();
        }
    }

    /**
     * Implementation of PrimaryKeySource 
     */
    public Object getPrimaryKey() {
        return getId();
    }
}
