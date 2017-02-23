#ifndef ACTOR_H_
#define ACTOR_H_

#include "GameConstants.h"
#include "GraphObject.h"
#include "Compiler.h"
class StudentWorld;

const int FOOD_SIZE_AT_START = 6000;
const int FOOD_FROM_DEAD_INSECT = 100;

const int GRID_SIZE = 64;

// make changes to this class as needed
// simplifies descriptions of points. as spec instructs, x and y are not member vars of graphobject's derived classes
class Coord {
public:
	Coord() : m_x(-1), m_y(-1) {} // this is dangerous if you use any coordinate that was not subsequently replaced.
	Coord(int col, int row) : m_x(col), m_y(row) {}
	int getCol() const { return m_x; }
	int getRow() const { return m_y; }

private:
	int m_x;
	int m_y;
};

class Actor : public GraphObject {
public:
	Actor(int imageID, int x, int y, StudentWorld* worldPtr, Direction dir, int depth = 2)
		: GraphObject(imageID, x, y, dir, depth), ptrToWorld(worldPtr), actorID(imageID), m_lastMove(0) {}
	virtual void doSomething() { move(); }
	virtual bool hasEnergyOrHealth() const { return true; } // override for those not food, pheromone, anthill, and animals
	virtual bool canBlockPath() const { return false; } // override for pebble
	virtual bool canStun() const { return false; } // override for water
	virtual bool canBeStunned() const { return false; } // override for animals
	virtual bool canBeEaten() const { return false; } // override for food, obviously
	virtual bool canBePoisoned() const { return false; } // override for insects, exc. anthill
	virtual bool canAttack() const { return false; } // override for some animals
	virtual bool canEat() const { return false; } // override for animals
	virtual bool canPoison() const { return false; } //override for poison
	virtual bool canFly() const { return false; } // override for adult grasshoppers
	virtual bool isStunned() const { return false; } // override for animals
	virtual bool isSleeping() const { return false; } // override for grasshoppers
	virtual bool isDead() const { return false; } // this is not right... need to pay attention with inheritance... ONLY SEES THIS!!!
	virtual bool die(); // if it can't die, returns false.
	virtual int getColonyNumber() const { return -1; } // if it can't be associated with a particular colony, return -1
	//virtual bool beEaten(int amount) { return false; } // intended for food
	//virtual void eat(int amount) {}
	virtual void addHealth(int amount) {}
	virtual int reduceHP(int amount) { return 0; } // couldn't reduce HP

	// is there any destruction in common?
	// may want something that gets ptrToWorld
	virtual ~Actor() {};
	Direction getRandomDirection()
	{
		int random = randInt(0, 3);
		switch (random)
		{
		case 0: return up;
		case 1: return right;
		case 2: return down;
		default: return left;
		}
	}
	int getActorID() const { return actorID; } // remove this if possible
	bool hasDoneSomething(int currentMove)  const { return currentMove == m_lastMove; };
	void updateLastMove(int currentMove) { m_lastMove = currentMove; } // does this need to be virtual? needs to be called to make sure life is OK.
	int getCurrentMove() const;
	StudentWorld* getPtrToWorld() const { return ptrToWorld; }
	/*void numberTimesCalled(); for testing that calls are made properly
	void incrementTimesCalled() { m_timesCalled++; }*/

private:
	StudentWorld* ptrToWorld;
	int actorID;
	int m_lastMove;
	//virtual bool isDead() const = 0;
	virtual void move() = 0;
	//int m_timesCalled; // delete this later.
};

// should probably have an ant-related class because of how both require compiler

class EnergeticActor : public Actor{
public:

	// should I have fn's here that reduce health/energy?
	EnergeticActor(int imageID, int x, int y, StudentWorld* worldPtr, int HP, int maxHealth = -1, Direction dir = right, int depth = 2)
		: Actor(imageID, x, y, worldPtr, dir, depth), m_healthPoints(HP), m_maxLimit(maxHealth) {} // added back m_maxHealth member which was set to default to -1
	// do I need to define my own copy constructor?
	// requires that the studentWorld has verified the amount of health that can be added in light of limits per turn and per square
	virtual void addHealth(int amount);
	int getHealth() const { return m_healthPoints; }
	virtual void decrementHP() { m_healthPoints--; } // should I add a check here for stuff dying? I need to add it somewhere
	virtual int reduceHP(int amount); // returns amount by which HP was reduced
	bool isDead() const { return m_healthPoints <= 0; }
	virtual bool die();
	int getMaxHealth() { return m_maxLimit; } // be aware that this will return -1 in many circumstances
	virtual ~EnergeticActor() {} // uh... what should this do?

private:
	int m_healthPoints;
	int m_maxLimit;
};

/*class ImmobileActor : public EnergeticActor {
public:
	ImmobileActor (int imageID, int x, int y, StudentWorld* worldPtr, int health, int maxLimit = -1) : EnergeticActor(imageID, x, y, worldPtr, health, right, 2), m_maxLimit(maxLimit) {}

};*/

