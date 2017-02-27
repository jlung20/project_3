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
	// default constructing this way is potentially valuable, as seen in my use of it in various parts of this project
	Coord() : m_x(-1), m_y(-1) {}
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
	virtual bool hasEnergyOrHealth() const = 0; // pure virtual because of clear split between those that do and don't have it
	virtual bool canBlockPath() const { return false; } // override for pebble
	virtual bool canStun() const { return false; } // override for water
	virtual bool canBeStunnedOrPoisoned() const { return false; } // override for adults and baby
	virtual bool canBeEaten() const { return false; } // override for food, obviously
	virtual bool canEat() const { return false; } // override for animals
	virtual bool canPoison() const { return false; } //override for poison
	virtual bool canFly() const { return false; } // override for adult grasshoppers
	virtual bool isAttackable(int colonyNumber, Actor* self = nullptr) { return false; } // determines what can be targeted
	virtual bool isDead() const = 0; // pure virtual because there's a clear split further down between those that can and can't die
	int getColonyNumber() const { return m_colonyNumber; } // if it can't be associated with a particular colony, return -1
	virtual void updateLastStunnedLocation(const Coord &newStunLocation) {} // ensures that ants and baby g's are stunned the proper number of times
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const { return false; } // if it's not an ant or baby grasshopper, this is fine
	virtual void setBitten() {} // shouldn't do anything for any other actor than ant.
	virtual void addMovesInactive(int amount) {}
	virtual void addHealth(int amount) {} // only applicable for EnergeticActors
	virtual void biteBack() {} // only applicable for adult grasshoppers
	virtual int reduceHP(int amount) { return 0; } // can't reduce HP for many things.
	virtual ~Actor() {};
	bool hasDoneSomething(int currentMove)  const { return currentMove == m_lastMove; };
	void updateLastMove(int currentMove) { m_lastMove = currentMove; } // doing this is useful for checking if doSomething has been called on a given tick

protected:
	// allows derived classes to know what their StudentWorld is
	StudentWorld* getPtrToWorld();
	// only used by derived classes
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
	int getCurrentMove() const;

private:
	StudentWorld* ptrToWorld;
	int m_lastMove; // last tick on which doSomething was called
	int m_colonyNumber; // -1 for most things
};

// includes all actors that have energy or health
class EnergeticActor : public Actor{
public:
	EnergeticActor(int imageID, int x, int y, StudentWorld* worldPtr, int HP, int maxHealth = -1, Direction dir = right, int colonyNumber = -1, int depth = 2)
		: Actor(imageID, x, y, worldPtr, dir, colonyNumber, depth), m_healthPoints(HP), m_maxLimit(maxHealth) {}
	virtual bool hasEnergyOrHealth() const { return true; }
	virtual void addHealth(int amount); // used both by StudentWorld and classes derived from this
	virtual int reduceHP(int amount); // returns amount by which HP was reduced
	virtual bool isDead() const { return m_healthPoints <= 0; }
	virtual ~EnergeticActor() {}

protected:
	int getHealth() const { return m_healthPoints; }

private:
	int m_healthPoints; // defining characteristic of class
	int m_maxLimit; // only applicable for pheromone
};

class Food : public EnergeticActor {
public:
	Food(int x, int y, int health, StudentWorld* worldPtr) : EnergeticActor(IID_FOOD, x, y, worldPtr, health) {}
	virtual void doSomething() {} // nothing to do
	virtual bool canBeEaten() const { return true; } // because it's food
	virtual ~Food() {}
};

// this class includes all actors who decrease their HP/energy each turn -- in other words, those that are hungry
class HungryActor : public EnergeticActor {
public:
	HungryActor(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int maxHealth,
		int colonyNumber, Compiler* compiler = nullptr, int depth = 2, Direction dir = right)
		: EnergeticActor(imageID, x, y, worldPtr, startingHealth, maxHealth, dir, colonyNumber, depth),
		m_compiler(compiler) {}
	virtual bool canEat() const { return true; }
	virtual ~HungryActor() {}

protected:
	Compiler* getCompiler() { return m_compiler; } // bad news if something tries to dereference what this returns if it's nullptr
	bool manageHealth(); // decrements health and returns false if it's time to die
	void decrementHP() { reduceHP(1); } // used only by classes derived from it.

private:
	Compiler* m_compiler; // nullptr if not passed in by ant or anthill
};

