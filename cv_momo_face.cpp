#include <opencv2/opencv.hpp>

int main()
{
    // 加载人脸识别器和猫咪脸部图像
    cv::CascadeClassifier faceCascade;
    faceCascade.load("haarcascade_frontalface_default.xml");
    cv::Mat catFace = cv::imread("R-C.png");

    // 打开摄像头
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cout << "无法打开摄像头" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true)
    {
        // 读取摄像头画面
        cap >> frame;

        // 转换为灰度图像
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // 人脸检测
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.3, 5);

        // 替换人脸为猫咪脸部
        for (const auto& face : faces)
        {
            cv::resize(catFace, catFace, cv::Size(face.width, face.height));
            cv::Mat roi = frame(face);
            catFace.copyTo(roi);
        }

        // 显示结果
        cv::imshow("猫咪脸部替换", frame);

        // 按下ESC键退出
        if (cv::waitKey(1) == 27)
            break;
    }

    // 释放资源
    cap.release();
    cv::destroyAllWindows();

    return 0;
}