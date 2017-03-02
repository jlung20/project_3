#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "Field.h"
#include <vector>
#include <map>
#include <string>

// need this comparison operator in order to use map
// need to put the definition in .cpp or it would've been defined multiple times
bool operator < (const Coord& first, const Coord& second);
bool operator == (const Coord& first, const Coord& second);

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
	 : GameWorld(assetDir), m_ticksElapsed(0), m_numAnthills(0), m_numCompilers(0), m_currentAnthillLeader(-1)
	{
	}

	virtual int init();
	virtual int move();
	virtual void cleanUp();
	int getTicksElapsed() const { return m_ticksElapsed; }

	// returns number of things of a certain type in current square
	int howManyAreThereAtCurrentSquare(int thingID, Coord location);

	// adds a new actor to the map or, if it's food or pheromone and one is already present, simply add to strength
	// need the foodQuantity parameter because there's no other way of knowing how much food to add
	void addNewDuringGame(int thingID, Coord location, int foodQuantity = 0, int colonyNum = -1, Compiler* compiler = nullptr);

	// returns true if at least one unit of food can (and is) consumed
	int eatFoodAtCurrentSquare(Coord current, int amount, Actor* eater);

	// used by poison and water pool to do their respective actions to all susceptible actor objects at a given location
	void attackAllAtCurrentSquare(Coord current, char ch);

	// bites a random enemy at the location if possible
	bool biteEnemy(int colonyNumber, Coord location, int damage, Actor* attacker);
	
	// returns true if there's a pebble at the location
	bool pathBlocked(Coord location);

	// returns true if there's an enemy one square ahead in Direction dir from location
	bool dangerAhead(Coord location, int colonyNumber, Actor::Direction dir);
	
	// returns true if there's a pheromone of the proper colony one square ahead in Direction from location
	bool pheromoneAhead(Coord location, int colonyNumber, Actor::Direction dir);

	~StudentWorld() { cleanUp(); }

private:
	std::string anthillNames[4];
	int m_ticksElapsed; // tick counter
	int m_numAnthills;
	int m_ants[4] = { 0, 0, 0, 0 }; // intended to keep track of how many ants each anthill has
	int m_antRank[4] = { 1, 1, 1, 1 }; // there is no leader. remove if I end up not going in this direction
	int m_currentAnthillLeader; // this keeps track of which anthill would win if the game were to end at any given time.
	// must check every time an ant dies or is generated that this remains true.
	// if it's -1, there is no leader.
	Compiler* compilerArr[4]; // this strange thing is here so that the compilers will only get deleted once
	int m_numCompilers;
	void determineCurrentLeader(); //maybe try again later. just a stub RN!!! sets player 0 as leader.
	//void sortHighScores(int *highScores, int numHighScores);
	void setAnthillNames();
	std::string getWinningAntName();
	std::map<Coord, std::vector<Actor*>> actorMap;   // virtual map of locations of things in world

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

	// determines the number of actors that can be bitten
	int numberToBite(Coord location, int colonyNumber, Actor* notThisGuy);

	// takes thingID and matches it based on properties of the actor
	int StudentWorld::identifyByThingID(Actor* actor) const;

	// removes all objects that have been marked dead
	void removeDeadSimulationObjects();

	void setDisplayText();

	std::string formatBetter(int ticks, int antOne, int antTwo, int antThree, int antFour, int leadingAnt);

	// returns pointer to the first actor of the proper type or nullptr if no such actor is found within the vector mapped from a certain key
	Actor* getPtrToThingAtCurrentSquare(int thingID, Coord location);

	Actor* getPtrToIthVictim(Coord location, int colonyNumber, Actor* notThisGuy, int victimNumber);

	// moves the location to one square in the given direction
	bool updateLocation(Coord &location, Actor::Direction dir);
};

#endif // STUDENTWORLD_H_
