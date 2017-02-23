#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"  // does this make sense?
#include "Field.h"
#include <vector>
#include <map>
#include <string>
//#include <iostream> // delete this later
//class Actor; // does this make more sense?

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

// need this comparison operator in order to use map
// need to put the definition in .cpp or it would've been defined multiple times
bool operator < (const Coord& first, const Coord& second);
bool operator == (const Coord& first, const Coord& second);

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
	 : GameWorld(assetDir), m_ticksElapsed(0), m_numAnthills(0), m_numCompilers(0)
	{
	}

	virtual int init();

	virtual int move();

	virtual void cleanUp();

	int getTicksElapsed() { return m_ticksElapsed; }

	// add a function for when stuff dies!!!

	// returns number of things in a current square-- doesn't say how much of say a certain pheromone there is at the square though
	// also... the actorPtr is set to point to the first element in the vector that matches thingID or nullptr if nothing matches
	int howManyAreThereAtCurrentSquare(int thingID, Coord location);

	// adds a new actor to the map or, if it's food or pheromone and one is already present, simply add to strength
	// need the foodQuantity parameter because there's no other way of knowing how much food to add
	void addNewDuringGame(int thingID, Coord location, int foodQuantity = 0, int colonyNum = -1, Compiler* compiler = nullptr);

	// returns true if at least one unit of food can (and is) consumed
	bool eatFoodAtCurrentSquare(Coord current, int amount, Actor* eater);

	bool actorDie(Actor* actor);

	// takes thingID and matches it based on properties of the actor
	int StudentWorld::identifyByThingID(Actor* actor) const;

	~StudentWorld() {
		//std::cerr << "Hello from destructor. Is this called before or after cleanUp()?" << std::endl;
		cleanUp(); 
	} // have this destructor also get rid of compilers

private:
	std::string anthillNames[4];
	int m_ticksElapsed; // tick counter
	int m_numAnthills;
	int m_ants[4] = { 0, 0, 0, 0 }; // intended to keep track of how many ants each anthill has
	int m_currentAnthillLeader; // this keeps track of which anthill would win if the game were to end at any given time.
	// must check every time an ant dies or is generated that this remains true. change if necessary.
	// if it's -1, there is no leader.
	Compiler* compilerArr[4]; // this strange thing is here so that the compilers will only get deleted once
	int m_numCompilers;
	void determineCurrentLeader(); //maybe try again later. just a stub RN!!! sets player 0 as leader.
	void setAnthillNames();
	std::string getWinningAntName();
	std::map<Coord, std::vector<Actor*>> actorMap;   // virtual map of locations of things in world
	// these are the actors that are added over the course of a tick. placed in this vector temporarily so that 
	// they don't screw up iterators
	//std::vector<Actor*> actorsToBeAdded;
	// actors that have moved from their original locations. placed here temporarily so they don't mess up iterators
	//std::map<Coord, std::vector<Actor*>> actorsToBeMoved;
	void addPtrInMappedVector (Coord coordinates, Actor* act) // adds a pointer to an Actor to the map at a certain location
	{
		actorMap[coordinates].push_back(act); // should this be insert?
	}

	// doesn't actually delete anything -- possibility of memory leaks here.
	// need to make sure that this is called in conjunction with addPtrInMappedVector or when deleting the actor
	// this simply removes a pointer to an Actor that was stored in a vector at a certain set of coordinates
	bool removePtrInMappedVector(Coord coordinates, Actor* act); // delete coordinate if vector will become empty?

	// this takes the item type and allocates memory for an instance of the subclass of Actor
	// then stores the pointer in the map in the coordinate pair that corresponds to location
	bool identifyAndAllocate(Field::FieldItem item, Coord location, int colonyNum = -1, Compiler* compiler = nullptr);

	// removes all objects that have been marked dead
	void removeDeadSimulationObjects();

	void setDisplayText();

	std::string formatBetter(int ticks, int antOne, int antTwo, int antThree, int antFour, int leadingAnt);

	//int howManyAreThereAtCurrentSquare(int thingID, Coord location, Actor* &actorPtr);

	// returns pointer to the first actor of the proper type or nullptr if no such actor is found within the vector mapped from a certain key
	Actor* getPtrToThingAtCurrentSquare(int thingID, Coord location);

	// also need a function that checks if there's anything mapped to (-1, -1) that's actually somewhere of interest.
};

#endif // STUDENTWORLD_H_
