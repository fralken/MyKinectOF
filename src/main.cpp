#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_CONTINUOUS);

	if (glfwInit()){
		ofGLWindowSettings settings;
		settings.setGLVersion(3, 2); //we define the OpenGL version we want to use

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

			settings.width = mode->width;
			settings.height = mode->height;
			settings.setPosition(ofVec2f(xpos, ypos));
			settings.windowMode = OF_WINDOW;
		}
		else {
			settings.width = 1920;
			settings.height = 1080;
			settings.windowMode = OF_FULLSCREEN;
		}

		ofCreateWindow(settings);

		// this kicks off the running of my app
		// can be OF_WINDOW or OF_FULLSCREEN
		// pass in width and height too:
		ofRunApp(new ofApp());

		glfwTerminate();
	}

	SetThreadExecutionState(ES_CONTINUOUS);
}
