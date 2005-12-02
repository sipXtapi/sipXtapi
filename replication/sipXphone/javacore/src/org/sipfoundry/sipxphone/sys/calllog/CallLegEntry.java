/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLegEntry.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.calllog;


import org.sipfoundry.stapi.* ;
import org.sipfoundry.util.Comparable;
import java.util.Hashtable;
import java.util.Enumeration;

/**
 * CallLegEntry.java
 *
 *
 * Created: Fri Sep 21 10:33:14 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class CallLegEntry implements java.io.Serializable, Comparable {



    /** call ID of this call leg */
    private String m_strCallID;

    /**
     *direction of this call leg
     * could be either INBOUND or OUTBOUND.
     * @see INBOUND
     * @see OUTBOUND
     */
    private int m_iDirection;

    /** remote address of this call leg.
     * If this call is INBOUND, then it will be
     * "From" address, if its OUTBOUND, it will be
     * "TO" addresss.
     */
    private PAddress m_addrRemoteAddress;

    /** local address of this call log. In most cases,
     * it will be the "usual" address of this leg.
     * It will be different in a scenario  like this
     * where a call to sales@pingtel.com is forwarded to you
     * in which case the local address is "sales@pingtel.com"
     * not the actual address of your phone.
     */
    private PAddress m_addrLocalAddress;

    /** starting time of this call leg in ms
     */
    private java.util.Date m_dateStartTime;

    /** end time of this cal leg in ms
     * not applicable if the cal was failed or missed.
     */
    private java.util.Date m_dateEndTime;

    /** state of this call leg.
     * This could be one of the following values:
     * ACTIVE, COMPLETE AND INCOMPLETE
     */
    private int m_iState;

    /** type of these calls
     * this could be one of the following values:
     * TRANSFERRED, FORWARDED.
     */
    private int m_iType;

    /** error code for the call.
     *  CAUSE_NORMAL (100) if normal. */
    private int m_iErrorCode;

    /** error text if the call was incomplete( failed or missed ).*/
    private String m_strErrorText;

    /** hashtable of keys and values
     *  This is useful when we come up with some extra information
     *  to be logged in a CallLegEntry and do not want the
     *  existing serialized call leg entries to still work.
     */
    private Hashtable m_hashKeysValues = new Hashtable();

    private CallLegSession m_callLegSession;

    /**
     * get the value of the field with the key strFieldName.
     * @return value of the key if any has been set, otherwise null.
     */
    public Object getOtherField( String strFieldName){
        return m_hashKeysValues.get( strFieldName);
    }

    /**
     * If both the params are not  NULL, sets the value of the key
     * strFieldName to the value of objFieldValue.
     * @param strFieldName key to set the value with
     * @param objFieldValue value to be set for the key
     */
    public void setOtherField( String strFieldName, Object objFieldValue ){
        if( ( strFieldName != null )  && (objFieldValue != null) ){
            m_hashKeysValues.put( strFieldName, objFieldValue);
        }
    }

    /**
     *  gets the name of all the "other" fields as an enumeration.
     *  @return enumeration of all "other" field names( or keys ).
     */
    public Enumeration getOtherFieldNames(){
        return m_hashKeysValues.keys();
    }

    /**
     * Get the value of callID.
     * @return value of callID.
     */
    public String getCallID() {
        return m_strCallID;
    }

    /**
     * Set the value of callID.
     * @param v  Value to assign to callID.
     */
    protected void setCallID(String  v) {
        this.m_strCallID = v;
    }


    /**
     * Get the value of direction.
     * @return value of direction.
     */
    public int getDirection() {
        return m_iDirection;
    }



    /**
     * Set the value of direction.
     * @param iDirection  Value to assign to direction.
     */
    protected void setDirection(int  iDirection) {
        this.m_iDirection = iDirection;
    }


    /**
     * Get the value of remoteAddress.
     * @return value of remoteAddress.
     */
    public PAddress getRemoteAddress() {
        return m_addrRemoteAddress;
    }

    /**
     * Set the value of remoteAddress.
     * @param addrRemoteAddress  Value to assign to remoteAddress.
     */
    protected void setRemoteAddress(PAddress  addrRemoteAddress) {
        this.m_addrRemoteAddress = addrRemoteAddress;
    }


    /**
     * Get the value of localAddress.
     * @return value of localAddress.
     */
    public PAddress getLocalAddress() {
        return m_addrLocalAddress;
    }

    /**
     * Set the value of localAddress.
     * @param addrLocalAddress  Value to assign to localAddress.
     */
    protected void setLocalAddress(PAddress  addrLocalAddress) {
        this.m_addrLocalAddress = addrLocalAddress;
    }


    /**
     * Get the value of startTime.
     * @return value of startTime.
     */
    public java.util.Date getStartTime() {
        return m_dateStartTime;
    }



    /**
     * Set the value of startTime.
     * @param dateStartTime  Value to assign to startTime.
     */
    protected void setStartTime(long  dateStartTime) {
        this.m_dateStartTime = new java.util.Date(dateStartTime);
    }


    /**
     * Get the value of endTime.
     * @return value of endTime.
     */
    public java.util.Date getEndTime() {
        return m_dateEndTime;
    }

    /**
     * Set the value of endTime.
     * @param dateEndTime  Value to assign to endTime.
     */
    protected void setEndTime(long  dateEndTime) {
        this.m_dateEndTime = new java.util.Date(dateEndTime);
    }


    /**
     * Get the value of state.
     * @return value of state.
     */
    public int getState() {
        return m_iState;
    }


    /**
     * Set the value of state.
     * @param v  Value to assign to state.
     */
    protected void setState(int  v) {
        this.m_iState = v;
    }


    /**
     * Get the value of type.
     * @return value of type.
     */
    public int getType() {
        return m_iType;
    }

    /**
     * Set the value of type.
     * @param v  Value to assign to type.
     */
    protected void setType(int  v) {
        this.m_iType = v;
    }


    /**
     * Get the value of errorCode.
     * @return value of errorCode.
     */
    public int getErrorCode() {
        return m_iErrorCode;
    }

    /**
     * Set the value of errorCode.
     * @param v  Value to assign to errorCode.
     */
    protected void setErrorCode(int  v) {
        this.m_iErrorCode = v;
    }


    /**
     * Get the value of errorText.
     * @return value of errorText.
     */
    public String getErrorText() {
        return m_strErrorText;
    }

    /**
     * Set the value of errorText.
     * @param v  Value to assign to errorText.
     */
    protected void setErrorText(String  v) {
        this.m_strErrorText = v;
    }



 /**
  * @return long gets the duration of the call in milliseconds.
  */
    public long getDuration(){
        long duration = -1;
        long startTime = -1;
        long endTime = -1;
        if( this.getStartTime() != null )
          startTime = this.getStartTime().getTime();
        if( this.getEndTime() != null )
          endTime = this.getEndTime().getTime();
        if( (startTime != -1) && (endTime != -1)){
            duration =endTime - startTime;
        }
        return duration;

    }





    /**
     * hack variable to set if the call was created and destroyed only
     * as we create and delete a call when we  call
     * playSound method in PMediaManager
     * @deprecated DO NOT EXPOSE
     */
    private boolean m_bCallCreatedEventOnly = false;

    /**
     * hack method to set if the call was created and destroyed only
     * as we create and delete a call when we  call
     * playSound method in PMediaManager
     * @deprecated DO NOT EXPOSE
     */
    protected  void setCallCreatedEventOnly(boolean bCallCreatedEventOnly){
          m_bCallCreatedEventOnly = bCallCreatedEventOnly;
    }

   /**
     * hack method to get if the call was created and destroyed only
     * as we create and delete a call when we  call
     * playSound method in PMediaManager
     * @deprecated DO NOT EXPOSE
     */
    public boolean getCallCreatedEventOnly(){
       return m_bCallCreatedEventOnly;
    }


  /**
   * @return CallLegSession the session associated with this entry.
   */
    public CallLegSession getCallLegSession(){
        return this.m_callLegSession;
    }

    /**
     * sets the call log session association to this call leg entry.
     * @param session CallLegSession to associate
     */
    protected void setCallLegSession(CallLegSession session){
        this.m_callLegSession = session;
    }

    /**
     * overrides equals method in Object class to tell
     * when two CallLegEntries can be called equal.
     * Two CallLenEntries are equal if their callIDs are equal
     * and their remote addresses are equal( if both of them are NOT NULL).
     * If one of the address is null, if the callIDs are equal, then
     * the call leg entries are considered equal.
     */
    public  boolean equals(Object obj ){
       boolean bRet = false;
       if( obj instanceof CallLegEntry ) {
          CallLegEntry entry = (CallLegEntry)(obj);
          if( entry != null ){
             if( (safeObjectCompare( entry.getCallID(),this.getCallID() ))
               && (safePAddressCompare( entry.getRemoteAddress(),
                this.getRemoteAddress() )) ){
               bRet = true;
            }
         }
       }
      return bRet;
    }

    /**
     * overrides hashCode method in Object class.
     */
    public int hashCode () {
        int hashCode = 0;

        if(this.getCallID() != null )
            hashCode += this.getCallID().hashCode();
        if( this.getRemoteAddress() != null )
          hashCode += this.getRemoteAddress().hashCode();

        if( hashCode == 0 )
          hashCode = super.hashCode();
        return hashCode;
    }

    /**
     * Safely determine if the two PAddresses are equal if they are both NOT
     * null. Only after they are both NOT NULL, then do check by "equals".
     * otherwise they are equal
     * kinda opposite to conventional thinking. It also strips "tag" token
     * from address if any before comparing.
     *
     */
    private boolean safePAddressCompare(PAddress objSource, PAddress objTarget)
    {
        if ((objSource == null) && (objTarget != null))
            return true ;

        if ((objSource != null) && (objTarget == null))
            return true ;

        if ((objSource == null) && (objTarget == null))
            return true ;
        else{
            return stripTag(objSource.getAddress())
                .equals(stripTag(objTarget.getAddress())) ;
        }
    }

    /**
     * Safely determine if the two specified objects  are identical.
     */
    private boolean safeObjectCompare(Object objSource, Object objTarget)
    {
        if ((objSource == null) && (objTarget != null))
            return false ;

        if ((objSource != null) && (objTarget == null))
            return false ;

        if ((objSource == null) && (objTarget == null))
            return true ;
        else
            return objSource.equals(objTarget) ;
    }

    /**
     * strips ";tag" from an address.
     */
    private String stripTag( String strAddress ){
        String strRet = strAddress;
        int index = -1;
        if( strAddress != null ){
            index = strAddress.indexOf(";tag");
            if( index > 1 )
                strRet = strAddress.substring(0, index);
        }
        return strRet;
    }

    /**
     * method to implement Comparable.
     * This is used to compare two CallLegEntries for
     * alpha-numeric comparison which, in turn, is based
     * on the alpha-numeric comparison of remote addresses.
     * @param obj CallLegEntry object to compare.
     */
    public int compareTo(Object obj){
        //for alpha numeric comparison
       int iRc = 0;
       CallLegEntry entry = (CallLegEntry)obj;
       if( entry != null ){
            if ( (this.getRemoteAddress() != null) &&
                 (entry.getRemoteAddress() != null) ){
                   iRc = this.getRemoteAddress().getAddress().compareTo
                    (entry.getRemoteAddress().getAddress()) ;
            }
       }
       return iRc;
    }


}// CallLegEntry




