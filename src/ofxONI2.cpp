#include "ofxONI2.h"

ofxONI2::ofxONI2() {
	ofLogVerbose("ofxONI2") << "Creating ofxONI2";
	deviceURI = "";

	bUseTexture = true;
	bGrabVideo = true;

	bGrabberInited = false;

	bNeedsUpdateDepth = false;
	bNeedsUpdateColor = false;
	bUpdateTex = false;
	bIsFrameNew = false;
}

ofxONI2::~ofxONI2() {
	close();
	clear();
}
	
bool ofxONI2::init(bool use_color_image, bool use_texture, bool colorize_depth_image) {
	if(isConnected()) {
		ofLogWarning("ofxONI2") << "init(): do not call init while ofxONI2 is running!";
		return false;
	}

	bUseTexture = use_texture;
	bGrabVideo = use_color_image;
	bColorizeDepthImage = colorize_depth_image;

	ref_max_depth = 0;
	
	// Initialize OpenNI
	openni::Status rc = openni::STATUS_OK;
	rc = openni::OpenNI::initialize();
	if(rc != openni::STATUS_OK) {
		ofLogWarning("ofxONI2") << "Error from OpenNI initialization: " << openni::OpenNI::getExtendedError();
		return false;
	}


	bGrabberInited = true;

	return true;
}

void ofxONI2::close() {
	if(isThreadRunning()) {
		stopThread();
		ofSleepMillis(10);
		waitForThread(false);
	}

	deviceURI = "";
	bIsFrameNew = false;
	bNeedsUpdateDepth = false;
	bNeedsUpdateColor = false;
	bUpdateTex = false;
}

void ofxONI2::clear() {
	if(isConnected()) {
		ofLogWarning("ofxONI2") << "clear(): do not call clear while ofxONI2 is running!";
		return;
	}
	openni::OpenNI::shutdown();

	depthPixelsRaw.clear();
	depthPixelsRawBack.clear();

	videoPixels.clear();
	videoPixelsBack.clear();

	depthPixels.clear();
	distancePixels.clear();

	depthTex.clear();
	videoTex.clear();

	bGrabberInited = false;
}

bool ofxONI2::openstreams(const char* deviceURI, openni::VideoMode* depthVideoMode, openni::VideoMode* colorVideoMode) {
	ofLogVerbose("ofxONI2") << "Opening OpenNI streams.";
	openni::Status rc = openni::STATUS_OK;

	// Open device
	rc = oni_device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		ofLogWarning("ofxONI2") << "OpenNI device open failed: " << openni::OpenNI::getExtendedError();
		return false;
	}

	// Open depth stream
	rc = oni_depth_stream.create(oni_device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK) {
		rc = oni_depth_stream.start();
		if (rc != openni::STATUS_OK) {
			ofLogWarning("ofxONI2") << "Couldn't open depth stream: " << openni::OpenNI::getExtendedError();
			oni_depth_stream.destroy();
			return false;
		}
	} else {
		ofLogWarning("ofxONI2") << "Couldn't open depth stream: " << openni::OpenNI::getExtendedError();
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

	if (!oni_depth_stream.isValid()) {
		ofLogWarning("ofxONI2") << "Depth stream is invalid: " << openni::OpenNI::getExtendedError();
		return false;
	}

	if (bGrabVideo && !oni_color_stream.isValid()) {
		ofLogWarning("ofxONI2") << "Color stream is invalid: " << openni::OpenNI::getExtendedError();
		return false;
	}

	*depthVideoMode = oni_depth_stream.getVideoMode();
	if(bGrabVideo) *colorVideoMode = oni_color_stream.getVideoMode();

	oni_streams = new openni::VideoStream*[2];
	oni_streams[0] = &oni_depth_stream;
	oni_streams[1] = &oni_color_stream;

	return true;
}