class Pheromone : public HungryActor {
public:
	// new Pheromone will always have initial health of 256
	Pheromone(int pheromoneID, int x, int y, StudentWorld* worldPtr, int colonyNum)
		: HungryActor(pheromoneID, x, y, worldPtr, 256, 768, colonyNum) {}
	virtual void doSomething() { decrementHP(); }
	virtual bool canEat() const { return false; }
	virtual ~Pheromone() {}
};

class Anthill : public HungryActor {
public:
	Anthill(int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: HungryActor(IID_ANT_HILL, x, y, worldPtr, 8999, -1, colonyNum, complr) {}
	virtual void doSomething();
	virtual ~Anthill() {}
};

// all actors that can move and that drop food upon dying
class Insect : public HungryActor {
public:
	Insect(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth, int colonyNum, Compiler* compiler)
		: HungryActor(imageID, x, y, worldPtr, startingHealth, -1, colonyNum, compiler, 1, Actor::getRandomDirection()), 
		 m_movesInactive(0){} // deleted m_movesStunned(0),
	virtual bool canBeStunnedOrPoisoned() const { return true; }
	virtual bool isAttackable(int colonyNumber, Actor* self);
	virtual void updateLastStunnedLocation(const Coord &newStunLocation);
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const; // returns if it's possible for something to be stunned at current location
	virtual void addMovesInactive(int amount) { m_movesInactive += amount; } // need it to be public so StudentWorld can call it
	virtual ~Insect() { die(); } // adds food to square where insect was right before destruction

protected:
	// these five functions return true if move was successfully made.
	// used only for single moves.
	bool moveUp();
	bool moveDown();
	bool moveRight();
	bool moveLeft();
	bool moveInCurrentDirection();

