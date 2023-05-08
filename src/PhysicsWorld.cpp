#include "PhysicsWorld.h"

#include "Utils/b3ResourcePath.h"
#include <Common/GameEntityManager.h>
#include "Common/LogicSystem.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh.h"
#include "OgreMovableObject.h"
#include "OgreItem.h"
#include "OgreSubItem.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"
#include <BulletSoftBody/btSoftBody.h>
#include <Bullet3Common/b3Logging.h>
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

#include <fstream>
#include <sstream>

#include <Windows.h>

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{
	exitPhysics();
}

void PhysicsWorld::createScene01(void)
{
	LogicGameState::createScene01();
	initPhysics();
}

void PhysicsWorld::update(float timeSinceLast)
{
	LogicGameState::update(timeSinceLast);
	syncPhysicsToGraphics(m_dynamicsWorld);
}

void PhysicsWorld::initPhysics()
{
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
	//	m_dynamicsWorld->getSolverInfo().m_singleAxisRollingFrictionThreshold = 0.f;//faster but lower quality
	m_dynamicsWorld->setGravity(btVector3(0, 0, -10));

	{
		///create a few basic rigid bodies
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(10.), btScalar(5.), btScalar(25.)));

		m_collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, -28));
		groundTransform.setRotation(btQuaternion(btVector3(0, 1, 0), SIMD_PI * (btScalar)0.03));
		//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != (btScalar)0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setFriction((btScalar).5);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		///create a few basic rigid bodies
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(100.), btScalar(100.), btScalar(50.)));

		m_collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, -54));
		//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != (btScalar)0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setFriction((btScalar).1);
		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
#define NUM_SHAPES 10
		btCollisionShape* colShapes[NUM_SHAPES] = {
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
			new btSphereShape(btScalar(0.5)),
		};
		for (int i = 0; i < NUM_SHAPES; i++)
			m_collisionShapes.push_back(colShapes[i]);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static

		float start_x = START_POS_X - ARRAY_SIZE_X / 2;
		float start_y = START_POS_Y;
		float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

		{
			int shapeIndex = 0;
			for (int k = 0; k < ARRAY_SIZE_Y; k++)
			{
				for (int i = 0; i < ARRAY_SIZE_X; i++)
				{
					for (int j = 0; j < ARRAY_SIZE_Z; j++)
					{
						startTransform.setOrigin((btScalar)SCALING * btVector3(
							btScalar(2.0 * i + start_x),
							btScalar(2.0 * j + start_z),
							btScalar(20 + 2.0 * k + start_y)));

						shapeIndex++;
						btCollisionShape* colShape = colShapes[shapeIndex % NUM_SHAPES];
						bool isDynamic = (mass != (btScalar)0.f);
						btVector3 localInertia(0, 0, 0);

						if (isDynamic)
							colShape->calculateLocalInertia(mass, localInertia);

						//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
						btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
						btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
						btRigidBody* body = new btRigidBody(rbInfo);
						body->setFriction((btScalar)1.f);
						body->setRollingFriction((btScalar).1);
						body->setSpinningFriction((btScalar)0.1);
						body->setAnisotropicFriction(colShape->getAnisotropicRollingFrictionDirection(), btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);

						m_dynamicsWorld->addRigidBody(body);
					}
				}
			}
		}
	}

	autogenerateGraphicsObjects(m_dynamicsWorld);

	if (0)
	{
		btSerializer* s = new btDefaultSerializer;
		m_dynamicsWorld->serialize(s);
		char resourcePath[1024];
		if (b3ResourcePath::findResourcePath("slope.bullet", resourcePath, 1024, 0))
		{
			FILE* f = fopen(resourcePath, "wb");
			fwrite(s->getBufferPointer(), s->getCurrentBufferSize(), 1, f);
			fclose(f);
		}
	}
}

void PhysicsWorld::syncPhysicsToGraphics(const btDiscreteDynamicsWorld* rbWorld)
{
	static int count = 0;
	count++;
	std::ostringstream stringCount;
	stringCount << "====================== Round = " << count << " ================ " << " \n";
	OutputDebugString(stringCount.str().c_str());
	int numCollisionObjects = rbWorld->getNumCollisionObjects();
	{
		B3_PROFILE("write all InstanceTransformToCPU");
		for (int i = 0; i < numCollisionObjects; i++)
		{
			//B3_PROFILE("writeSingleInstanceTransformToCPU");
			btCollisionObject* colObj = rbWorld->getCollisionObjectArray()[i];
			btCollisionShape* collisionShape = colObj->getCollisionShape();
			const btVector3 pos = colObj->getWorldTransform().getOrigin();
			btQuaternion orn = colObj->getWorldTransform().getRotation();
			int index = colObj->getUserIndex();
			if (index >= 0)
			{
				btTransform* transform = &colObj->getWorldTransform();
				btVector3* btorigin = &transform->getOrigin();
				Ogre::Vector3 origin((float)btorigin->getX(), (float)btorigin->getY(),
					(float)btorigin->getZ());
				Ogre::Quaternion rotation((float)transform->getRotation().getW(),
					(float)transform->getRotation().getX(), (float)transform->getRotation().getY(),
					(float)transform->getRotation().getZ());

				//log += std::format("X = %f, Y= %f, Z = %f \n", btorigin->getX(), btorigin->getY(), btorigin->getZ());
				std::ostringstream stringStream;
				double x = (float)btorigin->getX();
				double y = (float)btorigin->getY();
				double z = (float)btorigin->getZ();
				stringStream << "X = " << x << " Y = " << y << " Z = " << z << " \n";
				log += stringStream.str();
				OutputDebugString(stringStream.str().c_str());

				const size_t currIdx = mLogicSystem->getCurrentTransformIdx();
				GameEntity* entity = m_GraphicsEntitiesMap[index];
				entity->mTransform[currIdx]->vPos = origin;
				entity->mTransform[currIdx]->qRot = rotation;
			}
		}
	}
	{
		B3_PROFILE("writeTransforms");
	}
}
void PhysicsWorld::exitPhysics()
{
	freopen("CONOUT$", "w", stdout);
	std::ofstream o("simulation.log");
	o << log << std::endl;

	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	int i;
	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < m_collisionShapes.size(); j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;

	delete m_solver;

	delete m_broadphase;

	delete m_dispatcher;

	delete m_collisionConfiguration;
}

