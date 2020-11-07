#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>

// 200 - width
// Should take half of the image

using namespace cv;
using namespace std;

cv::Mat findText(cv::Mat);

std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
const int box_width = 200;
// Vec3b color(176, 158, 115);


int main(int argc, char** argv)
{
  cv::Mat src, im;
  src = cv::imread(filename);
  

  int height = src.rows;
  int width = src.cols;

  int mp = height / 2;
  cv::Mat cropped = src(cv::Rect(
    // X
    width / 2 - box_width / 2,
    // Y
    0,
    // Width
    box_width,
    // Height
    mp
  ));
  Mat frame_HSV, frame_threshold;
  cvtColor(cropped, frame_HSV, COLOR_BGR2HSV);

  int range = 30;
  Scalar min_l(11, 22, 134);
  Scalar max_l(41, 170, 210);
  // Scalar min_l(34 - range, (28 - range) / 255, (57 - range) / 255);
  // Scalar min_h(34 + range, (28 + range ) / 255, (57 + range) / 255);
  // Detect the object based on HSV Range Values
  inRange(
    frame_HSV,
    min_l,
    max_l,
    frame_threshold
  );

  int thresh = 100;
  RNG rng(12345);
  // Canny(frame_threshold, im, thresh, thresh * 2);

  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(frame_threshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );
  for( size_t i = 0; i< contours.size(); i++ )
  {
    Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
    drawContours( frame_threshold, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
  }
  im = frame_threshold;
  // tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  // ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  // ocr->SetPageSegMode(tesseract::PSM_AUTO);
  // ocr->SetImage(im.data, im.cols, im.rows, 3, im.step);
  // std::string outText = std::string(ocr->GetUTF8Text());
  // ocr->End();
  
  // std::cout << outText << "\n";

  cv::namedWindow( source_window );
  cv::imshow( source_window, im);

  cv::waitKey();

  return 0;
}