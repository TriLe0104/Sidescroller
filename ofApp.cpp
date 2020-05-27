#include "ofApp.h"

//----------------------------------------------------------------------------------
//
// Side scrolling shooter using sprite emitter system
//
//  Kevin M. Smith - CS 134 SJSU
//	Student: Tri Le

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofBackground(0, 0, 0);

	score = 0;
	health = 10;
	level = 1;

	gun = new Emitter(new SpriteSystem());
	gun->setChildSize(5, 5);

	turbForce = new TurbulenceForce(ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20));
	radialForce = new ImpulseRadialForce(10000.0);
	cyclicForce = new CyclicForce(10000);

	enemy = new Emitter(new SpriteSystem());

	if (defaultImage.loadImage("images/attack.gif")) {
		gun->setChildImage(defaultImage);
		gun->drawable = false;
		gun->setLifespan(4500);
		gun->setVelocity(glm::vec3(650, 0, 0));
		imageLoaded = true;
	}

	if (defaultImage.loadImage("images/aliensprite.png")) {
		enemy->setChildImage(defaultImage);
		enemy->drawable = false;
		enemy->setPosition(ofVec3f(ofGetWindowWidth()-100, ofGetWindowHeight() / 2, 0));
		enemy->setChildSize(10, 10);
		imageLoaded = true;
	}

	if (defaultImage.loadImage("images/marvel.png")) {
		player.setImage(defaultImage);
		player.setPosition(ofVec3f(100, ofGetWindowHeight() / 2, 0));
		start_point = ofVec3f(100, ofGetWindowHeight() / 2);
		finish_point = ofVec3f(700, 500);
		player.speed = 500;   // in pixels per second (screenspace 1 unit = 1 pixel)
		moveDir = MoveStop;
		imageLoaded = true;
	}

	if (defaultImage.loadImage("images/space-invaders-ship-scaled.png")) {
		bossEmitter.setChildImage(defaultImage);
		bossEmitter.setChildSize(defaultImage.getWidth(), defaultImage.getHeight());
		bossEmitter.haveChildImage = true;
		imageLoaded = true;
	}

	if (defaultImage.loadImage("images/background.jpg")) {
		background.setImage(defaultImage);
		background.setPosition(ofVec3f(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2, 0));
		imageLoaded = true;
	}

	bossEmitter.sys->add(boss);
	bossEmitter.sys->addForce(turbForce);
	bossEmitter.sys->addForce(radialForce);
//	bossEmitter.setVelocity(ofVec3f(-5, -5, 0));
	bossEmitter.setOneShot(true);
	bossEmitter.setGroupSize(5);
	bossEmitter.setLifespan(12);
	explosion.setEmitterType(DirectionalEmitter);
	bossEmitter.setRate(10);

	explosion.sys->addForce(turbForce);
	explosion.sys->addForce(radialForce);
	explosion.setVelocity(ofVec3f(0, 0, 0));
	explosion.setOneShot(true);
	explosion.setEmitterType(RadialEmitter);
	explosion.setGroupSize(20);

	fireSound.load("sounds/fire.wav");
	explodeSound.load("sounds/explode.wav");
	explodeSound.setVolume(.8f);
	hurtSound.load("sounds/hurt.wav");

	gui.setup();
	gui.add(rate.setup("rate", 2, 1, 10));
	gui.add(life.setup("life", 4, .1, 10));
	gui.add(velocity.setup("velocity", ofVec3f(-300, 0, 0), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));

	gui.add(lifespan.setup("Lifespan", .4, .1, 10.0));
	gui.add(rate2.setup("Rate", 1.0, .5, 60.0));
	gui.add(radius.setup("Radius", 4, 1, 10));
	
	bHide = true;
	gameStart = false;
	gun->start();
	explosion.start();
	enemy->start();
}

