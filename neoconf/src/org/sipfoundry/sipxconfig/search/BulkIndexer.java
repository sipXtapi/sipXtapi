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
package org.sipfoundry.sipxconfig.search;

import java.io.IOException;
import java.io.Serializable;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.store.Directory;
import org.hibernate.type.Type;

/**
 * Used to create index - optimized for indexing large sets.
 * 
 * No support for removing beans.
 */
public class BulkIndexer implements Indexer {
    private Directory m_directory;

    private Analyzer m_analyzer;

    private BeanAdaptor m_beanAdaptor;

    private IndexWriter m_writer;

    public void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames,
            Type[] types) {
        try {
            Document document = new Document();
            if (m_beanAdaptor.documentFromBean(document, bean, id, state, fieldNames, types)) {
                m_writer.addDocument(document, m_analyzer);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

    }

    public void removeBean(Object bean_, Serializable id_) {
        throw new UnsupportedOperationException("only used to add new beans");
    }

    public void open() {
        try {
            m_writer = new IndexWriter(m_directory, m_analyzer, true);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void close() {
        try {
            m_writer.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void setDirectory(Directory directory) {
        m_directory = directory;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }
}
