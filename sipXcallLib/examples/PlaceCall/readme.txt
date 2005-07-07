The PlaceCall example is included as part of the sipXcallLib project to
demonstate placing a call using the sipXtapi API.  This application is a
command line utility with the following switches:

    -d durationInSeconds (default=30 seconds)
    -t play tones (default = none)
    -f play file (default = none)
    -p SIP port (default = 5060)
    -r RTP port start (default = 9000)
    -R use rport as part of via (disabled by default)
    -u username (for authentication)
    -a password  (for authentication)
    -m realm  (for authentication)
    -i from identity
    -S stun server

For example, to place an authenticated call for 60 seconds:

PlaceCall -d 60 -R -u bandreasen -a password -m pingtel.com -i "Bob Andreasen"<sip:bandreasen@pingtel.com> -S stun.voipuser.org sip:target@host:port

Please see the README and INSTALL in the sipXcallLib for instructions on 
building sipXtapi.  For ReceiveCall, please either open and build using
the supplied .dsp (Windows) file or using automake/autoconf in the parent
directory (Linux).
