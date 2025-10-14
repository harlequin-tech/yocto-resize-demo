#include <iostream>

#include "frame.h"

static uint8_t buf[FRAME_MAX_SIZE];        // buffer for received response
static frame_t wireFrame;
/**
 * Initialize 0MQ context and set up publisher and sync service sockets.
 */
Publisher::Publisher(std::string publisherAddress, std::string publisherSyncAddress)
{
    std::cout << "    Creating publisher socket " << publisherAddress << std::endl;
    _context = zmq_ctx_new();

    //  Socket to talk to clients
    _publisher = zmq_socket(_context, ZMQ_PUSH);
    zmq_setsockopt(_publisher, ZMQ_SNDHWM, &_sndhwm, sizeof (int));
    if (zmq_bind(_publisher, publisherAddress.c_str()) << 0) {
        std::cerr << "   Error: failed to bind to " << publisherSyncAddress << " errno " << errno << std::endl;
    }

    std::cout << "    Creating publisher sync socket " << publisherSyncAddress << std::endl;
    //  Socket to receive subscription signals
    _syncService = zmq_socket(_context, ZMQ_REP);
    if (zmq_bind(_syncService, publisherSyncAddress.c_str()) < 0) {
        std::cerr << "   Error: failed to bind to " << publisherSyncAddress << " errno " << errno << std::endl;
    }
}

/**
 * Wait for the subscriber to send start on the sync socket.
 * @return 0 - success, -1 failure
 */
