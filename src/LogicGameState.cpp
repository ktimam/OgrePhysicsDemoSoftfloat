
#include "LogicGameState.h"
#include "Common/LogicSystem.h"
#include "Common/GameEntityManager.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"

#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

#include <iostream>

using namespace Demo;

namespace Demo
{
    LogicGameState::LogicGameState() :
        mLogicSystem( 0 )
    {
        std::cout << "LogicGameState 2 \n";

        char* data = (char*) malloc(135);
        std::cout << "malloc test 2 done\n";

        ///collision configuration contains default setup for memory, collision setup
        m_collisionConfiguration = new btDefaultCollisionConfiguration();
        //m_collisionConfiguration->setConvexConvexMultipointIterations();

        ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
        m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

        m_broadphase = new btDbvtBroadphase();

        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
        m_solver = sol;

        m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

        m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
    }
    //-----------------------------------------------------------------------------------
    LogicGameState::~LogicGameState()
    {
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::createScene01(void)
    {
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::update( float timeSinceLast )
    {
        if (m_dynamicsWorld)
        {
            m_dynamicsWorld->stepSimulation((btScalar)1.f / (btScalar)60.f, 10);
        }

        GameState::update( timeSinceLast );
    }


    btBoxShape* LogicGameState::createBoxShape(const btVector3& halfExtents)
    {
        btBoxShape* box = new btBoxShape(halfExtents);
        return box;
    }

    void LogicGameState::deleteRigidBody(btRigidBody* body)
    {
        m_dynamicsWorld->removeRigidBody(body);
        btMotionState* ms = body->getMotionState();
        delete body;
        delete ms;
    }

    btRigidBody* LogicGameState::createRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape, const btVector4& color = btVector4((btScalar)1, (btScalar)0, (btScalar)0, (btScalar)1))
    {
        btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            shape->calculateLocalInertia((btScalar)mass, localInertia);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

        btRigidBody::btRigidBodyConstructionInfo cInfo((btScalar)mass, myMotionState, shape, localInertia);

        btRigidBody* body = new btRigidBody(cInfo);
        //body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
        btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
        body->setWorldTransform(startTransform);
#endif  //

        body->setUserIndex(-1);
        m_dynamicsWorld->addRigidBody(body);
        return body;
    }

}
