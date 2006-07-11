/*
 * Copyright (c) 2002 Pingtel Corp.  (work in progress)
 *
 * This is an unpublished work containing Pingtel Corporation's confidential
 * and proprietary information.  Disclosure, use or reproduction without
 * written authorization of Pingtel Corp. is prohibited.
 */
package org.sipfoundry.sipxphone.sys.appclassloader;

import java.awt.Image;
import java.awt.Toolkit;
import java.net.URLConnection;
import java.awt.image.ImageProducer;
import java.awt.image.ImageConsumer;
import java.util.Vector;
import java.util.Enumeration;

/**
 * PImageProducer.java
 *
 * This is the Pingtel implementation of ImageProducer interface.
 *
 * Javadocs for the methods are copied from ImageProducer javadocs
 * as this class implements ImageProducer.
 * Created: Tue Apr 16 16:42:09 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */


public class PImageProducer implements ImageProducer
{
    ImageProducer m_imageProducer;
    Vector  m_vImageConsumers;

    /**
     * This method is used to register an ImageConsumer with the
     * ImageProducer for access to the image data during a later
     * reconstruction of the Image.  The ImageProducer may, at its
     * discretion, start delivering the image data to the consumer
     * using the ImageConsumer interface immediately, or when the
     * next available image reconstruction is triggered by a call
     * to the startProduction method.
     * @see #startProduction
     */
    public void addConsumer(ImageConsumer ic){
        if(m_vImageConsumers == null){
            m_vImageConsumers = new Vector();
        }
        m_vImageConsumers.addElement(ic);

    }

    /**
     * This method determines if a given ImageConsumer object
     * is currently registered with this ImageProducer as one
     * of its consumers.
     */
    public boolean isConsumer(ImageConsumer ic){
        boolean bRet = false;
        if(m_vImageConsumers == null){
            bRet = false;
        }else{
            bRet = m_vImageConsumers.contains(ic);
        }
        return bRet;
    }

    /**
     * This method removes the given ImageConsumer object
     * from the list of consumers currently registered to
     * receive image data.  It is not considered an error
     * to remove a consumer that is not currently registered.
     * The ImageProducer should stop sending data to this
     * consumer as soon as is feasible.
     */
    public void removeConsumer(ImageConsumer ic){
        if(m_vImageConsumers != null){
            m_vImageConsumers.removeElement(ic);
        }
    }

    /**
     * This method both registers the given ImageConsumer object
     * as a consumer and starts an immediate reconstruction of
     * the image data which will then be delivered to this
     * consumer and any other consumer which may have already
     * been registered with the producer.  This method differs
     * from the addConsumer method in that a reproduction of
     * the image data should be triggered as soon as possible.
     * @see #addConsumer
     */
    public void startProduction(ImageConsumer ic){
        if(m_imageProducer == null)
        {
            Image image = Toolkit.getDefaultToolkit().createImage(this);
            m_imageProducer = image.getSource();
        }
        if( ic != null){
            addConsumer(ic);
        }
        if(m_vImageConsumers != null){
            Enumeration enum = m_vImageConsumers.elements();
            while( enum.hasMoreElements() ){
                m_imageProducer.addConsumer( (ImageConsumer)(enum.nextElement()));
            }
        }
        m_imageProducer.startProduction(null);
    }

    /**
     * This method is used by an ImageConsumer to request that
     * the ImageProducer attempt to resend the image data one
     * more time in TOPDOWNLEFTRIGHT order so that higher
     * quality conversion algorithms which depend on receiving
     * pixels in order can be used to produce a better output
     * version of the image.  The ImageProducer is free to
     * ignore this call if it cannot resend the data in that
     * order.  If the data can be resent, then the ImageProducer
     * should respond by executing the following minimum set of
     * ImageConsumer method calls:
     * <pre>
     *	ic.setHints(TOPDOWNLEFTRIGHT | < otherhints >);
     *	ic.setPixels(...);	// As many times as needed
     *	ic.imageComplete();
     * </pre>
     * @see ImageConsumer#setHints
     */
    public void requestTopDownLeftRightResend(ImageConsumer ic){
    }

}

// PImageProducer