class Food : public EnergeticActor {   // not sure if there's a point to deriving it from ImmobileActor as opposed to EnergeticActor
public:
	Food(int x, int y, int health, StudentWorld* worldPtr) : EnergeticActor(IID_FOOD, x, y, worldPtr, health) {}  // should the check for whether it was added on the first turn be here?
	virtual void doSomething() {}
	virtual bool canBeEaten() const { return true; }
	//virtual bool beEaten(int amount);
	virtual ~Food() {}
};

// this class includes more than the insects defined in the spec, just because it made sense to me organizationally
class Insect : public EnergeticActor {
public:
	Insect(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int maxHealth, Direction dir = right) 
		: EnergeticActor(imageID, x, y, worldPtr, startingHealth, maxHealth, dir) , m_movesStunned(0) {}
	virtual bool canBeStunned() const { return true; }
	virtual bool canBePoisoned() const { return true; }
	virtual bool canEat() const { return true; }
	virtual bool canMove(Coord attemptedLocation);
	virtual bool isStunned() const { return m_movesStunned != 0; }
	virtual bool die();
	void decrementMovesStunned() { m_movesStunned--; }

	// these five functions return true if it was possible to move to a certain location.
	// still need to check if there's poison or water on the square...
	// not useful for AdultGrasshopper because there may be pebbles in between the start and destination.
	bool moveUp();
	bool moveDown();
	bool moveRight();
	bool moveLeft();
	bool moveInCurrentDirection();
	virtual ~Insect() {}
	//virtual void eat(); // fill in implementation.

private:
	int m_movesStunned;
};

// get randomInt for direction
class Grasshopper : public Insect {
public:
	Grasshopper(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int maxHealth) 
		: Insect(imageID, x, y, worldPtr, startingHealth, maxHealth, Actor::getRandomDirection()), 
		m_distanceToTravel(randInt(2, 10)), m_movesAsleep(0) {}
	virtual bool isAsleep() const { return m_movesAsleep != 0; }
	void decrementMovesAsleep() { m_movesAsleep--; }
	void goToSleep() { m_movesAsleep = 2; }
	bool areWeThereYet() const { return m_distanceToTravel <= 0; } // checks if it's walked
	void doneTraveling() { m_distanceToTravel = 0; }
	void oneStepCloser() { m_distanceToTravel--; }
	void addMoreDistance() { m_distanceToTravel = randInt(2, 10); }
	virtual void doSomething() { manageHealth(); }

	virtual ~Grasshopper() {} // look at comments for ant destructor
private:
	bool manageHealth();
	int m_distanceToTravel;
	int m_movesAsleep;
};

// passing in -1 for maxHealth because in my implementation, it would be problematic
// if I passed in 1600. after all, doSomething takes care of evolving if it becomes necessary
//override the addHealth function so that it signals a need for the BabyGrasshopper to grow up
class BabyGrasshopper : public Grasshopper { // starts with 500 HP
public:
	BabyGrasshopper(int x, int y, StudentWorld* worldPtr) 
		: Grasshopper(IID_BABY_GRASSHOPPER, x, y, worldPtr, 500, -1) {}
	virtual void doSomething();
	virtual ~BabyGrasshopper() {} // look at comments for ant destructor
private:
};

// pass in maxHealth = -1. have "hop" member fn
class AdultGrasshopper : public Grasshopper { // starts with 1600 HP
public:
	AdultGrasshopper(int x, int y, StudentWorld* worldPtr)
		: Grasshopper(IID_ADULT_GRASSHOPPER, x, y, worldPtr, 1600, -1) {}
	virtual void doSomething() {}
	virtual bool canFly() const { return true; }
	virtual bool canAttack() const { return true; }
	virtual bool canBeStunned() const { return false; }
	virtual bool canBePoisoned() const { return false; }
	virtual ~AdultGrasshopper() {}
private:
};

//think through this class's existence later.
//consider deriving pheromone from SuperAnt.
class SuperAnt : public Insect {
public:
	SuperAnt(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int maxHealth, int colonyNum, Direction dir)
		: Insect(imageID, x, y, worldPtr, startingHealth, maxHealth, dir), m_colonyNumber(colonyNum) {}
	virtual int getColonyNumber() const { return m_colonyNumber; }
	virtual bool die();
	// this return nullptr if you call it with a pheromone
	// is this even safe to do?
	//Compiler* getCompiler() const { return m_compiler; }
	virtual ~SuperAnt() {}
private:
	//Compiler* m_compiler;
	int m_colonyNumber;
};

