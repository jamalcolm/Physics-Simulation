#include "MyPhysicsEngine.h"

using namespace PhysicsEngine;

void MySimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count){

	//you can read the trigger information here
	for (PxU32 i = 0; i < count; i++)
	{
		//filter out contact with the planes
		if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
		{
			//check if eNOTIFY_TOUCH_FOUND trigger
			if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if (strcmp(pairs[i].otherActor->getName(), "ball") == 0)
				{
					//Ball has collided with the cloth 
					cout << "Ball has collided with cloth" << endl;
					cloth = true;
				}
				
				trigger = true;
				
			}
			//check if eNOTIFY_TOUCH_LOST trigger
			if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				//cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
				trigger = false;
			}
		}
	}
}

void MySimulationEventCallback::onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs)
{
	cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

	//check all pairs
	for (PxU32 i = 0; i < nbPairs; i++)
	{
		//check eNOTIFY_TOUCH_FOUND
		if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
		}
		//check eNOTIFY_TOUCH_LOST
		if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
		}
	}
}

//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		//trigger onContact callback for this pair of objects
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
	}

	return PxFilterFlags();
};



//Scene
void MyScene::SetVisualisation()
{
	//Set visualisation parameters
	px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 0.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_ANG_VELOCITY, 10.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 10.0f);

	//cloth visualisation
	px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
	px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);
}

//Custom initialisation function
void MyScene::CustomInit()
{
	boxes.clear();
	SetVisualisation();

	//Coefficient of friction of grass (according to https://www.engineeringtoolbox.com/friction-coefficients-d_778.html)
	GetMaterial()->setDynamicFriction(.35f);

	///Initialise and set the customised event callback
	my_callback = new MySimulationEventCallback();
	my_callback->myScene = this;
	px_scene->setSimulationEventCallback(my_callback);

	//Create plane
	plane = new Plane();
	plane->Color(PxVec3(50.f / 255.f, 110.f / 255.f, 10.f / 255.f));
	plane->Material(GetMaterial());
	Add(plane);

	//Create halfwayline
	halfwayLine = new Box(PxTransform(PxVec3(-75.f, 0.5f, 0.f)), PxVec3(5.f, 1.f, 1000.f));
	halfwayLine->SetKinematic(true);
	halfwayLine->Color(PxVec3(2.f, 2.f, 2.f));
	Add(halfwayLine);

	//Create catapult
	myCatapult = new Catapult(
		PxVec3(0.f, 1.f, 0.f)

	);
	myCatapult->AddToScene(this);

	//Create a rugby ball
	NewBall();
	
	//Create goal post and cloth
	myGoalPost = new GoalPost(PxTransform(PxVec3(-50.f, 0.f, 0.f)), PxVec3(0.5f, 0.5f, 0.5f), 10000000.f);
	Add(myGoalPost);

	cloth = new Cloth(PxTransform(PxVec3(-50.f, 59.f, 12.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), PxVec2(25.f, 28.f), 4, 4);
	cloth->Color(color_palette[2]);
	Add(cloth);

	//Create cloth trigger
	//Made invisible in the renderer, it triggers when collided with the ball
	myGoalPostTrigger = new Box(PxTransform(PxVec3(-50.f, 45.f, 0.f)), PxVec3(1.f, 20.f, 15.f), 1.0f);
	myGoalPostTrigger->Color(color_palette[0]);
	myGoalPostTrigger->SetTrigger(true);
	myGoalPostTrigger->SetKinematic(true);
	myGoalPostTrigger->Name("goalPost");
	Add(myGoalPostTrigger);

	//Different levels
	if (level == 0)
	{
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 20, 8.f);
	}

	else if (level == 1)
	{
		myWindmill = new Windmill(PxVec3(-20.0f, 0.0f, 0.0f));
		myWindmill->AddToScene(this);
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 25, 7.f);
	}

	else if (level == 2)
	{
		myGoalPost = new GoalPost(PxTransform(PxVec3(-50.f, 0.f, 0.f)), PxVec3(0.5f, 0.5f, 0.5f), 10000000.f);
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 26, 7.f);
	}

	else if (level == 3)
	{
		//1275 boxes
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 50, 10.f);
	}

	else if (level == 4)
	{
		//2850 boxes
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 75, 10.f);
	}

	else if (level == 5)
	{
		//5050 boxes
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 100, 10.f);
	}

	else if (level == 6)
	{
		//125250 boxes
		CreatePyramid(PxVec3(-100.f, 0.0f, 0.f), 200, 10.f);
	}
}

//Rotate catatpult left by setting global pose
void MyScene::RotateLeft()
{
	angle += 0.1f;
	((PxRigidActor*)myCatapult->base->Get())->setGlobalPose(PxTransform(PxVec3(0.f, 1.f, 0.f), PxQuat(angle, PxVec3(0.f, 1.f, 0.f))));
}

//Rotate catapuult right by setting global pose
void MyScene::RotateRight()
{
	angle -= 0.1f;
	((PxRigidActor*)myCatapult->base->Get())->setGlobalPose(PxTransform(PxVec3(0.f, 1.f, 0.f), PxQuat(angle, PxVec3(0.f, 1.f, 0.f))));
}

//Return a random float between 0 and 1
PxReal RandomFloat()
{
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r;
}

