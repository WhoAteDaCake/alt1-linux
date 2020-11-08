#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <math.h>

#include <CIEDE2000.h>

// 200 - width
// Should take half of the image

// 12 - 34
// 45 - 241
// 150 - 251

using namespace cv;
using namespace std;

cv::Mat findText(cv::Mat);

std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
const int box_width = 200;

// Returns a Vec3f with converted LAB color scheme
Vec3f RBG2LAB(float r, float g, float b) {
  Mat output;
  Mat rgb(1, 1, CV_32FC3, Scalar(r / 255.0, g / 255.0, b / 255.0));

  cvtColor(rgb, output, COLOR_RGB2Lab);
  return output.at<Vec3f>(0, 0);
}

int main(int argc, char** argv)
{
  cv::Mat src, im;
  // by default color will be bgr
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
  Mat mask, tmp_mask;
  /*
    OpenCV normalizes Lab to 255 range
    L ← L * 255/100 ; a ← a + 128 ; b ← b + 128
    But if we use 32F it doesn't 
   */

  Vec3b final_res;
  float c_dist = -1;
  
  cropped.convertTo(tmp_mask, CV_32FC3, 1.0/0xff);
  cvtColor(tmp_mask, mask, COLOR_BGR2Lab);
  // Color: (172, 134, 174)
  // Color: (67.450980, 6, 46)

  Vec3f c1 = RBG2LAB(199,158,80);
  CIEDE2000::LAB lab1 = { c1[0], c1[1], c1[2]};
  CIEDE2000::LAB lab2;
  int c_x, c_y;

  for (int x = 0; x < mask.rows; x += 1) {
    for (int y = 0; y < mask.cols; y += 1) {
      Vec3f c2 = mask.at<cv::Vec3f>(x,y);
      lab2 = { c2[0], c2[1], c2[2] };
      float dist = CIEDE2000::CIEDE2000(lab1, lab2);
      if (c_dist == -1 || dist < c_dist) {
        c_dist = dist;
        c_x = x;
        c_y = y;
      }
    }
  }
  Vec3b c2 = cropped.at<cv::Vec3b>(c_x,c_y);
  printf("Color: (%d, %d, %d) distance: %f\n", c2[2], c2[1], c2[0], c_dist);
  // printf("Color: (%d, %d, %d) distance: %f\n", final_res[0], final_res[1], final_res[2], c_dist);

  // int range = 30;
  // Scalar min_l(11, 22, 134);
  // Scalar max_l(41, 170, 210);
  // Scalar min_l(34 - range, (28 - range) / 255, (57 - range) / 255);
  // Scalar min_h(34 + range, (28 + range ) / 255, (57 + range) / 255);
  // Detect the object based on HSV Range Values
  // inRange(
  //   frame_HSV,
  //   min_l,
  //   max_l,
  //   frame_threshold
  // );

  // int thresh = 100;
  // RNG rng(12345);
  // Canny(frame_threshold, im, thresh, thresh * 2);

  // vector<vector<Point>> contours;
  // vector<Vec4i> hierarchy;
  // findContours(frame_threshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );
  // for( size_t i = 0; i< contours.size(); i++ )
  // {
  //   Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
  //   drawContours( frame_threshold, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
  // }
  // im = frame_threshold;
  // tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  // ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  // ocr->SetPageSegMode(tesseract::PSM_AUTO);
  // ocr->SetImage(im.data, im.cols, im.rows, 3, im.step);
  // std::string outText = std::string(ocr->GetUTF8Text());
  // ocr->End();
  
  // std::cout << outText << "\n";

  // cv::namedWindow( source_window );
  // cv::imshow( source_window, im);

  // cv::waitKey();

  return 0;
}