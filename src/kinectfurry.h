#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxDelaunay.h"
#include "ofxGui.h"

class kinectfurry : public ofBaseApp{

public:
        ofxKinect kinect;
        ofEasyCam cam;
        bool showGui;
        int angle;
        int colorAlpha;

        ofVboMesh convertedMesh;
        ofxDelaunay del;
        ofImage blob;
	
        ofxPanel gui;
        ofxSlider<float> noiseAmount;
        ofxToggle useRealColors;
        ofxSlider<int> pointSkip;
        ofxSlider<int> distnect;

        ofxSlider<float> alpha;
        ofxSlider<float> hairLeng;
        ofxSlider<float> cR;
        ofxSlider<float> cB;
        ofxSlider<float> cG;

        ofxSlider<float> camX;
        ofxSlider<float> camY;
        ofxSlider<float> camZ;

        ofxToggle noTassellation;
        ofxToggle stopnoise;
        ofxToggle realTime; 
        ofxToggle xDebug;

        ofShader shader;

	void setup(){
	    ofSetFrameRate(60);
	    ofSetVerticalSync(true);
	    shader.setGeometryInputType(GL_LINES);
	    shader.setGeometryOutputType(GL_TRIANGLE_STRIP);
	    shader.setGeometryOutputCount((5 + 1) * (4 + 1) * 2);
	    shader.load( "shaders/vertex.glsl", "shaders/fragment.glsl", "shaders/geom_hair.glsl");
	    printf("Max output vertices: %i\n", shader.getGeometryMaxOutputCount());
	    cam.setFarClip(100000);
	    colorAlpha=255;
	    kinect.init();
	    kinect.open();
	    kinect.setRegistration(true);
	    blob.allocate(640,480,OF_IMAGE_GRAYSCALE);//OF_IMAGE_COLOR);
	    showGui = true;
	    kinect.setCameraTiltAngle(0);

	    gui.setup();
	    gui.setPosition(ofPoint(10,10));
	    gui.add(stopnoise.setup("Noise furry",false));
	    gui.add(realTime.setup("real time",true));
	    gui.add(noiseAmount.setup("Noise vert", 0.0, 0.0,100.0));
	    gui.add(pointSkip.setup("Point Skip", 2, 2, 10));
	    gui.add(distnect.setup("dist nect", 1100, 0, 5000));

	    gui.add(alpha.setup("Alpha", 0.6,0.,1.0));
	    gui.add(hairLeng.setup("hair leng", 0,-600,600));
	    gui.add(cR.setup("r",    0.,0.,1.0));
	    gui.add(cG.setup("g", 0.,0.,1.0));
	    gui.add(cB.setup("b",  0.,0.,1.0));
	    gui.add(camX.setup("camX",  0,-1000,1000));
	    gui.add(camY.setup("camY",  0,-1000,1000));
	    gui.add(camZ.setup("camZ",  0,-1000,1000));
	    gui.add(xDebug.setup("debug",true));
            gui.add(noTassellation.setup("no tassellation",true));

	    gui.loadFromFile("settings.xml");
	}

