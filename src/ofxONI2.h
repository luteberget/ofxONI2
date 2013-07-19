#pragma once

#include "ofMain.h"

#undef Status
#undef STATUS
// Tested only with OpenNI 2.2 (on ubuntu linux x64)
#include <OpenNI.h>


#include "ofxBase3DVideo.h"

class ofxONI2 : public ofxBase3DVideo, protected ofThread {
	public:
		ofxONI2();
		~ofxONI2();

	// Main functions
	//
	//
		// initializes resources, must be called before open()
		// use_color_image can be set to false to avoid loading the color images (for speed)
		// use_textures can be set to false to avoid converting the pixels to textures
		// colorize_depth_image creates a color image with depth represented as hue, else the image is grayscale
		bool init(bool use_color_image = true, bool use_texture = true, bool colorize_depth_image = true);

		// clear resources
		void clear();

		// open connection and start grabbing images
		// deviceURI gives the OpenNI device identifier (see OpenNI docs)
		bool open(const char* deviceURI = openni::ANY_DEVICE);

		// close connection and stop grabbing images
		void close();

		// is the connection currently open?
		bool isConnected();

		// is the current frame new? This is positive when
		// either the depth frame or the color image is updated.
		bool isFrameNew();

		// update the pixel buffers and textures (if b_usetextures ).
		void update();


	// Pixel data
	//
	//
		// Raw pixel pointer for the color image:
		unsigned char* getPixels();

		// Raw pixel pointer for the (hue colorized or grayscale) depth image:
		unsigned char* getDepthPixels();

		// Raw pixel pointer for the raw 16 bit depth image (in mm)
		// OpenNI 2.2 uses typedef uint16_t openni::DepthPixel
		unsigned short* getRawDepthPixels();

		// Raw pixel pointer to float millimeter distance image:
		float* getDistancePixels();

		// ofPixel objects
		ofPixels& getPixelsRef();
		ofPixels& getDepthPixelsRef();
		ofShortPixels& getRawDepthPixelsRef();
		ofFloatPixels& getDistancePixelsRef();

		// Textures:
		// Color image as texture
		ofTexture& getTextureReference();
		// Depth image as texture
		ofTexture& getDepthTextureReference();


	// Device capabilities (OpenNI does not offer any, use ofxKinect instead)
	//
	//
		bool hasAccelControl() { return false; }
		bool hasCamTiltControl() { return false; }
		bool hasLedControl() { return false; }

	// Drawing functions
	//
	//
		// Enable/disable texture updates
		void setUseTexture(bool use_texture);

		// Draw the video texture
		void draw(float x, float y, float w, float h);
		void draw(float x, float y);
		void draw(const ofPoint& point);
		void draw(const ofRectangle& rect);

		// Draw the depth texture
		void drawDepth(float x, float y, float w, float h);
		void drawDepth(float x, float y);
		void drawDepth(const ofPoint& point);
		void drawDepth(const ofRectangle& rect);

		// Draw 3D
		void draw3D();

	// Util functions
	//
	//
		// Size of color/depth images. Should match.
		float getWidth();
		float getHeight();


	protected:

		string deviceURI;
		
		bool bUseTexture;
		ofTexture videoTex;
		ofTexture depthTex;
		bool bGrabberInited;

		ofPixels videoPixels;
		ofPixels depthPixels;
		ofShortPixels depthPixelsRaw;
		ofFloatPixels distancePixels;

		openni::VideoFrameRef oni_depth_frame;
		openni::VideoFrameRef oni_color_frame;

	 	openni::Device oni_device;

		openni::VideoStream oni_depth_stream;
		openni::VideoStream oni_color_stream;


		bool bIsFrameNew;
		bool bNeedsUpdateColor;
		bool bNeedsUpdateDepth;
		bool bUpdateTex;
		bool bGrabVideo;
		bool bColorizeDepthImage;
		
		// Back-buffers
		ofShortPixels depthPixelsRawBack; 
		ofPixels videoPixelsBack;

		int stream_width, stream_height;

		virtual void updateDepthPixels();

	 	unsigned short ref_max_depth;


	private:
		// The actual opening commands, returning video modes. Given as a seperate function to allow for NiTE
		// to open the device instead in subclass ofxNiTEUserTracker.
		virtual bool openstreams(const char* deviceURI, openni::VideoMode* depthVideoMode, openni::VideoMode* colorVideoMode);

		virtual void threadedFunction();

		openni::VideoStream **oni_streams;
};
