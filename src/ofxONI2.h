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

	private:

		// Back-buffers
		ofShortPixels depthPixelsRawBack; 
		ofPixels videoPixelsBack;

		bool bIsFrameNew;
		bool bNeedsUpdateColor;
		bool bNeedsUpdateDepth;
		bool bUpdateTex;
		bool bGrabVideo;
		bool bColorizeDepthImage;

		void threadedFunction();
		void updateDepthPixels();

	// 	void generate_grid();

	// 	ofVec3f xy2i_to_xyz3f(int x, int y);
	// 	ofVec3f pixel3f_to_world3f( ofVec3f p);
	// 	cv::Point3f pixel3f_to_world3f( cv::Point3f p);

 	openni::VideoFrameRef oni_depth_frame;
 	openni::VideoFrameRef oni_color_frame;

	// 	
	// 	ofImage depth_image;
	// 	ofImage color_image;
	// 	ofImage depth_map;
	// 	ofImage users_image;

	// 	ofMesh kinect_mesh;
	// 	ofVboMesh kinect_grid;
	 	unsigned short ref_max_depth;
	// private:
	//

	 	openni::Device oni_device;

		openni::VideoStream oni_depth_stream;
		openni::VideoStream oni_color_stream;

		openni::VideoStream **oni_streams;

	// 	bool b_updated_usertracker = false;

	// 	nite::UserTracker usertracker;
	// 	nite::UserTrackerFrameRef usertrackerframe;



	// 	ofTexture depth_texture;
	// 	ofTexture color_texture;

	// 	int kinect_width, kinect_height;
	
		int stream_width, stream_height;

	// 	bool b_has_changed_depth, b_has_changed_color;


	// 	void addMeshFace(ofVec3f a, ofVec3f b, ofVec3f c);
	// 	void addMeshFace(ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d);
	// 	void addMeshTexture(ofVec2f a, ofVec2f b, ofVec2f c);
	// 	void addMeshTexture(ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d);
};
