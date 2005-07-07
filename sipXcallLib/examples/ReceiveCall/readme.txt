The ReceiveCall example is included as part of the sipXcallLib project to
demonstate receiving a call using the sipXtapi API.  This application is a
command line utility with the following switches:

    -d durationInSeconds (default=30 seconds)
    -t playT tones (default = none)
    -f play file (default = none)
    -p SIP port (default = 5060)
    -r RTP port start (default = 9000)
    -l loopback audio (2 second delay)
    -i line identity (e.g. sip:122@pingtel.com)
    -u username (for authentication)
    -a password  (for authentication)
    -m realm  (for authentication)
    -S stun server

For example, to automatically register a line appearance, answer it, any
place a audio prompt: 

ReceiveCall -u bandreasen -a password -m pingtel.com -i "Bob Andreasen"<sip:bandreasen@pingtel.com> -S stun.voipuser.org -f greeting.wav

Please see the README and INSTALL in the sipXcallLib for instructions on 
building sipXtapi.  For ReceiveCall, please either open and build using
the supplied .dsp (Windows) file or using automake/autoconf in the parent
directory (Linux).

