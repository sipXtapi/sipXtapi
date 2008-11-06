
= Troubleshooting =

Q: When linking mstream I'm getting a lot of unresolved methods from Url class like this:

sipXmediaAdapterLib/sipXmediaMediaProcessing/src/CpPhoneMediaInterface.cpp:1374: undefined reference to `Url::Url(char const*, int)'
sipXmediaAdapterLib/sipXmediaMediaProcessing/src/CpPhoneMediaInterface.cpp:1376: undefined reference to `Url::Url(Url const&)'
sipXmediaAdapterLib/sipXmediaMediaProcessing/src/CpPhoneMediaInterface.cpp:1376: undefined reference to `Url::~Url()'

A: You need to compile sipXmediaLib and sipXmedaAdapterLib with DISABLE_STREAM_PLAYER preprocessor
definition set. Under Linux you can do this by passing --disable-stream-player to ./configure
of sipXmediaLib and sipXmediaAdapterLib.