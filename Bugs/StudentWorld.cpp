#include "StudentWorld.h"
#include "Compiler.h"
#include "Actor.h"
#include <string>
#include <sstream>
#include <iomanip>
//#include <iostream> // remove later
using namespace std;
// make tester programs!

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

bool operator < (const Coord& first, const Coord& second) // need this comparison operator in order to use map
{
	if (first.getCol() != second.getCol())
		return first.getCol() < second.getCol();
	else
		return first.getRow() < second.getRow();
}

bool operator == (const Coord& first, const Coord& second)
{
	return (first.getCol() == second.getCol()) && (first.getRow() == second.getRow());
}

int StudentWorld::init()
{
	int anthillCount = 0;
	Coord anthillCoords[4]; // contains an ordered set of coordinates corresponding to anthills 0 through anthillCount

	std::string fieldFileName;
	Field f;
	std::string fieldFile = getFieldFilename();
	if (f.loadField(fieldFile) != Field::LoadResult::load_success)
		return GWSTATUS_LEVEL_ERROR; // something bad happened! should it return this or just false?
	for (int i = 0; i < VIEW_HEIGHT; i++)
	{
		for (int j = 0; j < VIEW_WIDTH; j++)
		{
			Field::FieldItem item = f.getContentsOf(j, i);
			if (item == Field::anthill0) // necessary for ensuring that coordinates correspond to proper anthill
			{
				anthillCount++;
				Coord anthill(j, i);
				anthillCoords[0] = anthill;
			}
			else if (item == Field::anthill1)
			{
				anthillCount++;
				Coord anthill(j, i);
				anthillCoords[1] = anthill;
			}
			else if (item == Field::anthill2)
			{
				anthillCount++;
				Coord anthill(j, i);
				anthillCoords[2] = anthill;
			}
			else if (item == Field::anthill3)
			{
				anthillCount++;
				Coord anthill(j, i);
				anthillCoords[3] = anthill;
			}
		}
	}

	m_numAnthills = anthillCount; // useful later on for displaying
	// MIGHT WANT TO UNCOMMENT THIS LATER!!!


	/*if (anthillCount == 0 || anthillCount > 4)
		return GWSTATUS_LEVEL_ERROR; // is this right?*/

	std::vector<std::string> fileNames = getFilenamesOfAntPrograms();

	for (int i = 0; i < anthillCount; i++)
	{
		Compiler *compilerForEntrant = new Compiler;
		std::string error;
	//	anthillNames[i] = fileNames[i];
		if (!compilerForEntrant->compile(fileNames[i], error))
		{
			// entrant 0’s source code had a syntax error!
			// send this error to our framework to warn the user.
			// do it JUST like this!
			setError(fileNames[i] + " " + error);
			// return an error to tell our simulation framework
			// that something went wrong, and it’ll inform the user
			return GWSTATUS_LEVEL_ERROR;
		}
		compilerArr[m_numCompilers] = compilerForEntrant; // refer to StudentWorld.h for an explanation of why I think it makes sense to have a compiler arrray
		m_numCompilers++; // only add to tally if it's a good compiler
		//identifyAndAllocate(Field::anthill0, anthillCoords[i], i, compilerForEntrant); what is this line doing here?
		switch (i)
		{
		case 0: identifyAndAllocate(Field::anthill0, anthillCoords[i], i, compilerForEntrant); break;
		case 1: identifyAndAllocate(Field::anthill1, anthillCoords[i], i, compilerForEntrant); break;
		case 2: identifyAndAllocate(Field::anthill2, anthillCoords[i], i, compilerForEntrant); break;
		case 3:	identifyAndAllocate(Field::anthill3, anthillCoords[i], i, compilerForEntrant); break;
		}
	}

	setAnthillNames();

	Coord outOfBounds;
	actorMap[outOfBounds] = vector<Actor*>();

	for (int i = 0; i < VIEW_HEIGHT; i++)
	{
		for (int j = 0; j < VIEW_WIDTH; j++)
		{
			Coord current(j, i);
			// new stuff below this
			if (actorMap.find(current) == actorMap.end())
				actorMap[current] = vector<Actor*>();
			// new stuff above this
			Field::FieldItem item = f.getContentsOf(j, i); // changed to i, j
			 // NOTE TO SELF: make darn sure this constructs things in the right place
			// this loop ignores anthills because they have already been allocated above. they're separated because it makes
			// the code above a little less ugly
			if (item == Field::anthill0 || item == Field::anthill1 || item == Field::anthill2 || item == Field::anthill3)
				continue;
			else
				identifyAndAllocate(item, current);
		}
	}
	
	m_ticksElapsed = 0;
	m_currentAnthillLeader = -1;
	// don't forget about allocating ants here

	return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move()
{
	m_ticksElapsed++;

	if (!actorMap.empty())
	{
		// keep an iterator to previous or next
		// maybe have a special call if something was just created?
		auto mapIterator = actorMap.begin();
		// I guess here's as good a place as any to explain what this "outOfBounds" coordinate is and what it does.
		// Because the spec requires us to keep everything in a single STL data structure, I have this coordinate
		// reserved in the map for actors added during the course of the game. I do this so as to avoid the possibility
		// of an actor's doSomething function not being called. It also eliminates problems with vector iterators getting screwy.
		const Coord outOfBounds;
		while (mapIterator != actorMap.end()) // this may not work as expected
		{
			if (mapIterator->second.empty())
			{
				// do nothing
			}
		/*	else if (mapIterator->first == outOfBounds) // shouldn't ever hit this or it means that something is not going right and stuff is staying in a weird map location
			{
				mapIterator++;
			}*/
			else
			{
				auto vecIterator = mapIterator->second.begin();

				// what if I held a vector of things to be added to the grid? Just so I could keep track of them more easily?
				/*for (int vecIterator = 0; vecIterator < mapIterator->second.size(); vecIterator++) // this implementation will require some more work later. and right now!!! usd to be a range-based for loop
				{
					int oldX = (mapIterator->second)->getX(), oldY = vecIterator->getY();
					Coord old(oldX, oldY);
					if (!vecIterator->isDead() && !vecIterator->hasDoneSomething(m_ticksElapsed))
					{
						vecIterator->doSomething();
						vecIterator->updateLastMove(m_ticksElapsed);
					}
					if (vecIterator->getX() != oldX || vecIterator->getY() != oldY)
					{
						actorsToBeMoved[old].push_back(vecIterator);
						vecIterator = nullptr;
						//Coord current(vecIterator->getX(), vecIterator->getY());
						//addPtrInMappedVector(current, vecIterator);
						//removePtrInMappedVector(old, vecIterator);
					}
				}*/
				for (size_t vecIterator = 0; vecIterator < mapIterator->second.size(); vecIterator++) // this was a pain. hope it works. iterating through like a normal array.
				{
					int oldX = (mapIterator->second[vecIterator])->getX(), oldY = (mapIterator->second[vecIterator])->getY();
					Coord old(oldX, oldY);
					if (!(mapIterator->second[vecIterator])->isDead() && !(mapIterator->second[vecIterator])->hasDoneSomething(m_ticksElapsed))
					{
						(mapIterator->second[vecIterator])->doSomething();
						(mapIterator->second[vecIterator])->updateLastMove(m_ticksElapsed);
					}
					if ((mapIterator->second[vecIterator])->getX() != oldX || (mapIterator->second[vecIterator])->getY() != oldY)
					{
						//actorsToBeMoved[old].push_back(vecIterator);
						//vecIterator = nullptr;
						Coord current((mapIterator->second[vecIterator])->getX(), (mapIterator->second[vecIterator])->getY());
						addPtrInMappedVector(current, (mapIterator->second[vecIterator]));
						removePtrInMappedVector(old, (mapIterator->second[vecIterator]));
						// because everything after will be shifted left, i need this to make sure that this captures the one that shifts to what used to be held by the actor ptr that was move
						vecIterator--;
					}
				}
				/*for (auto iter = mapIterator->second.begin(); iter != mapIterator->second.end(); iter++)
				{
					if (mapIterator->second[iter] == nullptr)
					{

						removePtrInMappedVector(old, vecIterator);
					}
				}
				for (auto VecIterator : mapIterator->second)
				{
					if (mapIterator->second(vecIterator) == nullptr)
					if (vecIterator == nullptr)
						removePtrInMappedVector(old, vecIterator);
				}*/
			}
			mapIterator++;
		}
		/*for (auto actorPtr : actorsToBeAdded) // delete this if I can confirm that actorPtr works fine
		{
			actorPtr->doSomething();
			Coord current(actorPtr->getX(), actorPtr->getY());
			addPtrInMappedVector(current, actorPtr);
		}
		actorsToBeAdded.clear();*/
		for (auto actorPtr : actorMap[outOfBounds])
		{
			actorPtr->doSomething();
			Coord current(actorPtr->getX(), actorPtr->getY());
			addPtrInMappedVector(current, actorPtr);
		}
		actorMap[outOfBounds].clear();
		removeDeadSimulationObjects();
	}
	setDisplayText();
	determineCurrentLeader();
	//determineCurrentLeader() is not properly implemented at this point. fix later.
	if (m_ticksElapsed == 2000)
	{
		if (m_currentAnthillLeader != -1)
		{
			setWinner(getWinningAntName()); // determine winner
			return GWSTATUS_PLAYER_WON;
		}
		else
			return GWSTATUS_NO_WINNER;
	}
	// the simulation is not yet over, continue!
	return GWSTATUS_CONTINUE_GAME;
	// after each any colony produces an ant, compare with all others to see which one has the most. so I should probably store that information somewhere...
}

void StudentWorld::cleanUp()
{
	while (!actorMap.empty())
	{
		auto iter = actorMap.begin();
		//auto vec = iter->second.begin();
		while (!iter->second.empty())
		{
			Actor* ptr = iter->second.back();
			delete ptr;
			iter->second.pop_back();
		}
		actorMap.erase(iter);
	}
	//cerr << "Hello from cleanup" << endl;

	//for (int i = m_numCompilers; i >= 0; i++) // let's hope this works
	for (int i = 0; i < m_numCompilers; i++)
	{
		m_numCompilers--;
		delete compilerArr[i];
	}
	m_numCompilers = 0;
}

// be aware of how this is used. memory leak if actor pointer is not deleted or stored in another vector
bool StudentWorld::removePtrInMappedVector(Coord coordinates, Actor* act) // delete coordinate if vector will become empty?
{
	auto beginIterator = actorMap[coordinates].begin();
	auto endIterator = actorMap[coordinates].end();
	auto foundIterator = find(beginIterator, endIterator, act);
	if (foundIterator == endIterator)
		return false;
	else
	{
		actorMap[coordinates].erase(foundIterator);
		//if (actorMap[coordinates].size() == 0)  // if addPtrInMappedVector is dumb, remove this. be aware that this will slow the game down over time if removed.
		//	actorMap.erase(coordinates);
		return true;
	}
}

bool StudentWorld::identifyAndAllocate(Field::FieldItem item, Coord location, int colonyNum, Compiler* compiler)
{
	switch (item)
	{
	case (Field::empty): return true;
	case (Field::anthill0):   // TODO: push_back ptr to anthill0
	case (Field::anthill1):
	case (Field::anthill2):
	case (Field::anthill3):
	{
		Actor* anty = new Anthill(location.getCol(), location.getRow(), this, compiler, colonyNum);
		actorMap[location].push_back(anty);
		return true;
	}
	// cases left to deal with: food, grasshopper, water, rock, poison
	case (Field::food):
	{
		Actor* foodie = new Food(location.getCol(), location.getRow(), FOOD_SIZE_AT_START, this);
		actorMap[location].push_back(foodie);
		return true;
	}
	case (Field::grasshopper):
	{
		Actor* grassy = new BabyGrasshopper(location.getCol(), location.getRow(), this);
		actorMap[location].push_back(grassy);
		return true;
	}
	case (Field::water):
	{
		Actor* watery = new WaterPool(location.getCol(), location.getRow(), this);
		actorMap[location].push_back(watery);
		return true;
	}
	case (Field::rock):
	{
		Actor* rocky = new Pebble(location.getCol(), location.getRow(), this);
		actorMap[location].push_back(rocky);
		return true;
	}
	case (Field::poison):
	{
		Actor* badNews = new Poison(location.getCol(), location.getRow(), this);
		actorMap[location].push_back(badNews);
		return true;
	}
	default:
	{
		return false;
	}
	}
}

// need to pay attention to restrictions on adding things for pheromones and food
// default arguments are foodQuantity = 0, colonyNum = -1, compiler = nullptr
void StudentWorld::addNewDuringGame(int thingID, Coord location, int foodQuantity, int colonyNum, Compiler* compiler)
{
	Coord outOfBounds; // but in a good way (I hope). sets location to (-1, -1)
	switch (thingID)
	{
	case (IID_PHEROMONE_TYPE0) :
	case (IID_PHEROMONE_TYPE1) :
	case (IID_PHEROMONE_TYPE2) :
	case (IID_PHEROMONE_TYPE3) :
	{
		Actor* actorPtr = getPtrToThingAtCurrentSquare(thingID, location);
		// need to check if there are others of the exact same type on the square
		if (actorPtr == nullptr)  // should new Pheromones and new Adult Grasshoppers have the chance to move during the same turn during which they're created?
		{
			Actor* newPheromonePtr = new Pheromone(thingID, location.getCol(), location.getRow(), this, thingID - IID_PHEROMONE_TYPE0);
			// might want to uncomment the line below later... just depends on how I end up dealing with new things
			//addPtrInMappedVector(location, newPheromonePtr); // adds to the back of the current mapped vector
			//newPheromonePtr->doSomething(); // also might not be exactly right
			//actorsToBeAdded.push_back(newPheromonePtr); // so i don't have to use this vector
			//Coord outOfBounds; // but in a good way (I hope)
			addPtrInMappedVector(outOfBounds, newPheromonePtr);
		}
		else
		{
			// dynamic cast the actor pointer to pheromone pointer
			//Pheromone* pherPtr = dynamic_cast<Pheromone*> (actorPtr);
			actorPtr->addHealth(256);
			//pherPtr->addHealth(256);
		}
		return;
	}

	case (IID_FOOD) :
	{
		Actor* actorPtr = getPtrToThingAtCurrentSquare(thingID, location);
		// need to check if there are others of the exact same type on the square
		if (actorPtr == nullptr)
		{
			Actor* newFoodPtr = new Food(location.getCol(), location.getRow(), foodQuantity, this);
			//addPtrInMappedVector(location, newFoodPtr); // adds to the back of the current mapped vector
			//newFoodPtr->doSomething();
			//actorsToBeAdded.push_back(newFoodPtr);
			addPtrInMappedVector(outOfBounds, newFoodPtr);
		}
		else
		{
			// dynamic cast the actor pointer to pheromone pointer
			//Food* foodPtr = dynamic_cast<Food*> (actorPtr);
			//foodPtr->addHealth(foodQuantity);
			actorPtr->addHealth(foodQuantity);
		}
		return;
	}

	case (IID_ADULT_GRASSHOPPER):    // the deletion of the baby grasshopper will presumably get done before this.
	{
		Actor* hopsPtr = new AdultGrasshopper(location.getCol(), location.getRow(), this);
		//addPtrInMappedVector(location, hopsPtr);
		//hopsPtr->doSomething();
		//actorsToBeAdded.push_back(hopsPtr);
		addPtrInMappedVector(outOfBounds, hopsPtr);
		return;
	}

	case (IID_ANT_TYPE0) : 
	case (IID_ANT_TYPE1) :
	case (IID_ANT_TYPE2) :
	case (IID_ANT_TYPE3) : // complete this implementation very, very soon.
	{
		// looks weird syntactically, but thingID is the same as colony number for ants
		Actor* newAntPtr = new Ant(thingID, location.getCol(), location.getRow(), this, compiler, thingID);
		// alternatively, added to first vector.
		//newAntPtr->doSomething();
		// update ant rankings and call anthill comparison thing
		switch (thingID)
		{
		case (IID_ANT_TYPE0): m_ants[0]++; break;
		case (IID_ANT_TYPE1): m_ants[1]++; break;
		case (IID_ANT_TYPE2): m_ants[2]++; break;
		case (IID_ANT_TYPE3): m_ants[3]++; break;
		}
		//actorsToBeAdded.push_back(newAntPtr);
		addPtrInMappedVector(outOfBounds, newAntPtr);
		return;
	}

	default: { return; }
	}
}

// fix this junk.
int StudentWorld::howManyAreThereAtCurrentSquare(int thingID, Coord location)
{
	int thingCount = 0;
	// need to check (-1, -1) for something that is secretly, but only very temporarily not mapped at the correct location
	Coord outOfBounds; // refer to notes earlier about what this means
	if (!actorMap[outOfBounds].empty())
	{
		for (auto vecPtr : actorMap[outOfBounds])
		{
			if (identifyByThingID(vecPtr) == thingID)
			{
				thingCount++;
			}
		}
	}
	map<Coord, vector<Actor*>>::iterator iter = actorMap.find(location);
	if (iter != actorMap.end())
	{
		for (auto vecPtr : iter->second)
		{
			//if (vecPtr->getActorID() == thingID)
			if (identifyByThingID(vecPtr) == thingID)
			{
				thingCount++;
			}
		}
	}
	return thingCount;
}

/*int StudentWorld::howManyAreThereAtCurrentSquare(int thingID, Coord location, Actor* &actorPtr)
{
	int thingCount = 0;
	// need to check (-1, -1) for something that is secretly, but only very temporarily not mapped at the correct location
	Coord outOfBounds; // refer to notes earlier about what this means
	if (!actorMap[outOfBounds].empty())
	{
		for (auto vecPtr : actorMap[outOfBounds])
		{
			if (identifyByThingID(vecPtr) == thingID)
			{
				thingCount++;
				if (thingCount == 1)
					actorPtr = vecPtr;
			}
		}
	}
	map<Coord, vector<Actor*>>::iterator iter = actorMap.find(location);
	if (iter != actorMap.end())
	{
		for (auto vecPtr : iter->second)
		{
			//if (vecPtr->getActorID() == thingID)
			if (identifyByThingID(vecPtr) == thingID)
			{
				thingCount++;
				if (thingCount == 1)
					actorPtr = vecPtr;
			}
		}
	}
	if (thingCount == 0)
		actorPtr = nullptr;
	return thingCount;
}*/

void StudentWorld::removeDeadSimulationObjects()
{
	if (!actorMap.empty())
	{
		for (auto coordIterator = actorMap.begin(); coordIterator != actorMap.end(); coordIterator++)
		{
			if (coordIterator->second.empty())
			{
				// do nothing
			}
			else
			{
				//auto vecIterator = coordIterator->second;

				//for (auto vecIterator = coordIterator->second.begin(); vecIterator != coordIterator->second.begin();) 
				// this implementation will require some more work later. aka NOW!!! Is it better?
				for (size_t i = 0; i < coordIterator->second.size(); i++)
				{
					Actor *current = coordIterator->second[i];
					if (current->isDead())
					{
						int actorType = identifyByThingID(current);
						if (actorType == IID_ANT_TYPE0) // if it's an ant, then you'd better decrement ant count for the right anthill
							m_ants[0]--;
						else if (actorType == IID_ANT_TYPE1)
							m_ants[1]--;
						else if (actorType == IID_ANT_TYPE2)
							m_ants[2]--;
						else if (actorType == IID_ANT_TYPE3)
							m_ants[3]--;

						Coord toDelete(current->getX(), current->getY());
						removePtrInMappedVector(toDelete, current);
						i--; // since it's a vector and you're deleting stuff, there shouldn't be an issue with reallocation. at least i hope.
						delete current;
					}
				}
			}
		}
	}
}

// maybe it would be better to determine current leader some other way
// needs to be called every time an ant is generated or dies.
void StudentWorld::determineCurrentLeader()
{
	// this is not the correct implementation. but at least it compiles.
	m_currentAnthillLeader = 0;
	/*if (m_ticksElapsed == 1)
		return;
	if (m_currentAnthillLeader == -1) // fix this. I'm stupid. the leader at this stage is whoever has more than 5 ants.
	{
		int maxAnts = m_ants[0];
		int leader = 0;
		for (int i = 1; i < m_numAnthills; i++)
		{
			if (m_ants[i] > maxAnts)
			{
				maxAnts = m_ants[i];
				leader = i;
			}
		}
		bool isGreatest = true;
		for (int i = 0; i < m_numAnthills; i++)
		{
			if (i != leader)
			{
				if (m_ants[i] == maxAnts)
				{
					isGreatest = false;
					break;
				}
			}
		}
		if (isGreatest)
		{
			m_currentAnthillLeader = leader;
		}
	}
	else
	{
		for (int i = 0; i < m_numAnthills; i++) // well darn. what if something goes down...
		{
			if (m_ants[i] > m_ants[m_currentAnthillLeader])
			{
				m_currentAnthillLeader = i;
			}
		}
	}*/
}

void StudentWorld::setAnthillNames()
{
	string antColonyName;
	for (auto iter : actorMap)
	{
		for (auto vecIter : actorMap[iter.first])
		{
			if (identifyByThingID(vecIter) == IID_ANT_HILL)
			{
				//auto anthillPtr = dynamic_cast<Anthill*>(vecIter);
				//antColonyName = anthillPtr->getCompiler()->getColonyName();
				int colonyNum = vecIter->getColonyNumber();
				antColonyName = compilerArr[colonyNum]->getColonyName();
				anthillNames[colonyNum] = antColonyName;
			}
		}
	}
}

string StudentWorld::getWinningAntName()
{
	return anthillNames[m_currentAnthillLeader];
}

void StudentWorld::setDisplayText()
{
	int ticks = getTicksElapsed();
	int antsAnt0, antsAnt1, antsAnt2, antsAnt3;
	int winningAntNumber;
	antsAnt0 = m_ants[0]; // is this a problem if I have fewer than four ants?
	antsAnt1 = m_ants[1];
	antsAnt2 = m_ants[2];
	antsAnt3 = m_ants[3];
	winningAntNumber = m_currentAnthillLeader;
	// Create a string from your statistics, of the form:
	// Ticks: 1134 - AmyAnt: 32 BillyAnt: 33 SuzieAnt*: 77 IgorAnt: 05
	string s = formatBetter(ticks, antsAnt0, antsAnt1, antsAnt2, antsAnt3, winningAntNumber);
	// Finally, update the display text at the top of the screen with your
	// newly created stats
	setGameStatText(s); // calls our provided GameWorld::setGameStatText
}

string StudentWorld::formatBetter(int ticks, int antOne, int antTwo, int antThree, int antFour, int leadingAnt)
{
	ostringstream formattedString;
	//string tickString = stoi(ticks);
	formattedString << "Ticks : " << setw(4) << 2000 - ticks << " -";
	for (int i = 0; i < m_numAnthills; i++)
	{
		// this annoying step is necessitated by how the game is formatted in the simulation given.
		// adds only only space before the first contestant's name
		if (i == 0)
			formattedString << " ";
		else
			formattedString << "  ";
		if (i == leadingAnt)
			formattedString << anthillNames[i] << "*: ";
		else
			formattedString << anthillNames[i] << ": ";
		if (m_ants[i] < 10)
			formattedString << "0" << m_ants[i];
		else
			formattedString << m_ants[i];
	}

	// remove below later

	/*if (!actorMap.empty())
		cout << "not empty";
	else
		cout << "empty";*/

	string goodString = formattedString.str();
	return goodString;
}

bool StudentWorld::eatFoodAtCurrentSquare(Coord current, int amount, Actor *eater)
{
	Actor* foodPtr = getPtrToThingAtCurrentSquare(IID_FOOD, current);
	if (foodPtr == nullptr)
		return false;
	else
	{
		int amountEaten = foodPtr->reduceHP(amount);
		if (amountEaten <= 0)
			return false;
		else
		{
			eater->addHealth(amountEaten);
			return true;
		}
	}
}

Actor* StudentWorld::getPtrToThingAtCurrentSquare(int thingID, Coord location)
{
	//map<Coord, vector<Actor*>>::iterator iter = actorMap.find(location);
	//if (iter != actorMap.end())
	//{
	for (auto vecPtr : actorMap[location])
	{
		//if (vecPtr->getActorID() == thingID)
		if (identifyByThingID(vecPtr) == thingID)
		{
			return vecPtr;
		}
	}
	//}
	Coord outOfBounds;
	for (Actor* actorPtr : actorMap[outOfBounds])
	{
		if (identifyByThingID(actorPtr) == thingID)
		{
			return actorPtr;
		}
	}
	return nullptr;
}

int StudentWorld::identifyByThingID (Actor* actor) const
{
	int thingID = -1;
	if (actor->canBlockPath())
		thingID = IID_ROCK;
	else if (actor->canPoison())
		thingID = IID_POISON;
	else if (actor->canBeEaten())
		thingID = IID_FOOD;
	else if (actor->canStun())
		thingID = IID_WATER_POOL;
	else if (actor->canFly())
		thingID = IID_ADULT_GRASSHOPPER;
	else if (actor->canEat() && !actor->canBePoisoned()) // I might need to do more in another function to find out whose anthill it is
		thingID = IID_ANT_HILL;
	else if (actor->getColonyNumber() == -1)
		thingID = IID_BABY_GRASSHOPPER;
	else if (actor->canBeStunned())
		thingID = actor->getColonyNumber(); // means it's an ant
	else if (!actor->canBePoisoned())
		thingID = actor->getColonyNumber() + IID_PHEROMONE_TYPE0; // for various types of pheromones
		/*const int IID_ANT_TYPE0 = 0;	// up to four different competitor colonies
	const int IID_ANT_TYPE1 = 1;
	const int IID_ANT_TYPE2 = 2;
	const int IID_ANT_TYPE3 = 3;
	-const int IID_ANT_HILL = 4;
	-const int IID_POISON = 5;
	-const int IID_FOOD = 6;
	-const int IID_WATER_POOL = 7;
	-const int IID_ROCK = 8;
	-const int IID_BABY_GRASSHOPPER = 9;
	-const int IID_ADULT_GRASSHOPPER = 10;
	const int IID_PHEROMONE_TYPE0 = 11;
	const int IID_PHEROMONE_TYPE1 = 12;
	const int IID_PHEROMONE_TYPE2 = 13;
	const int IID_PHEROMONE_TYPE3 = 14;*/
	// have a check for it returning -1 so I know that something went wrong.
	//if (actor->getActorID() != thingID)
	//	cerr << "Something went wrong with identify function" << "actorID: " << actor->getActorID() << "thingID: " << thingID << endl;
	return thingID;
}
/*
void StudentWorld::poisonAllAtCurrentSquare(Coord current)
{
	for (Actor* actorPtr : actorMap[current])
	{
		if (actorPtr->canBePoisoned() && !actorPtr->hasBeenAttackedHere(current))
		{
			actorPtr->bePoisoned();
		}
	}
	Coord outOfBounds;
	for (Actor* actorPtr : actorMap[outOfBounds])
	{
		if (actorPtr->getX() == current.getCol() && actorPtr->getY() == current.getRow())
		{
			if (actorPtr->canBePoisoned() && !actorPtr->hasBeenAttackedHere(current))
			{
				actorPtr->bePoisoned();
			}
		}
	}
}

void StudentWorld::stunAllAtCurrentSquare(Coord current)
{
	for (Actor* actorPtr : actorMap[current])
	{
		if (actorPtr->canBeStunned() && !actorPtr->hasBeenAttackedHere(current))
		{
			actorPtr->bePoisoned();
		}
	}
	Coord outOfBounds;
	for (Actor* actorPtr : actorMap[outOfBounds])
	{
		if (actorPtr->getX() == current.getCol() && actorPtr->getY() == current.getRow())
		{
			if (actorPtr->canBePoisoned() && !actorPtr->hasBeenAttackedHere(current))
			{
				actorPtr->bePoisoned();
			}
		}
	}
}*/

bool StudentWorld::pathBlocked(Coord location)
{
	for (Actor* maybeRocky : actorMap[location])
	{
		if (maybeRocky->canBlockPath())
			return true;
	}
	return false;
}

// make sure that implementation of pheromone constructor makes sense