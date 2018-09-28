#include "ExampleApp.h"

#define FONTSTASH_IMPLEMENTATION
#include <fontstash.h>
#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>

#include <config/VRDataIndex.h>

using namespace basicgraphics;
using namespace std;
using namespace glm;

const double ExampleApp::GRAVITY = 9.8;

ExampleApp::ExampleApp(int argc, char** argv) : VRApp(argc, argv)
{
	_lastTime = 0.0;
    _curFrameTime = 0.0;
    time = 0.0;
}

ExampleApp::~ExampleApp()
{
	glfonsDelete(fs);
	shutdown();
}

void ExampleApp::onAnalogChange(const VRAnalogEvent &event) {
    // This routine is called for all Analog_Change events.  Check event->getName()
    // to see exactly which analog input has been changed, and then access the
    // new value with event->getValue().
    
	if (event.getName() == "FrameStart") {
		_lastTime = _curFrameTime;
		_curFrameTime = event.getValue();
	}


}

void ExampleApp::onButtonDown(const VRButtonEvent &event) {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
	//You can respond to individual events like this:
	/*
    if (event.getName() == _paintOnEvent) {
        _painting = true;
    }
    else if (event.getName() == _grabOnEvent) {
        _grabbing = true;
    }
	*/

	//std::cout << "ButtonDown: " << event.getName() << std::endl;

}

void ExampleApp::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been released.

	//std::cout << "ButtonUp: " << event.getName() << std::endl;
}

void ExampleApp::onCursorMove(const VRCursorEvent &event) {
	// This routine is called for all mouse move events. You can get the absolute position
	// or the relative position within the window scaled 0--1.
	
	//std::cout << "MouseMove: "<< event.getName() << " " << event.getPos()[0] << " " << event.getPos()[1] << std::endl;
}

void ExampleApp::onTrackerMove(const VRTrackerEvent &event) {
    // This routine is called for all Tracker_Move events.  Check event->getName()
    // to see exactly which tracker has moved, and then access the tracker's new
    // 4x4 transformation matrix with event->getTransform().

	// We will use trackers when we do a virtual reality assignment. For now, you can ignore this input type.
}

void ExampleApp::drawSphere(float angle, float distance, float radius, vec4 color){
    
    // a 3D unit vector in the XY plane
    vec3 direction(glm::cos(angle), glm::sin(angle), 0);
    
    // point = point + scalar*vector
    vec3 position = vec3(0,0,0) + distance*direction;
    
    Sphere sphere(position, radius, color);
    sphere.draw(_shader, glm::mat4(1.0));
    
    Line connector(vec3(0,0,0), position, vec3(0,0,1), 0.05, vec4(0,0,0,1) );
    connector.draw(_shader, glm::mat4(1.0));
}

void ExampleApp::onRenderGraphicsContext(const VRGraphicsState &renderState) {
    // This routine is called once per graphics context at the start of the
    // rendering process.  So, this is the place to initialize textures,
    // load models, or do other operations that you only want to do once per
    // frame.
    
	// Is this the first frame that we are rendering after starting the app?
    if (renderState.isInitialRenderCall()) {

		//For windows, we need to initialize a few more things for it to recognize all of the
		// opengl calls.
		#ifndef __APPLE__
			glewExperimental = GL_TRUE;
			GLenum err = glewInit();
			if (GLEW_OK != err)
			{
				std::cout << "Error initializing GLEW." << std::endl;
			}
		#endif     


        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_MULTISAMPLE);

		// This sets the background color that is used to clear the canvas
		glClearColor(0.2f, 0.6f, 1.0f, 1.0f);

		// This load shaders from disk, we do it once when the program starts up.
		reloadShaders();

		initializeText();
        
        ground.reset(new Box(vec3(-40, -2, 3), vec3(40, -4, -100), vec4(0.3, 0.9, 0.4, 1.0)));
        target1.reset(new Box(vec3(20, -2, -40), vec3(23, 10, -30), vec4(0.6, 0.4, 0.2, 1.0)));
        target2.reset(new Box(vec3(25, -2, -40), vec3(28, 10, -30), vec4(0.6, 0.4, 0.2, 1.0)));
        target3.reset(new Box(vec3(22, 10, -40), vec3(26, 20, -30), vec4(0.6, 0.4, 0.2, 1.0)));
        
        launcher.reset(new Line(vec3(-30, -2, -35), vec3(-30, 5, -35), vec3(0,0,1), 0.5, vec4(0,0,0,1)));
        
        bird.reset(new Sphere(_position, 1.0, vec4(1,0,0,1)));
        
    }
    
    // Update the time once per frame
    double deltaTime = _curFrameTime - _lastTime;
    time += deltaTime*0.25;
    if (time > 4){
        time = 0;
    }
    
    
    _position = birdPos(time);

}

