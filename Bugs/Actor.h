#ifndef ACTOR_H_
#define ACTOR_H_

#include "GameConstants.h"
#include "GraphObject.h"
#include "Compiler.h"
class StudentWorld;

const int FOOD_SIZE_AT_START = 6000;
const int FOOD_FROM_DEAD_INSECT = 100;

// go through all these comments and change as necessary
// remove unnecessary data members and functions.
// check that all private data members are good.

// simplifies descriptions of points. as spec instructs, x and y are not member vars of graphobject's derived classes
class Coord {
public:
	Coord() : m_x(-1), m_y(-1) {} // this is dangerous if you use any coordinate that was not subsequently replaced.
	Coord(int col, int row) : m_x(col), m_y(row) {}
	int getCol() const { return m_x; }
	int getRow() const { return m_y; }
	void setCol(int col) { m_x = col; }
	void setRow(int row) { m_y = row; }

private:
	int m_x;
	int m_y;
};

class Actor : public GraphObject {
public:
	Actor(int imageID, int x, int y, StudentWorld* worldPtr, Direction dir, int colonyNumber = -1, int depth = 2)
		: GraphObject(imageID, x, y, dir, depth), ptrToWorld(worldPtr), m_lastMove(-1), m_colonyNumber(colonyNumber) {}
	virtual void doSomething() = 0;
	virtual bool hasEnergyOrHealth() const = 0; // override for those not food, pheromone, anthill, and animals
	virtual bool canBlockPath() const { return false; } // override for pebble
	virtual bool canStun() const = 0; // override for water
	virtual bool canBeStunnedOrPoisoned() const { return false; } // override for animals
	virtual bool canBeEaten() const { return false; } // override for food, obviously
	virtual bool canAttack() const { return false; } // override for ants and adult grasshoppers
	virtual bool canEat() const { return false; } // override for animals
	virtual bool canPoison() const { return false; } //override for poison
	virtual bool canFly() const { return false; } // override for adult grasshoppers
	virtual bool canBite() const { return false; } // override for some animals
	virtual bool isAttackable(int colonyNumber, Actor* self = nullptr) { return false; } // determines what can be targeted
	virtual bool isInactive() const { return false; } // override for insects, who can sleep and be stunned 
	virtual bool isDead() const = 0; // pure virtual because there's a clear split further down between those that can and can't die
	virtual int getColonyNumber() const { return m_colonyNumber; } // if it can't be associated with a particular colony, return -1
	virtual void updateLastStunnedLocation(const Coord &newStunLocation) {} // ensures that ants and baby g's are stunned the proper number of times
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const { return false; } // if it's not an ant or baby grasshopper, this is fine
	virtual void setBitten() {} // shouldn't do anything for any other actor than ant.
	virtual void addMovesInactive(int amount) {}
	virtual void addHealth(int amount) {} // only applicable for EnergeticActors
	virtual void biteBack() {} // only applicable for adult grasshoppers
	virtual int reduceHP(int amount) { return 0; } // can't reduce HP for many things.
	virtual ~Actor() {};
	bool hasDoneSomething(int currentMove)  const { return currentMove == m_lastMove; };
	void updateLastMove(int currentMove) { m_lastMove = currentMove; } // does this need to be virtual? needs to be called to make sure life is OK.
	int getCurrentMove() const;
	/*void numberTimesCalled(); for testing that calls are made properly
	void incrementTimesCalled() { m_timesCalled++; }*/

protected:
	// allows derived classes to know what their StudentWorld is
	StudentWorld* getPtrToWorld() { return ptrToWorld; }
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

private:
	StudentWorld* ptrToWorld;
	int m_lastMove;
	int m_colonyNumber;
	//virtual bool isDead() const = 0;
	//virtual void move() = 0;
	//int m_timesCalled; // delete this later.
};

// 	Actor(int imageID, int x, int y, StudentWorld* worldPtr, Direction dir, int colonyNumber = -1, int depth = 2)

class EnergeticActor : public Actor{
public:
	EnergeticActor(int imageID, int x, int y, StudentWorld* worldPtr, int HP, int maxHealth = -1, Direction dir = right, int colonyNumber = -1, int depth = 2)
		: Actor(imageID, x, y, worldPtr, dir, colonyNumber, depth), m_healthPoints(HP), m_maxLimit(maxHealth) {} // added back m_maxHealth member which was set to default to -1
	// do I need to define my own copy constructor?
	// requires that the studentWorld has verified the amount of health that can be added in light of limits per turn and per square
	virtual bool hasEnergyOrHealth() const { return true; }
	virtual bool canStun() const { return false; }
	virtual void addHealth(int amount);
	int getHealth() const { return m_healthPoints; }
	//virtual void decrementHP() { m_healthPoints--; } // should I add a check here for stuff dying? I need to add it somewhere
	virtual int reduceHP(int amount); // returns amount by which HP was reduced
	bool isDead() const { return m_healthPoints <= 0; }
	//virtual bool die(); // do I need this?
	int getMaxHealth() const { return m_maxLimit; } // be aware that this will return -1 in many circumstances
	virtual ~EnergeticActor() {}
	//virtual int getColonyNumber() const { return m_colonyNumber; }

protected: // delete if not used

private:
	int m_healthPoints;
	int m_maxLimit;
	int m_colonyNumber;
};

