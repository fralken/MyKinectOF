#pragma once

#include "ofMain.h"

#include "ofxGui.h"
#include "ofxKinectForWindows2.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxKFW2::Device kinect;
		ICoordinateMapper* coordinateMapper;

		ofImage foregroundImg;
		ofImage backgroundImg;
		ofImage frameImg;

		vector<ofVec2f> mappedCoords;
		int numBodiesTracked;
		bool bHaveAllStreams;
		bool bShowBodies;

		void greenScreenFromDepthFrame(ofShortPixelsRef depthPix, ofPixelsRef bodyIndexPix, ofPixelsRef colorPix);
		void greenScreenFromColorFrame(ofShortPixelsRef depthPix, ofPixelsRef bodyIndexPix, ofPixelsRef colorPix);
};