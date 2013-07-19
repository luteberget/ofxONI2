#pragma once

#include "ofMain.h"
#include "ofxNiTEUserTracker.h"


class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	ofxNiTEUserTracker usertracker;
};

