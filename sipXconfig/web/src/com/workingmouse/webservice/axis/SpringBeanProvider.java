package com.workingmouse.webservice.axis;

import javax.servlet.ServletContext;
import javax.xml.rpc.ServiceException;
import javax.xml.rpc.server.ServletEndpointContext;

import org.apache.axis.AxisFault;
import org.apache.axis.Constants;
import org.apache.axis.MessageContext;
import org.apache.axis.utils.Messages;
import org.springframework.web.context.WebApplicationContext;

/**
 * Utility class used by the {@link com.workingmouse.webservice.axis.SpringBeanMsgProvider}
 * and the {@link com.workingmouse.webservice.axis.SpringBeanRPCProvider} to retrieve
 * Spring-managed beans from a Spring WebApplicationContext.
 * 
 * @author Tom Czarniecki (cThomas AT workingmouse DOT com)
 */
public class SpringBeanProvider {

    /**
     * The server-config.wsdd service parameter used to provide the name of the 
     * Spring-managed bean to use as the web service end-point.
     */
    public static final String BEAN_OPTION_NAME = "springBean";

    private WebApplicationContext webAppCtx;

    /**
     * Return a bean bound with the given beanName from the WebApplicationContext.
     */
    public Object getBean(MessageContext msgContext, String beanName) throws Exception {
        initWebAppContext(msgContext);
        return webAppCtx.getBean(beanName);
    }

    /**
     * Return the class of a bean bound with the given beanName in the WebApplicationContext.
     */
    public Class getBeanClass(MessageContext msgContext, String beanName) throws AxisFault {
        try {
            Object bean = getBean(msgContext, beanName);
            return bean.getClass();

        } catch (Exception e) {
            throw new AxisFault(Messages.getMessage("noClassForService00", beanName), e);
        }
    }

    private void initWebAppContext(MessageContext msgContext) throws ServiceException {
        if (webAppCtx == null) {
            Object context = msgContext.getProperty(Constants.MC_SERVLET_ENDPOINT_CONTEXT);
            if (context instanceof ServletEndpointContext) {
                ServletEndpointContext servletEndpointContext = (ServletEndpointContext) context;
                ServletContext servletCtx = servletEndpointContext.getServletContext();
                webAppCtx = (WebApplicationContext) servletCtx
                    .getAttribute(SpringAxisServlet.SERVLET_CONTEXT_ATTRIBUTE);
                if (webAppCtx == null) {
                    throw new ServiceException(
                        "Cannot find SpringAxisServlet's WebApplicationContext"
                                + " as ServletContext attribute ["
                                + SpringAxisServlet.SERVLET_CONTEXT_ATTRIBUTE
                                + "]");
                }
            } else {
                throw new ServiceException("Invalid context - expected ["
                        + ServletEndpointContext.class.getName()
                        + "], actual ["
                        + context
                        + "]");
            }
        }
    }
}