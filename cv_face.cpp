#include <opencv2/opencv.hpp>

int main() {
    // ����һ��VideoCapture�������ڴ�����ͷ
    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "Failed to open the camera." << std::endl;
        return -1;
    }

    // ��������ʶ���Haar����������
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("D:/app/vcpkg/vcpkg/buildtrees/opencv4/src/4.7.0-87379d1df6.clean/data/haarcascades/haarcascade_frontalface_default.xml")) {
        std::cout << "Failed to load the face cascade classifier." << std::endl;
        return -1;
    }

    // ��������
    cv::namedWindow("Face Detection", cv::WINDOW_NORMAL);

    while (true) {
        // ��ȡͼ��֡
        cv::Mat frame;
        capture >> frame;

        // ת��Ϊ�Ҷ�ͼ��
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // �������
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, cv::Size(30, 30));

        // �ڼ�⵽��������Χ���ƾ��ο�
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 2);
        }

        // ��ʾ���ͼ��
        cv::imshow("Face Detection", frame);

        // ����Esc���˳�ѭ��
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // �ͷ���Դ
    capture.release();
    cv::destroyAllWindows();

    return 0;
}