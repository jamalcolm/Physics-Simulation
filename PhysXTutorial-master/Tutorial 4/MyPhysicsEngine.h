#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>
#include <math.h>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = { PxVec3(46.f / 255.f,9.f / 255.f,39.f / 255.f),PxVec3(217.f / 255.f,0.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,45.f / 255.f,0.f / 255.f),PxVec3(255.f / 255.f,140.f / 255.f,54.f / 255.f),PxVec3(4.f / 255.f,117.f / 255.f,111.f / 255.f) };

	struct FilterGroup
	{
		enum Enum
		{
			ACTOR0 = (1 << 0),
			ACTOR1 = (1 << 1),
			ACTOR2 = (1 << 2)
			//add more if you need
		};
	};

	//Rugby ball
	class RugbyBall : public ConvexMesh
	{

	public:

		RugbyBall(PxTransform pose = PxTransform(PxIdentity), PxReal density = 1.f, int rings=4, int separations=2) :
			ConvexMesh(GetMesh(rings, separations), pose, density)
		{
			
		}

		//Algorithm for generating the mesh 
		vector<PxVec3> GetMesh(int _r, int _s)
		{
			//r=rings (no. of sides)
			//s=separations (how many rings between start and end?)
			int r = _r;
			int s = _s;

			vector<PxVec3> rVerts;
			rVerts.push_back(PxVec3(-2, 0, 0));

			float x, y, z, rj, rr;
			for (float i = 0; i < s; i++)
			{
				x = -2 + (i + 1) * 4 / (s + 1);

				float nx = x;
				x = pow(abs(x/2), 0.5f);
				x = copysign(x, nx)*2.f;

				for (int j = 0; j < r; j++)
				{
					rj = (j * PxPi * 2 / r);
					rr = (sin(((nx/4.f)+0.5f) * PxPi));
					
					float rr2 = rr;
					rr = pow(abs(rr), 1.0f);
					rr = copysign(rr,rr2);
					y = sin(rj)*rr;
					z = cos(rj)*rr;
					rVerts.push_back(PxVec3(x, y, z));
				}
			}

			rVerts.push_back(PxVec3(2, 0, 0));

			return vector<PxVec3>(begin(rVerts), end(rVerts));
		}
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;
		Scene* myScene;

		MySimulationEventCallback() : trigger(false) {}
		bool cloth = false;

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count);

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs);


		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//Catapuly class
	class Catapult
	{
		

	public:

		CatapultBase* base;
		CatapultArm* arm;
		RevoluteJoint* revolute;
		DistanceJoint* distance;
		Catapult(const PxVec3& dimensions = PxVec3(0.f, 0.f, 0.f))
		{
			base = new CatapultBase(PxTransform(PxVec3(dimensions)));
			arm = new CatapultArm(PxTransform(PxVec3(dimensions.x+2.f, dimensions.y+10.f, dimensions.z)));
			revolute = new RevoluteJoint
			(
				base,
				PxTransform(PxVec3(2.0f,10.0f,0.0f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))),
				arm,	
				PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f)))
			);
			
		}

		//Add catapult ot scene
		void AddToScene(Scene* scene)
		{
			scene->Add(base);
			scene->Add(arm);
		}

		//Remove catapult
		~Catapult()
		{
			delete revolute;
		}
	};

	//Windmill class
	class Windmill
	{

	public:
		WindmillBase* base;
		WindmillSpokes* spokes;
		RevoluteJoint* revolute;

		Windmill(const PxVec3& dimensions = PxVec3(0.f, 0.f, 0.f))
		{
			base = new WindmillBase(PxTransform(PxVec3(dimensions)), PxVec3(0.5f,0.5f,0.5f), 1000000.f);
			spokes = new WindmillSpokes(PxTransform(PxVec3(dimensions) + PxVec3(9.0f, 23.0f, 0.0f)));

			revolute = new RevoluteJoint
			(
				base,
				PxTransform(PxVec3(8.0f, 20.0f, 0.0f), PxQuat(0.f, PxVec3(0.f, 1.f, 0.f))),
				spokes,
				PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(0.f, PxVec3(0.f, 1.f, 0.f)))
			);
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(base);
			scene->Add(spokes);
		}
	};
	
	///Custom scene class
	class MyScene : public Scene
	{
	public:
		Plane* plane;
		Box* box;
		MySimulationEventCallback* my_callback;
		Catapult* myCatapult;
		GoalPost* myGoalPost;
		Box* myGoalPostTrigger;
		Cloth* cloth;
		RevoluteJoint* goalJoint;
		vector<RugbyBall*> rugbyBall;
		vector <RevoluteJoint*> joints;
		vector<Box*> boxes;
		Windmill* myWindmill;
		RugbyBall* mySelectedActor;

		Box* halfwayLine;

		float drivePower = 0.0f;
		bool hasBall = false;
		int balls=-1;

		int modelRings = 4;
		int modelSeps = 2;
		int modelIndex = 0;

		int noOfBalls = 0;

		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene() {};

		///A custom scene class
		void SetVisualisation();

		//Custom scene initialisation
		virtual void CustomInit();		

		//Custom udpate function
		virtual void CustomUpdate();

		//YEET the rugby ball
		virtual void RemoveJoint();

		//Power up
		virtual void PowerUp();

		//New ball
		virtual void NewBall();

		//Create pyramid of boxes
		virtual void CreatePyramid(PxVec3 position, int size, float boxsize);

		//Win condition for level
		int level = 0;
		float boxPercent = 0.f;
		virtual bool BoxCondition(int _level, Box* _box);

		virtual void NextLevel();
		

		virtual void RotateLeft();
		virtual void RotateRight();
		float angle = 0.0f;

		virtual RugbyBall* CreateBall();

		void SetRugbyBallModel();

		string GetCurrentTestCase();

		/// An example use of key release handling
		void ExampleKeyReleaseHandler()
		{
			cerr << "I am realeased!" << endl;
		}

		/// An example use of key presse handling
		void ExampleKeyPressHandler()
		{
			cerr << "I am pressed!" << endl;
		}
	};
}
