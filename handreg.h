#ifndef HAND_REG__H
#define HAND_REG__H

#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

void show_image(const std::string name);
cv::Mat transforms(cv::Mat frame);
bool compareContourAreas (std::vector<cv::Point> contour1, std::vector<cv::Point> contour2);
cv::Point get_center(cv::Mat mask);
#endif
