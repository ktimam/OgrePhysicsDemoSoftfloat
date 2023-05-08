
#ifndef _Demo_MyGameState_H_
#define _Demo_MyGameState_H_

#include "OgrePrerequisites.h"
#include "Common/TutorialGameState.h"

///btBulletDynamicsCommon.h is the main Bullet include file, contains most common include files.
#include "btBulletDynamicsCommon.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

namespace Demo
{
    class LogicSystem;
    struct GameEntity;
    struct MovableObjectDefinition;

    class LogicGameState : public GameState
    {
    protected:

        //keep the collision shapes, for deletion/cleanup
        btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
        btBroadphaseInterface* m_broadphase;
        btCollisionDispatcher* m_dispatcher;
        btConstraintSolver* m_solver;
        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btDiscreteDynamicsWorld* m_dynamicsWorld;

        LogicSystem         *mLogicSystem;

    public:
        LogicGameState();
        ~LogicGameState();

        void _notifyLogicSystem( LogicSystem *logicSystem )     { mLogicSystem = logicSystem; }

        virtual void createScene01(void);
        virtual void update( float timeSinceLast );
        btBoxShape* createBoxShape(const btVector3& halfExtents);
        void deleteRigidBody(btRigidBody* body);
        btRigidBody* createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape, const btVector4& color);
    };
}

#endif