	bool manageDamage(); // decrements number of moves inactive. returns true if the insect can move during the turn.
	void setRandomDirection() { setDirection(getRandomDirection()); }
	int getMovesInactive() { return m_movesInactive; }
	void updatePreviousLocation(); // changes previousLocation to a newer location
	virtual bool attackEnemy(int colonyNumber, Coord location, int damage); // tries to bite enemy. returns true if successful

private:
	bool die(); // adds 100 food to square where insect died
	void decrementMovesInactive() { m_movesInactive--; }
	int m_movesInactive; // combines moves stunned and asleep
	Coord m_locationLastStunned; // updates when stunned
	Coord m_previousLocation; // updates the tick after an ant reaches a new square
	bool canMove(const Coord &attemptedLocation);
};

// the big kahuna. not your average auntie.
class Ant : public Insect {
public:
	Ant(int imageID, int x, int y, StudentWorld* worldPtr, Compiler* complr, int colonyNum)
		: Insect(imageID, x, y, worldPtr, 1500, colonyNum, complr),
		m_instructionCounter(0), m_lastRandInt(0), m_foodQuantityHeld(0),
		m_wasBitten(false), m_wasBlocked(false), m_anthillCoords(x, y) {}
	virtual void setBitten() { m_wasBitten = true; } // if bitten on square, this needs to be changed
	virtual void doSomething();
	virtual ~Ant() {}

private:
	void updateLastRandomNumber(int lastRand) { m_lastRandInt = lastRand; }
	void addPheromone(Coord current); // calls addNewDuringGame, which either adds a new pheromone or adds the appropriate quantity to an existing pheromone
	void generateRandomNumberUpTo(std::string operand); // uses provided randInt function
	bool isThereFoodHere(Coord current); // determines if there's food at a given square
	bool isThereEnemyHere(Coord current); // determines if there's an enemy at a given square
	int m_instructionCounter; // which instruction should be executed
	int m_lastRandInt; // don't access before initializing... make sure that whatever uses this is aware of how it starts
	int m_foodQuantityHeld; // amount of food it's holding
	int reduceHeldFood(int amount); // this and the following two manipulate m_foodQuantityHeld
	void addHeldFood(int amount);
	void pickupFood(Coord current);
	Coord m_anthillCoords; // location of anthill
	bool conditionFulfilled(Compiler::Command cmd, Coord location);
	bool m_wasBitten; // have function to manage this.
	bool m_wasBlocked; // have function to manage this.
	void rotateClockwise();
	void rotateCounterClockwise();
};

// defined doSomething here but left a pure virtual for the doAdultOrImmatureThings - needs to be able to evolve
// if BabyGrasshopper, but bite and hop in AdultGrasshopper
class Grasshopper : public Insect {
public:
	Grasshopper(int imageID, int x, int y, StudentWorld* worldPtr, int startingHealth) 
		: Insect(imageID, x, y, worldPtr, startingHealth, -1, nullptr), 
		m_distanceToTravel(randInt(2, 10)) {}
	virtual void doSomething();
	virtual ~Grasshopper() {}
protected: // only classes derived from grasshopper require these functions
	void goToSleep() { addMovesInactive(2); }
	bool areWeThereYet() const { return m_distanceToTravel <= 0; } // checks if it's arrived
	void doneTraveling() { m_distanceToTravel = 0; } // will be called if it's unable to keep going in a certain direction
	void oneStepCloser() { m_distanceToTravel--; }
	void addMoreDistance() { m_distanceToTravel = randInt(2, 10); } // adds random amount of distance between 2 and 10 units

private:
	virtual bool doAdultOrImmatureThings() = 0;
	int m_distanceToTravel; // distance still to go
};

// feeble little guy. but its best days are ahead of it. usually.
class BabyGrasshopper : public Grasshopper {
public:
	BabyGrasshopper(int x, int y, StudentWorld* worldPtr) 
		: Grasshopper(IID_BABY_GRASSHOPPER, x, y, worldPtr, 500) {}
	virtual ~BabyGrasshopper() {}
private:
	bool evolve(); // kills baby grasshopper and adds adult grasshopper
	virtual bool doAdultOrImmatureThings();
};

class AdultGrasshopper : public Grasshopper {
public:
	AdultGrasshopper(int x, int y, StudentWorld* worldPtr)
		: Grasshopper(IID_ADULT_GRASSHOPPER, x, y, worldPtr, 1600) {}
	virtual bool canFly() const { return true; } // needs the following functions because it's so special
	virtual bool canBeStunnedOrPoisoned() const { return false; }
	virtual bool canBeStunnedAtCurrentSquare(const Coord &current) const { return false; }
	virtual void biteBack();
	virtual ~AdultGrasshopper() {} // a virtual destructor is required for each one, parts not shared with the Actor base class would not be deleted
private:
	virtual bool doAdultOrImmatureThings();
	bool fly(); // it flies man. it flies.
	int findNumberOfOpenSquares(); // finds number of open squares within a ten square radius
	bool isValidSquare(Coord square); // determies if the square is valid so it doesn't hop away
	Coord findNthOpenSquare(int n); // finds nth square that's open
};

class ImmortalActor : public Actor {
public:
	ImmortalActor(int imageID, int x, int y, StudentWorld* worldPtr, int depth = 2) 
		: Actor(imageID, x, y, worldPtr, right, -1, depth){}
	virtual bool hasEnergyOrHealth() const{ return false; } // inanimate things can't be characterized as having health
	virtual bool isDead() const { return false; } // inanimate things can't die
	virtual ~ImmortalActor() {}
};

class Pebble : public ImmortalActor {
public:
	Pebble(int x, int y, StudentWorld* worldPtr) : ImmortalActor(IID_ROCK, x, y, worldPtr, 1) {}
	virtual void doSomething() {} // nothing for doSomething to do
	virtual bool canBlockPath() const { return true; }
	virtual ~Pebble() {}
};

// poison and water pool share doSomething functions because doSomething calls the same 
// function, attackAllActorsAtCurrentSquare, which the two implement differently
class Attacker : public ImmortalActor {
public:
	Attacker(int thingID, int x, int y, StudentWorld* worldPtr) : ImmortalActor(thingID, x, y, worldPtr) {}
	virtual void doSomething() { attackAllActorsAtCurrentSquare(); }
	//virtual bool canAttack() { return true; }
	virtual ~Attacker() {}
private:
	virtual void attackAllActorsAtCurrentSquare() = 0;
};

class Poison : public Attacker {
public:
	Poison(int x, int y, StudentWorld* worldPtr) : Attacker(IID_POISON, x, y, worldPtr) {}
	virtual bool canPoison() const { return true; } // because it's poisonous
	virtual ~Poison() {}
private:
	virtual void attackAllActorsAtCurrentSquare(); // implemented slightly differently than for water pool
};

class WaterPool : public Attacker {
public:
	WaterPool(int x, int y, StudentWorld* worldPtr) : Attacker(IID_WATER_POOL, x, y, worldPtr) {}
	virtual bool canStun() const { return true; } // because it's a water pool
	virtual ~WaterPool() {}
private:
	virtual void attackAllActorsAtCurrentSquare();
};

#endif // ACTOR_H_