// shares fewer characteristics. mainly just because there should be a getCompiler function in one place
// as well as how they have the same MaxHealth (in other words, no limit)
class GoodAnt : public SuperAnt
{
public:
	GoodAnt(int imageID, int x, int y, StudentWorld* worldPtr, Compiler* complr, int startingHealth, int colonyNum, Direction dir)
	: SuperAnt(imageID, x, y, worldPtr, startingHealth, -1, colonyNum, dir), m_compiler(complr) {}
	// is this even safe to do?
	Compiler* getCompiler() const { return m_compiler; }
private:
	Compiler* m_compiler;
};

class Anthill : public GoodAnt {
public:
	Anthill(int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: GoodAnt(IID_ANT_HILL, x, y, worldPtr, complr, 8999, colonyNum, right) {}//,
		//m_compiler(complr), m_colonyNumber(colonyNum)
	virtual bool canBeStunned() const { return false; }
	virtual bool canBePoisoned() const { return false; }
	virtual void doSomething() {}
	virtual bool canMove(Coord attemptedLocation) { return false; }
	virtual ~Anthill() {}
private:
	// interpreter instead of this? have fn that calls this? be aware of deletion of this. don't double delete when i delete ants.
	/*Compiler* m_compiler;
	int m_colonyNumber;*/
};

// make sure that ant class specifies that it points to a certain anthill perhaps and then access instructions that way?
// there's a lot I need to add to this class
class Ant : public GoodAnt {
public:
	Ant(int imageID, int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: GoodAnt(imageID, x, y, worldPtr, complr, 1500, colonyNum, Actor::getRandomDirection()),
		m_instructionCounter(0), m_commandCounter(0), m_wasBitten(false), m_wasBlocked(false), m_lastSquare(x,y) {}
		//, m_colonyNumber(imageID), m_compiler(complr) {}
	// HEY!!! MAY NOT WANT INSTRUCTION COUNTER TO BE SET TO 0!!!
	//virtual int getColonyNumber() const{ return m_colonyNumber; }
	virtual void doSomething() {}
	virtual ~Ant() {} // should this do anything? have it call StudentWorld to tell it to add food at a given spot? or should StudentWorld do that?

private:
	int m_instructionCounter; // initialize this to 0?
	int m_lastRandInt; // don't access before initializing...
	int m_foodQuantityHeld; // don't access before initializing
	int m_commandCounter;
	Coord m_lastSquare; // was initialized properly? update right after moving? make sure it works with other parts
	bool m_wasBitten; // have function to manage this.
	bool m_wasBlocked; // have function to manage this.
	//int m_colonyNumber;  // get this from the imageID
	//Compiler* m_compiler;
};

//need to specify the pheromoneID when creating it
class Pheromone : public SuperAnt {
public:
	// if I'm adding a new Pheromone, it will always have initial health of 256, right?
	Pheromone(int pheromoneID, int x, int y, StudentWorld* worldPtr, int colonyNum)
		: SuperAnt(pheromoneID, x, y, worldPtr, 256, 768, colonyNum, right) {}//{ m_colony = pheromoneID - IID_PHEROMONE_TYPE0; }
	virtual void doSomething() {}
	// the following are sort of redundant. sigh.
	virtual bool canBeStunned() const { return false; }
	virtual bool canBePoisoned() const { return false; }
	virtual bool canEat() const { return false; }
	virtual bool canMove(Coord attemptedLocation) { return false; }
	//virtual int getColonyNumber() const { return m_colony; }
	virtual ~Pheromone() {}

private:
	//int m_colony;
};

class ImmortalActor : public Actor {
public:
	ImmortalActor(int imageID, int x, int y, StudentWorld* worldPtr, int depth = 2) : Actor(imageID, x, y, worldPtr, right, depth){}
	virtual bool hasEnergyOrHealth() const{ return false; }
	virtual bool die();
	virtual ~ImmortalActor() {}
};

class Pebble : public ImmortalActor {
public:
	Pebble(int x, int y, StudentWorld* worldPtr) : ImmortalActor(IID_ROCK, x, y, worldPtr, 1) {}
	virtual void doSomething() { /*incrementTimesCalled(); numberTimesCalled();*/  }
	virtual bool canBlockPath() const { return true; }
	virtual ~Pebble() {}
};


// may want to modify the class structure and bring poison and waterpool together
class Poison : public ImmortalActor {
public:
	Poison(int x, int y, StudentWorld* worldPtr) : ImmortalActor(IID_POISON, x, y, worldPtr) {}
	virtual void doSomething() {}
	virtual bool canPoison() const { return true; }
	virtual ~Poison() {}
};

class WaterPool : public ImmortalActor {
public:
	WaterPool(int x, int y, StudentWorld* worldPtr) : ImmortalActor(IID_WATER_POOL, x, y, worldPtr) {}
	virtual void doSomething() {}
	virtual bool canStun() const { return true; }
	virtual ~WaterPool() {}
};

// constructor should only get parameters that are unique for the given object
// remember that there are some insects that will add food to the world if they die

#endif // ACTOR_H_
