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

#include <chrono>
#include <thread>

#include <dbscan.h>
#include <CIEDE2000.h>

typedef std::map<int, std::vector<cv::Point>> Clusters;

std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
int box_width = 200;
cv::RNG rng(12345);

struct Similarty {
  float distance;
  int x;
  int y;
};

struct ClusterMeta {
  std::vector<cv::Point> points;
  // Bounding box corners:
  // Left top corner
  cv::Point ltc;
  // Right bottom corner 
  cv::Point rbc;
  int width;
  int height;
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

/** 
 * - Removes isolated pixels by checking within provided box radius
 * - If there are less than min_n neighbours, it will remove all pixels within the box
 */ 
cv::Mat remove_isolated_pixels(cv::Mat &image, cv::Size box, int min_n) {
  cv::Mat dst = image.clone();

  std::vector<cv::Point2i> ls;

  for (int x = 0; x < (dst.rows - box.height); x += box.height) {
    for (int y = 0; y < (dst.cols - box.width); y += box.width) {
      // printf("Runing (%d, %d)\n", x, y);
      // Empty vector
      ls.clear();
      for (int x1= 0; x1 < box.height && x + x1 < dst.rows; x1 += 1) {
        for (int y1 = 0; y1 < box.width && y + y1 < dst.cols; y1 += 1) {
          int value = (uchar)dst.at<uchar>(x + x1, y + y1);
          if (value == 255) {
            ls.push_back(cv::Point2i(x + x1, y + y1));
          }
        }
      }
      // Validate that enough neighbours exist
      if (ls.size() != 0 && ls.size() < min_n) {
        for (cv::Point2i p: ls) {
          dst.at<uchar>(p.x, p.y) = 1;
        }
      }
    }
  }
  return dst;
}

/**
 * Clusters pixels by their coordinates 
 * @param image - CV_8U matrix containing, containing only black/white pixels
 * @param min_n - Minimum number of neighbours for clustering algorithm
 * @param epsilon - Cluster range (should be estimated text width)
 * @return Map containing a list of coordinates in each cluster
 */
Clusters cluster_pixels(cv::Mat &image, int min_n, float epsilon) {
  std::vector<dbscan::Point> points;

  for (int x = 0; x < image.rows; x += 1) {
    for (int y = 0; y < image.cols; y += 1) {
      int value = (uchar)image.at<uchar>(x, y);
      if (value == 255) {
          // Remember when using point the x and y are inverted,
          // so src.at(i,j) is using (i,j) as (row,column)
          // but Point(x,y) is using (x,y) as (column,row)
          dbscan::Point p;
          p.clusterID = UNCLASSIFIED;
          p.x = y;
          p.y = x;
          p.z = 0;
          points.push_back(p);
      }
    }
  }

  dbscan::DBSCAN ds(10, epsilon, points);
  ds.run();

  Clusters point_map;
  for (int i = 0; i < ds.getTotalPointSize(); i += 1) {
    dbscan::Point p = ds.m_points[i];
    if (p.clusterID == UNCLASSIFIED) {
      continue;
    }
    cv::Point2i coord = cv::Point2i(p.x, p.y);

    auto search = point_map.find(p.clusterID);
    if (search != point_map.end()) {
      search->second.push_back(coord);
    } else {
      std::vector<cv::Point2i> points = {coord};
      point_map[p.clusterID] = points;
    }
  }
  return point_map;
}

/**
 * Return clusters which are presumed to be text rectangles
 */
std::map<int, ClusterMeta> rectangles_only (Clusters& clusters) {
  std::map<int, ClusterMeta> filtered;
  for (auto &it: clusters) {
    cv::Point2i lc(0, 0);
    cv::Point2i br(0, 0);

    for (cv::Point2i p: it.second) {
      if (lc.x == 0 || lc.x > p.x) {
        lc.x = p.x;
      }
      if (lc.y == 0 || lc.y > p.y) {
        lc.y = p.y;
      }
      if (br.x == 0 || br.x < p.x) {
        br.x = p.x;
      }
      if (br.y == 0 || br.y < p.y) {
        br.y = p.y;
      }
    }
    int width = abs(br.x - lc.x);
    int height = abs(lc.y - br.y);
    // We have a text rectangle 
    if (width > height * 2) {
      ClusterMeta meta = { it.second, lc, br, width, height };
      filtered[it.first] = meta;
    }
  }
  return filtered;
}

/**
 * @param clusters found clusters that might contain text
 * @param rows rows of original matrix
 * @param cols cols of original matrix
 */
std::vector<std::string> extract_text(std::map<int, ClusterMeta> &clusters, int rows, int cols) {
  std::vector<std::string> found;

  tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  // ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetPageSegMode(tesseract::PSM_AUTO);
  
  // TODO: Potential optimisation of normalizing coordinates, instead
  // of doing 2 clones
  // So we can create a smaller Matrix straight away
  for (auto &it: clusters) {
    // Could move outside, as clusters should never overlap
    cv::Mat cluster_view(rows, cols, CV_8UC3, cv::Vec3b(255, 255, 255));
    auto data = it.second;
    for (cv::Point2i p: data.points) {
      cluster_view.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 0, 0);
    }
    cv::Mat piece = cluster_view(cv::Rect(
      // X
      data.ltc.x,
      // Y
      data.ltc.y,
      // Width
      data.width,
      // Height (Xp drops start in middle of the screen)
      data.height
    ));
    // OCR

    ocr->SetImage(piece.data, piece.cols, piece.rows, 3, piece.step);
    std::string outText = std::string(ocr->GetUTF8Text());
    if (outText.empty()) {
      // std::cout << "Nothing found \n";
    } else {
      found.push_back(outText);
      // std::cout << outText << std::endl;
    }

    ocr->Clear();
  }
  ocr->End();
  // Should move ocr outside for optimistaion ?
  delete ocr;

  return found;
}

std::vector<std::string> detect_text(
  cv::Mat image,
  cv::Mat mask,
  cv::Vec3f text_color,
  int cluster_radius,
  float similarity_cut_off
) {
  std::vector<Similarty> ls = find_similar(text_color, mask);

  // 
  int i = 0;
  for (; i < ls.size() && ls[i].distance < similarity_cut_off; i+= 1) {}
  ls.resize(i);

  // Draw points on an empty Mat
  cv::Mat output(image.rows, image.cols, CV_8U, 1);
  for (Similarty& s: ls) {
    output.at<uchar>(s.x, s.y) = (uchar)255;
  }

  // Text will flow horizontally, meaning if we are scanning, it's
  // best to try with a rectange
  cv::Mat cleaned = remove_isolated_pixels(output, cv::Size(8, 4), 4);

  // Run clustering algorithm to isolate groups
  auto clusters = cluster_pixels(cleaned, 10, cluster_radius);
  
  auto rectangle_clusters = rectangles_only(clusters);
  return extract_text(rectangle_clusters, image.rows, image.cols);
}

int main() {
  float similarity_cut_off = 25.0;

  auto [ cropped, mask ] = xp_drop_image(filename);

  cv::Vec3f c1 = RBG2LAB(199,158,80);
  std::vector<std::string> found_text =
    detect_text(cropped, mask, c1, box_width, similarity_cut_off);
  
  printf("------------------------------------\n");
  for (auto text: found_text) {
    printf("[%s]\n", text.c_str());
  }

  return 0;
}