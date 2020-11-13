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


typedef std::map<int, std::vector<cv::Point>> Clusters;

std::string filename = "/home/augustinas/projects/github/alt1-linux/opencv/tmp/xp_drop.jpg";
const char* source_window = "Source image";
int box_width = 200;
cv::RNG rng(12345);
cv::Mat tmp;

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

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

static inline bool similarity_sort(Similarty i, Similarty j) {
  bool result = i.distance < j.distance;
  return result;
}


Window *winlist (Display *disp, unsigned long *len) {
    Atom prop = XInternAtom(disp,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;
 
    errno = 0;
    if (XGetWindowProperty(disp,XDefaultRootWindow(disp),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list) != Success) {
        perror("winlist() -- GetWinProp");
        return 0;
    }
     
    return (Window*)list;
}
 
 
char *winame (Display *disp, Window win) {
    Atom prop = XInternAtom(disp,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;
 
    errno = 0;
    if (XGetWindowProperty(disp,win,prop,0,1024,False,XA_STRING,
                &type,&form,&len,&remain,&list) != Success) {
        perror("winlist() -- GetWinProp");
        return NULL;
    }
 
    return (char*)list;
}

cv::Mat screenshot(Display *disp, Window window_id) {
  XWindowAttributes attr;
  XGetWindowAttributes(disp, window_id, &attr);

  int width = attr.width;
  int height = attr.height;
  XImage* img = XGetImage(
    disp,
    window_id,
    0,
    0,
    attr.width,
    attr.height,
    AllPlanes,
    ZPixmap
  );

  int bits_per_px = img->bits_per_pixel;

  cv::Mat output(height, width, bits_per_px > 24 ? CV_8UC4 : CV_8UC3, img->data), result;
  if (bits_per_px > 24) {
    cv::cvtColor(output, result, cv::COLOR_RGBA2RGB);
  } else {
    result = output;
  }
  XDestroyImage(img);
  return result;
  // return img;
  //Mat(Size(Height, Width), Bpp > 24 ? CV_8UC4 : CV_8UC3, &Pixels[0]); 
  // cv::Mat cvImage = cv::Mat(attr.height, attr.width, CV_8UC4, img->data);
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
cv::Mat remove_isolated_pixels(cv::Mat &input, cv::Size box, int min_n) {
  cv::Mat image = input.clone();

  std::vector<cv::Point2i> ls;
  for (int x = 0; x < (image.rows - box.height); x += box.height) {
    for (int y = 0; y < (image.cols - box.width); y += box.width) {
      // Box iteration
      for (int x1= 0; x1 < box.height && x + x1 < image.rows; x1 += 1) {
        for (int y1 = 0; y1 < box.width && y + y1 < image.cols; y1 += 1) {
          int value = (uchar)image.at<uchar>(x + x1, y + y1);
          if (value == 255) {
            ls.push_back(cv::Point2i(x + x1, y + y1));
          }
        }
      }
      // Validate that enough neighbours exist
      if (ls.size() != 0 && ls.size() < min_n) {
        for (cv::Point2i p: ls) {
          image.at<uchar>(p.x, p.y) = 1;
        }
      }
      ls.clear();
    }
  }
  return image;
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
    // + higher than font 12
    // Might change in the future
    if ((float)width > 1.5 * (float)height && height > 10) {
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

  tesseract::TessBaseAPI ocr = tesseract::TessBaseAPI();
  ocr.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr.SetVariable("user_defined_dpi", "300");
  // ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr.SetPageSegMode(tesseract::PSM_AUTO);
  
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
    cv::Mat tmp = cluster_view(cv::Rect(
      // X
      data.ltc.x,
      // Y
      data.ltc.y,
      // Width
      data.width,
      // Height (Xp drops start in middle of the screen)
      data.height
    ));
    cv::Mat piece;
    cv::resize(tmp, piece, cv::Size(), 2, 2);
    // OCR
    // printf("Width: %d, Height: %d\n", piece.cols, piece.rows);

    ocr.SetImage(piece.data, piece.cols, piece.rows, 3, piece.step);
    std::string text = std::string(ocr.GetUTF8Text());
    if (!text.empty()) {
      // Remove training new line
      if (text[text.length()-1] == '\n') {
        text.erase(text.length()-1);
      }
      // Make sure to remove whitespace around it
      trim(text);
      // If it was only character, need another check
      if (!text.empty()) {
        found.push_back(text);
      }
    }

    ocr.Clear();
  }
  ocr.End();
  // Should move ocr outside for optimistaion ?
  return found;
}

std::vector<std::string> detect_text(
  cv::Mat &image,
  cv::Mat &mask,
  cv::Vec3f &text_color,
  int cluster_radius,
  float similarity_cut_off
) {
  std::vector<Similarty> ls = find_similar(text_color, mask);

  int i = 0;
  for (; i < ls.size() && ls[i].distance < similarity_cut_off; i+= 1) {}
  ls.resize(i);

  // Draw points on an empty Mat
  cv::Mat output(image.size(), CV_8U, (uchar) 1);
  for (Similarty& s: ls) {
    output.at<unsigned char>(s.x, s.y) = (unsigned char)255;
  }

  // Text will flow horizontally, meaning if we are scanning, it's
  // best to try with a rectange
  cv::Mat cleaned = remove_isolated_pixels(output, cv::Size(8, 4), 4);

  // DEBUG
  cv::Vec3b white(255, 255, 255);
  tmp = cv::Mat(cleaned.size(), CV_8UC3, cv::Vec3b(0, 0, 0));
  for (int x = 0; x < cleaned.rows; x += 1) {
    for (int y = 0; y < cleaned.cols; y += 1) {
      if (cleaned.at<uchar>(x, y) == 255) {
        tmp.at<cv::Vec3b>(x, y) = cv::Vec3b(255, 255, 255);
      }
    }
  }
  // cv::imshow( source_window, tmp);
  // cv::waitKey(1);

  // Run clustering algorithm to isolate groups
  auto clusters = cluster_pixels(cleaned, 10, cluster_radius);
  
  auto rectangle_clusters = rectangles_only(clusters);
  return extract_text(rectangle_clusters, image.rows, image.cols);
}

int main(int argc, char* argv[]) {
  // Logging tool
  loguru::init(argc, argv);

  

  float similarity_cut_off = 25.0;

  int i;
  unsigned long len;
  Display *disp = XOpenDisplay(NULL);
  Window *list;

  if (!disp) {
    puts("no display!");
    return -1;
  }

  list = (Window*)winlist(disp,&len);
  Window runescape;

  for (i=0;i<(int)len;i++) {
    std::string name(winame(disp,list[i]));
    if (name == "RuneScape") {
      runescape = list[i];
      printf("Window found\n");
      break;
      }   
  }

  cv::Vec3f c1 = RBG2LAB(199,158,80);
  cv::namedWindow( source_window );
  while (true) {
    LOG_F(INFO, "Process starting");
    cv::Mat cvImage = screenshot(disp, runescape);
    LOG_F(INFO, "Screenshot taken");
    
    int height = cvImage.rows;
    int width = cvImage.cols;
    cv::Mat cropped = cvImage(cv::Rect(
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
    LOG_F(INFO, "Image ready for processing");

    std::vector<std::string> found_text =
      detect_text(cropped, mask, c1, box_width, similarity_cut_off);

    for (auto text: found_text) {
      LOG_F("Text found: [%s]\n", text.c_str());
    }
    sleep(0.1);
  }
   


  // auto [ cropped, mask ] = xp_drop_image(filename);

  // cv::Vec3f c1 = RBG2LAB(199,158,80);
  // std::vector<std::string> found_text =
  //   detect_text(cropped, mask, c1, box_width, similarity_cut_off);
  
  // printf("------------------------------------\n");
  // for (auto text: found_text) {
  //   printf("[%s]\n", text.c_str());
  // }
  // cv::Mat cvImage = screenshot(disp, runescape);
  // cv::namedWindow( source_window );
  // cv::imshow( source_window, cvImage);
  
  // cv::waitKey();

  return 0;
}