// EnergeticActor(int imageID, int x, int y, StudentWorld* worldPtr, int HP, int maxHealth = -1, Direction dir = right, int colonyNumber = -1, int depth = 2)

class Food : public EnergeticActor {   // not sure if there's a point to deriving it from ImmobileActor as opposed to EnergeticActor
public:
	Food(int x, int y, int health, StudentWorld* worldPtr) : EnergeticActor(IID_FOOD, x, y, worldPtr, health) {}  // should the check for whether it was added on the first turn be here?
	virtual void doSomething() {}
	virtual bool canBeEaten() const { return true; }
	//virtual bool beEaten(int amount);
	virtual ~Food() {}
};

// this class includes all actors who decrease their HP/energy each turn -- in other words, those that are hungry
class HungryActor : public EnergeticActor {
public:
	HungryActor(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int maxHealth,
		int colonyNumber, Compiler* compiler = nullptr, int depth = 2, Direction dir = right)
		: EnergeticActor(imageID, x, y, worldPtr, startingHealth, maxHealth, dir, colonyNumber, depth),
		m_compiler(compiler) {}//, m_movesStunned(0) {}
	//virtual bool canBePoisoned() const { return true; }
	virtual bool canEat() const { return true; }
	// move movement to insect class
	//virtual bool canMove(Coord attemptedLocation);
	// move this to insect and check if it's been stunned at the current square? no... simply check if it's inactive (isInactive()) and add fn that performs other behavior
	//virtual bool isStunned() const { return m_movesStunned != 0; }
	//virtual bool die(); // would it make sense if only Insect has a die function? protected too, right?
	//void decrementMovesStunned() { m_movesStunned--; }
	void decrementHP() { reduceHP(1); } // doesn't need to be virtual because i'm not redeclaring it, right?
	virtual ~HungryActor() {}

protected:
	Compiler* getCompiler() { return m_compiler; } // NEED TO MAKE DEAD SURE THAT NOTHING TRIES TO DEREFRENCE THIS BEFORE CHECKING THAT IT'S NULLPTR
	bool manageHealth();
	//virtual void eat(); // fill in implementation.

private:
	//int m_movesStunned; // maybe change to is inactive?
	Compiler* m_compiler; // need to fix this. passed in only if ant or anthill. otherwise, set to nullptr
};

class Pheromone : public HungryActor {
public:
	// if I'm adding a new Pheromone, it will always have initial health of 256, right?
	Pheromone(int pheromoneID, int x, int y, StudentWorld* worldPtr, int colonyNum)
		: HungryActor(pheromoneID, x, y, worldPtr, 256, 768, colonyNum) {}
	virtual void doSomething() {}
	// the following are sort of redundant. sigh.
	virtual bool canEat() const { return false; }
	//virtual bool canMove(Coord attemptedLocation) { return false; } // won't need this if I move movement to Insect
	//virtual int getColonyNumber() const { return m_colony; }
	virtual ~Pheromone() {}

private:
	//int m_colony;
};

