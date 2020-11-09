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

#include <dbscan.h>
#include <CIEDE2000.h>


std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
int box_width = 200;
cv::RNG rng(12345);

struct Similarty {
  float distance;
  int x;
  int y;
};

bool similarity_sort(Similarty i, Similarty j) {
  bool result = i.distance < j.distance;
  return result;
}

// Returns a Vec3f with converted LAB color scheme
cv::Vec3f RBG2LAB(float r, float g, float b) {
  cv::Mat output;
  cv::Mat rgb(1, 1, CV_32FC3, cv::Scalar(r / 255.0, g / 255.0, b / 255.0));

  cv::cvtColor(rgb, output, cv::COLOR_RGB2Lab);
  return output.at<cv::Vec3f>(0, 0);
}

/**
 * - Read the image
 * - Crop out the xp drop
 * - Convert image to CV_32FC3 for further convertion to LAB
 *  - Without this, Lab values get converted to 255 range
 * - Convert BGR to Lab formats
 * - Return tuple with cropped, Lab converted images
 */
std::tuple<cv::Mat, cv::Mat> xp_drop_image(std::string& fname) {
  cv::Mat src, im;
  src = cv::imread(fname);

  int height = src.rows;
  int width = src.cols;

  cv::Mat cropped = src(cv::Rect(
    // X
    width / 2 - box_width / 2,
    // Y
    0,
    // Width
    box_width,
    // Height (Xp drops start in middle of the screen)
    height / 2
  ));

  cv::Mat mask, tmp_mask;
  /*
    OpenCV normalizes Lab to 255 range
    L ← L * 255/100 ; a ← a + 128 ; b ← b + 128
    But if we use 32F it doesn't 
   */
  cropped.convertTo(tmp_mask, CV_32FC3, 1.0/0xff);
  cv::cvtColor(tmp_mask, mask, cv::COLOR_BGR2Lab);
  return std::make_tuple(cropped, mask);
}

/**
 * Given a color and an image (Both in Lab color format)
 * Will return a list of coordinates and how close they are to the color
 */
std::vector<Similarty> find_similar(cv::Vec3f &col, cv::Mat& image) {
  CIEDE2000::LAB lab1 = { col[0], col[1], col[2]};
  CIEDE2000::LAB lab2;

  std::vector<Similarty> distances;

  for (int x = 0; x < image.rows; x += 1) {
    for (int y = 0; y < image.cols; y += 1) {
      cv::Vec3f c2 = image.at<cv::Vec3f>(x,y);
      lab2 = { c2[0], c2[1], c2[2] };
      float dist = CIEDE2000::CIEDE2000(lab1, lab2);
      Similarty tmp;
      tmp.distance = dist;
      tmp.x = x;
      tmp.y = y;
      distances.push_back(tmp);
    }
  }
  std::sort(distances.begin(), distances.end(), similarity_sort);
  return distances;
}

cv::Mat remove_isolated_pixels(cv::Mat &image) {
  int erosion_size = 1;
  int s = erosion_size * 2 + 1;

  // cv::Mat kernel1 = cv::getStructuringElement(
  //   cv::MORPH_RECT,
  //   cv::Size(s, s),
  //   cv::Point(erosion_size,erosion_size)
  // );

  cv::Mat kernel1 = (cv::Mat_<uchar>(3, 3) <<
        0, 0, 0,
        0, 1, 0,
        0, 0, 0);
  cv::Mat kernel2 = (cv::Mat_<uchar>(3, 3) <<
        1, 1, 1,
        1, 0, 1,
        1, 1, 1);
  cv::Mat image_eroded, reversed_eroded, difference, difference_inverted, reversed, output;

  // Invert image colors
  cv::bitwise_not(image, reversed);

  cv::morphologyEx(image, image_eroded, cv::MORPH_ERODE, kernel1, cv::Point(-1,-1), 1);
  cv::morphologyEx(reversed, reversed_eroded, cv::MORPH_ERODE, kernel2, cv::Point(-1,-1), 1);
  cv::bitwise_and(image_eroded, reversed_eroded, difference);

  // // Create the mask
  cv::bitwise_not(difference, difference_inverted);
  cv::bitwise_and(image, image, output, difference_inverted);

  return difference_inverted;
}

int main() {
  float similarity_cut_off = 25.0;

  auto [ cropped, mask ] = xp_drop_image(filename);

  cv::Vec3f c1 = RBG2LAB(199,158,80);
  std::vector<Similarty> ls = find_similar(c1, mask);
  int i = 0;
  for (; i < ls.size() && ls[i].distance < similarity_cut_off; i+= 1) {}
  ls.resize(i);

  // Draw points on an empty Mat
  cv::Mat output(cropped.rows, cropped.cols, CV_8U, 1);
  for (Similarty& s: ls) {
    output.at<uchar>(s.x, s.y) = 255;
  }

  cv::Mat result = remove_isolated_pixels(output);

  cv::Mat tmp, final_result;

  cv::hconcat(output, result, tmp);
  cv::resize(tmp, final_result, cv::Size(), 2, 2);

  cv::namedWindow( source_window );
  cv::imshow( source_window, final_result);
  
  cv::waitKey();

  return 0;
}