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

    Subscriber subscriber(publisherAddress, publisherSyncAddress);
    Publisher publisher(resizerAddress, resizerSyncAddress);

    while (1) {
        std::cout << "    waiting for a subscriber" << std::endl;
        publisher.waitForSubscriber();

        std::cout << "    connecting to publisher" << std::endl;
        subscriber.connectToPublisher();
        cv::Size newFrameSize(std::stoi(width), std::stoi(height));

        subscriber.setFrameWidth(std::stoi(width));
        subscriber.setFrameHeight(std::stoi(height));

        // resize every receive frame until end of stream
        uint32_t frameCount = 0;
        while (!subscriber.endOfStream()) {
            cv::Mat frame;
            if ((subscriber.receiveFrame(frame) < 0) || subscriber.endOfStream()) {
                break;
            }
            frameCount++;

            // Resize the frame using bilinear interpolation
            cv::Mat resizedFrame;
            cv::resize(frame, resizedFrame, newFrameSize, 0, 0, cv::INTER_LINEAR);
            if (publisher.sendFrame(resizedFrame,
                                    subscriber.frameWidth(),
                                    subscriber.frameHeight(),
                                    subscriber.frameFPS()) < 0) {
                std::cerr << "Error: Failed to publish resized frame." << std::endl;
                break;
            }
        }

        std::cout << std::endl;
        std::cout << "    stream ended after " << frameCount << " frames" << std::endl;
        // send end of stream to subscriber(s)
        publisher.end();
        //publisher.waitForSubscriberDisconnect();
    }

    return 0;
}