struct MyConvertPointerSizeT
{
	union {
		const void* m_ptr;
		size_t m_int;
	};
};
bool shapePointerCompareFunc(const btCollisionObject* colA, const btCollisionObject* colB)
{
	MyConvertPointerSizeT a, b;
	a.m_ptr = colA->getCollisionShape();
	b.m_ptr = colB->getCollisionShape();
	return (a.m_int < b.m_int);
}
void PhysicsWorld::autogenerateGraphicsObjects(btDiscreteDynamicsWorld* rbWorld)
{
	Ogre::v1::MeshPtr v1Mesh;
	Ogre::MeshPtr v2Mesh;

	//Load the v1 mesh. Notice the v1 namespace
	//Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
	//bit would prohibit us from reading the data for importing.
	v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
		"sphere.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);

	bool halfPosition = true;
	bool halfUVs = true;
	bool useQtangents = true;

	//Create a v2 mesh to import to, with a different name (arbitrary).
	//Import the v1 mesh to v2
	v2Mesh = Ogre::MeshManager::getSingleton().createByImportingV1(
		"sphere.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		v1Mesh.get(), halfPosition, halfUVs, useQtangents);

	//sort the collision objects based on collision shape, the gfx library requires instances that re-use a shape to be added after eachother

	btAlignedObjectArray<btCollisionObject*> sortedObjects;
	sortedObjects.reserve(rbWorld->getNumCollisionObjects());
	for (int i = 0; i < rbWorld->getNumCollisionObjects(); i++)
	{
		btCollisionObject* colObj = rbWorld->getCollisionObjectArray()[i];
		sortedObjects.push_back(colObj);
	}
	sortedObjects.quickSort(shapePointerCompareFunc);
	for (int i = 0; i < sortedObjects.size(); i++)
	{
		btCollisionObject* colObj = sortedObjects[i];
		//btRigidBody* body = btRigidBody::upcast(colObj);
		//does this also work for btMultiBody/btMultiBodyLinkCollider?
		btSoftBody* sb = btSoftBody::upcast(colObj);
		if (sb)
		{
			colObj->getCollisionShape()->setUserPointer(sb);
		}
		colObj->setUserIndex(i);
		createCollisionShapeGraphicsObject(colObj);
		int colorIndex = colObj->getBroadphaseHandle()->getUid() & 3;

		/*btVector4 color;
		color = sColors[colorIndex];
		if (colObj->getCollisionShape()->getShapeType() == STATIC_PLANE_PROXYTYPE)
		{
			color.setValue(1, 1, 1, 1);
		}
		createCollisionObjectGraphicsObject(colObj, color);*/

	}
}

void PhysicsWorld::createCollisionShapeGraphicsObject(btCollisionObject* body)
{
	btTransform* transform = &body->getWorldTransform();
	int indx = body->getUserIndex();
	//const Ogre::Vector3 origin(-5.0f, 0.0f, 0.0f);
	btVector3* btorigin = &transform->getOrigin();
	Ogre::Vector3 origin((float)btorigin->getX(), (float)btorigin->getY(),
		(float)btorigin->getZ());
	Ogre::Quaternion rotation((float)transform->getRotation().getW(),
		(float)transform->getRotation().getX(), (float)transform->getRotation().getY(),
		(float)transform->getRotation().getZ());
	GameEntityManager* geMgr = mLogicSystem->getGameEntityManager();

	MovableObjectDefinition* entitydefinition = new MovableObjectDefinition();
	entitydefinition->meshName = "sphere.mesh Imported";
	entitydefinition->resourceGroup = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
	entitydefinition->moType = MoTypeItem;
	//entitydefinition->submeshMaterials.push_back("Examples/TransparentBlue50");

	GameEntity*  entity = geMgr->addGameEntity(Ogre::SCENE_DYNAMIC, entitydefinition, origin,
		rotation,
		Ogre::Vector3::UNIT_SCALE * 0.002);

	m_GraphicsEntitiesMap[indx] = entity;
}
