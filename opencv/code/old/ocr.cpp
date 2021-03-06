#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// int main(int argc, char *argv[]) {
//   string outText, imPath = argv[1];
//   Mat im = imread(imPath, IMREAD_COLOR);
//   tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
//   ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
//   ocr->SetPageSegMode(tesseract::PSM_AUTO);
//   ocr->SetImage(im.data, im.cols, im.rows, 3, im.step);
//   outText = string(ocr->GetUTF8Text());
//   cout << outText;
//   ocr->End();
// }

int main(int argc, char *argv[]) {
  string outText, imPath = argv[1];
  Mat im = imread(imPath, IMREAD_COLOR);
  tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
  ocr->SetPageSegMode(tesseract::PSM_AUTO);
  ocr->SetImage(im.data, im.cols, im.rows, 3, im.step);
  outText = string(ocr->GetUTF8Text());
  cout << outText;
  ocr->End();
}