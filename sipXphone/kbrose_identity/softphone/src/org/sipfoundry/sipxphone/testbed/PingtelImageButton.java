/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.testbed ;

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.util.*;
import java.io.*;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
/**
 * The PingtelImageButton class is used to easily implement an image button with
 * 4 states (normal, highlighted, down, disabled) using GIF files.
 */
public class PingtelImageButton extends Component implements ActionListener
{
    Image normal_image = null;
    Image highlight_image = null;
    Image disabled_image = null;
    Image down_image = null;
    Image lit_image = null;
    Image lit_down_image = null;
    Image label_image = null;

	MediaTracker the_tracker;
    boolean mAltDown = false;
    boolean mousedown_flag;
	boolean disabled_flag;
	boolean mouseover_flag;
	boolean lit_flag;
	boolean hasMultipleImages;
	PingtelButtonListener buttonListener;
	PingtelButtonEvent evt;
    TestbedButtonEvent m_TestbedButtonEvent;
    String  m_strKey       = "" ;
    String mTagString = "";
    int iVisible = 0;
    int startx;
    int starty;
    int lastX = 0;
    int lastY = 0;
    int nLabelStartX = 0;
    int nLabelStartY = 0;
    int mnLabelStartXInc = 0;
    int mnLabelStartYInc = 0;
    int width;
    int height;
    boolean mbCanMoveImage = false;
    String strButtonDefault = "";
    String strButtonPressed = "";
    String strButtonOver = "";
    String strButtonLitDefault = "";
    String strButtonLitPressed = "";
    String strButtonLabel = "";

    ResourceBundle  m_skinProps ;

	/**
	 * The constructor of PingtelImageButton
	 * @param button_name A string representing the main name of the button.  The buttons
	 * must be correctly named for this class to work.  The normal button has the form
	 * name.gif.  The highlighted button has the form nameov.gif.  The down button has the
	 * form namedn.gif.  The disabled button has the form namedis.gif.
	 * @param the_applet The main applet.
	 */

