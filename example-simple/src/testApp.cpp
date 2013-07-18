#include "testApp.h"

void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);

	depthcam.init();
	depthcam.open();
}

void testApp::update() {
	depthcam.update();
}

void testApp::draw() {
	depthcam.draw(0,0);
	depthcam.drawDepth(depthcam.getWidth(), 0);
}

void testApp::keyPressed(int key) {
}
