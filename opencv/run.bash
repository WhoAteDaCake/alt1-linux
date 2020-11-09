#!/usr/bin/env bash
FLAGS="$(pkg-config --cflags --libs opencv4 tesseract x11) "

echo "File: $1"
echo "Output: $2"
echo "Directory $3"

g++ -g -O3 -std=c++17 \
  $1 \
  $3/opencv/code/include/CIEDE2000.cpp \
  $3/opencv/code/include/dbscan.cpp \
  -I$3/opencv/code/include \
  $FLAGS \
  -o $2
# ./basic_ocr "/home/augustinas/projects/github/alt1-linux/opencv/chat4.png"