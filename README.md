- framePublisher <file.mp4>
    -  breaks an mp4 file into frames and publishes them via its publish socket.
- frameResizer \<width\> \<height\>
    - subscribes to the framePublisher and resizes each frame it recieves.  It then publishes the resized frame.
- frameSaver <outfile.mp4>
    - subscribes to the frameResizer and then writes frames to the output mp4 file.
- frameMonitor
    - tbd
 
The frameResizer app can be configered as a microservice as follows.
1. edit /etc/systemd/system/frameResizer.service and set the frame size as desired (e.g. 640 480).
2. Run the following commands:
    1. systemctl daemon-reload
    2. systemctl enable frameService.service
    3. systemctl start frameService.service
3. To view the log from frameService.service run
    1. jounralctl -u frameService.service -f  