	// if properties file had imgL1 as the root name
	// it would load
	// imgB1Default,imgB1Over,imgB1Pressed
	public PingtelImageButton(ResourceBundle skinProps, String skinFolder, String tagString, String strKey)
	{
	    String TagName;
	    String imgFileName;
        m_skinProps = skinProps ;

//        String locationTesting = m_skinProps.getString("LocationTesting");
//        if (locationTesting.equals("1") || locationTesting.equalsIgnoreCase("Yes"))
//            mbCanMoveImage = true;

   		m_strKey   = strKey;
        mTagString = tagString;

	    TagName = tagString + "Default";
        imgFileName = skinFolder + m_skinProps.getString(TagName);
        if (imgFileName.indexOf("none") == -1)
            normal_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonDefault = m_skinProps.getString(TagName);;


	    TagName = tagString + "Press";
        imgFileName = skinFolder + m_skinProps.getString(TagName);
        if (imgFileName.indexOf("none") == -1)
            down_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonPressed = m_skinProps.getString(TagName);


	    TagName = tagString + "Over";
        imgFileName = skinFolder + m_skinProps.getString(TagName);
        if (imgFileName.indexOf("none") == -1)
            highlight_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonOver = m_skinProps.getString(TagName);

	    TagName = tagString + "LitDefault";
        imgFileName = skinFolder + m_skinProps.getString(TagName);
        if (imgFileName.indexOf("none") == -1)
            lit_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonLitDefault = m_skinProps.getString(TagName);

	    TagName = tagString + "LitPress";
        imgFileName = skinFolder + m_skinProps.getString(TagName);
        if (imgFileName.indexOf("none") == -1)
            lit_down_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonLitPressed = m_skinProps.getString(TagName);


        //heres how we get the label (if available)
	    TagName = tagString + "Label";
        imgFileName = skinFolder + m_skinProps.getString(TagName);

        if (imgFileName.indexOf("none") == -1)
            label_image = Toolkit.getDefaultToolkit().getImage(imgFileName);
        strButtonLabel = m_skinProps.getString(TagName);

        String strStartX = m_skinProps.getString(tagString + "StartX");
        String strStartY = m_skinProps.getString(tagString + "StartY");
        String strVisible = m_skinProps.getString(tagString + "Visible");

        try
        {
           iVisible = Integer.parseInt(strVisible);

            if (iVisible == 1)
            {

                startx =   Integer.parseInt(strStartX);
                starty =   Integer.parseInt(strStartY);

                setBounds(  Integer.parseInt(strStartX),
                        Integer.parseInt(strStartY),
                        width,
                        height);
            }
            else
                setBounds(0,0,0,0);



        }
        catch (Exception e)
        {
            iVisible = 0;
            System.out.println("ERROR: Missing Bad StartX and/or StartY for button: " + tagString);
            setBounds(0,0,0,0);
        }



//        System.out.println("Normal Image: " + normal_image);
//        System.out.println("Down Image: " + down_image);

        if (label_image != null)
        {
            String strLabelStartX = m_skinProps.getString(tagString + "LabelStartX");
            String strLabelStartY = m_skinProps.getString(tagString + "LabelStartY");

            //this next condition is true if the tag is not there.  This means the user doesn't want a label
            //on this button
            if (strLabelStartX.startsWith("img") || strLabelStartY.startsWith("img"))
            {
                nLabelStartX =   0;
                nLabelStartY =   0;
                label_image = null;
            }
            else
            {
                try
                {
                    nLabelStartX = 0;
                    nLabelStartY = 0;

                    if (strLabelStartX.startsWith("+"))
                    {
                        int incX = Integer.parseInt(strLabelStartX.substring(1));
                        mnLabelStartXInc = incX;
                        nLabelStartX = startx + incX;
                    }
                    if (strLabelStartX.startsWith("-"))
                    {
                        int incX = Integer.parseInt(strLabelStartX.substring(1));
                        incX *=-1;
                        mnLabelStartXInc = incX;
                        nLabelStartX = startx + incX;
                    }

                    if (strLabelStartY.startsWith("+"))
                    {
                        int incY = Integer.parseInt(strLabelStartY.substring(1));
                        nLabelStartY = starty + incY;
                        mnLabelStartYInc = incY;
                    }
                    if (strLabelStartY.startsWith("-"))
                    {
                        int incY = Integer.parseInt(strLabelStartY.substring(1));
                        incY *=-1;
                        mnLabelStartYInc = incY;
                        nLabelStartY = starty + incY;
                    }

                    if (nLabelStartX == 0)
                        nLabelStartX = Integer.parseInt(strLabelStartX);
                    if (nLabelStartY == 0)
                        nLabelStartY = Integer.parseInt(strLabelStartY);


//                        System.out.println("label = " + label_image);
//                        System.out.println("labelx = " + nLabelStartX);
//                        System.out.println("labely = " + nLabelStartY);
//                        System.out.println("mnLabelStartXInc="+mnLabelStartXInc);
//                        System.out.println("mnLabelStartYInc="+mnLabelStartYInc);
                }
                catch(Exception e)
                {
                    System.out.println("******** ERROR parsing LabelStartX or LabelStartY for image " + mTagString);
                    e.printStackTrace();
                    nLabelStartX =   0;
                    nLabelStartY =   0;
                    label_image = null;
                }
            }
        }

		if (normal_image != null)
		{
    	    //def,down,disabled,over
            CreateButton(normal_image,down_image,null,highlight_image);

            height = normal_image.getHeight(null);
            width  = normal_image.getWidth(null);

            setSize(width,height);
        }


	}

	public PingtelImageButton(Image image_start, Image image_down)
	{
		this(image_start, image_down, null) ;
    }

	public PingtelImageButton(Image image_start, Image image_down, String strKey)
	{
		CreateButton(image_start, image_down, null, null);
		m_strKey = strKey;
    }

	public PingtelImageButton(Image image_start, Image image_down, Image image_disabled,Image image_over)
	{
		CreateButton(image_start, image_down, image_disabled, image_over);


    }


