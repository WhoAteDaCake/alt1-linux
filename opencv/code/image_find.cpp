// Find close enough colors
// Filter out to top n matches
// Clean out empty pixels - This step prevents one off pixels from being assigned.
// Perform the grouping algortihm

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <math.h>
#include <vector>
#include <algorithm> 
#include <map>
#include <tuple>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <chrono>
#include <thread>

#include <dbscan.h>
#include <CIEDE2000.h>

#include <loguru.hpp>
#include <loguru.cpp>

std::string number_img_path = "/home/augustinas/projects/github/alt1-linux/opencv/images/cutout/2.png"
std::string main_img_path = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop_small.png"

int main() {
  
}