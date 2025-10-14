#include <string>
#include <iostream>
#include <zmq.h>

#include <frame.h>

int main(int argc, char *argv[])
{
    std::cout << "Frame Resizer for mp4 frame stream" << std::endl;

    if (argc < 3) {
        std::cerr << "Missing arguments.  Usage frameResizer <width> <height>" << std::endl;
        exit(1);
    } else if (argc > 3) {
        std::cerr << "Too many arguments.  Usage frameResizer <width> <height>" << std::endl;
        exit(1);
    }

    std::string width = argv[1];
    std::string height = argv[2];

    Subscriber subscriber("tcp://127.0.0.1:5555", "tcp://127.0.0.1:5556");
    Publisher publisher("tcp://*:5557", "tcp://*:5558");

    std::cout << "    waiting for a subscriber" << std::endl;
    publisher.waitForSubscriber();

    std::cout << "    connecting to publisher" << std::endl;
    subscriber.connectToPublisher();
    cv::Size newFrameSize(std::stoi(width), std::stoi(height));

    subscriber.setFrameWidth(std::stoi(width));
    subscriber.setFrameHeight(std::stoi(height));

    // resize every receive frame until end of stream
    while (!subscriber.endOfStream()) {
        cv::Mat frame;
        if (subscriber.receiveFrame(frame) < 0) {
            break;
        }

        // Resize the frame using bilinear interpolation
        cv::Mat resizedFrame;
        cv::resize(frame, resizedFrame, newFrameSize, 0, 0, cv::INTER_LINEAR);
        if (publisher.publishFrame(resizedFrame,
                                   subscriber.frameWidth(),
                                   subscriber.frameHeight(),
                                   subscriber.frameFPS()) < 0) {
            std::cerr << "Error: Failed to publish resized frame." << std::endl;
            break;
        }
    }

    // send end of stream to subscriber(s)
    publisher.end();
    publisher.waitForSubscriberDisconnect();

    return 0;
}
