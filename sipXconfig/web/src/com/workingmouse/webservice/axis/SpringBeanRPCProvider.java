package com.workingmouse.webservice.axis;

import org.apache.axis.AxisFault;
import org.apache.axis.Handler;
import org.apache.axis.MessageContext;
import org.apache.axis.handlers.soap.SOAPService;
import org.apache.axis.providers.java.RPCProvider;

/**
 * Axis provider for RPC-style services that uses Spring Framework
 * to retrieve service classes and resolve their dependencies.
 * Simply delegates to {@link com.workingmouse.webservice.axis.SpringBeanProvider}.
 * <p>
 * To use this class:<br>
 * 1. Configure web.xml to use the axis servlet
 * 2. Configure web.xml to use the spring @see org.springframework.web.context.ContextLoaderListener.
 * 3. Configure your server-config.wsdd service to use this class as the service handler.
 * 
 * Here is an example:
 * <pre>
 * &lt;service name="hdWebService.jws" provider="Handler" style="rpc"&gt;
 *      &lt;parameter name="handlerClass" value="com.workingmouse.webservice.axis.SpringBeanRPCProvider"/&gt;
 *      &lt;parameter name="wsdlTargetNamespace" value="http://healthdec.com"/&gt;
 *      &lt;parameter name="springBean" value="hdWebService"/&gt;
 *      &lt;parameter name="springBeanClass" value="com.healthdec.hdSOAPServer.WebService"/&gt;
 *      &lt;parameter name="allowedMethods" value="findAllInstances"/&gt;
 *      &lt;beanMapping qname="myNS:Instance" xmlns:myNS="urn:BeanService" languageSpecificType="java:com.healthdec.domain.dm.Instance"/&gt;
 *      &lt;beanMapping qname="myNS:CRFType" xmlns:myNS="urn:BeanService" languageSpecificType="java:com.healthdec.domain.dm.CRFType"/&gt;
 *      &lt;beanMapping qname="myNS:CRFQuestionType" xmlns:myNS="urn:BeanService" languageSpecificType="java:com.healthdec.domain.dm.CRFQuestionType"/&gt;
 * &lt;/service&gt;
 * </pre> 
 * 
 * 4. Configure a Spring-managed bean in your applicationContext xml file that will act as the web service end point.
 * <pre>
 * &lt;bean id=&quot;productWS&quot; class=&quot;com.workingmouse.webservice.ProductWebServices&quot;&gt;
 * &lt;/bean&gt;
 * </pre>
 * 
 * @author Tom Czarniecki (cThomas AT workingmouse DOT com)
 * 
 * @see com.workingmouse.webservice.axis.SpringBeanProvider
 */
public class SpringBeanRPCProvider extends RPCProvider {

    private final SpringBeanProvider provider = new SpringBeanProvider();
    private String serviceClassName;
    private String springBeanName;
    
    
    /* (non-Javadoc)
     * @see org.apache.axis.providers.BasicProvider#initServiceDesc(org.apache.axis.handlers.soap.SOAPService, org.apache.axis.MessageContext)
     * 
     * Get the name and class of the Spring bean that will implement the Webservice methods.
     * 
     */
    public void initServiceDesc(SOAPService service, MessageContext arg1)
            throws AxisFault {
        setServiceClassName((String)service.getOption(SpringBeanProvider.BEAN_CLASS_OPTION_NAME));
        setSpringBeanName((String)service.getOption(SpringBeanProvider.BEAN_OPTION_NAME));
        super.initServiceDesc(service, arg1);
    }
    /**
     * @see org.apache.axis.providers.java.JavaProvider#makeNewServiceObject(org.apache.axis.MessageContext, java.lang.String)
     */
    protected Object makeNewServiceObject(MessageContext msgContext, String clsName)
        throws Exception {
        if (msgContext != null) {
            return provider.getBean(msgContext, getSpringBeanName());
        }
        else {
            return null;
        }
    }

    /**
     * @see org.apache.axis.providers.java.JavaProvider#getServiceClass(java.lang.String, org.apache.axis.handlers.soap.SOAPService, org.apache.axis.MessageContext)
     */
    protected Class getServiceClass(String clsName, SOAPService service, MessageContext msgContext) {
        return provider.getBeanClass(clsName);
    }

    /**
     * @see org.apache.axis.providers.java.JavaProvider#getServiceClassNameOptionName()
     */
    protected String getServiceClassNameOptionName() {
        return SpringBeanProvider.BEAN_OPTION_NAME;
    }
    public void setServiceClassName(String serviceClassName) {
        this.serviceClassName = serviceClassName;
    }
    /**
     * @see org.apache.axis.providers.java.JavaProvider#getServiceClassName(org.apache.axis.Handler)
     */
    protected String getServiceClassName(Handler arg0) {
        return getServiceClassName();
    }

    /**
     * @return Returns the serviceClassName.
     */
    public String getServiceClassName() {
        return serviceClassName;
    }
    /**
     * @param serviceClassName The serviceClassName to set.
     */
    
    /**
     * @return Returns the springBeanName.
     */
    public String getSpringBeanName() {
        return springBeanName;
    }
    /**
     * @param springBeanName The springBeanName to set.
     */
    public void setSpringBeanName(String springBeanName) {
        this.springBeanName = springBeanName;
    }
}