//Update
void MyScene::CustomUpdate()
{
	//Decrease drive velocity over time
	myCatapult->revolute->DriveVelocity(-drivePower);
	if (drivePower > 0)
	{
		drivePower -= 0.05f;
		if (drivePower < 0.f)
			drivePower = 0.f;
	}

	//Get percentage of boxes under height
	int noOfBoxes = 0;
	for (int i = 0; i < boxes.size(); i++)
	{
		if (BoxCondition(level, boxes[i]))
		{
			boxes[i]->Color(PxVec3(1.f, 0.f, 0.f));
			noOfBoxes++;
		}
	}
	boxPercent = (float)noOfBoxes / (float)boxes.size();

	//Change cloth colour when collided
	if (my_callback->cloth)
	{
		cloth->Color(color_palette[3]);
		((PxCloth*)cloth->Get())->setExternalAcceleration(PxVec3(0.0f, 0.0f, 11.0f));
	}
};

//Return true if box under a certain height
bool MyScene::BoxCondition(int _level, Box* _box)
{
	PxTransform pose = ((PxRigidBody*)_box->Get())->getGlobalPose();
	return (pose.p.y <= 10.f);
}

//Powerup every time the button is pressed
void MyScene::PowerUp()
{
	drivePower += 1.f;
}

//Set break force of arm/ball joint to 0 so it breaks away
void MyScene::RemoveJoint()
{
	joints[balls]->joint->setBreakForce(0.f, 0.f);
	hasBall = false;
}

//Create new ball and assign joint to it
void MyScene::NewBall()
{
	if (!hasBall)
	{
		RugbyBall* nextBall = CreateBall();
		nextBall->Name("ball");
		rugbyBall.push_back(nextBall);
		balls++;

		//Material based on leather. Modern rugby balls are made of rubber, but they probably used leather in medieval times
		//static: 0.61, dynamic: 0.52 http://www.engineershandbook.com/Tables/frictioncoefficients.htm
		PxMaterial* mMaterial;
		mMaterial = (GetPhysics())->createMaterial(0.61f, 0.52f, 0.5f);
		rugbyBall[balls]->Material(mMaterial);

		RevoluteJoint* rJoint;
		rJoint = new RevoluteJoint(
			myCatapult->arm,
			PxTransform(PxVec3(7.0f, 1.0f, 0.0f)),
			rugbyBall[balls],
			PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));
		joints.push_back(rJoint);
		hasBall = true;
	}

}

//Code for creating new ball
RugbyBall* MyScene::CreateBall()
{
	RugbyBall* myRugbyBall;
	myRugbyBall = new RugbyBall(PxTransform(PxVec3(7.6f, 7.0f, 0.0f)), 40.f, modelRings, modelSeps);
	myRugbyBall->Color(PxVec3(RandomFloat(), RandomFloat(), RandomFloat()));
	selected_actor = (PxRigidDynamic*)(myRugbyBall->Get());
	Add(myRugbyBall);
	hasBall = true;
	noOfBalls++;
	return myRugbyBall;
	
}

//Create pyramid of boxes
void MyScene::CreatePyramid(PxVec3 position, int size, float boxsize)
{
	int x = size;
	int y = 0;
	int s = size;

	//Care taken so that friction is not so low that pyramid falls on its own, but not too high that it won't move
	PxMaterial* mMaterial; 
	mMaterial = (GetPhysics())->createMaterial(0.4f, 0.5f, 0.8f);

	for (int i = 0; i < ((size)*(size + 1) / 2); i++)
	{
		Box* newBox = new Box(PxTransform(PxVec3(0.f, y + 0.5f, 0.f + (float)(x - size / 2.f + y / 2.f- 0.5f))*boxsize + position), PxVec3(boxsize / 2.f, boxsize / 2.f, boxsize / 2.f), 0.1f);
		newBox->Material(mMaterial);
		boxes.push_back(newBox);
		Add(newBox);

		x -= 1;
		if (x <= 0)
		{
			s--;
			y++;
			x = s;
		}
	}
}

//Rugby ball model is calculated at runtime
void MyScene::SetRugbyBallModel()
{
	switch (modelIndex)
	{
	case 1:
		modelRings = 6;
		modelSeps = 3;
		break;
	case 2:
		modelRings = 8;
		modelSeps = 4;
		break;
	case 3:
		modelRings = 12;
		modelSeps = 12;
		break;
	case 4:
		modelRings = 16;
		modelSeps = 16;
		break;
	default:
		modelRings = 4;
		modelSeps = 2;
		break;
	}

	cout << "Set model rings to " << modelRings << ", model separations to " << modelSeps << endl;
	modelIndex = (modelIndex++) % 5;
}

//Go to next level
void MyScene::NextLevel()
{
	noOfBalls = 0;
	level++;
}

//Retrun current test case
string MyScene::GetCurrentTestCase()
{
	string testCase;
	switch (level)
	{
	case 0:
		testCase = "level1";
	case 1:
		testCase = "level2";
	case 2:
		testCase = "level3";
	case 3:
		testCase = "many_boxes_1";
	case 4:
		testCase = "many_boxes_2";
	case 5:
		testCase = "many_boxes_3";
	case 6:
		testCase = "too_many_boxes";
	default:
		testCase = "default";
		break;
	}
	return testCase;
}