#pragma once

#include "ofMain.h"
#include "ofxONI2.h"


class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	ofxONI2 depthcam;
};

