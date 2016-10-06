
/*
Copyright 2016 Roman Seviaryn


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

/*
Software originaly developed by Roman Seviaryn for the University of Windsor
Last Modifified: 2016-07-02 by Roman Seviaryn
*/
#include "opencv2/opencv.hpp"

#include <fstream>
#include <string>
#include <ctime>
#include <vector>

int main(int, char**)
{
	std::string ans;
	std::cout << "does your computer have an inbuilt webcam?(y/n)" << std::endl;
	std::cin >> ans;

	double dist;
	std::cout << "How far away is the camera from your hand?(cm)" << std::endl;
	std::cin >> dist;
	double hor = dist * (7.0 / 10.0);
	// height of the camera based on distance from hand. Only works for specific cameras.
	// reccomend re-calibration of these numbers when using a different camera

	cv::VideoCapture cap; // goes to default
	if (ans == "y")
	{
		// captures the second (non-integrated) webcam
		std::cout << "Attempting to open external webcam." << std::endl;
		cap.open(1);
	}
	else
	{
		//reverts to default
		std::cout << "attempting to open default camera." << std::endl;
		cap.open(0);
	}

	if (!cap.isOpened())
	{  // check if we succeeded
		std::cout << "Failed to find camera. Press any key to end." << std::endl;
		cv::waitKey();
		return -1;
	}
	cv::SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 10;
	params.maxThreshold = 200;

	// Filter by Area
	params.filterByArea = true;
	params.minArea = 150;
	params.maxArea = 1500;

	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.8;
	params.maxCircularity = 1;

	// Filter by Convexity
	params.filterByConvexity = true;
	params.minConvexity = 0.85;
	params.maxConvexity = 1;

	// Filter by Inertia
	params.filterByInertia = false;
	params.minInertiaRatio = 0.01;

	cv::KeyPoint previous;
	std::vector<cv::KeyPoint> keypoints;
	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

	cv::Mat finalim, origin;

	cv::namedWindow("Mask", 1);
	cv::namedWindow("Original");

	std::fstream fs("data.csv", std::fstream::out);
	clock_t begin = clock();
	for (;;)
	{
		// processing image
		cap >> origin; // get a new frame from camera
		cv::cvtColor(origin, finalim, CV_RGB2GRAY);
		cv::GaussianBlur(finalim, finalim, cv::Size(9, 9), 2, 2);
		cv::threshold(finalim, finalim, 75, 255, CV_THRESH_BINARY);
		detector->detect(finalim, keypoints);

		// tracking point on skin
		for (cv::KeyPoint point : keypoints)
		{
			std::cout << "(" + std::to_string(point.pt.x) + ", " + std::to_string(point.pt.y) + ")" << std::endl;

			//converting to cm height
			double cm_height = (point.pt.y * hor) / 480.0;
			// basic trig ratio. replace 480 with the vertical resolution of the camera you are using.

			fs << std::to_string((clock() - begin) / (CLOCKS_PER_SEC / 1000)) + "," + std::to_string(cm_height) << std::endl;
		}

		//display
		cv::drawKeypoints(origin, keypoints, origin, cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		cv::imshow("Original", origin);
		cv::imshow("Mask", finalim);
		if (cv::waitKey(1) >= 0) break;
	}

	// the camera will be deinitialized automatically in VideoCapture destructor
	fs.close();

	return 0;
}