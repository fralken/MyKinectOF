#include "ofApp.h"

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424
#define DEPTH_SIZE DEPTH_WIDTH * DEPTH_HEIGHT

#define COLOR_WIDTH 1920
#define COLOR_HEIGHT 1080
#define COLOR_SIZE COLOR_WIDTH * COLOR_HEIGHT

#ifdef HD
	#define FRAME_WIDTH COLOR_WIDTH
	#define FRAME_HEIGHT COLOR_HEIGHT
	#define PROJ_COORD ofxKFW2::ProjectionCoordinates::ColorCamera
#else
	#define FRAME_WIDTH DEPTH_WIDTH
	#define FRAME_HEIGHT DEPTH_HEIGHT
	#define PROJ_COORD ofxKFW2::ProjectionCoordinates::DepthCamera
#endif

//--------------------------------------------------------------
void ofApp::setup(){
	kinect.open();
	//kinect.initDepthSource();
	//kinect.initColorSource();
	//kinect.initBodySource();
	//kinect.initBodyIndexSource();
	kinect.initMultiSource({
		FrameSourceTypes_Depth,
		FrameSourceTypes_Color,
		FrameSourceTypes_Body,
		FrameSourceTypes_BodyIndex });

	if (kinect.getSensor()->get_CoordinateMapper(&coordinateMapper) < 0) {
		ofLogError() << "Could not acquire CoordinateMapper!";
	}

	numBodiesTracked = 0;
	bHaveAllStreams = false;
	bShowBodies = false;

	ofSetBackgroundColor(0, 0, 0);

	//backgroundImg.loadImage("monument_valley.png");
	//foregroundImg.loadImage("monument_valley-fg.png");
	backgroundImg.loadImage("the_starry_night_1889.png");
	foregroundImg.loadImage("the_starry_night_1889-fg.png");

	scaleImages(backgroundImg, foregroundImg);

#ifdef SHADER
	#ifdef HD
	shader.load("shaders/greenscreen.vert", "shaders/greenscreen_hd.frag");
	#else
	shader.load("shaders/greenscreen.vert", "shaders/greenscreen_ld.frag");
	#endif
#else
	frameImg.clone(backgroundImg);
#endif

	updateFrameRect(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();

	// Get pixel data
	if (kinect.getDepthSource()->isFrameNew() &&
		kinect.getColorSource()->isFrameNew() &&
		kinect.getBodySource()->isFrameNew() &&
		kinect.getBodyIndexSource()->isFrameNew())
	{
		auto depthPix = kinect.getDepthSource()->getPixels();
		auto bodyIndexPix = kinect.getBodyIndexSource()->getPixels();
		auto colorPix = kinect.getColorSource()->getPixels();

		// Make sure there's some data here, otherwise the cam probably isn't ready yet
		if (!depthPix.size() || !bodyIndexPix.size() || !colorPix.size()) {
			bHaveAllStreams = false;
			return;
		}
		else {
			bHaveAllStreams = true;
		}

		// Count number of tracked bodies
		numBodiesTracked = 0;
		auto& bodies = kinect.getBodySource()->getBodies();
		for (auto& body : bodies) {
			if (body.tracked) {
				numBodiesTracked++;
			}
		}

#ifndef SHADER
		frameImg.clone(backgroundImg);

	#ifdef HD
		greenScreenFromColorFrame(depthPix, bodyIndexPix, colorPix);
	#else
		greenScreenFromDepthFrame(depthPix, bodyIndexPix, colorPix);
	#endif

		// Update the images since we manipulated the pixels manually. This uploads to the
		// pixel data to the texture on the GPU so it can get drawn to screen
		frameImg.update();
#endif
	}
}

#ifndef SHADER
void ofApp::greenScreenFromDepthFrame(ofShortPixelsRef depthPix, ofPixelsRef bodyIndexPix, ofPixelsRef colorPix)
{
	// Do the depth space -> color space mapping
	// More info here:
	// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.coordinatemapper.mapdepthframetocolorspace.aspx
	// https://msdn.microsoft.com/en-us/library/dn785530.aspx
	kinect.getDepthSource()->getColorInDepthFrameMapping(coordMapping);
	ofVec2f* mappedCoords = reinterpret_cast<ofVec2f*>(coordMapping.getData());

	// Loop through the depth image
	for (int y = 0; y < DEPTH_HEIGHT; y++) {
		for (int x = 0; x < DEPTH_WIDTH; x++) {
			int index = (y * DEPTH_WIDTH) + x;

			// This is the check to see if a given pixel is inside a tracked  body or part of the background.
			// If it's part of a body, the value will be that body's id (0-5), or will > 5 if it's
			// part of the background
			// More info here:
			// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.bodyindexframe.aspx
			int val = bodyIndexPix[index];
			if (val != 0xff) {
				// For a given (x,y) in the depth image, lets look up where that point would be
				// in the color image
				ofVec2f mappedCoord = mappedCoords[index];

				// Mapped x/y coordinates in the color can come out as floats since it's not a 1:1 mapping
				// between depth <-> color spaces i.e. a pixel at (100, 100) in the depth image could map
				// to (405.84637, 238.13828) in color space
				// So round the x/y values to ints so that we can look up the nearest pixel
				int colorX = (int)(mappedCoord.x + 0.5);
				int colorY = (int)(mappedCoord.y + 0.5);

				// Make sure it's within some sane bounds, and skip it otherwise
				if (colorX >= 0 && colorY >= 0 && colorX < COLOR_WIDTH && colorY < COLOR_HEIGHT)
				{
					// Finally, pull the color from the color image based on its coords in
					// the depth image
					frameImg.setColor(x + frameOffset.x, y + frameOffset.y, colorPix.getColor(colorX, colorY));
				}
			}
		}
	}
}

void ofApp::greenScreenFromColorFrame(ofShortPixelsRef depthPix, ofPixelsRef bodyIndexPix, ofPixelsRef colorPix)
{
	// Do the color space -> depth space mapping
	// More info here:
	// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.coordinatemapper.mapcolorframetodepthspace.aspx
	// https://msdn.microsoft.com/en-us/library/dn785530.aspx
	kinect.getDepthSource()->getDepthInColorFrameMapping(coordMapping);
	ofVec2f* mappedCoords = reinterpret_cast<ofVec2f*>(coordMapping.getData());

	for (int y = 0; y < COLOR_HEIGHT; y++) {
		for (int x = 0; x < COLOR_WIDTH; x++) {
			int index = (y * COLOR_WIDTH) + x;
			// For a given (x,y) in the color image, lets look up where that point would be
			// in the depth image
			ofVec2f mappedCoord = mappedCoords[index];

			// The sentinel value is -inf, -inf, meaning that no depth pixel corresponds to this color pixel.
			if (mappedCoord.x != -std::numeric_limits<float>::infinity() && 
				mappedCoord.y != -std::numeric_limits<float>::infinity()) {
				// Mapped x/y coordinates in the color can come out as floats since it's not a 1:1 mapping
				// between color <-> depth spaces 
				// So round the x/y values to ints so that we can look up the nearest pixel
				int depthX = (int)(mappedCoord.x + 0.5f);
				int depthY = (int)(mappedCoord.y + 0.5f);

				// Make sure it's within some sane bounds, and skip it otherwise
				if (depthX >= 0 && depthX < DEPTH_WIDTH && depthY >= 0 && depthY < DEPTH_HEIGHT) {
					int depthIndex = (depthY * DEPTH_WIDTH) + depthX;

					// This is the check to see if a given pixel is inside a tracked  body or part of the background.
					// If it's part of a body, the value will be that body's id (0-5), or will > 5 if it's
					// part of the background
					// More info here:
					// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.bodyindexframe.aspx
					int val = bodyIndexPix[depthIndex];
					if (val != 0xff) {
						frameImg.setColor(x + frameOffset.x, y + frameOffset.y, colorPix.getColor(x, y));
					}
				}
			}
		}
	}
}
#endif

void ofApp::updateFrameRect(int width, int height) {
	float bgWidth = backgroundImg.getWidth();
	float bgHeight = backgroundImg.getHeight();

	float widthRatio = (float)width / bgWidth;
	float heightRatio = (float)height / bgHeight;
	
	float w;
	float h;
	float ratio;
	if (widthRatio < heightRatio) {
		w = width;
		h = bgHeight * widthRatio;
		ratio = widthRatio;
	}
	else {
		w = bgWidth * heightRatio;
		h = height;
		ratio = heightRatio;
	}

	float x = (width - w) / 2;
	float y = (height - h) / 2;

	frameRect.set(x + frameOffset.x * ratio, y + frameOffset.y * ratio, FRAME_WIDTH * ratio, FRAME_HEIGHT * ratio);
	backgroundRect.set(x, y, w, h);
}

void ofApp::scaleImages(ofImage& background, ofImage& foreground) {
	float widthRatio = (float)FRAME_WIDTH / background.getWidth();
	float heightRatio = (float)FRAME_HEIGHT / background.getHeight();

	if (widthRatio != 1.0 || heightRatio != 1.0) {
		float w;
		float h;
		if (widthRatio > heightRatio) {
			w = FRAME_WIDTH;
			h = background.getHeight() * widthRatio;
		}
		else {
			w = background.getWidth() * heightRatio;
			h = FRAME_HEIGHT;
		}

		background.resize(w, h);
		foreground.resize(w, h);

		frameOffset.x = (w - FRAME_WIDTH) / 2;
		frameOffset.y = (h - FRAME_HEIGHT) / 2;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
#ifdef SHADER
	if (bHaveAllStreams) {
	#ifdef HD
		kinect.getDepthSource()->getDepthInColorFrameMapping(coordMapping);
	#else
		kinect.getDepthSource()->getColorInDepthFrameMapping(coordMapping);
	#endif
		coordMappingTex.allocate(coordMapping);
	}

	if (coordMappingTex.isAllocated()) {
		shader.begin();
		shader.setUniform2f("frameOffset", frameOffset);
		shader.setUniformTexture("color", kinect.getColorSource()->getTexture(), 1);
		shader.setUniformTexture("bodyIndex", kinect.getBodyIndexSource()->getTexture(), 2);
		shader.setUniformTexture("foreground", foregroundImg.getTextureReference(), 3);
		shader.setUniformTexture("coordMapping", coordMappingTex, 4);
		backgroundImg.draw(backgroundRect);
		shader.end();

		if (bShowBodies)
			kinect.getBodySource()->drawProjected(
				frameRect.x, frameRect.y, frameRect.width, frameRect.height, PROJ_COORD);
	}
#else
	frameImg.draw(backgroundRect);
	foregroundImg.draw(backgroundRect);
	if (bShowBodies)
		kinect.getBodySource()->drawProjected(
			frameRect.x, frameRect.y, frameRect.width, frameRect.height, PROJ_COORD);
#endif

	stringstream ss;
	ss << "fps : " << ofGetFrameRate() << endl;
	ss << "Tracked bodies: " << numBodiesTracked;
	if (!bHaveAllStreams) ss << endl << "Not all streams detected!";
	ofDrawBitmapStringHighlight(ss.str(), 20, 20);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'b')
		bShowBodies = !bShowBodies;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	updateFrameRect(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
