#pragma once

#include "ofxONI2.h"
#include "NiTE.h"

struct ofxNiTEUserData {
	ofVec3f boundingBoxMin;
	ofVec3f boundingBoxMax;
	ofVec3f centerOfMass;
	short id;
	bool isVisible;
	bool isSkeletonAvailable;
	map<nite::JointType,ofVec3f> skeletonPoints;

	static ofVec3f toOf(const nite::Point3f& p) { return ofVec3f(p.x,p.y,p.z); }
	static ofVec3f toOf(const NitePoint3f& p) { return ofVec3f(p.x,p.y,p.z); }

	ofxNiTEUserData() {};
	//~ofxNiTEUserData() {};

	ofxNiTEUserData(const nite::UserData &data) {
		boundingBoxMin = toOf(data.getBoundingBox().min);
		boundingBoxMin = toOf(data.getBoundingBox().max);
		centerOfMass   = toOf(data.getCenterOfMass());
		id = data.getId();
		isVisible = data.isVisible();
		isSkeletonAvailable = (data.getSkeleton().getState() == nite::SKELETON_TRACKED);

		skeletonPoints[nite::JOINT_HEAD		 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_HEAD		 ).getPosition());
		skeletonPoints[nite::JOINT_NECK		 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_NECK		 ).getPosition());
		skeletonPoints[nite::JOINT_LEFT_SHOULDER ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_SHOULDER] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER).getPosition());
		skeletonPoints[nite::JOINT_LEFT_ELBOW	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW	 ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_ELBOW	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW	 ).getPosition());
		skeletonPoints[nite::JOINT_LEFT_HAND	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_HAND	 ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_HAND	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND	 ).getPosition());
		skeletonPoints[nite::JOINT_TORSO	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_TORSO	 ).getPosition());
		skeletonPoints[nite::JOINT_LEFT_HIP	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_HIP	 ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_HIP	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP	 ).getPosition());
		skeletonPoints[nite::JOINT_LEFT_KNEE	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE	 ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_KNEE	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE	 ).getPosition());
		skeletonPoints[nite::JOINT_LEFT_FOOT	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT	 ).getPosition());
		skeletonPoints[nite::JOINT_RIGHT_FOOT	 ] = toOf(data.getSkeleton().getJoint(nite::JOINT_RIGHT_FOOT	 ).getPosition());
	}
};


class ofxNiTEUserTracker : public ofxONI2 {
	public:
		bool init(bool use_color_image = true, bool use_texture = true, bool colorize_depth_image = true);
		virtual void updateDepthPixels();
		ofShortPixels& getUserMapRef() { return userMap; }
		vector<ofxNiTEUserData>& getUserData() { return userData; }

		ofEvent<short> newUserEvent;
		ofEvent<short> lostUserEvent;

	private:
		nite::UserTracker usertracker;
		nite::UserTrackerFrameRef usertrackerframe;

		ofShortPixels userMap;
		ofShortPixels userMapBack;
		ofTexture userMapTex;

		vector<ofxNiTEUserData> userData;
		vector<ofxNiTEUserData> userDataBack;

		virtual bool openstreams(const char* deviceURI, openni::VideoMode* depthVideoMode, openni::VideoMode* colorVideoMode);
		virtual void threadedFunction();

		bool bUseSkeletonTracking;

};