	public void CreateButton(Image image_start, Image image_down, Image image_disabled,Image image_over)
	{
	    m_TestbedButtonEvent = new TestbedButtonEvent();

		//  Create the images

		if (image_start != null)
		    normal_image = image_start;

		if (image_down != null)
		    down_image = image_down;

		if (image_disabled != null)
		    disabled_image = image_disabled;

		if (image_over != null)
		    highlight_image = image_over;


		the_tracker = new MediaTracker(this);


        if (normal_image != null)
		    the_tracker.addImage(normal_image, 0);
        if (down_image != null)
		    the_tracker.addImage(down_image, 0);
        if (highlight_image != null)
		    the_tracker.addImage(highlight_image, 0);
        if (disabled_image != null)
		    the_tracker.addImage(disabled_image, 0);
        if (lit_image != null)
		    the_tracker.addImage(lit_image, 0);
        if (lit_down_image != null)
		    the_tracker.addImage(lit_down_image, 0);
        if (label_image != null)
		    the_tracker.addImage(label_image, 0);


		try
		{
			the_tracker.waitForID(0);
		}
		catch(InterruptedException e)
		{
		}


        //the button starts unclicked, enabled, and unhighlighted
        mousedown_flag = false;
		disabled_flag = false;
		mouseover_flag = false;

		evt = new PingtelButtonEvent(this, 0);

		addMouseListener(new PingtelMouseListener());
		addMouseMotionListener(new PingtelMouseMotionListener());

        //make sure the canvas image is drawn
        repaint();
	}

    /** returns the default non-pressed image*/
    public Image getDefaultImage()
	{
        return normal_image;
    }

    /** returns the pressed image*/
    public Image getPressedImage()
	{
        return down_image;
    }

    /** returns the mouse over image */
    public Image getOverImage()
	{
        return highlight_image;
    }

    /** returns the Lit NON-Pressed image */
    public Image getLitImage()
	{
        return lit_image;
    }
    /** returns the Lit Pressed image */
    public Image getLitPressedImage()
	{
        return lit_image;
    }

    /** returns the Label image */
    public Image getLabelImage()
	{
        return label_image;
    }
    /** returns the Label Offset X */
    public int getLabelStartX()
	{
        return nLabelStartX;
    }
    /** returns the Label Offset Y */
    public int getLabelStartY()
	{
        return nLabelStartY;
    }
    /** returns the Label Offset X */
    public int getStartX()
	{
        return startx;
    }
    /** returns the Label Offset Y */
    public int getStartY()
	{
        return starty;
    }

