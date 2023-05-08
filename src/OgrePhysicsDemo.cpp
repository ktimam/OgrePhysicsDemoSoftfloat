
#include "Common/GraphicsSystem.h"
#include "Common/LogicSystem.h"
#include "Common/GameEntityManager.h"
#include "GraphicsGameState.h"

#include "OgreCamera.h"
#include "Common/CameraController.h"
#include "PhysicsWorld.h"
// #include "LogicGameState.h"

#include "Common/Threading/YieldTimer.h"

#include "OgreWindow.h"
#include "OgreTimer.h"

#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

#include <iostream>
#include "MyCamera.h"

using namespace Demo;

extern const double cFrametime;
const double cFrametime = 1.0 / 25.0;

extern bool gFakeFrameskip;
bool gFakeFrameskip = false;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT __stdcall WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT);
#endif

int main();

unsigned long renderThread( Ogre::ThreadHandle *threadHandle );
unsigned long logicThread( Ogre::ThreadHandle *threadHandle );
THREAD_DECLARE( renderThread );
THREAD_DECLARE( logicThread );

struct ThreadData
{
    GraphicsSystem  *graphicsSystem;
    LogicSystem     *logicSystem;
    Ogre::Barrier   *barrier;
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main()
#endif
{
    GraphicsGameState graphicsGameState("Ogre Physics Demo");
    GraphicsSystem graphicsSystem( &graphicsGameState );
    MyCamera* cameracontroller = new MyCamera(&graphicsSystem);
    graphicsGameState.SetCameraController(cameracontroller);
    
    char* data = (char*) malloc(135);
    std::cout << "malloc test done\n";

    // LogicGameState logicGameState;
    PhysicsWorld logicGameState;
    LogicSystem logicSystem( &logicGameState );
    Ogre::Barrier barrier( 2 );

    graphicsGameState._notifyGraphicsSystem( &graphicsSystem );
    logicGameState._notifyLogicSystem( &logicSystem );

    graphicsSystem._notifyLogicSystem( &logicSystem );
    logicSystem._notifyGraphicsSystem( &graphicsSystem );

    GameEntityManager gameEntityManager( &graphicsSystem, &logicSystem );

    ThreadData threadData;
    threadData.graphicsSystem   = &graphicsSystem;
    threadData.logicSystem      = &logicSystem;
    threadData.barrier          = &barrier;

    Ogre::ThreadHandlePtr threadHandles[2];
    threadHandles[0] = Ogre::Threads::CreateThread( THREAD_GET( renderThread ), 0, &threadData );
    threadHandles[1] = Ogre::Threads::CreateThread( THREAD_GET( logicThread ), 1, &threadData );

    Ogre::Threads::WaitForThreads( 2, threadHandles );

    std::cout << "Hello \n";
    return 0;
}


//---------------------------------------------------------------------
unsigned long renderThreadApp( Ogre::ThreadHandle *threadHandle )
{
    ThreadData *threadData = reinterpret_cast<ThreadData*>( threadHandle->getUserParam() );
    GraphicsSystem *graphicsSystem  = threadData->graphicsSystem;
    Ogre::Barrier *barrier          = threadData->barrier;

    graphicsSystem->setAlwaysAskForConfig(false);
    graphicsSystem->initialize( "Ogre Physics Demo" );
    Ogre::Camera* camera = graphicsSystem->getCamera();
    /*camera->setPosition(Ogre::Vector3(20, 50, 0));
    camera->setOrientation(Ogre::Quaternion(-0.14, -0.17, -0.73, -0.64));*/
    camera->setPosition(Ogre::Vector3(20, 40, 5));
    camera->setOrientation(Ogre::Quaternion(-0.14, -0.15, -0.7, -0.7));

    barrier->sync();

    if( graphicsSystem->getQuit() )
    {
        graphicsSystem->deinitialize();
        return 0; //User cancelled config
    }

    graphicsSystem->createScene01();
    barrier->sync();

    graphicsSystem->createScene02();
    barrier->sync();

    Ogre::Window *renderWindow = graphicsSystem->getRenderWindow();

    Ogre::Timer timer;

    Ogre::uint64 startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem->getQuit() )
    {
        graphicsSystem->beginFrameParallel();
        graphicsSystem->update( timeSinceLast );
        graphicsSystem->finishFrameParallel();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        if( gFakeFrameskip )
            Ogre::Threads::Sleep( 120 );

        Ogre::uint64 endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    barrier->sync();

    graphicsSystem->destroyScene();
    barrier->sync();

    graphicsSystem->deinitialize();
    barrier->sync();

    return 0;
}
unsigned long renderThread( Ogre::ThreadHandle *threadHandle )
{
    unsigned long retVal = -1;

    try
    {
        retVal = renderThreadApp( threadHandle );
    }
    catch( Ogre::Exception& e )
    {
   #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!",
                     MB_OK | MB_ICONERROR | MB_TASKMODAL );
   #else
        std::cerr << "An exception has occured: " <<
                     e.getFullDescription().c_str() << std::endl;
   #endif

        abort();
    }

    return retVal;
}
//---------------------------------------------------------------------
unsigned long logicThread( Ogre::ThreadHandle *threadHandle )
{
    ThreadData *threadData = reinterpret_cast<ThreadData*>( threadHandle->getUserParam() );
    GraphicsSystem *graphicsSystem  = threadData->graphicsSystem;
    LogicSystem *logicSystem        = threadData->logicSystem;
    Ogre::Barrier *barrier          = threadData->barrier;

    logicSystem->initialize();
    barrier->sync();

    if( graphicsSystem->getQuit() )
    {
        logicSystem->deinitialize();
        return 0; //Render thread cancelled early
    }

    logicSystem->createScene01();
    barrier->sync();

    logicSystem->createScene02();
    barrier->sync();

    Ogre::Window *renderWindow = graphicsSystem->getRenderWindow();

    Ogre::Timer timer;
    YieldTimer yieldTimer( &timer );

    Ogre::uint64 startTime = timer.getMicroseconds();

    int count = 0; 
    while( !graphicsSystem->getQuit() && count++ <= 250)
    {
        logicSystem->beginFrameParallel();
        logicSystem->update( static_cast<float>( cFrametime ) );
        logicSystem->finishFrameParallel();

        logicSystem->finishFrame();

        if( gFakeSlowmo )
            Ogre::Threads::Sleep( 120 );

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        //YieldTimer will wait until the current time is greater than startTime + cFrametime
        startTime = yieldTimer.yield( cFrametime, startTime );
    }

    barrier->sync();

    logicSystem->destroyScene();
    barrier->sync();

    logicSystem->deinitialize();
    barrier->sync();

    return 0;
}
