#define CAML_NAME_SPACE

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

extern "C"
{
  CAMLprim value get_windows()
  {
    CAMLparam0();
    CAMLlocal2(result, tmp);

    unsigned long len = 0;
    Display *disp = XOpenDisplay(NULL);
    Window *list = nullptr;

    result = Val_emptylist;

    if (disp) {
        list = (Window*)winlist(disp,&len);
        for (int i=0;i<(int)len;i++) {
          std::string name(winame(disp,list[i]));
          if (name == "RuneScape") {
            tmp = caml_alloc(2, 0);
            Store_field(tmp, 0, Val_long(list[i]));
            Store_field(tmp, 1, result);
            result = tmp;
          } 
        } 
    } else {
      result = caml_alloc(2, 0);
      Store_field(result, 0, Val_long(-1));
      Store_field(result, 1, Val_emptylist);
    }
    
    CAMLreturn(result);
  }
}