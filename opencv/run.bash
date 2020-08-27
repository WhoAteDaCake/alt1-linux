FLAGS="$(pkg-config --cflags --libs opencv4 tesseract)"

g++ -O3 -std=c++11 $1 $FLAGS -o basic_ocr
# ./basic_ocr "/home/augustinas/Pictures/fish2.png"