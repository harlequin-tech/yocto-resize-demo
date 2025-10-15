#include <string>
#include <iostream>
#include <zmq.h>

#include <frame.h>

int main(int argc, char *argv[])
{
    std::cout << "Frame Publisher for mp4 videos" << std::endl;

    if (argc < 2) {
        std::cerr << "Missing arguments.  Usage framePublisher <file.mp4>" << std::endl;
    } else if (argc > 2) {
        std::cerr << "Too many arguments.  Usage frameResizer <file.mp4>" << std::endl;
    }

    Publisher publisher(publisherAddress, publisherSyncAddress);
    Video video;
    std::string videoPath = argv[1];

    // wait for a subscriber
    std::cout << "    waiting for a subscriber" << std::endl;
    publisher.waitForSubscriber();

    std::cout << "    opening video " << videoPath << std::endl;
    if (video.open(videoPath) < 0) {
        std::cerr << "Error: Failed to open video file " << videoPath << std::endl;
        exit(1);
    }

    // while the video file is open, read a frame and publish it.
    std::cout << "    processing frames " << std::endl;
    uint32_t frameCount = 0;
    while (video.isOpen()) {
        cv::Mat frame;
        if (video.readFrame(frame) < 0) {
            std::cerr << "Error: Failed to read video frame." << video.frameNumber() << std::endl;
            break;
        }
        frameCount++;
        if (publisher.sendFrame(frame, video.frameWidth(), video.frameHeight(), video.frameFPS()) < 0) {
            std::cerr << "Error: Failed to publish video frame." << video.frameNumber() << std::endl;
            break;
        }
        printf("    Frame %8d         \r", frameCount);
    }

    std::cout << std::endl;

    // send end of stream to subscriber(s)
    std::cout << "    closing publisher" << std::endl;
    publisher.end();

    std::cout << "    done" << std::endl;

    return 0;
}
