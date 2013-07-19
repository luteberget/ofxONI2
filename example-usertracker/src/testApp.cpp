#include "testApp.h"

void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);

	usertracker.init();
	usertracker.open();
}

void testApp::update() {
	usertracker.update();
}

void testApp::draw() {
	cout << ofGetFrameRate() << endl;
	usertracker.draw(0,0);
	usertracker.drawDepth(usertracker.getWidth(), 0);
}

void testApp::keyPressed(int key) {
}
