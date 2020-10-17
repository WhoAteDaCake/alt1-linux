#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

RNG rng(12345);

int main(int argc, char** argv) {

    Mat src_gray;
    CommandLineParser parser( argc, argv, "{@input | HappyFish.jpg | input image}" );
    Mat src = imread( samples::findFile( parser.get<String>( "@input" ) ) );
    if( src.empty())
    {
      cout << "Could not open or find the image!\n" << endl;
      cout << "Usage: " << argv[0] << " <Input image>" << endl;
      return -1;
    }
    // Convert image to black/gray
    cvtColor( src, src_gray, COLOR_BGR2GRAY );

    // Find edges using canny algorithm
    Mat canny_output;
    int thresh = 100;
    Canny( src_gray, canny_output, thresh, thresh*2 );


    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    // dilate(canny_output, canny_output, Mat(), Point(-1,-1));
    findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );

    // Iterate through found countours
    vector<vector<Point>> squares;
    for(size_t i = 0; i < contours.size(); i++) {
        Point nw, ne, se, sw;
        // Set first point as the corners
        nw = ne = se = sw = contours[i][0];
        for (size_t j = 0; j < contours[i].size(); j+= 1) {
            Point c = contours[i][j];
            // North west
            if (c.x <= nw.x && c.y <= nw.y) {
                nw = c;
                continue;
            }
            // North east
            if (c.x >= ne.x && c.y <= ne.y) {
                ne = c;
                continue;
            }
            // South west
            if (c.x <= sw.x && c.y >= sw.y) {
                sw = c;
                continue;
            }
            // South east
            if (c.x >= se.x && c.y >= se.y) {
                se = c;
                continue;
            }
        }
        double t,b,l,r;
        t = cv::norm(nw-ne);
        b = cv::norm(sw-se);
        l = cv::norm(nw-nw);
        r = cv::norm(ne-se);

        // Rectangle expected
        if (!(t > l && t > r && b > l && b > r) || (t < 30)) {
            continue;
        }

        printf("t: %f b: %f l: %f r: %f\n", t, b, l, r);

        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        rectangle(src, nw, se, color, 2);
        // circle( src, ne, 5,  Scalar(255, 0, 0), 2, 8, 0);
        // circle( src, nw, 5,  Scalar(0, 255, 0), 2, 8, 0);
        // circle( src, se, 5,  Scalar(0, 0, 255), 2, 8, 0);
        // circle( src, sw, 5,  Scalar(255, 255, 255), 2, 8, 0);
    }


    const char* source_window = "Source";
    namedWindow( source_window );
    imshow( source_window, src );

    // Wait before exit
    waitKey();
    return 0;
}