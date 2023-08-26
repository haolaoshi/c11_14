#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
    cv::Mat image = cv::imread("C:/Users/bonjour/Videos/1.jpg", cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cout << "Failed to load image." << std::endl;
        return -1;
    }

    cv::namedWindow("Image", cv::WINDOW_NORMAL);
    cv::imshow("Image", image);
    cv::waitKey(0);

    return 0;
}