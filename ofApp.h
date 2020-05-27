#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Sprite.h"
#include "Emitter.h"
#include "ParticleEmitter.h"
#include "TransformObject.h"
#include "Particle.h"

typedef enum { MoveStop, MoveLeft, MoveRight, MoveUp, MoveDown } MoveDir;

class Emitter;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void checkCollisions();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void updateSprite();
		void moveSprite(MoveDir);
		void stopSprite();
		void startAccel();
		void stopAccel();
		float modulateAccel(float);

		Emitter *gun, *enemy;
		ParticleEmitter explosion;

		ParticleEmitter bossEmitter;
		Particle boss;

		CyclicForce *cyclicForce;
		ImpulseRadialForce *radialForce;
		TurbulenceForce *turbForce;

		ofVec3f start_point, finish_point;
		float start_time, finish_time;
		MoveDir moveDir;
		bool accel;
		ofVec3f startAccelPoint;

		Sprite player, background;

		ofSoundPlayer fireSound, explodeSound, hurtSound;

		ofImage defaultImage;
		ofVec3f mouse_last;
		bool imageLoaded;

		bool bHide;
		bool gameStart;
		int score;
		int health;
		int level;

		ofxFloatSlider rate;
		ofxFloatSlider life;
		ofxVec3Slider velocity;
		ofxVec3Slider position;
		ofxLabel screenSize;
		ofxFloatSlider radius;
		ofxFloatSlider lifespan;
		ofxFloatSlider rate2;

		ofxPanel gui;

};