vec3 ExampleApp::birdPos(double time) {
    double e = time / 4.0;
    
    //TODO: does this make sense?
    return vec3(e*52-30,30*(-e*e+1.2*e)+5,-35);
}


void ExampleApp::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye/camera.  This is the place to actually
    // draw the scene.
    
	// clear the canvas and other buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Setup the view matrix to set where the camera is located in the scene
	glm::vec3 eye_world = glm::vec3(0, 10, 50);
	glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	// When we use virtual reality, this will be replaced by:
	// eye_world = glm::make_vec3(renderState.getCameraPos())
	// view = glm::make_mat4(renderState.getViewMatrix());

	// Setup the projection matrix so that things are rendered in perspective
	GLfloat windowHeight = renderState.index().getValue("WindowHeight");
	GLfloat windowWidth = renderState.index().getValue("WindowWidth");
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 100.0f);
	// When we use virtual reality, this will be replaced by:
	// projection = glm::make_mat4(renderState.getProjectionMatrix())
	
	// Setup the model matrix
	glm::mat4 model = glm::mat4(1.0);
    
	// Tell opengl we want to use this specific shader.
	_shader.use();
	
	_shader.setUniform("view_mat", view);
	_shader.setUniform("projection_mat", projection);
	
	_shader.setUniform("model_mat", model);
	_shader.setUniform("normal_mat", mat3(transpose(inverse(model))));
	_shader.setUniform("eye_world", eye_world);


    ground->draw(_shader, model);
    target1->draw(_shader, model);
    target2->draw(_shader, model);
    target3->draw(_shader, model);
    launcher->draw(_shader, model);
    
    // TODO: make sure bird velocity makes sense. (hint, hint)
    
    
    
    
    
    mat4 translation = translate(mat4(1.0), _position);
    bird->draw(_shader, translation * model);
}

void ExampleApp::drawText(const std::string text, float xPos, float yPos, GLfloat windowHeight, GLfloat windowWidth) {
	//float lh = 0;
	//fonsVertMetrics(fs, NULL, NULL, &lh);
	//double width = fonsTextBounds(fs, text.c_str(), NULL, NULL) + 40;
	//double height = lh + 40;

	_textShader.use();
	_textShader.setUniform("projection_mat", glm::ortho(0.f, windowWidth, windowHeight, 0.f, -1.f, 1.f));
	_textShader.setUniform("view_mat", glm::mat4(1.0));
	_textShader.setUniform("model_mat", glm::mat4(1.0));
	_textShader.setUniform("lambertian_texture", 0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fonsDrawText(fs, xPos, yPos, text.c_str(), NULL);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	_shader.use();

}

void ExampleApp::reloadShaders()
{
	_shader.compileShader("texture.vert", GLSLShader::VERTEX);
	_shader.compileShader("texture.frag", GLSLShader::FRAGMENT);
	_shader.link();
	_shader.use();
}

void ExampleApp::initializeText() {
	int fontNormal = FONS_INVALID;
	fs = nullptr;

	fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
	if (fs == NULL) {
		assert(false);//Could not create stash
	}

	fontNormal = fonsAddFont(fs, "sans", "DroidSansMono.ttf");
	if (fontNormal == FONS_INVALID) {
		assert(false);// Could not add font normal.
	}

	unsigned int black = glfonsRGBA(0, 0, 0, 255);

	fonsClearState(fs);
	fonsSetSize(fs, 20);
	fonsSetFont(fs, fontNormal);
	fonsSetColor(fs, black);
	fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

	_textShader.compileShader("textRendering.vert", GLSLShader::VERTEX);
	_textShader.compileShader("textRendering.frag", GLSLShader::FRAGMENT);
	_textShader.link();
}
