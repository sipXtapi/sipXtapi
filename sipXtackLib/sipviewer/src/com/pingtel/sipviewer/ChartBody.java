package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;


public class ChartBody extends Component implements Scrollable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected static final int DEFAULT_MAX_WIDTH_PER_KEY    = 200 ;
    protected static final int DEFAULT_MAX_HEIGHT           = 100 ;
    protected static final int ARROW_HEIGHT                 = 10 ;
    protected static final int ARROW_SHORTEN_WIDTH          = 6 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected SIPChartModel m_model ;
    protected SIPInfoPanel  m_infoPanel ;
    protected int           m_iMouseOver ;
    protected String        m_strMatchBranchId ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public ChartBody(SIPChartModel model, SIPInfoPanel infoPanel)
    {
        m_model = model ;
        m_iMouseOver = -1 ;
        m_infoPanel = infoPanel ;

        m_model.addChartModelListener(new icChartModelListener());
        addMouseListener(new icMouseListener());
        addMouseMotionListener(new icMouseMotionListener()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void paintEntry(Graphics g, int index)
    {
        Dimension dimSize = getSize() ;             // TODO: Only get once
        Dimension dimRowSize = getMinimumSize() ;   // TODO: Only get once

        int ixOffset = dimSize.width / m_model.getNumKeys() ;
        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor entry = (ChartDescriptor) m_model.getEntryAt(index) ;
            String strBranchId = entry.dataSource.getThisBranchId() ;
            boolean bEast = true ;

            Rectangle rectAreaText = new Rectangle(
                    ixOffset*entry.sourceColumn + ixOffset/2,
                    (dimRowSize.height)*(index+1),
                    (entry.targetColumn-entry.sourceColumn)*ixOffset,
                    dimRowSize.height-(ARROW_HEIGHT+2)) ;

            Rectangle rectAreaArrow = new Rectangle(
                    ixOffset*entry.sourceColumn + ixOffset/2,
                    (dimRowSize.height)*(index+1),
                    (entry.targetColumn-entry.sourceColumn)*ixOffset,
                    ARROW_HEIGHT) ;

            if (rectAreaArrow.width < 0)
            {
                int oldX = rectAreaArrow.x ;
                rectAreaArrow.x = rectAreaArrow.x + rectAreaArrow.width ;
                rectAreaArrow.width = rectAreaArrow.width * - 1;

                rectAreaText.x = rectAreaArrow.x ;
                rectAreaText.width = rectAreaArrow.width ;

                bEast = false ;
            }

            // Adjust rectangle so that horz lines don't touch the verts.
            rectAreaArrow.x += ARROW_SHORTEN_WIDTH ;
            rectAreaArrow.width -= (ARROW_SHORTEN_WIDTH*2) ;

            if (m_iMouseOver == index)
            {
                g.setColor(Color.red) ;
            }
            else if ((m_strMatchBranchId != null)
                    && (m_strMatchBranchId.length() > 0)
                    && (m_strMatchBranchId.equals(strBranchId)))
            {
                g.setColor(Color.green) ;
            }
            else
            {
                g.setColor(Color.white) ;
            }
            if (entry.dataSource.isRequest())
            {
                GUIUtils.drawArrow(g, rectAreaArrow, bEast, null, GUIUtils.LINE_SOLID) ;
            }
            else
            {
                boolean bIsProvision = false ;

                String strResponseCode = entry.dataSource.getResponseCode() ;
                if ((strResponseCode != null) && strResponseCode.startsWith("1"))
                {
                    bIsProvision = true ;
                }

                if (bIsProvision)
                {
                    GUIUtils.drawArrow(g, rectAreaArrow, bEast, null, GUIUtils.LINE_DOTTED) ;
                }
                else
                {
                    GUIUtils.drawArrow(g, rectAreaArrow, bEast, null, GUIUtils.LINE_DASHED) ;
                }

            }

            int xTextOffset = GUIUtils.calcXOffset(entry.label, g, rectAreaText, GUIUtils.ALIGN_CENTER) ;
            g.drawString(entry.label, xTextOffset, rectAreaText.y) ;
        }
    }


    public void paint(Graphics g)
    {
        int iEntries = m_model.getSize() ;
        Dimension dimSize = getSize() ;
        Dimension dimRowSize = getMinimumSize() ;
        int iNumKeys = m_model.getNumKeys() ;

        // Paint Background
        g.setColor(Color.black) ;
        g.fillRect(0, 0, dimSize.width, dimSize.height) ;

        // Draw vertical lines
        if ((dimRowSize.width > 0) && (iNumKeys > 0))
        {
            int iOffset = dimSize.width / iNumKeys ;

            for (int i=0; i<iNumKeys; i++)
            {
                g.setColor(Color.yellow);
                g.drawLine((iOffset*i) + iOffset / 2,
                0,
                (iOffset*i) + iOffset / 2,
                dimSize.height) ;
            }
        }

        // Paint all elements
        if ((iEntries > 0) && (dimRowSize.width > 0) && (iNumKeys > 0))
        {
            for (int i=0; i<iEntries; i++)
            {
                paintEntry(g, i) ;
            }
        }
    }


    public Dimension getPreferredSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY*(m_model.getNumKeys()+1) ;
        int height = DEFAULT_MAX_HEIGHT ;

        Graphics g = getGraphics() ;
        if (g != null)
        {
            Font f = getFont() ;
            if (f != null)
            {
                FontMetrics fm =  g.getFontMetrics(f) ;
                height = fm.getHeight() + ARROW_HEIGHT ;
            }
        }

        height = height * (m_model.getSize() + 1) ;

        return new Dimension(width, height) ;
    }


    public Dimension getMinimumSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY*(m_model.getNumKeys()+1) ;
        int height = DEFAULT_MAX_HEIGHT ;

        Graphics g = getGraphics() ;
        if (g != null)
        {
            Font f = getFont() ;
            if (f != null)
            {
                FontMetrics fm =  g.getFontMetrics(f) ;
                height = fm.getHeight() + ARROW_HEIGHT ;
            }
        }

        return new Dimension(width, height) ;
    }


    public Dimension getPreferredScrollableViewportSize()
    {
        return getPreferredSize() ;
    }


    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction)
    {
        return 100;
    }

    public boolean getScrollableTracksViewportHeight()
    {
        return false ;
    }


    public boolean getScrollableTracksViewportWidth()
    {
        return true ;
    }


    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction)
    {
        return 5 ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected int PointToIndex(Point point)
    {
        Dimension size = getMinimumSize() ;
        int index = (point.y - (size.height / 2)) / size.height ;

        if (index >= m_model.getSize())
        {
            index = -1 ;
        }

        return index ;
    }


    protected SipBranchData getSipBranchData(int index)
    {
        SipBranchData objRC = null ;
        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            objRC = desc.dataSource ;
        }
        return objRC ;
    }


    protected String getMessageText(int index)
    {
        String strRC = null ;

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            strRC = desc.dataSource.getMessage() ;
        }
        return strRC ;
    }


    protected void setMouseOver(int iIndex)
    {
        int iOldIndex = m_iMouseOver ;
        m_iMouseOver = iIndex ;
        if (iOldIndex != m_iMouseOver)
        {
            Graphics g = getGraphics() ;
            if (g != null)
            {
                if (iOldIndex != -1)
                {
                    m_strMatchBranchId = null ;
                    repaintAllMatchingBranchId(iOldIndex, false) ;
                }

                if (m_iMouseOver != -1)
                {
                    repaintAllMatchingBranchId(m_iMouseOver, true) ;
                    m_infoPanel.populate(getSipBranchData(m_iMouseOver)) ;
                }
                else
                {
                    m_infoPanel.clear() ;
                }
            }
        }
    }


    /**
     * Repaint all entries that match the branchId
     */
    protected void repaintAllMatchingBranchId(int iSourceIndex, boolean bSetMatchBranchId)
    {
        String strBranchId = null ;

        if ((iSourceIndex >= 0) && (iSourceIndex < m_model.getSize()))
        {
            ChartDescriptor sourceDesc = m_model.getEntryAt(iSourceIndex) ;
            strBranchId = sourceDesc.dataSource.getThisBranchId() ;
            if (bSetMatchBranchId)
                m_strMatchBranchId = strBranchId ;

            if ((strBranchId != null) && (strBranchId.length() > 0))
            {
                Graphics g = getGraphics() ;
                if (g != null)
                {
                    for (int i=0; i<m_model.getSize(); i++)
                    {
                        ChartDescriptor desc = m_model.getEntryAt(i) ;
                        if (strBranchId.equals(desc.dataSource.getThisBranchId()))
                        {
                            paintEntry(g, i) ;
                        }
                    }
                }
            }
            else
            {
                Graphics g = getGraphics() ;
                if (g != null)
                {
                    paintEntry(g, iSourceIndex) ;
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested classes
////

    protected class icMouseListener implements MouseListener
    {
        public void mouseClicked(MouseEvent e)
        {
            int iPos = PointToIndex(e.getPoint());
            String strText = getMessageText(iPos) ;

            JDialog dialog = new JDialog() ;
            dialog.setTitle("SIP Message");
            Container cont = dialog.getRootPane() ;
            cont.setLayout(new GridBagLayout());
            GridBagConstraints gbc = new GridBagConstraints() ;

            gbc.weightx = 1.0 ;
            gbc.weighty = 1.0 ;
            gbc.fill = GridBagConstraints.BOTH ;

            cont.add(new JScrollPane(new JTextArea(strText)), gbc) ;

            dialog.setSize(300, 200);
            dialog.pack() ;

            dialog.setVisible(true) ;
        }

        public void mousePressed(MouseEvent e)
        {

        }

        public void mouseReleased(MouseEvent e)
        {

        }

        public void mouseEntered(MouseEvent e)
        {

        }

        public void mouseExited(MouseEvent e)
        {
            setMouseOver(-1);
        }
    }


    public class icMouseMotionListener implements MouseMotionListener
    {
        public void mouseDragged(MouseEvent e)
        {

        }


        public void mouseMoved(MouseEvent e)
        {
            setMouseOver(PointToIndex(e.getPoint()));
        }
    }


    protected class icChartModelListener implements ChartModelListener
    {
        public void keyAdded(int position)
        {
            repaint() ;
        }


        public void keyDeleted(int position)
        {
            repaint() ;
        }


        public void keyMoved(int oldPosition, int newPosition)
        {
            repaint() ;
        }


        public void entryAdded(int startPosition, int endPosition)
        {
            repaint() ;
        }


        public void entryDeleted(int startPosition, int endPosition)
        {
            repaint() ;
        }
    }
}