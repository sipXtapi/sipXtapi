package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

public class PortingTodoTest extends TestCase {
	private boolean m_supress = true;
	
	public void testSortableTableModel() {
		assertTrue("Abstract table model, class need expression evaluator OrderByTableColumn", m_supress);		
	}
	
	public void testDownloadCapacity() {
		assertTrue("Set download capacity in hivemodule.xml", m_supress);				
	}
	
	public void testDateColumn() {
		assertTrue("DateColumn needs expression evaluator", m_supress);
	}
	
	public void testLocalizedTableRenderer() {
		assertTrue("LocalizedTableRendererSource Message.getMessage w/default value", m_supress);
	}
		
	public void testTapestryUtilsGetMessages() {
		assertTrue("TapestryUtils.getMessages", m_supress);
	}
    
    public void testLogout() {
        assertTrue("Logout functionality", m_supress);        
    }
    
    public void testExceptionPage() {
//        /**
//         * Call super implementation but log exception first. Default implementation only logs it the
//         * are problems on exception page
//         */
//        protected void activateExceptionPage(IRequestCycle cycle, ResponseOutputStream output,
//                Throwable cause) throws ServletException {
//            LOG.warn("Tapestry exception", cause);
//            super.activateExceptionPage(cycle, output, cause);
//        }
//
//        /**
//         * To display standard "developers" error page call super.getExceptionPage instead
//         */
//        protected String getExceptionPageName() {
//            return "InternalErrorPage";
//        }
        assertTrue("Exception Page handling", m_supress);                
    }
}
