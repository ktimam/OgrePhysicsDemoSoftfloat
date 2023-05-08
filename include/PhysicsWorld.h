#pragma once
#include "LogicGameState.h"
#include "ogrestd/map.h"

///create 125 (5x5x5) dynamic object
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (ARRAY_SIZE_X * ARRAY_SIZE_Y * ARRAY_SIZE_Z + 1024)

///scaling of the objects (0.1 = 20 centimeter boxes )
#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3

using namespace Ogre;
using namespace Demo;

class PhysicsWorld :
    public LogicGameState
{
private:
    map<uint, GameEntity*>::type m_GraphicsEntitiesMap;
    String log;
public:
    PhysicsWorld();
    ~PhysicsWorld();
    
    virtual void createScene01(void) override;
    virtual void update(float timeSinceLast) override;
    void initPhysics();
    void syncPhysicsToGraphics(const btDiscreteDynamicsWorld* rbWorld);
    void exitPhysics();
    void autogenerateGraphicsObjects(btDiscreteDynamicsWorld* rbWorld);
    void createCollisionShapeGraphicsObject(btCollisionObject* softBody);
};

