#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	if (glfwInit()){
		// for multi monitors with different DPI we must set DPI awareness 
		// in the application Manifest (for all configurations -> debug, release)
		// like so:
		// DPI Awareness -> Per Monitor High DPI Aware
		// (Project Property Page -> Manifest Tool -> Input And Output)
		int count;
		const auto monitors = glfwGetMonitors(&count);
		if (count > 1) {
			int xpos, ypos;
			glfwGetMonitorPos(monitors[1], &xpos, &ypos);
			auto mode = glfwGetVideoMode(monitors[1]);

			ofGLWindowSettings settings;
			settings.width = mode->width;
			settings.height = mode->height;
			settings.setPosition(ofVec2f(xpos, ypos));
			settings.windowMode = OF_WINDOW;
			ofCreateWindow(settings);
		}
		else {
			ofSetupOpenGL(1024, 768, OF_FULLSCREEN);			// <-------- setup the GL context
		}

		// this kicks off the running of my app
		// can be OF_WINDOW or OF_FULLSCREEN
		// pass in width and height too:
		ofRunApp(new ofApp());

		glfwTerminate();
	}
}
