#include "ofxNiTEUserTracker.h"

bool ofxNiTEUserTracker::init(bool use_color_image, bool use_texture, bool colorize_depth_image) {
	bool inited = ofxONI2::init(use_color_image, use_texture, colorize_depth_image);

	bUseSkeletonTracking = true;

	// Initialize NiTE
	//
	if(inited) {
		nite::Status rc;
		rc = nite::NiTE::initialize();
		if(rc != nite::STATUS_OK) {
			ofLogWarning("ofxONI2") << "Error in NiTE initialization.";
			return false;
		}
	}

	return inited;
}


bool ofxNiTEUserTracker::openstreams(const char* deviceURI, openni::VideoMode* depthVideoMode, openni::VideoMode* colorVideoMode) {
	ofLogVerbose("ofxONI2") << "Starting NiTE user tracker";
	openni::Status rc = openni::STATUS_OK;
	nite::Status nrc = nite::STATUS_OK;

	// Open device with OpenNI
	rc = oni_device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		ofLogWarning("ofxONI2") << "OpenNI device open failed: " << openni::OpenNI::getExtendedError();
		return false;
	}

	// Create usertracker with NiTE
	nrc = usertracker.create(&oni_device);
	if(nrc != nite::STATUS_OK) {
		ofLogWarning("ofxONI2") << "Couldn't start NiTE user tracker.";
		return false;
	}

	// Open color stream
	if(bGrabVideo) {
		rc = oni_color_stream.create(oni_device, openni::SENSOR_COLOR);
		if (rc == openni::STATUS_OK) {
			rc = oni_color_stream.start();
			if (rc != openni::STATUS_OK) {
				ofLogWarning("ofxONI2") << "Couldn't open color stream: " << openni::OpenNI::getExtendedError();
				oni_color_stream.destroy();
				return false;
			}
		} else {
			ofLogWarning("ofxONI2") << "Couldn't open color stream: " << openni::OpenNI::getExtendedError();
			return false;
		}
	}

	// Read first frame from usertracker to examine video modes
	usertracker.readFrame(&usertrackerframe);
	if (!usertrackerframe.isValid()) {
		ofLogWarning("ofxONI2") << "Could not get first frame from NiTE user tracker. (usertrackerframe)";
		return false;
	}

	oni_depth_frame = usertrackerframe.getDepthFrame();
	if (!oni_depth_frame.isValid()) {
		ofLogWarning("ofxONI2") << "Could not get first frame from NiTE user tracker. (oni_depth_frame)";
		return false;
	}

	*depthVideoMode = oni_depth_frame.getVideoMode();
	if(bGrabVideo) *colorVideoMode = oni_color_stream.getVideoMode();


	return true;
}

bool firstskel = true;
void ofxNiTEUserTracker::threadedFunction() {
	ofLogVerbose("ofxONI2") << "Starting NiTE User Tracker update thread.";

	if(!userMap.isAllocated() || !userMapBack.isAllocated()) {
		userMap.allocate(stream_width, stream_height, 1);
		userMapBack.allocate(stream_width, stream_height, 1);
	}

	while(isThreadRunning()) {
		// ofLogVerbose("ofxONI2") << "thread is running";
		// ofLogVerbose("ofxONI2") << "usertracker valid: " << usertracker.isValid();
		usertracker.readFrame(&usertrackerframe);
		// ofLogVerbose("ofxONI2") << "frame got";
		oni_depth_frame = usertrackerframe.getDepthFrame();
		lock();
		depthPixelsRawBack.setFromPixels((unsigned short*) oni_depth_frame.getData(), 
			stream_width, stream_height, 1);
		bNeedsUpdateDepth = true;

		// Update user tracker map and send new/lost user events.

		const nite::Array<nite::UserData>& users  = usertrackerframe.getUsers();
		userDataBack.clear();
		for(int i = 0; i < users.getSize(); i++) {
			const nite::UserData& user = users[i];
			short userid = user.getId();
			if(user.isNew()) {
				ofNotifyEvent(newUserEvent, userid);

				if(bUseSkeletonTracking) {
					usertracker.startSkeletonTracking(user.getId());
				}
			}
			if(user.isLost()) {
				ofNotifyEvent(lostUserEvent, userid);
			}

			userDataBack[userid] = ofxNiTEUserData(user);
		}
		userMapBack.setFromPixels((unsigned short*) usertrackerframe.getUserMap().getPixels(),
				stream_width, stream_height, 1);
		unlock();

		if(bGrabVideo) {
			oni_color_stream.readFrame(&oni_color_frame);
			lock();
			videoPixelsBack.setFromPixels((unsigned char*) oni_color_frame.getData(),
				stream_width, stream_height, OF_IMAGE_COLOR);
			bNeedsUpdateColor = true;
			unlock();
		}
	}

	ofLogVerbose("ofxONI2") << "Ending NiTE User Tracker update thread.";
}

void ofxNiTEUserTracker::updateDepthPixels() {
	ofxONI2::updateDepthPixels();

	if(lock()) {
		userMap = userMapBack;
		userData = userDataBack;
		unlock();
	}
}
