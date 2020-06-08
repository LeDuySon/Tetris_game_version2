#include <cmath>
#include <iostream>
#include <string>
#include "handreg.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

void show_image(const std::string name){
    Mat image;
    image = imread(name);
    if(!image.data )
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", image );

    waitKey(0);
}


cv::Mat transforms(cv::Mat frame){
    Mat maskS;
    cvtColor(frame, frame, CV_BGR2HSV);
    inRange(frame, Scalar(0,30,60,0), Scalar(20,150,255,0), maskS);
    erode(maskS, maskS, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    dilate(maskS, maskS, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    GaussianBlur(maskS, maskS, Size(5,5), 1.5, 1.5);
    return maskS;
}

bool compareContourAreas (vector<Point> contour1, vector<Point> contour2) {
    double i = fabs( contourArea(Mat(contour1)) );
    double j = fabs( contourArea(Mat(contour2)) );
    return (i < j);
}

cv::Point get_center(cv::Mat mask){
    vector<vector<Point> > contours;
    Mat contourOutput = mask.clone();
    findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE );
    if(contours.size() != 0){
        sort(contours.begin(), contours.end(), compareContourAreas);
        vector<Point> biggestContour = contours[contours.size()-1];
        if(contourArea(Mat(biggestContour)) > 200){
             Moments m = moments(biggestContour,true);
            //compute center coordinate
            Point p(m.m10/m.m00, m.m01/m.m00);
            return p;
        }
    }
    return Point(0, 0);


}


