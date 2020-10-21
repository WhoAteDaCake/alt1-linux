#define CAML_NAME_SPACE


// Needs to be included before X11
#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <string>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <errno.h>

#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#define SUCCESS 0
#define NO_DISPLAY 1
#define NO_ATTRIBUTES 2

extern "C"
{
  CAMLprim value screenshot_window(
    value a_file_name,
    value a_window_id,
    value a_width,
    value a_height
  )
  {
    CAMLparam4(a_file_name, a_window_id, a_width, a_height);
    CAMLlocal1(result);
    const char* file_name = String_val(a_file_name);
    int width = Int_val(a_width);
    int height = Int_val(a_height);

    Window window_id = (Window) Int_val(a_window_id);
    XImage* img{nullptr};   

    Display *disp = XOpenDisplay(NULL);
    int retcode = SUCCESS;
    if (disp) {
      XWindowAttributes attr;
      if (XGetWindowAttributes(disp, window_id, &attr) != 0) {
        img = XGetImage(
          disp,
          window_id,
          0,
          0,
          attr.width,
          attr.height,
          AllPlanes,
          ZPixmap
        );
        cv::Mat cvImg = cv::Mat(attr.height, attr.width, CV_8UC4, img->data);
        // cv::Mat outputImg;
        // // Resize if needed
        // if (width != 0 && height != 0) {
        //   float w_ratio = ((float) width) / ((float) attr.width);
        //   float h_ratio = ((float) height) / ((float) attr.height);
        //   cv::resize(cvImg, outputImg, cv::Size(), w_ratio, h_ratio);
        // } else {
        //   outputImg = cvImg;
        // }
        // imwrite(file_name, outputImg);
        imwrite(file_name, cvImg);
      } else {
        retcode = NO_ATTRIBUTES;
      }
    } else {
      retcode = NO_DISPLAY;
    }
    
    if (img != nullptr) {
      XDestroyImage(img);
    }
    XCloseDisplay(disp); 

    result = Val_int(retcode);
    CAMLreturn(result);
  }
}