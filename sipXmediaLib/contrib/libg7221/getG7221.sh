#!/bin/sh
# Gets G.722.1 lib from free switch
versionName="freeswitch-1.6.19"

wget https://files.freeswitch.org/freeswitch-releases/${versionName}.tar.gz
tar -zxvf ${versionName}.tar.gz ${versionName}/libs/libg722_1
rm ${versionName}.tar.gz

mkdir src
for file in ${versionName}/libs/libg722_1/src/*.{c,h,in,am} 
do
  cp $file src
done

for dir in config doc src/g722_1 tests test-data test-data/itu test-data/local
do
  mkdir $dir
  for file in ${versionName}/libs/libg722_1/${dir}/*
  do
    cp $file $dir
  done
done

for file in COPYING ChangeLog INSTALL NEWS README AUTHORS configure.ac Makefile.am g722_1.pc.in g722_1.spec.in
do
  cp ${versionName}/libs/libg722_1/$file .
done

#rm  -rf ${versionName}