//--------------------------------------------------------------
void ofApp::update() {

	if (gameStart)
	{
		if (health > 0)
		{
		explosion.setLifespan(lifespan);
		explosion.setRate(rate2);
		explosion.setParticleRadius(radius);
		explosion.update();
		checkCollisions();

		gun->setPosition(ofVec3f((player.trans)));
		gun->update();

		enemy->setLifespan(life * 1000);
		enemy->setRate(rate);
		enemy->update();
		ofVec3f v = velocity;

		enemy->setVelocity(ofVec3f(v.x, ofRandom(-v.x / 2, v.x / 2), 0));
		bossEmitter.setVelocity(ofVec3f(v.x, 0, 0));
		enemy->setPosition(ofVec3f(ofGetWindowWidth() - 100, ofRandom(350, ofGetWindowWidth() - 350), 0));
		bossEmitter.setPosition(ofVec3f(ofGetWindowWidth() + 50, ofRandom(450, ofGetWindowWidth() - 450), 0));
		updateSprite();
		bossEmitter.update();
		}
		else
		{
			gameStart = false;
		}
		//Scales the game difficulty, level increases every 10 points increase the speed and spawn rate of enemy
		if (score % 11 == 10 && health > 0)
		{
			level += 1;
			rate = rate + .5;
			velocity = velocity + ofVec3f(-50, 0, 0);
			bossEmitter.sys->reset();
			bossEmitter.start();
			score = 0;
		}
	}
}


//--------------------------------------------------------------
void ofApp::draw(){
	if (health > 0)
	{
		ofDrawBitmapString("Press spacebar to start", ofGetWindowWidth() / 2, ofGetWindowHeight() / 2);
	}
	else
	{
		ofDrawBitmapString("GAME OVER, press r to try again", ofPoint(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2));
	}
	if (gameStart)
	{
		ofBackground(0, 0, 0);
		background.draw();
		string scoreText;
		scoreText += "Score: " + std::to_string(score);
		ofDrawBitmapString(scoreText, ofPoint(10, 35));

		string healthText;
		healthText += "Health: " + std::to_string(health);
		ofDrawBitmapString(healthText, ofPoint(10, 20));

		string levelText;
		levelText += "Level: " + std::to_string(level);
		ofDrawBitmapString(levelText, ofPoint(10, 50));

		bossEmitter.draw();
		gun->draw();
		enemy->draw();
		player.draw();
		explosion.draw();
		if (!bHide) {
			gui.draw();
		}
	}
}


void ofApp::checkCollisions() {

	// find the distance at which the two sprites (missles and invaders) will collide
	// detect a collision when we are within that distance.
	//
	float collisionDist1 = gun->childHeight / 2 + enemy->childHeight / 2;
	float collisionDist2 = gun->childHeight / 2 + bossEmitter.childHeight / 2;
	// Loop through all the missiles, then remove any invaders that are within
	// "collisionDist" of the missiles.  the removeNear() function returns the
	// number of missiles removed.
	//
	for (int i = 0; i < gun->sys->sprites.size(); i++) 
	{
		if (enemy->sys->removeNear(gun->sys->sprites[i].trans, collisionDist1) > 0 || bossEmitter.sys->removeNear(gun->sys->sprites[i].trans, collisionDist2) > 0) {
			score += 1;
			explodeSound.play();
			explosion.sys->reset();
			explosion.setPosition(gun->sys->sprites[i].trans + ofVec3f(enemy->childWidth,0,0));
			explosion.start();
		}
	}
	if (enemy->sys->removeNear(player.trans, collisionDist1) || bossEmitter.sys->removeNear(player.trans, collisionDist2) && life > 0) {
		health += -1;
		hurtSound.play();
	}
}


//--------------------------------------------------------------
float ofApp::modulateAccel(float dist) {
	return sin(dist * PI) * 5.0 + 1.0;
}

