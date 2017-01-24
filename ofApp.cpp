#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	ofSetEscapeQuitsApp(false);
	
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofSetWindowShape(WIDTH, HEIGHT);
	
	/********************
	********************/
	gui.setup( "Parameters", "settings.xml" );
	
	gui.add( SmoothFilterThreshTime.setup( "SmoothTime", 0.3, 0, 1.0 ) );	// 小さくする程Responceが良くなる.
	gui.add( NonLinearFilter_ThreshLev.setup( "NonLinLev", 0, 0, 0.3 ) );	// 0.02
	gui.add( b_DownLimit.setup( "Down limit", false ) );
	gui.add( Down_per_Frame_60fps.setup( "Down60fps", 0.015, 0.001, 0.2 ) );
	
	/********************
	********************/
	udpConnection.Create();
	udpConnection.Connect("127.0.0.1",12345);
	udpConnection.SetNonBlocking(true);
	
	/********************
	********************/
	music.load("ClubLife By Tiesto Podcast 471.wav");
	music.setLoop(true);
	music.play();
	
	// ここでvolume cnotrolすると、fft結果に影響が出るので注意
	// 全体音量でControlする分には、影響が出ない.
	music.setVolume(1.0);

	//Set spectrum values to 0
	for (int i=0; i<N; i++) {
		spectrum[i] = 0.0f;
	}
	
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	ofSoundUpdate();
	
	//Get current spectrum with N bands
	float *val = ofSoundGetSpectrum( N );
	//We should not release memory of val,
	//because it is managed by sound engine
	
	/********************
	********************/
	float val_ave[N];
	
	/* */
	static float LastINT_sec = 0;
	float now = ofGetElapsedTimef();
	float dt = ofClamp(now - LastINT_sec, 0, 0.1);
	LastINT_sec = now;
	
	/* average */
	double SmoothFilterAlpha;
	if(0 < SmoothFilterThreshTime){
		double tangent = 1 / SmoothFilterThreshTime;
		
		if(dt < SmoothFilterThreshTime)	SmoothFilterAlpha = tangent * dt;
		else							SmoothFilterAlpha = 1;
	}else{

		SmoothFilterAlpha = 1;
	}
	
	/* Non Linear */
	double NonLinearFilter_k;
	if(0 < NonLinearFilter_ThreshLev){
		NonLinearFilter_k = 1/NonLinearFilter_ThreshLev;
	}else{
		NonLinearFilter_k = 0;
	}
	
	/* Down Speed */
	const float Down_per_ms = Down_per_Frame_60fps / 16.6;
	float DownRatio = Down_per_ms * dt * 1000;
	
	/* */
	for ( int i=0; i<N; i++ ) {
		/* */
		val_ave[i] = SmoothFilterAlpha * val[i] + (1 - SmoothFilterAlpha) * spectrum[i];
		
		/* */
		double diff = val_ave[i] - spectrum[i];
		if(0 < NonLinearFilter_ThreshLev){
			if( (0 <= diff) && (diff < NonLinearFilter_ThreshLev) ){
				diff = NonLinearFilter_k * pow(diff, 2);
			}else if( (-NonLinearFilter_ThreshLev < diff) && (diff < 0) ){
				diff = -NonLinearFilter_k * pow(diff, 2);
			}else{
				diff = diff;
			}
		}
		float val_NonLinearFilter_out = spectrum[i] + diff;
		
		/* */
		if(b_DownLimit){
			spectrum[i] *= (1 - DownRatio);
			spectrum[i] = max( spectrum[i], val_NonLinearFilter_out );
		}else{
			spectrum[i] = val_NonLinearFilter_out;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	/********************
	********************/
	// ofBackground(0, 0, 0, 0);
	ofBackground(50);

	/********************
	********************/
	static float LastINT;
	float now = ofGetElapsedTimef();
	if(0.033 < now - LastINT){
		LastINT = now;
		
		string message="";
		
		message += ofToString(20) + "|" + ofToString(20) + "|" + ofToString(255) + "[/p]";
		
		for(int i = 0; i < N; i++){
			message += ofToString(spectrum[i]) + "|";
		}
		
		udpConnection.Send(message.c_str(),message.length());
	}
	
	/********************
	********************/
	gui.draw();
	
	
	/********************
	********************/
	printf("%5.1f\r", ofGetFrameRate());
	fflush(stdout);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 's':
			gui.saveToFile( "settings.xml" );
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

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

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