    /** returns the Label Offset Y */
    public void flushSkinInfo(FileWriter file)
	{
        System.out.println("PingtelImageButton.flushSkinInfo: "+mTagString);
        String line;
        char buffer[] = new char[160];
        try
        {
            line = mTagString + "Visible=" + iVisible + "\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "StartX=" + getStartX() + "\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "StartY=" + getStartY() + "\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "Default=" + strButtonDefault + "\r\n";
            if (strButtonDefault.equals(mTagString + "Default"))
                line = mTagString + "Default=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "Press=" + strButtonPressed + "\r\n";
            if (strButtonPressed.equals(mTagString + "Press"))
                line = mTagString + "Press=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "Over=" + strButtonOver + "\r\n";
            if (strButtonOver.equals(mTagString + "Over"))
                line = mTagString + "Over=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "LitDefault=" + strButtonLitDefault + "\r\n";
            if (strButtonLitDefault.equals(mTagString + "LitDefault"))
                line = mTagString + "LitDefault=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "LitPress=" + strButtonLitPressed + "\r\n";
            if (strButtonLitPressed.equals(mTagString + "LitPress"))
                line = mTagString + "LitPress=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            line = mTagString + "Label=" + strButtonLabel + "\r\n";
            if (strButtonLabel.equals(mTagString + "Label"))
                line = mTagString + "Label=none\r\n";
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            if (mnLabelStartXInc == 0)
                line = mTagString + "LabelStartX=" + getLabelStartX() + "\r\n";
            else
            {
                if (mnLabelStartXInc > 0)
                    line = mTagString + "LabelStartX=+" + mnLabelStartXInc + "\r\n";
                else
                    line = mTagString + "LabelStartX=" + mnLabelStartXInc + "\r\n";
            }
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());

            if (mnLabelStartYInc == 0)
                line = mTagString + "LabelStartY=" + getLabelStartY() + "\r\n\r\n";
            else
            {
                if (mnLabelStartYInc > 0)
                    line = mTagString + "LabelStartY=+" + mnLabelStartYInc + "\r\n";
                else
                    line = mTagString + "LabelStartY=" + mnLabelStartYInc + "\r\n";
            }
            line.getChars(0,line.length(),buffer,0);
            file.write(buffer,0,line.length());
            System.out.println("Done.");
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

	/**
	 * Overridden method to telling the layout manager the size of the button
	 * which is defined by the size of the image. This assumes that the images are
	 * the same size.
	 * @return Dimension of the button
	 */
    public Dimension getMinimumSize()
	{
        return new Dimension(normal_image.getWidth(null),normal_image.getHeight(null));
    }

	/**
	 * Overridden method to telling the layout manager the size of the button
	 * which is defined by the size of the image. This assumes that the images are
	 * the same size.
	 * @return Dimension of the button
	 */
    public Dimension getMaximumSize()
	{
        return new Dimension(normal_image.getWidth(null),normal_image.getHeight(null));
    }

	/**
	 * Overridden method to telling the layout manager the size of the button
	 * which is defined by the size of the image. This assumes that the images are
	 * the same size.
	 * @return Dimension of the button
	 */
    public Dimension getPreferredSize()
	{
        return new Dimension(normal_image.getWidth(null),normal_image.getHeight(null));
    }

	/**
	 * Sets the button's highlighted state
	 * @param highlight Flag telling whether to highlight or unhighlight the button
	 */
	public void highlightButton(boolean highlight)
	{
		mouseover_flag = highlight;
		repaint();
	}

	public void lightButton(boolean light)
	{
	    if (lit_image != null && lit_flag != light)
	    {
		    lit_flag = light;
		    repaint();
		}
	}

	/**
	 * Sets the button's depressed state
	 * @param depress Flag telling whether to depress the button or not
	 */
	public void depressButton(boolean depress)
	{
		mousedown_flag = depress;
		repaint();
	}

	/**
	 * Sets the button's disabled state
	 * @param disable Flag telling whether to disable or enable the button
	 */
	public void disableButton(boolean disable)
	{
		disabled_flag = disable;
		repaint();
	}

    public void update(Graphics g)
    {
        paint(g);
    }

	/**
	 * Paints the button depending on the various state flags
	 * @param g Graphics object
	 */
    public void paint(Graphics g)
	{
	    int x = 0;
	    int y = 0;



	    if (iVisible == 0)
	        return;

	    if (normal_image != null)
	    {
	        Rectangle r;
	        r = g.getClipBounds();
		    x = (int)r.width-1;
		    y = (int)r.height-1;
		}


        //determine which image to draw based on the mouse state
		if (!disabled_flag)
    	{
   	        if (normal_image != null &&
   	            !lit_flag && !mousedown_flag)
       	    	g.drawImage(normal_image,0,0,this);

       	    if (lit_image != null && lit_flag == true)
   	        {

                if (mousedown_flag && lit_down_image != null)
       	            g.drawImage(lit_down_image,0,0,this);
       	        else
       	            g.drawImage(lit_image,0,0,this);
            }
            else
            if (mousedown_flag)
            {
        	    if (down_image != null)
	        		g.drawImage(down_image,0,0,this);
            }
            else
			if (mouseover_flag)
			{
				//  Highlighted state
			    if (highlight_image != null)
				    g.drawImage(highlight_image,0,0,this);
			}
		}
		else
		{
			//  Disabled state
		    if (disabled_image != null)
    				g.drawImage(disabled_image,0,0,this);
		}
    }


	public void addPingtelButtonListener (PingtelButtonListener listener)
	{
		buttonListener = listener;
	}

    public class PingtelMouseMotionListener implements MouseMotionListener
    {
        public void mouseMoved(MouseEvent e)
        {
            lastX = e.getX();
            lastY = e.getY();
        }

        public void mouseDragged(MouseEvent e)
        {
            if (mbCanMoveImage && (e.getModifiers() & (e.ALT_MASK|e.CTRL_MASK|e.SHIFT_MASK)) ==
                 (e.ALT_MASK|e.CTRL_MASK|e.SHIFT_MASK))
            {
                Point p = getLocation();
                if (e.getX() != lastX || e.getY()!= lastY)
                {
                    if (e.getX() > lastX)
                        p.x += e.getX()-lastX;
                    if (e.getY() > lastY)
                        p.y += e.getY()-lastY;
                    if (e.getX() < lastX)
                        p.x -= lastX-e.getX();
                    if (e.getY() < lastY)
                        p.y -= lastY-e.getY();
                    setLocation(p);
                    System.out.println("Button " + mTagString + "  moved to: " +
                                    p.x + ","+p.y);
                    startx = p.x;
                    starty = p.y;
                }
            }
        }
    }

	public class PingtelMouseListener implements MouseListener
	{

		public void mouseClicked(MouseEvent e)
		{
            if ((e.getModifiers() & (e.BUTTON2_MASK | e.ALT_MASK|e.CTRL_MASK|e.SHIFT_MASK)) ==
                 (e.BUTTON2_MASK | e.ALT_MASK|e.CTRL_MASK|e.SHIFT_MASK))
            {
                Point p = getLocation();
                System.out.println(mTagString + "StartX="+p.x);
                System.out.println(mTagString + "StartY="+p.y);
            }
		}
		public void mouseEntered(MouseEvent e)
		{
			//  Process mouseover
			if (down_image != null || lit_down_image != null)
			{
			    setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));

			    if (highlight_image != null)
		            highlightButton(true);
			}
		}

