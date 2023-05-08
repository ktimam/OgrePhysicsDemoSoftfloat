#include "MyCamera.h"
#include "OgreCamera.h"
#include "Common/GraphicsSystem.h"

MyCamera::MyCamera(GraphicsSystem* graphicsSystem):
	CameraController(graphicsSystem)
{
	//mCameraYaw   = 45;
	//mCameraPitch = 45;
}

void MyCamera::mouseMoved(const SDL_Event& arg)
{
	//CameraController::mouseMoved(arg);
	Ogre::Camera* camera = mGraphicsSystem->getCamera();
}
