libopus (codecs):
wget http://downloads.xiph.org/releases/opus/opus-1.2.1.tar.gz
tar -zxvf opus-1.2.1.tar.gz
mv opus-1.2.1 opus

# For Android build copied config.h from linux build (after .configure --enable-fixed-point)

libopusfile (opus file read and validation utilities):
wget https://downloads.xiph.org/releases/opus/opusfile-0.11.tar.gz
tar -zxvf opusfile-0.11.tar.gz
mv opusfile-0.11 opusfile

libogg (ogg file container utilities):
wget http://downloads.xiph.org/releases/ogg/libogg-1.3.4.tar.gz
tar -zxvf libogg-1.3.4.tar.gz
mv libogg-1.3.4 libogg

libopusenc (opus file creation utilities):
wget https://archive.mozilla.org/pub/opus/libopusenc-0.2.1.tar.gz
tar -zxvf libopusenc-0.2.1.tar.gz
mv libopusenc-0.2.1 libopusenc