int Publisher::waitForSubscriber()
{
    std::cout << "    waitForSubscriber()" << std::endl;
    //  - wait for synchronization request
    uint8_t buf[SYNC_MAX_SIZE];
    std::cout << "    Waiting for message from subscriber via sync socket" << std::endl;
    int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
    if (len < 0) {
        std::cerr << "Error: zmq_recv failed with errno " << errno << std::endl;
        return -1;
    }
    // check message content?  Not really needed,

    //  - send synchronization reply
    std::string payload = "READY";
    strncpy((char *)buf, payload.c_str(), sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    std::cout << "    Sending response message to subscriber via sync socket" << std::endl;
    int res = zmq_send(_syncService, buf, sizeof(buf), 0);
    if (res < 0) {
        std::cerr << "Error: zmq_send failed with errno " << errno << std::endl;
        return -1;
    }
    _subscribed = true;

    return 0;
}

int Publisher::waitForSubscriberDisconnect()
{
    uint8_t buf[SYNC_MAX_SIZE];
    std::cout << "    waitForSubscriberDisconnect()" << std::endl;
    std::cout << "    Waiting for message from subscriber via sync socket" << std::endl;
    int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
    if (len < 0) {
        std::cerr << "Error: zmq_recv failed with errno " << errno << std::endl;
        return -1;
    }
    std::cout << "    Received \"" << (char *)buf << "\" from subscriber" << std::endl;
    std::cout << "    Sending response message to subscriber via sync socket" << std::endl;
    std::string endMessage = "END";

    strncpy((char *)buf, endMessage.c_str(), sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    std::cout << "    Sending \"END\" response message to subscriber via sync socket" << std::endl;
    int res = zmq_send(_syncService, buf, sizeof(buf), 0);
    if (res < 0) {
        std::cerr << "Failed to send start message, errno " << errno << std::endl;
    }
    return 0;
}

/**
 * Publish a single frame.
 * @param frame - the frame to publish
 * @retval 0 - success
 * @retval -1 - failure
 */
int Publisher::sendFrame(cv::Mat &frame, uint32_t frameWidth, uint32_t frameHeight, double frameFPS)
{
    wireFrame.msg.header.frameSizeBytes = frame.total() * frame.elemSize();

    uint32_t wireFrameTotalSize = sizeof(wireFrame.msg.header) + wireFrame.msg.header.frameSizeBytes;
    uint32_t maxFrameSize = sizeof(wireFrame.buf) - sizeof(wireFrame.msg.header);

    if (wireFrame.msg.header.frameSizeBytes > maxFrameSize) {
        std::cerr << "Error: frame buffer size " << wireFrame.msg.header.frameSizeBytes << " execeeded (requested " << maxFrameSize << ")" << std::endl;
        return -1;
    }

    wireFrame.msg.header.magic = 0xDEADBEEF;
    wireFrame.msg.header.frameLength = wireFrameTotalSize;
    wireFrame.msg.header.frameWidth = frameWidth;
    wireFrame.msg.header.frameHeight = frameHeight;
    wireFrame.msg.header.frameFPS = frameFPS;
    memcpy(wireFrame.msg.frame, frame.data, wireFrame.msg.header.frameSizeBytes);

#if 0
    std::string frameStr = "FRAME";

    if (zmq_send(_publisher, frameStr.c_str(), frameStr.length(), ZMQ_SNDMORE) < 0) {
        std::cerr << "Error: zmq_send FRAME failed with errno " << errno << std::endl;
        return -1;
    }
#endif
    if (zmq_send(_publisher, wireFrame.buf, wireFrameTotalSize, 0) < 0) {
        std::cerr << "Error: zmq_send failed with errno " << errno << std::endl;
        return -1;
    }
    _publishCount++;
    frame.release();
    return 0;
}

int Publisher::end()
{
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
    std::cout << "    Subscribing to publisher " << publisherAddress << std::endl;
    _context = zmq_ctx_new();
    //  Socket to receive from publisher
    _subscriber = zmq_socket(_context, ZMQ_PULL);

    uint32_t highWaterMark = sizeof(frame_t);       // max bytes supported by the socket
    zmq_setsockopt(_subscriber, ZMQ_SNDHWM, &highWaterMark, sizeof (int));
    if (zmq_connect(_subscriber, publisherAddress.c_str()) < 0) {
        std::cerr << "   Error: failed to connect to " << publisherSyncAddress << " errno " << errno << std::endl;
    }

    std::cout << "    Subscribing to publisher sync " << publisherSyncAddress << std::endl;
    //  Socket to receive subscription signals
    _syncService = zmq_socket(_context, ZMQ_REQ);
    if (zmq_connect(_syncService, publisherSyncAddress.c_str()) < 0) {
        std::cerr << "   Error: failed to connect to " << publisherSyncAddress << " errno " << errno << std::endl;
    }
}

/**
 * Connect to publisher via the sync socket.
 */
int Subscriber::connectToPublisher()
{
    uint8_t buf[SYNC_MAX_SIZE];
    std::string startMessage = "START";
    //  - send a synchronization request
    strncpy((char *)buf, startMessage.c_str(), sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    std::cout << "    Sending \"" << startMessage << "\"to publisher sync socket" << std::endl;
    if (zmq_send(_syncService, buf, sizeof(buf), 0) < 0) {
        std::cerr << "Failed to send start message, errno " << errno << std::endl;
    }

    //  - wait for synchronization reply
    std::cout << "    Waiting for response from publisher sync socket" << std::endl;
    int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
    if (len < 0) {
        std::cerr << "Failed to receive start ack, errno " << errno << std::endl;
        // try again
        if (zmq_send(_syncService, startMessage.c_str(), startMessage.length()+1, 0) < 0) {
            std::cerr << "Failed to send start message, errno " << errno << std::endl;
        }
        std::cout << "    Waiting for response from publisher sync socket" << std::endl;
        int len = zmq_recv(_syncService, buf, sizeof(buf), 0);
        if (len < 0) {
            std::cerr << "Failed to receive start ack, errno " << errno << std::endl;
        }
        return -1;
    }
    std::cout << "    Received \"" << (char *)buf << "\" from publisher" << std::endl;
    _streamEnded = false;
    //  Subscribe to the 'frame' topic
#if 0
    if (zmq_setsockopt(_subscriber, ZMQ_SUBSCRIBE, "FRAME", strlen ("FRAME")) < 0) {
        std::cerr << "Failed to subscribe to frame messages, errno " << errno << std::endl;
        return -1;
    }
#endif

    return 0;
}

/**
 * Wait for a frame from the publisher.
 * @param frame - return the frame heare
 * @return 0 - success, -1 - failure
 */
int Subscriber::receiveFrame(cv::Mat &frame)
{
#if 0
    char pubMsg[128];
    int len = zmq_recv(_subscriber, pubMsg, sizeof(pubMsg), 0);
    pubMsg[len+1] = 0;
#endif
    int len = zmq_recv(_subscriber, wireFrame.buf, sizeof(wireFrame.buf), 0);
    if (len < 0) {
        std::cerr << "   Error: zmq_recv failed with errno =  " << errno << std::endl;
        return -1;
    }
    if (wireFrame.msg.header.endOfStream) {
        std::cout << "    received end-of-stream" << std::endl;
        _streamEnded = true;
        return 0;
    }

    // record mp4 dimensions unless manually set
    if (!_frameWidthSet) {
        _frameWidth = wireFrame.msg.header.frameWidth;
        std::cout << "    frame width " << _frameWidth << std::endl;
        _frameWidthSet = true;
    }

    if (!_frameHeightSet) {
        _frameHeight = wireFrame.msg.header.frameHeight;
        std::cout << "    frame height " << _frameHeight << std::endl;
        _frameHeightSet = true;
    }
    if (!_frameFPSSet) {
        _frameFPS = wireFrame.msg.header.frameFPS;
        std::cout << "    frame fps " << _frameFPS << std::endl;
        _frameFPSSet = true;
    }
    frame = cv::Mat(wireFrame.msg.header.frameHeight, wireFrame.msg.header.frameWidth, CV_8UC3, wireFrame.msg.frame);

    return 0;
}

int Subscriber::end()
{
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

    _frameWidth = frame.cols;
    _frameHeight = frame.rows;
    _frameFPS = _cap.get(cv::CAP_PROP_FPS);

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