	void generatedMesh(){
	   if(kinect.isFrameNew()) {
		del.reset();
		unsigned char* pix = new unsigned char[640*480];
		for(int x=0;x<640;x+=1) {
		     for(int y=0;y<480;y+=1) {
	                  float distance = kinect.getDistanceAt(x, y);
			  int pIndex = x + y * 640;
			  pix[pIndex] = 0;
			  if(distance > 100 && distance < distnect) { // 
					pix[pIndex] = 255;
			  }
		     }
		}
		blob.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
		int numPoints = 0;
		for(int x=0;x<640;x+=pointSkip) {
		   for(int y=0;y<480;y+=pointSkip) {
			int pIndex = x + 640 * y;
			if(blob.getPixels()[pIndex]> 0) {
				ofVec3f wc = kinect.getWorldCoordinateAt(x, y);
				wc.x = x - 320.0;
		                wc.y = y - 240.0;
				if(abs(wc.z) > 100 && abs(wc.z ) < 2000) { // 
					wc.z = -wc.z;
				        wc.x += ofSignedNoise(wc.x,wc.z)*noiseAmount;
                                	wc.y += ofSignedNoise(wc.y,wc.z)*noiseAmount;
					wc.x = ofClamp(wc.x, -320,320);
					wc.y = ofClamp(wc.y, -240,240);
					del.addPoint(wc);
				}
				numPoints++;
			}
		    }
		}
	
		if(numPoints >0) del.triangulate();
		for(int i=0;i<del.triangleMesh.getNumVertices();i++) { del.triangleMesh.addColor(ofColor(0,0,0)); }
			for(int i=0;i<del.triangleMesh.getNumIndices()/3;i+=1) {
				ofVec3f v = del.triangleMesh.getVertex(del.triangleMesh.getIndex(i*3));
				v.x = ofClamp(v.x, -319,319);
				v.y = ofClamp(v.y, -239, 239);
				ofColor c = kinect.getColorAt(v.x+320.0, v.y+240.0);
				c.a = colorAlpha;
				del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3),c);
				del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+1),c);
				del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+2),c);
			}

		convertedMesh.clear();
		for(int i=0;i<del.triangleMesh.getNumIndices()/3;i+=1) {
			int indx1 = del.triangleMesh.getIndex(i*3);
			ofVec3f p1 = del.triangleMesh.getVertex(indx1);
			int indx2 = del.triangleMesh.getIndex(i*3+1);
			ofVec3f p2 = del.triangleMesh.getVertex(indx2);
			int indx3 = del.triangleMesh.getIndex(i*3+2);
			ofVec3f p3 = del.triangleMesh.getVertex(indx3);
			ofVec3f triangleCenter = (p1+p2+p3)/3.0;
			triangleCenter.x += 320;
			triangleCenter.y += 240;
			triangleCenter.x = floor(ofClamp(triangleCenter.x, 0,640));
			triangleCenter.y = floor(ofClamp(triangleCenter.y, 0,480));
			int pixIndex = triangleCenter.x + triangleCenter.y * 640;
			if(pix[pixIndex] > 0) {
				convertedMesh.addVertex(p1);
				convertedMesh.addColor(del.triangleMesh.getColor(indx1));
				convertedMesh.addVertex(p2);
				convertedMesh.addColor(del.triangleMesh.getColor(indx2));
				convertedMesh.addVertex(p3);
				convertedMesh.addColor(del.triangleMesh.getColor(indx3));
			}
		}
		delete pix;
	    }
	}
	
	void update(){
	    ofSetWindowTitle(ofToString(ofGetFrameRate()));
	    kinect.update();
	    if(realTime){
	         generatedMesh();
	    }
            cam.setPosition(camX, camY, camZ);
	}

	void draw(){
	        ofBackgroundGradient( ofColor(210), ofColor(10), OF_GRADIENT_BAR);

		glEnable(GL_DEPTH_TEST);
		cam.begin();

		shader.begin();
	        shader.setUniform1f("hairLeng",hairLeng);
	        shader.setUniform1f("time", ofGetElapsedTimef() );
		if(stopnoise) shader.setUniform1f( "timex", ofSignedNoise( ofGetElapsedTimef() ) );
		else          shader.setUniform1f( "timex", 0. );
		float tass=0.;
		if(noTassellation==true) tass = 1.;
	        shader.setUniform1f("noTass",tass);
	        shader.setUniformMatrix4f("projection",cam.getProjectionMatrix());
	        shader.setUniformMatrix4f("modelview", cam.getModelViewMatrix() );
	        shader.setUniform1f("alpha", alpha);
	        shader.setUniform3f("colored", cR,cG,cB);
	
		cam.setScale(1,-1,1);
		ofTranslate(0, -80, 1100);
//		ofFill();
//		glPushAttrib(GL_ALL_ATTRIB_BITS);
//		glShadeModel(GL_FLAT);
//		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
		convertedMesh.draw();
//		glShadeModel(GL_SMOOTH);
//		glPopAttrib();
	 	
		shader.end();
		
		cam.end();
		glDisable(GL_DEPTH_TEST);

		if(xDebug){	
			blob.draw(280,20,blob.getWidth()/3,blob.getHeight()/3);
			ofPushMatrix();
			ofTranslate(650,-250,0);
			del.draw();
			ofPopMatrix();
			ofPushMatrix();
			ofTranslate(1100,-250,0);
			convertedMesh.drawWireframe();
			ofPopMatrix();
		}

	        if(showGui) {
	                ofPushStyle();
	                ofSetColor(255,255,255,255);
	                gui.draw();
	                ofPopStyle();
	        }

	}

	void keyPressed(int key){
		if(key == ' ') {
			showGui=!showGui;
			if(showGui==false) xDebug=false;
		}
		if(key == 'f')	
	           ofToggleFullscreen();
		if(key == 's')
                   gui.saveToFile("settings.xml");
		if(key == OF_KEY_UP){
	                angle++;
	        	if(angle>30) angle=30;
		                kinect.setCameraTiltAngle(angle);
		}
	        if(key == OF_KEY_DOWN){
	                angle--;
	                if(angle<-30) angle=-30;
	        	        kinect.setCameraTiltAngle(angle);
			}
	}

	void exit() {
		kinect.setCameraTiltAngle(0);
		kinect.close();
	}

};	