class Anthill : public HungryActor {
public:
	Anthill(int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: HungryActor(IID_ANT_HILL, x, y, worldPtr, 8999, -1, colonyNum, complr) {}//,
																				  //m_compiler(complr), m_colonyNumber(colonyNum)
	virtual void doSomething();
	//virtual bool canMove(Coord attemptedLocation) { return false; } won't need this.
	virtual ~Anthill() {}
private:
	// interpreter instead of this? have fn that calls this? be aware of deletion of this. don't double delete when i delete ants.
	/*Compiler* m_compiler;
	int m_colonyNumber;*/
};

//think through this class's existence later.
//consider deriving pheromone from SuperAnt.
class Insect : public HungryActor {
public:
	Insect(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int colonyNum, Compiler* compiler)
		: HungryActor(imageID, x, y, worldPtr, startingHealth, -1, colonyNum, compiler, 1, Actor::getRandomDirection()), 
		 m_movesInactive(0){} // deleted m_movesStunned(0),
	virtual bool canBeStunned() const { return true; } // maybe revise this implementation. 
	// I mean this in general. 
	// Like is a property of the insect one that allows it to be stunned?
	virtual bool canBePoisoned() const { return true; }
	//virtual bool canAttack() const { return true; }
	virtual bool canBite() const { return true;	}
	//virtual bool isStunned() const { return m_movesStunned != 0; } // need to change some function that calls this in StudentWorld
	virtual bool isInactive() const { return m_movesInactive != 0; }
	//virtual bool isSleeping() const { return m_movesAsleep != 0; } // can I remove this?
	//virtual bool hasBeenStunnedOrPoisonedAtCurrentSquare(Coord current) const { //change this implementation }
	//void decrementMovesStunned() { m_movesStunned--; }
	virtual bool isAttackable(int colonyNumber, Actor* self);
	//void decrementMovesInactive() { m_movesInactive--; }
	virtual void updateLastStunnedLocation(const Coord &newStunLocation);
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const; // returns if it's possible for something to be stunned at current location
	virtual void addMovesInactive(int amount) { m_movesInactive += amount; } // need it to be public so StudentWorld can call it
	// remember to override for adult grasshoppers
	//maybe add more about poisoning
	
	// have beStunned function
	virtual ~Insect() {}

protected:
	bool die(); // debating the existence of this
	// should these functions be moved somewhere else? yes. move to insect.
	// these five functions return true if it was possible to move to a certain location.
	// still need to check if there's poison or water on the square...
	// not useful for AdultGrasshopper because there may be pebbles in between the start and destination.
	bool moveUp();
	bool moveDown();
	bool moveRight();
	bool moveLeft();
	bool moveInCurrentDirection();
	bool manageDamage();
	void setRandomDirection() { setDirection(getRandomDirection()); }
	int getMovesInactive() { return m_movesInactive; }
	void updatePreviousLocation();
	virtual bool attackEnemy(int colonyNumber, Coord location, int damage);

