#include<opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;


inline bool exists_test(const string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

// /////////////////////////////////////////////////////
// Procedure to save videofiles from images
// inputs:
//      filename = output video filename
//      videoOutputPath = path to save output videofile
//      frameSize = width and height of images
//      fps = frmaes per second on output video file
//      dirPath = path to dir with images
void videoWriter(string filename,string videoOutputPath, 
                Size frameSize, double fps, 
                string dirPath)
{
    VideoCapture inputVideo(dirPath);
    Mat frame;

    VideoWriter outputVideo;
    int codec = VideoWriter::fourcc('M', 'P', 'E', 'G');
    outputVideo.open(videoOutputPath + filename, codec, fps, frameSize, true);

    if (!outputVideo.isOpened())
    {
        cout << "[ERROR] Could not open the output video for write: " << filename << endl;
        return;
    }

    vector<String> fn;
    glob(dirPath, fn, false);

    size_t imgCount = fn.size();
    for (size_t iCount = 0; iCount < imgCount; iCount++){
        cout << "Frame " + to_string(iCount) + " out of " + to_string(imgCount) << endl;
        frame = imread(fn[iCount]);
        Mat xframe;
        resize(frame, xframe, frameSize);
        outputVideo.write(xframe);

    }
    outputVideo.release();
    return;
}

int main(int argc, char* argv[])
{
    bool showFrames = false;
    bool saveImages = false;
    bool saveVideo = true;

    string filename = "path\\to\\videoFile";
    string savedir = "path\\to\\dir\\toSaveFiles";
    string foregroundPath = "path\\to\\dir\\withForegroundImages";
    string backgroundPath = "path\\to\\dir\\withBackgroundImages";

	VideoCapture cap;

    // make simple frame and MOG2 mask
    Mat frame, MOGMask, foregroundImg, backgroundImage, backgroundImg;

    if (exists_test(filename)) {
        cap.open(filename);
    }
    
    // init MOG2 BackgroundSubstractor
    Ptr<BackgroundSubtractor> BackgroundSubstractor;
    BackgroundSubstractor = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();;

    int iCount = 0;

    while (true) {
        iCount++;

        cap >> frame;
        if (frame.empty())
            break;

        // update the background model
        BackgroundSubstractor->apply(frame, MOGMask);

        // some works with noises on frame
        GaussianBlur(MOGMask, MOGMask, Size(11, 11), 3.5, 3.5);
        threshold(MOGMask, MOGMask, 10, 255, THRESH_BINARY);

        // get the frame number and write it on the current frame
        rectangle(frame, Point(10, 2), Point(100, 20),
            cv::Scalar(255, 255, 255), -1);

        stringstream ss;
        ss << cap.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();

        putText(frame, frameNumberString.c_str(), Point(15, 15),
            FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

        // zeros images
        foregroundImg = Scalar::all(0);
        backgroundImg = Scalar::all(0);

        // using mask to cut foreground
        frame.copyTo(foregroundImg, MOGMask);

        // invert mask and cut background
        backgroundImage = 255 - MOGMask;
        frame.copyTo(backgroundImg, backgroundImage);

        if (showFrames){
            imshow("Frame", frame);
            imshow("BackGround", backgroundImg);
            imshow("ForeGround", foregroundImg);
            imshow("MOG2 Mask", MOGMask);
        }

        if (saveImages){
            if (i_count != 1) {
            string counter = std::to_string(iCount);

            imwrite(savedir + "/original/Frames" + counter + ".jpg", frame);
            imwrite(savedir + "/background/Frames" + counter + ".jpg", backgroundImg);
            imwrite(savedir + "/foreground/Frames" + counter + ".jpg", foregroundImg);
        }
        }
        
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }

    if (saveVideos){
        Size frameSize = Size((int)cap.get(CAP_PROP_FRAME_WIDTH), (int)cap.get(CAP_PROP_FRAME_HEIGHT));
        int fps = (double)cap.get(CAP_PROP_FRAME_COUNT);

        videoWriter("outputVideoForeground.mpg4", foregroundPath, frameSize, fps, savedir + "/foreground/");
        videoWriter("outputVideoBackground.mp4", backgroundPath, frameSize, fps, savedir + "/background/");
    }

	return 0;
}