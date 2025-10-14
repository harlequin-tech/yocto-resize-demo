#ifndef FRAME_H_
#define FRAME_H_

#include <opencv2/opencv.hpp>
#include <zmq.h>

#define FRAME_MAX_SIZE 10*1024*1024        // max size of a frame message in bytes
#define SYNC_MAX_SIZE  1024             // max size of a synchronization message in bytes
                                      
/**
 * format of a frame on the wire
 * @param msg - decoded frame message
 * @param buf - raw bytes on the wire
 */
typedef union {
    struct [[gnu::packed]] {
        struct {
            uint32_t frameLength;
            uint32_t frameCount;
            bool endOfStream;
            uint32_t frameWidth;
            uint32_t frameHeight;
            double frameFPS;
            uint32_t frameSizeBytes;
        } header;
        uint8_t frame[];                // actual raw bytes of frame
    } msg;
    uint8_t buf[FRAME_MAX_SIZE];        // raw bytes of header + frame
} frame_t;


// Video frame publisher
class Publisher {
public:
    Publisher(std::string publisherAddress, std::string publisherSyncAddress);
    ~Publisher();
    int waitForSubscriber();
    int waitForSubscriberDisconnect();
    int publishFrame(cv::Mat &frame, uint32_t frameWidth, uint32_t frameHeight, double frameFPS);
    int end();
    bool isSubscribed() { return _subscribed; }
private:
    std::string _publisherAddress;
    std::string _publisherSyncAddress;
    void *_context = nullptr;           // 0MQ context
    void *_publisher = nullptr;          // 0MQ publisher socket
    void *_syncService = nullptr;       // 0MQ receive socket for subscriber sync
    int _sndhwm = 1100000;
    uint32_t _publishCount = 0;         // Number of frames published
    bool _subscribed = false;           // true if there is a subscriber

};

// Video frame subscriber
class Subscriber {
public:
    Subscriber(std::string publisherAddress, std::string publisherSyncAddress);
    ~Subscriber();
    int connectToPublisher();
    int receiveFrame(cv::Mat &frame);
    int end();
    bool endOfStream() { return _streamEnded; };
    bool isConnected() { return _subscribed; }
    uint32_t frameWidth() { return _frameWidth; }
    uint32_t frameHeight() { return _frameHeight; }
    uint32_t frameFPS() { return _frameFPS; }
    void setFrameWidth(uint32_t width) { _frameWidth = width; _frameWidthSet = true; }
    void setFrameHeight(uint32_t height) { _frameHeight = height; _frameHeightSet = true; }
    void setFrameFPS(double fps) { _frameFPS = fps; _frameFPSSet = true; }
private:
    void *_context = nullptr;               // 0MQ context
    void *_subscriber = nullptr;            // 0MQ subscriber socket (Up stream)
    void *_syncService = nullptr;           // 0MQ transmit socket for subscriber sync
    int _sndhwm = 1100000;
    uint32_t _publishCount = 0;           // Number of frames published
    bool _subscribed = false;             // true if there is a subscriber
    bool _streamEnded = true;
    uint32_t _frameWidth;
    uint32_t _frameHeight;
    uint32_t _frameFPS;                   // frames per second in source mp4 file
    bool _frameWidthSet = false;
    bool _frameHeightSet = false;
    bool _frameFPSSet = false;
};


// Video frame reader
class Video {
public:
    int open(std::string videoFileName = "input.mp4");
    int readFrame(cv::Mat &frame);
    int close();
    int isOpen() { return _fileIsOpen; }
    uint32_t frameNumber() { return _frameIndex; }
    uint32_t frameWidth() { return _frameWidth; }
    uint32_t frameHeight() { return _frameHeight; }
    double frameFPS() { return _frameFPS; }
private:
    cv::VideoCapture _cap;               // OpenCV video capture instance
    uint32_t _frameIndex;                // current read frame
    bool _fileIsOpen = false;            // true when file is open
    uint32_t _frameWidth = 0;
    uint32_t _frameHeight = 0;
    double _frameFPS = 0;
};

#endif // FRAME_H_
