
#ifndef _Demo_CameraController_H_
#define _Demo_CameraController_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class CameraController
    {
    protected:
        bool                mUseSceneNode;

        bool                mSpeedMofifier;
        bool                mWASD[4];
        bool                mSlideUpDown[2];
        float               mCameraYaw;
        float               mCameraPitch;

        GraphicsSystem      *mGraphicsSystem;

        public: float       mCameraBaseSpeed;
        public: float       mCameraSpeedBoost;

    public:
        CameraController( GraphicsSystem *graphicsSystem, bool useSceneNode=false );

        virtual void update( float timeSinceLast );

        /// Returns true if we've handled the event
        virtual bool keyPressed( const SDL_KeyboardEvent &arg );
        /// Returns true if we've handled the event
        virtual bool keyReleased( const SDL_KeyboardEvent &arg );

        virtual void mouseMoved( const SDL_Event &arg );
    };
}

#endif