	virtual bool canMove(const Coord &attemptedLocation); // should it be virtual? will ant override it? might call it, right? and then do more stuff
private:
	//int m_movesStunned; // consider removing this later
	void decrementMovesInactive() { m_movesInactive--; }
	int m_movesInactive;
	//int m_movesAsleep; // same.
	Coord m_locationLastStunned;
	Coord m_previousLocation; // update after remaining in place for a move? probably first option. or at the beginning?
	//bool hasBeenStunnedAtCurrentSquare(Coord current) const;
	//Compiler* m_compiler;
};

// make sure that ant class specifies that it points to a certain anthill perhaps and then access instructions that way?
// there's a lot I need to add to this class
class Ant : public Insect {
public:
	Ant(int imageID, int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: Insect(imageID, x, y, worldPtr, 1500, colonyNum, complr),
		m_instructionCounter(0), m_lastRandInt(0), m_foodQuantityHeld(0), m_commandCounter(0), 
		m_wasBitten(false), m_wasBlocked(false), m_anthillCoords(x, y) {} //, m_lastSquare(x, y) {}
	virtual void setBitten() { m_wasBitten = true; }
	//, m_colonyNumber(imageID), m_compiler(complr) {}
	// HEY!!! MAY NOT WANT INSTRUCTION COUNTER TO BE SET TO 0!!!
	//virtual int getColonyNumber() const{ return m_colonyNumber; }
	virtual void doSomething();
	virtual ~Ant() {} // should this do anything? have it call StudentWorld to tell it to add food at a given spot? or should StudentWorld do that?

private:
	void updateLastRandomNumber(int lastRand) { m_lastRandInt = lastRand; }
	void addPheromone(Coord current); // have this add pheromone using addnewduringgame and passing in colony number and other necessary stuff. addNew does the rest, I think.
	void generateRandomNumberUpTo(std::string operand);
	bool isThereFoodHere(Coord current);
	bool isThereEnemyHere(Coord current);
	int m_instructionCounter;
	int m_lastRandInt; // don't access before initializing... make sure that whatever uses this is aware of how it starts
	int m_foodQuantityHeld;
	int m_commandCounter;
	int reduceHeldFood(int amount);
	void addHeldFood(int amount);
	void pickupFood(Coord current);
	Coord m_anthillCoords;
	bool conditionFulfilled(Compiler::Command cmd, Coord location);
	//Coord m_lastSquare; // was initialized properly? update right after moving? make sure it works with other parts. might not be necessary b/c of what's in other function
	bool m_wasBitten; // have function to manage this.
	bool m_wasBlocked; // have function to manage this.
					   //int m_colonyNumber;  // get this from the imageID
					   //Compiler* m_compiler;
};

// define doSomething here but leave a pure virtual for the doExtraStuff - do no extra stuff in BabyGrasshopper, but bite and whatever in AdultGrasshopper
class Grasshopper : public Insect {
public:
	Grasshopper(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth) 
		: Insect(imageID, x, y, worldPtr, startingHealth, -1, nullptr), 
		m_distanceToTravel(randInt(2, 10)) {}             // deleted m_movesAsleep(0)
	//virtual bool isAsleep() const { return m_movesAsleep != 0; }
	//void decrementMovesAsleep() { m_movesAsleep--; }
	void goToSleep() { addMovesInactive(2); }
	bool areWeThereYet() const { return m_distanceToTravel <= 0; } // checks if it's arrived
	void doneTraveling() { m_distanceToTravel = 0; }
	void oneStepCloser() { m_distanceToTravel--; }
	virtual void doSomething();
	void addMoreDistance() { m_distanceToTravel = randInt(2, 10); }
	virtual ~Grasshopper() {} // look at comments for ant destructor
protected:

private:
	virtual bool doAdultOrImmatureThings() = 0;
	int m_distanceToTravel;
};

// have a do differentiated stuff function?

// passing in -1 for maxHealth because in my implementation, it would be problematic
// if I passed in 1600. after all, doSomething takes care of evolving if it becomes necessary
//override the addHealth function so that it signals a need for the BabyGrasshopper to grow up
class BabyGrasshopper : public Grasshopper { // starts with 500 HP
public:
	BabyGrasshopper(int x, int y, StudentWorld* worldPtr) 
		: Grasshopper(IID_BABY_GRASSHOPPER, x, y, worldPtr, 500) {}
	virtual bool canAttack() const { return false; }
	//virtual void doSomething();
	virtual ~BabyGrasshopper() {} // look at comments for ant destructor
private:
	bool evolve();
	virtual bool doAdultOrImmatureThings();
};

// pass in maxHealth = -1. have "hop" member fn
class AdultGrasshopper : public Grasshopper {
public:
	AdultGrasshopper(int x, int y, StudentWorld* worldPtr)
		: Grasshopper(IID_ADULT_GRASSHOPPER, x, y, worldPtr, 1600) {}
	//virtual void doSomething() {}
	virtual bool canFly() const { return true; }
	virtual bool canBeStunned() const { return false; }
	virtual bool canBePoisoned() const { return false; }
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const { return false; }
	virtual void biteBack();
	//virtual bool hasBeenStunnedOrPoisonedAtCurrentSquare(Coord current) const { return false; } // may or may not truly need this here
	virtual ~AdultGrasshopper() {}
private:
	virtual bool doAdultOrImmatureThings();
	bool fly();
	int findNumberOfOpenSquares();
	bool isValidSquare(Coord square);
	Coord findNthOpenSquare(int n);
};

class ImmortalActor : public Actor {
public:
	ImmortalActor(int imageID, int x, int y, StudentWorld* worldPtr, int depth = 2) 
		: Actor(imageID, x, y, worldPtr, right, -1, depth){}
	virtual bool canStun() const { return false; }
	virtual bool hasEnergyOrHealth() const{ return false; }
	virtual bool isDead() const { return false; }
	//virtual bool die();
	virtual ~ImmortalActor() {}
};

class Pebble : public ImmortalActor {
public:
	Pebble(int x, int y, StudentWorld* worldPtr) : ImmortalActor(IID_ROCK, x, y, worldPtr, 1) {}
	virtual void doSomething() { /*incrementTimesCalled(); numberTimesCalled();*/  }
	virtual bool canBlockPath() const { return true; }
	virtual ~Pebble() {}
};

class Attacker : public ImmortalActor {
public:
	Attacker(int thingID, int x, int y, StudentWorld* worldPtr) : ImmortalActor(thingID, x, y, worldPtr) {}
	virtual void doSomething() { attackAllActorsAtCurrentSquare(); }
	virtual bool canAttack() { return true; }
	virtual ~Attacker() {}
private:
	virtual void attackAllActorsAtCurrentSquare() = 0;
};

class Poison : public Attacker {
public:
	Poison(int x, int y, StudentWorld* worldPtr) : Attacker(IID_POISON, x, y, worldPtr) {}
	//virtual void doSomething() {}
	virtual bool canPoison() const { return true; }
	virtual ~Poison() {}
private:
	virtual void attackAllActorsAtCurrentSquare();
};

class WaterPool : public Attacker {
public:
	WaterPool(int x, int y, StudentWorld* worldPtr) : Attacker(IID_WATER_POOL, x, y, worldPtr) {}
	virtual bool canStun() const { return true; }
	virtual ~WaterPool() {}
private:
	virtual void attackAllActorsAtCurrentSquare();
};

#endif // ACTOR_H_
