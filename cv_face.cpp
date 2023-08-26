#include <opencv2/opencv.hpp>

int main() {
    // 创建一个VideoCapture对象用于打开摄像头
    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "Failed to open the camera." << std::endl;
        return -1;
    }

    // 加载人脸识别的Haar级联分类器
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("D:/app/vcpkg/vcpkg/buildtrees/opencv4/src/4.7.0-87379d1df6.clean/data/haarcascades/haarcascade_frontalface_default.xml")) {
        std::cout << "Failed to load the face cascade classifier." << std::endl;
        return -1;
    }

    // 创建窗口
    cv::namedWindow("Face Detection", cv::WINDOW_NORMAL);

    while (true) {
        // 读取图像帧
        cv::Mat frame;
        capture >> frame;

        // 转换为灰度图像
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // 检测人脸
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, cv::Size(30, 30));

        // 在检测到的人脸周围绘制矩形框
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 2);
        }

        // 显示结果图像
        cv::imshow("Face Detection", frame);

        // 按下Esc键退出循环
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // 释放资源
    capture.release();
    cv::destroyAllWindows();

    return 0;
}