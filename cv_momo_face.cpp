#include <opencv2/opencv.hpp>

int main()
{
    // ��������ʶ������è������ͼ��
    cv::CascadeClassifier faceCascade;
    faceCascade.load("haarcascade_frontalface_default.xml");
    cv::Mat catFace = cv::imread("R-C.png");

    // ������ͷ
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cout << "�޷�������ͷ" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true)
    {
        // ��ȡ����ͷ����
        cap >> frame;

        // ת��Ϊ�Ҷ�ͼ��
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // �������
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.3, 5);

        // �滻����Ϊè������
        for (const auto& face : faces)
        {
            cv::resize(catFace, catFace, cv::Size(face.width, face.height));
            cv::Mat roi = frame(face);
            catFace.copyTo(roi);
        }

        // ��ʾ���
        cv::imshow("è�������滻", frame);

        // ����ESC���˳�
        if (cv::waitKey(1) == 27)
            break;
    }

    // �ͷ���Դ
    cap.release();
    cv::destroyAllWindows();

    return 0;
}