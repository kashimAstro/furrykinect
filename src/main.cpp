#include "kinectfurry.h"
#include "ofAppGlutWindow.h"

int main(){
    ofGLWindowSettings settings;
    settings.setGLVersion(4,1); /// < select your GL Version here for furrykinect minimum is 3.1 (Important for mac, because otherwise it doesn't build with the selected version by default) 
    ofCreateWindow(settings); ///< create your window here
    ofRunApp(new kinectfurry());
}
