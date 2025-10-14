#include <string>
#include <iostream>

#include "frame.h"

/**
 * Initialize 0MQ context and set up publisher and sync service sockets.
 */
Publisher::Publisher(std::string publisherAddress, std::string publisherSyncAddress)
{
    _context = zmq_ctx_new();

    //  Socket to talk to clients
    _publisher = zmq_socket(_context, ZMQ_PUB);
    zmq_setsockopt(_publisher, ZMQ_SNDHWM, &_sndhwm, sizeof (int));
    zmq_bind(_publisher, publisherAddress.c_str());

    //  Socket to receive subscription signals
    _syncService = zmq_socket(_context, ZMQ_REP);
    zmq_bind(_syncService, publisherSyncAddress.c_str());
}

int Publisher::waitForSubscriber()
{
    //  - wait for synchronization request
    uint8_t buf[SYNC_MAX_SIZE];
    int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
    if (len < 0) {
        std::cerr << "Error: zmq_recv failed with errno " << errno << std::endl;
        return -1;
    }
    // check message content?  Not really needed,

    //  - send synchronization reply
    std::string payload = "READY";
    int res = zmq_send(_syncService, payload.c_str(), payload.length()+1, 0);
    if (res < 0) {
        std::cerr << "Error: zmq_send failed with errno " << errno << std::endl;
        return -1;
    }
    _subscribed = true;

    return 0;
}

/**
 * Publish a single frame.
 * @param frame - the frame to publish
 * @retval 0 - success
 * @retval -1 - failure
 */
int Publisher::publishFrame(cv::Mat &frame)
{
    frame_t wireFrame;

    wireFrame.msg.header.frameSizeBytes = frame.total() * frame.elemSize();

    uint32_t wireFrameTotalSize = sizeof(wireFrame.msg.header) + wireFrame.msg.header.frameSizeBytes;
    uint32_t maxFrameSize = sizeof(wireFrame.buf) - sizeof(wireFrame.msg.header);


    if (wireFrame.msg.header.frameSizeBytes > maxFrameSize) {
        std::cerr << "Error: frame buffer size " << wireFrame.msg.header.frameSizeBytes << " execeeded (requested " << maxFrameSize << ")" << std::endl;
        return -1;
    }
    if (zmq_send(_publisher, wireFrame.buf, wireFrameTotalSize, 0) < 0) {
        std::cerr << "Error: zmq_send failed with errno " << errno << std::endl;
        return -1;
    }
    _publishCount++;
    return 0;
}

int Publisher::end()
{
    frame_t wireFrame;

    memset(wireFrame.buf, 0, sizeof(wireFrame.buf));
    wireFrame.msg.header.endOfStream = true;

    // send the end of stream message
    if (zmq_send(_publisher, wireFrame.buf, sizeof(wireFrame.msg.header), 0) < 0) {
        std::cerr << "Error: zmq_send failed with errno " << errno << std::endl;
        return -1;
    }

    return 0;
}

/**
 * Clean up 0MQ sockets and context.
 */
Publisher::~Publisher()
{
    zmq_close(_publisher);
    zmq_close(_syncService);
    zmq_ctx_destroy(_context);
}

Subscriber::Subscriber(std::string publisherAddress, std::string publisherSyncAddress)
{
    _context = zmq_ctx_new();

    //  Socket to receive from publisher
    _subscriber = zmq_socket(_context, ZMQ_REQ);

    uint32_t highWaterMark = sizeof(frame_t);       // max bytes supported by the socket
    zmq_setsockopt(_subscriber, ZMQ_SNDHWM, &highWaterMark, sizeof (int));

    zmq_bind(_subscriber, publisherAddress.c_str());

    //  Socket to receive subscription signals
    _syncService = zmq_socket(_context, ZMQ_PUB);
    zmq_connect(_syncService, publisherSyncAddress.c_str());
}

int Subscriber::connectToPublisher()
{
    uint8_t buf[FRAME_MAX_SIZE];        // buffer for received response

    std::string startMessage = "START";

    //  - send a synchronization request
    zmq_send(_syncService, startMessage.c_str(), startMessage.length()+1, 0);

    //  - wait for synchronization reply
    int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
    if (len < 0) {
        return -1;
    }
    return 0;
}

int Subscriber::receiveFrame(cv::Mat &frame)
{
    frame_t wireFrame;

    int len = zmq_recv(_syncService, wireFrame.buf, sizeof(wireFrame.buf), 0);
    if (len < 0) {
        return -1;
    }

    cv::Mat receivedFrame(wireFrame.msg.header.frameHeight, wireFrame.msg.header.frameWidth, CV_8UC3, wireFrame.msg.frame);
    frame = receivedFrame.clone();

    return 0;
}

int Subscriber::end()
{
    uint8_t buf[FRAME_MAX_SIZE];        // buffer for received response
    
    // disconnect from the upstream publisher
    std::string endMessage = "END";
    if (zmq_send(_syncService, endMessage.c_str(), endMessage.length() + 1, 0) < 0) {
        return -1;
    }

    //  - wait for synchronization reply
    if (zmq_recv(_syncService, buf, sizeof(buf), 0) < 0) {
        return -1;
    }
    
    return 0;
}

Subscriber::~Subscriber()
{
    zmq_close(_subscriber);
    zmq_close(_syncService);
    zmq_ctx_destroy(_context);
}


/**
 * Open a video file ready to read frames from.
 * @param videoPath - path to the video file to open
 * @retval 0 - success
 * @retval -1 - failed to open video file
 */
int Video::open(std::string videoPath) 
{
    if (isOpen()) {
        Video::close();
    }
    _cap = cv::VideoCapture(videoPath);
    if (!_cap.isOpened()) {
        _fileIsOpen = false;
        return -1;
    }
    _fileIsOpen = true;
    _frameIndex = 0;
    return 0;
}

/**
 * Read the next frame from the video file.
 * @param frame - output for the read frame
 * retval 0 - success
 * retval -1 failed to read frame
 */
int Video::readFrame(cv::Mat &frame)
{
    if (!isOpen()) {
        return -1;
    }

    _cap >> frame;

    if (frame.empty()) {
        Video::close();
        return -1;
    }

    _frameIndex++;
    return 0;
}

/**
 * Close the open video file.
 * @retval 0 - success
 * @retval -1 - video file not open
 */
int Video::close()
{
    if (_fileIsOpen) {
        _cap.release();
        _fileIsOpen = false;
    } else {
        return -1;
    }
    return 0;
}