		public void mouseExited(MouseEvent e)
		{
			//  Process mouseout
			setCursor(Cursor.getDefaultCursor());

			if (highlight_image != null)
    			highlightButton(false);
		}
		public void mousePressed(MouseEvent e)
		{
		    if (down_image != null)
		    {
			    //  Show the button in a depressed state if the left mouse button is pressed
			    if ((e.getModifiers() | MouseEvent.BUTTON1_MASK) == MouseEvent.BUTTON1_MASK)
			    {
				    depressButton(true);
				    if (contains(e.getPoint()))
				    {
				        int iButtonId = m_TestbedButtonEvent.getButtonId(m_strKey);
            //            m_TestbedButtonEvent.JNI_postButtonEvent(button_id,BUTTON_DOWN);

                    TestbedButtonEvent btnEventDown = new TestbedButtonEvent(iButtonId, PButtonEvent.BUTTON_DOWN);

				    }
			    }
			}
		}
		public void mouseReleased(MouseEvent e)
		{
		    if (down_image != null)
		    {
			    if ((e.getModifiers() | MouseEvent.BUTTON1_MASK) == MouseEvent.BUTTON1_MASK)
			    {
                    if (mousedown_flag)
                    {
    				    //  Return the button to its normal state if the left mouse button is released
	    			    depressButton(false);

				    //  If the mouse cursor is over the button, process the mouse click
				        int iButtonId = m_TestbedButtonEvent.getButtonId(m_strKey);
                        TestbedButtonEvent btnEventUp   = new TestbedButtonEvent(iButtonId, PButtonEvent.BUTTON_UP);

				    }
			    }
			}
		}
	}

	public class PingtelButtonEvent extends AWTEvent
	{
		PingtelImageButton buttonSource;
		int eventID;

		public PingtelButtonEvent( Event event )
		{
			super(event);
		}

		public PingtelButtonEvent( PingtelImageButton source, int id )
		{
			super((Object) source, id);
			buttonSource = source;
		}

		public PingtelImageButton getButtonSource()
		{
			return buttonSource;
		}
	}

	public interface PingtelButtonListener extends EventListener
	{
		abstract void buttonClicked( PingtelButtonEvent e );
	}

    public void actionPerformed(ActionEvent e) {
        // Call TestbedButtonEvent with this button

  }

}



