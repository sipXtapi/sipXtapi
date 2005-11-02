package com.workingmouse.webservice.axis;

import javax.servlet.ServletContext;
import javax.xml.rpc.ServiceException;
import javax.xml.rpc.server.ServletEndpointContext;

import org.apache.axis.Constants;
import org.apache.axis.MessageContext;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Utility class used by the {@link com.workingmouse.webservice.axis.SpringBeanMsgProvider}
 * and the {@link com.workingmouse.webservice.axis.SpringBeanRPCProvider} to retrieve
 * Spring-managed beans from a Spring WebApplicationContext.
 * 
 * @author Tom Czarniecki (cThomas AT workingmouse DOT com)
 */
public class SpringBeanProvider {

    private Log log = LogFactory.getLog(getClass());
    
    /**
     * The server-config.wsdd service parameter used to provide the name of the 
     * Spring-managed bean to use as the web service end-point.
     */
    public static final String BEAN_OPTION_NAME = "springBean";
    public static final String BEAN_CLASS_OPTION_NAME = "springBeanClass";

    private WebApplicationContext webAppCtx;

    /**
     * Return a bean bound with the given beanName from the WebApplicationContext.
     */
    public Object getBean(MessageContext msgContext, String beanName) throws Exception {
        initWebAppContext(msgContext);
        if (webAppCtx != null) {
            return webAppCtx.getBean(beanName);
        }
        else {
            return null;
        }
    }

    /**
     * Return the class of a bean bound with the given beanName in the WebApplicationContext.
     */
    public Class getBeanClass(String className) {
        Class result = null;
        try {
            result = Class.forName(className);
        }
        catch (ClassNotFoundException e) {
            log.debug("class "+className+" not found");
        }
        return result;
    }

    private void initWebAppContext(MessageContext msgContext) throws ServiceException {
        log.info("initializing app context for spring-axis integration");
        if (webAppCtx == null) {
            if (msgContext != null) {
                Object context = msgContext.getProperty(Constants.MC_SERVLET_ENDPOINT_CONTEXT);
                if (context instanceof ServletEndpointContext) {
                    ServletEndpointContext servletEndpointContext = (ServletEndpointContext) context;
                    ServletContext servletCtx = servletEndpointContext.getServletContext();                
                    webAppCtx = WebApplicationContextUtils.getWebApplicationContext(servletCtx);
                    if (webAppCtx == null) {
                        log.info("failed to retrieve webapp context for spring-axis integration");
                        throw new ServiceException(
                            "Cannot find WebApplicationContext from org.springframework.web.context.ContextLoaderListener");                                   
                    }
                } else {
                    log.info("failed to retrieve webapp context for spring-axis integration because this is an incorrect servlet context!");
                    throw new ServiceException("Invalid context - expected ["
                            + ServletEndpointContext.class.getName()
                            + "], actual ["
                            + context
                            + "]");
                }
            }
            else {
                log.info("null msg context!");
            }
        }
    }
}