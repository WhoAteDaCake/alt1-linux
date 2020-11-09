#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <math.h>
#include <vector>
#include <algorithm> 
#include <map> 

#include <dbscan.h>
#include <CIEDE2000.h>

// 200 - width
// Should take half of the image

// 12 - 34
// 45 - 241
// 150 - 251

using namespace cv;
using namespace std;

cv::Mat findText(cv::Mat);
Mat detect_text(Mat large);

std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
const int box_width = 200;
RNG rng(12345);


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
  // int c_x, c_y;
  vector<Similarty> distances;

  for (int x = 0; x < mask.rows; x += 1) {
    for (int y = 0; y < mask.cols; y += 1) {
      Vec3f c2 = mask.at<cv::Vec3f>(x,y);
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
  cv::Mat output(cropped.rows, cropped.cols, CV_8UC3, cv::Scalar(0, 0, 0));
  vector<dbscan::Point> points;

  for (int i = 0; i < distances.size(); i += 1) {
    Similarty d1 = distances[i];
    dbscan::Point p;
    p.clusterID = UNCLASSIFIED;
    p.x = d1.x;
    p.y = d1.y;
    p.z = 0;
    points.push_back(p);
    output.at<Vec3b>(d1.x, d1.y) = Vec3b(255, 255, 255);

    if (d1.distance > 25.0) {
      // Vec3b c2 = cropped.at<cv::Vec3b>(d1.x,d1.y);
      // printf("Color: (%d, %d, %d) distance: %f\n", c2[2], c2[1], c2[0], d1.distance);
      break;
    }
  }
  // Cluser points
  dbscan::DBSCAN ds(10, box_width, points);
  ds.run();

  map<int, Vec3b> colors; 
  map<int, vector<Point>> point_map;
  for (int i = 0; i < ds.getTotalPointSize(); i += 1) {
    dbscan::Point p = ds.m_points[i];
    
    // Vec3b color;
    auto search = point_map.find(p.clusterID);
    if (search != point_map.end()) {
      search->second.push_back(Point(p.x, p.y));
      // color = search->second;
    } else {
      vector<Point> points;
      points.push_back(Point(p.x, p.y));
      point_map[p.clusterID] = points;
      // color = Vec3b(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256));
      // colors.insert({p.clusterID, color});
    }
   
    // output.at<Vec3b>(p.x, p.y) = color;
  }

  cv::namedWindow( source_window );
  
  for (auto it = point_map.begin(); it != point_map.end(); ++it) {
    Vec3b color = Vec3b(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256));
    Point lc(-1, -1);
    Point br(-1, -1);
    int i = 0;
    for (Point p: it->second) {
      i += 1;
      if (lc.x == -1 || lc.x > p.x && lc.y > p.y) {
        lc = Point(p.x, p.y);
      }
      if (br.x == -1 || br.x < p.x && br.y < p.x) {
        br = Point(p.x, p.y);
      }
      // if (lc.x == -1 || lc.x > p.x) {
      //   lc.x = p.x;
      // }
      // if (lc.y == -1 || lc.y > p.y) {
      //   lc.y = p.y;
      // }
      // if (br.x == -1 || br.x < p.x) {
      //   br.x = p.x;
      // }
      // if (br.y == -1 || br.y < p.y) {
      //   br.y = p.y;
      // }
      output.at<Vec3b>(p.x, p.y) = color;
    }
    printf("Corners(%d) [%d, %d] [%d, %d]\n", i, lc.x, lc.y, br.x, br.y);
    circle(output,
      lc,
      2,
      color,
      FILLED,
      LINE_8 );
    circle(output,
      br,
      2,
      color,
      FILLED,
      LINE_8 );
    // cv::rectangle(output, lc, br, color);
    
    // cv::imshow( source_window, output);
    // cv::waitKey();
    // drawRec(img, (p.x, y), (x + w, y + h), 255, 1)
  }
  // imwrite("output.jpg", output);
  // Mat result = detect_text(output);
  Mat tmp;
  cv::resize(output, tmp, cv::Size(), 2, 2);
  cv::namedWindow( source_window );
  cv::imshow( source_window, tmp);

  cv::waitKey();

  return 0;
}