#!/usr/bin/env bash
FLAGS="$(pkg-config --cflags --libs opencv4 tesseract x11) "

g++ -O3 -std=c++17 \
  $1 \
  ./code/include/CIEDE2000.cpp \
  ./code/include/dbscan.cpp \
  -I./code/include \
  $FLAGS \
  -o basic_ocr
# ./basic_ocr "/home/augustinas/projects/github/alt1-linux/opencv/chat4.png"