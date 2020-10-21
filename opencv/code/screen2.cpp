#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <errno.h>

/* compile -lX11 */
 
Window *winlist (Display *disp, unsigned long *len);
char *winame (Display *disp, Window win); 
 
int main(int argc, char *argv[]) {

    int i;
    unsigned long len;
    Display *disp = XOpenDisplay(NULL);
    Window *list;
    XImage* img{nullptr};
 
    if (!disp) {
      puts("no display!");
      return -1;
    }
 
    list = (Window*)winlist(disp,&len);
    Window runescape = (Window) 67108875;
  
    // for (i=0;i<(int)len;i++) {
    //     std::string name(winame(disp,list[i]));
    //     if (name == "RuneScape") {
    //       runescape = list[i];
    //       printf("Window found\n");
    //       break;
    //     }   
    // }
    XWindowAttributes attr;
    if (XGetWindowAttributes(disp, runescape, &attr) != 1) {
      perror("main() -- XGetWindowAttributes");
    } else {
      img = XGetImage(
        disp,
        runescape,
        0,
        0,
        attr.width,
        attr.height,
        AllPlanes,
        ZPixmap
      );
      cv::Mat cvImg = cv::Mat(attr.height, attr.width, CV_8UC4, img->data);
      imwrite("test.jpg", cvImg);
    }

    if(img != nullptr) {
      XDestroyImage(img);
    }
    XFree(list);
    XCloseDisplay(disp);
    return 0;
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