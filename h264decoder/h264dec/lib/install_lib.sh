#!/bin/sh
##创建动态库的符号连接
ln -s libavcodec.so.55.52.102 libavcodec.so
ln -s libavcodec.so.55.52.102 libavcodec.so.55

ln -s libavformat.so.55.33.100 libavformat.so
ln -s libavformat.so.55.33.100 libavformat.so.55

ln -s libavutil.so.52.66.100 libavutil.so
ln -s libavutil.so.52.66.100 libavutil.so.52

ln -s libswscale.so.2.5.102 libswscale.so
ln -s libswscale.so.2.5.102 libswscale.so.2

rm -f /etc/ld.so.conf.d/ffmpeg4lib.conf
echo $PWD > /etc/ld.so.conf.d/ffmpeg4lib.conf

ldconfig



