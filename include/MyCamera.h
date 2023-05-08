#pragma once
#include "Common/CameraController.h"

using namespace Demo;

class MyCamera :
    public CameraController
{
public:
    MyCamera(GraphicsSystem* graphicsSystem);

    void mouseMoved(const SDL_Event& arg) override;
};

