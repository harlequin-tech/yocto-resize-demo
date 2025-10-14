#include <string>
#include <iostream>
#include <zmq.h>

#include <frame.h>

int main(int argc, char *argv[])
{
    cv::Mat frame;

    std::cout << "Frame Saver for resized mp4 frame stream" << std::endl;
    if (argc < 2) {
        std::cerr << "Missing arguments.  Usage frameSaver <file.mp4>" << std::endl;
        exit(1);
    } else if (argc > 2) {
        std::cerr << "Too many arguments.  Usage frameSaver <file.mp4>" << std::endl;
        exit(1);
    }

    std::string outputVideoPath = argv[1];

    //Subscriber subscriber("ipc://tmp/frameResizer", "ipc://tmp/frameResizerSync");
    Subscriber subscriber("tcp://127.0.0.1:5557", "tcp://127.0.0.1:5558");

    std::cout << "    connecting to publisher" << std::endl;
    subscriber.connectToPublisher();

    // read in the first frame to populate frame dimensions
    if (subscriber.receiveFrame(frame) < 0) {
        std::cerr << "Error: Failed to receive first frame" << std::endl;
        exit(1);
    }

    cv::Size frameSize(subscriber.frameWidth(), subscriber.frameHeight()); // Width and height of frames
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    cv::VideoWriter videoWriter(outputVideoPath.c_str(), fourcc, subscriber.frameFPS(), frameSize, true);

    // Check if the video writer was opened successfully
    if (!videoWriter.isOpened()) {
        std::cerr << "Error: Could not open the video writer." << std::endl;
        exit(1);
    }
                                                                               
    // write frames to the mp4 file until end of stream
    do {
        videoWriter.write(frame);
        if (subscriber.receiveFrame(frame) < 0) {
            break;
        }
    } while (!subscriber.endOfStream());

    videoWriter.release();

    subscriber.end();

    return 0;
}