bool ofxONI2::open(const char* deviceURI) {
	if(!bGrabberInited) {
		ofLogWarning("ofxONI2") << "open(): cannot open, init not called";
		return false;
	}

	openni::VideoMode depthVideoMode;
	openni::VideoMode colorVideoMode;

	bool streams_opened = openstreams(deviceURI, &depthVideoMode, &colorVideoMode);
	if(!streams_opened) {
		ofLogWarning("ofxONI2") << "could not open streams (openstreams())";
		return false;
	}

	if (bGrabVideo) {
		int oni_depthWidth = depthVideoMode.getResolutionX();
		int oni_depthHeight = depthVideoMode.getResolutionY();
		int oni_colorWidth = colorVideoMode.getResolutionX();
		int oni_colorHeight = colorVideoMode.getResolutionY();

		if (oni_depthWidth == oni_colorWidth && oni_depthHeight == oni_colorHeight) {
			stream_width = oni_depthWidth;
			stream_height = oni_depthHeight;
		} else {
			ofLogWarning("ofxONI2") <<  "Error - expect color and depth streams to be in same resolution"; 
			return false;
		}
	} else {
		stream_width = depthVideoMode.getResolutionX();
		stream_height = depthVideoMode.getResolutionY();
	}


	// Require the depth image to be in the OpenNI format PIXEL_FORMAT_DEPTH_1_MM,
	// and the color image to be of in format PIXEL_FORMAT_RGB888. If this should fail,
	// maybe it should be specified by some setPixelFormat first?

	ofLogVerbose("ofxONI2") << "OpenNI depth pixel format " << depthVideoMode.getPixelFormat();
	if(depthVideoMode.getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_1_MM) {
		ofLogWarning("ofxONI2") << "Expected depth image format to be PIXEL_FORMAT_DEPTH_1_MM";
		return false;
	}

	if(bGrabVideo) {
		ofLogVerbose("ofxONI2") << "OpenNI color pixel format " << colorVideoMode.getPixelFormat();
		if(colorVideoMode.getPixelFormat() != openni::PIXEL_FORMAT_RGB888) {
			ofLogWarning("ofxONI2") << "Expected color image format to be PIXEL_FORMAT_RGB888";
			return false;
		}
	}


	ofLogVerbose("ofxONI2") << "Image size " << stream_width << "x" << stream_height << endl;

	depthPixelsRaw.allocate(stream_width, stream_height, 1);
	depthPixelsRawBack.allocate(stream_width, stream_height, 1);

	videoPixels.allocate(stream_width, stream_height, OF_IMAGE_COLOR);
	videoPixelsBack.allocate(stream_width, stream_height, OF_IMAGE_COLOR);

	depthPixels.allocate(stream_width,stream_height, 1);
	distancePixels.allocate(stream_width, stream_height, 1);

	depthPixelsRaw.set(0);
	depthPixelsRawBack.set(0);
	videoPixels.set(0);
	videoPixelsBack.set(0);
	depthPixels.set(0);
	distancePixels.set(0);

	if(bUseTexture) {
		depthTex.allocate(stream_width, stream_height, GL_RGB);
		videoTex.allocate(stream_width, stream_height, GL_RGB);
	}

	startThread(true, false);  // blocking, not verbose

	return true;

}


// Reads next pixel image (either depth or color) into openFrameworks format.

void ofxONI2::update() {
	if(!bGrabberInited) return;
	if(!(bNeedsUpdateDepth or bNeedsUpdateColor)) return;

	bIsFrameNew = true;
	
	if(lock()) {

		if(bNeedsUpdateColor) {
			videoPixels = videoPixelsBack;
			bNeedsUpdateColor = false;
		}

		if(bNeedsUpdateDepth) {
			depthPixelsRaw = depthPixelsRawBack;
		}

		unlock();

		if(bNeedsUpdateDepth) {
			updateDepthPixels();
			bNeedsUpdateDepth = false;
		}

		if(bUseTexture) {
			depthTex.loadData(depthPixels.getPixels(), stream_width, stream_height, GL_RGB);
			videoTex.loadData(videoPixels.getPixels(), stream_width, stream_height, GL_RGB);
		}

	}

}

void ofxONI2::updateDepthPixels() {

	if(ref_max_depth == 0) {
		unsigned short *pDPixel = depthPixelsRaw.getPixels();
		for(int i = 0; i < stream_width*stream_height; i++, pDPixel++) {
			if(*pDPixel > ref_max_depth) ref_max_depth = *pDPixel;
		}

		ofLogVerbose("ofxONI2") << "Max depth establised to " << ref_max_depth;
	}

	unsigned short *rawpixel = depthPixelsRaw.getPixels();
	unsigned char  *depthpixel = depthPixels.getPixels();
	float	       *floatpixel = distancePixels.getPixels();
	ofColor c;

	for(int i = 0; i < stream_width*stream_height; i++) {
		if(rawpixel[i] > 0) {
			unsigned char hue = (unsigned char)(255.0 * (((double)rawpixel[i]) / ref_max_depth));
			c = ofColor::fromHsb(hue,255,255);
		} else {
			c = 0;
		}

		depthpixel[3*i + 0] = c.r;
		depthpixel[3*i + 1] = c.g;
		depthpixel[3*i + 2] = c.b;

		floatpixel[i] = depthpixel[i];
	}
}

