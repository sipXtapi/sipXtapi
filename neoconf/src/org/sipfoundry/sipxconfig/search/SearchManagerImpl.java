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
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryParser.ParseException;
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.TermQuery;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SearchManagerImpl extends HibernateDaoSupport implements SearchManager {
    private static final Log LOG = LogFactory.getLog(SearchManagerImpl.class);

    private IndexSource m_indexSource;

    private Analyzer m_analyzer;

    private BeanAdaptor m_beanAdaptor;

    public void setIndexSource(IndexSource indexSource) {
        m_indexSource = indexSource;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    public List search(String queryText) {
        try {
            Query query = parseUserQuery(queryText);
            return search(query);
        } catch (IOException e) {
            LOG.error("search by user query error", e);
        } catch (ParseException e) {
            LOG.info(e.getMessage());
        }
        return Collections.EMPTY_LIST;
    }

    public List search(Class entityClass, String queryText) {
        try {
            Query userQuery = parseUserQuery(queryText);
            Term classTerm = new Term(Indexer.CLASS_FIELD, entityClass.getName());
            TermQuery classQuery = new TermQuery(classTerm);
            BooleanQuery query = new BooleanQuery();
            query.add(classQuery, true, false);
            query.add(userQuery, true, false);
            return search(query);
        } catch (IOException e) {
            LOG.error("search by class error", e);
        } catch (ParseException e) {
            LOG.info(e.getMessage());
        }

        return Collections.EMPTY_LIST;
    }

    private List search(Query query) throws IOException {
        IndexSearcher searcher = null;
        try {
            searcher = m_indexSource.getSearcher();
            Hits hits = searcher.search(query);
            List found = hits2beans(hits);
            return found;
        } finally {
            LuceneUtils.closeQuietly(searcher);
        }
    }

    private List hits2beans(Hits hits) throws IOException {
        HibernateTemplate hibernate = getHibernateTemplate();
        final int hitCount = hits.length();
        List results = new ArrayList(hitCount);
        for (int i = 0; i < hitCount; i++) {
            Document document = hits.doc(i);
            Identity identity = m_beanAdaptor.getBeanIdentity(document);
            if (identity == null) {
                continue;
            }
            Object found = hibernate.load(identity.getBeanClass(), identity.getBeanId());
            results.add(found);
        }
        return results;
    }

    private Query parseUserQuery(String queryText) throws ParseException {
        QueryParser parser = new QueryParser(Indexer.DEFAULT_FIELD, m_analyzer);
        return parser.parse(queryText);
    }
}
