==============================================================================
                     s i p X e z P h o n e   R E A D M E 
==============================================================================

sipXezPhone is SIP soft phone client for Microsoft Windows and Linux.  The phone 
client supports hold, mute, client-mixed conferencing, blind transfer, and two way video.

Contents:

  Basic Configuration
  Release Notes

===================
Basic Configuration
===================
Configuration Settings for the SIPFoundry sipXezPhone:

Identity: 	The sip url that can be used to contact you at this phone
		e.g. - sip:115@pingtel.com

Realm:		The "group" name on the proxy/registrar server that
		your Username belongs to.
		e.g. - pingtel.com

Username:	The authentication username for the account on
		your proxy/registrar server.

Password:	The authentication password for the account on
		your proxy/registrar server.

Proxy Server:	Hostname of your outbound proxy/registrar server.

Enable r-port:	Check this box if your want SIP signal and RTP packets to
		be sent back to the original (return) port that requested them.
		This setting is useful if you are behind a firewall.

Stun Server:	Hostname of a server running a STUN service.
		STUN is an IP address "reflection" service that will allow
		you to use the phone if you are behind a firewall.
		e.g. - larry.gloo.net

Auto Answer:    Enable auto answer mode to automatically answer all incoming calls.
	        ( NOTE:  Auto Answer mode is not yet fully implemented and may cause
                  issues and/or crashes if used.         )


===================
Releae Notes
===================
The following are known issues in sipXezPhone

	Pressing the hold button during a conference will
        not put all conference members on hold.

        Pressing the enter key causes the phone to dial the 
        number or sip url in the dial string edit box, even
        if the current focus is in the conference member 
        edit box.

        Video periodically crashes due to a known issue with
        the 3rd party video processing library.

        The only camera that has been tested is the Logitech WebCam.

	The Received Calls, Placed Calls, and Contact List screens are
        not yet implemented.

	Auto Answer mode is not yet fully implemented and may cause
        issues and/or crashes if used.         

	Tearing down a video call may take 15+ seconds before it is completed.
        This is caused by a CPU spin in the video processing libraries.
        Attempting to setup calls during the call tear down process will fail.
	


  