void ofApp::updateSprite() {

	// 
	// calculate distance to travel for this update
	//
	float dist = player.speed * 1 / ofGetFrameRate();
	ofVec3f dir;
	ofRectangle r = ofGetWindowRect();

	//
	//  if the accelerator modifer key is pressed, accelerate and
	//  deacclerate sprite from starting position to window edge
	//
	if (accel) {

		switch (moveDir)
		{
		case MoveUp:
		{
			float totalDist = startAccelPoint.y;
			float frac = player.trans.y / totalDist;
			dir = ofVec3f(0, -dist * modulateAccel(frac), 0);
		}
		break;
		case MoveDown:
		{
			float totalDist = r.getMaxY() - startAccelPoint.y;
			float frac = player.trans.y / totalDist;
			dir = ofVec3f(0, dist * modulateAccel(frac), 0);
		}
		break;
		case MoveLeft:
		{
			float totalDist = startAccelPoint.x;
			float frac = player.trans.x / totalDist;
			dir = ofVec3f(-dist * modulateAccel(frac), 0, 0);
		}
		break;
		case MoveRight:
		{
			float totalDist = r.getMaxX() - startAccelPoint.x;
			float frac = player.trans.x / totalDist;
			dir = ofVec3f(dist * modulateAccel(frac), 0, 0);
			break;
		}
		}

	}
	else
	{
		switch (moveDir)
		{
		case MoveUp:
			dir = ofVec3f(0, -dist, 0);
			break;
		case MoveDown:
			dir = ofVec3f(0, dist, 0);
			break;
		case MoveLeft:
			dir = ofVec3f(-dist, 0, 0);
			break;
		case MoveRight:
			dir = ofVec3f(dist, 0, 0);
			break;
		}
	}
	if ((player.trans.y + dir.y) < (ofGetWindowHeight() - 150) && (player.trans.y + dir.y) > 150 && (player.trans.x + dir.x) < (ofGetWindowWidth() - 50) && (player.trans.x + dir.x) > 70) {
		player.trans += dir;
	}
}

void ofApp::moveSprite(MoveDir dir) {
	moveDir = dir;
}

void ofApp::stopSprite() {
	moveDir = MoveStop;
}

void ofApp::startAccel() {
	startAccelPoint = player.trans;
	accel = true;
}

void ofApp::stopAccel() {
	accel = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
//	cout << "mouse( " << x << "," << y << ")" << endl;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	ofPoint mouse_cur = ofPoint(x, y);
	ofVec3f delta = mouse_cur - mouse_last;

	if ((player.trans.y + delta.y) < (ofGetWindowHeight() - 150) && (player.trans.y + delta.y) > 150 && (player.trans.x + delta.x) < (ofGetWindowWidth() - 50) && (player.trans.x + delta.x) > 70) {
		player.trans += delta;
		mouse_last = mouse_cur;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	mouse_last = ofPoint(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

void ofApp::keyPressed(int key) {
	switch (key) {
	case 'C':
	case 'c':
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		bHide = !bHide;
		break;
	case 'R':
		health = 10;
		break;
	case 'r':
		health = 10;
		break;
	case 's':
		break;
	case 'u':
		break;
	case ' ':
		if (gameStart)
		{
		gun->setRate(2);
		fireSound.play();
		}
		gameStart = true;
		break;
	case '.':
		player.speed += 30;
		break;
	case ',':
		player.speed -= 100;
		break;
	case OF_KEY_RIGHT:
		moveSprite(MoveRight);
		break;
	case OF_KEY_LEFT:
		moveSprite(MoveLeft);
		break;
	case OF_KEY_UP:
		moveSprite(MoveUp);
		break;
	case OF_KEY_DOWN:
		moveSprite(MoveDown);
		break;
	case OF_KEY_ALT:
		break;
	case OF_KEY_CONTROL:
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	}
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case OF_KEY_LEFT:
	case OF_KEY_RIGHT:
	case OF_KEY_UP:
	case OF_KEY_DOWN:
		stopSprite();
		stopAccel();
		break;
	case ' ':
		gun->setRate(0);
		break;
	case OF_KEY_ALT:
		break;
	case OF_KEY_CONTROL:
		break;
	case OF_KEY_SHIFT:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

