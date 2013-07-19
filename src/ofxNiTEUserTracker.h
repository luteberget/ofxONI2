#pragma once

#include "ofxONI2.h"
#include "NiTE.h"

class ofxNiTEUserTracker : public ofxONI2 {
	public:
		bool init(bool use_color_image = true, bool use_texture = true, bool colorize_depth_image = true);
		virtual void updateDepthPixels();

	private:
		nite::UserTracker usertracker;
		nite::UserTrackerFrameRef usertrackerframe;

		virtual bool openstreams(const char* deviceURI, openni::VideoMode* depthVideoMode, openni::VideoMode* colorVideoMode);
		virtual void threadedFunction();
};