void ofxONI2::threadedFunction() {
	int kinect_changed_index;
	openni::Status rc;

	ofLogVerbose("ofxONI2") << "Starting ofxONI2 update thread";

	while(isThreadRunning()) {
		rc = openni::OpenNI::waitForAnyStream(oni_streams, 2, &kinect_changed_index, openni::TIMEOUT_FOREVER);

		if(rc == openni::STATUS_OK) {
			if(kinect_changed_index == 0)  {
				oni_depth_stream.readFrame(&oni_depth_frame);
				lock();
				depthPixelsRawBack.setFromPixels((unsigned short*) oni_depth_frame.getData(), 
					stream_width, stream_height, 1);
				bNeedsUpdateDepth = true;
				unlock();
			} else if(kinect_changed_index == 1) {
				oni_color_stream.readFrame(&oni_color_frame);
				lock();
				videoPixelsBack.setFromPixels((unsigned char*) oni_color_frame.getData(),
					stream_width, stream_height, OF_IMAGE_COLOR);
				bNeedsUpdateColor = true;
				unlock();
			}
		}
	}

	ofLogVerbose("ofxONI2") << "Ending ofxONI2 update thread";
}


bool ofxONI2::isConnected() {
	return isThreadRunning();
}

bool ofxONI2::isFrameNew() {
	if(isThreadRunning() && bIsFrameNew) {
		bIsFrameNew = false;
		return true;
	} else {
		return false;
	}
}

unsigned char* ofxONI2::getPixels() {
	return videoPixels.getPixels();
}

unsigned char* ofxONI2::getDepthPixels() {
	return depthPixels.getPixels();
}

unsigned short* ofxONI2::getRawDepthPixels() {
	return depthPixelsRaw.getPixels();
}

float* ofxONI2::getDistancePixels() {
	return distancePixels.getPixels();
}

ofPixels& ofxONI2::getPixelsRef() {
	return videoPixels;
}

ofPixels& ofxONI2::getDepthPixelsRef() {
	return depthPixels;
}

ofShortPixels& ofxONI2::getRawDepthPixelsRef() {
	return depthPixelsRaw;
}

ofFloatPixels& ofxONI2::getDistancePixelsRef() {
	return distancePixels;
}

ofTexture& ofxONI2::getTextureReference() {
	if(!videoTex.bAllocated()) {
		ofLogWarning("ofxONI2") << "getTextureReference video texture not allocated";
	}

	return videoTex;
}

ofTexture& ofxONI2::getDepthTextureReference() {
	if(!depthTex.bAllocated()) {
		ofLogWarning("ofxONI2") << "getDepthTextureReference depth texture not allocated";
	}

	return depthTex;
}

void ofxONI2::setUseTexture(bool use_texture) {
	bUseTexture = use_texture;
}

void ofxONI2::draw(float x, float y, float w, float h) {
	if(bUseTexture && bGrabVideo) {
		videoTex.draw(x,y,w,h);
	}
}

void ofxONI2::draw(float x, float y) {
	draw(x,y,stream_width,stream_height);
}

void ofxONI2::draw(const ofPoint& point) {
	draw(point.x, point.y);
}

void ofxONI2::draw(const ofRectangle& rect) {
	draw(rect.x, rect.y, rect.width, rect.height);
}


void ofxONI2::drawDepth(float x, float y, float w, float h) {
	if(bUseTexture) {
		depthTex.draw(x,y,w,h);
	}
}

void ofxONI2::drawDepth(float x, float y) {
	drawDepth(x,y,stream_width,stream_height);
}

void ofxONI2::drawDepth(const ofPoint& point) {
	drawDepth(point.x, point.y);
}

void ofxONI2::drawDepth(const ofRectangle& rect) {
	drawDepth(rect.x, rect.y, rect.width, rect.height);
}

void ofxONI2::draw3D() {
	// not implemented yet
	draw(0,0);
}

float ofxONI2::getWidth() { return stream_width; }
float ofxONI2::getHeight() { return stream_height; }
