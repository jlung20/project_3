#include "Actor.h"
#include "StudentWorld.h"
#include "GraphObject.h"
#include "Compiler.h"
#include <string>
//#include <iostream> // remove this after use
#include <cstdlib>

// HEY! ADD COMMENTS!!!
StudentWorld* Actor::getPtrToWorld()
{
	return ptrToWorld;
}
int Actor::getCurrentMove() const
{
	return ptrToWorld->getTicksElapsed();
}

bool Insect::die() // all insects
{
	if (!isDead())
		return false;
	else
	{
		Coord currentLocation(this->getX(), this->getY());
		getPtrToWorld()->addNewDuringGame(IID_FOOD, currentLocation, 100); // add food to map
		return true;
	}
}

// adds amount to HP unless there is a limit, in which case it only
void EnergeticActor::addHealth(int amount) {
	if (isDead() && canEat()) // for the case that it's a dead anthill or grasshopper or ant so they can't revive
		return;
	else // if it's anything else, add as much health as fulfills other restrictions
	{
		if (m_maxLimit == -1)
			m_healthPoints += amount;
		else
		{
			int maxToAdd = m_maxLimit - m_healthPoints;
			amount < maxToAdd ? m_healthPoints += amount : m_healthPoints += maxToAdd;
		}
	}
}

int EnergeticActor::reduceHP(int amount)  // returns the amount by which HP was reduced
{
	if (isDead())
		return 0;
	int maxReduction = m_healthPoints - amount;
	if (maxReduction < 0)
	{
		int originalHealth = m_healthPoints;
		m_healthPoints = 0;
		return originalHealth;
	}
	else
	{
		m_healthPoints -= amount;
		return amount;
	}
}

void Anthill::doSomething()
{
	Coord current(getX(), getY());
	if (!manageHealth())
		return;
	else if (getPtrToWorld()->eatFoodAtCurrentSquare(current, 10000, this) > 0)
		return;
	else if (getHealth() >= 2000) // still need to finish up determining the leader.
	{
		// StudentWorld takes care of adjusting its ant count and leader
		getPtrToWorld()->addNewDuringGame(getColonyNumber(), current, 0, getColonyNumber(), getCompiler()); // make sure on the other side that compiler isn't nullptr
		reduceHP(1500);
	}
}

// returns if it's possible to move to a certain location
bool Insect::canMove(const Coord &attemptedLocation)
{
	//Actor* uselessPtr;
	return !getPtrToWorld()->pathBlocked(attemptedLocation); // remove all of the things of this form. changed from identify actors at current square
}

// the following move functions are pretty self-explanatory
// it is worth noting, however, that these functions don't decrement number of moves
// in a certain direction
bool Insect::moveInCurrentDirection()
{
	Direction currentDir = getDirection();
	switch (currentDir)
	{
	case up : { return moveUp(); }
	case down : { return moveDown(); }
	case right: { return moveRight(); }
	case left: { return moveLeft(); }
	default: { return false; }
	}
}

// I decided against combining the following four functions because I think it's slightly more readable
// with them being separated.
bool Insect::moveUp()
{
	Coord desiredLocation(getX(), getY() + 1);
	if (canMove(desiredLocation))
	{
		moveTo(desiredLocation.getCol(), desiredLocation.getRow());
		return true;
	}
	else
		return false;
}

bool Insect::moveDown()
{
	Coord desiredLocation(getX(), getY() - 1);
	if (canMove(desiredLocation))
	{
		moveTo(desiredLocation.getCol(), desiredLocation.getRow());
		return true;
	}
	else
		return false;
}

bool Insect::moveLeft()
{
	Coord desiredLocation(getX() - 1, getY());
	if (canMove(desiredLocation))
	{
		moveTo(desiredLocation.getCol(), desiredLocation.getRow());
		return true;
	}
	else
		return false;
}

bool Insect::moveRight()
{
	Coord desiredLocation(getX() + 1, getY());
	if (canMove(desiredLocation))
	{
		moveTo(desiredLocation.getCol(), desiredLocation.getRow());
		return true;
	}
	else
		return false;
}

bool Insect::canBeStunnedAtCurrentSquare(const Coord &current) const
{
	if (!(current == m_locationLastStunned)) // this branch checks where the insect was last stunned
		return true; // if not at current location, return that it can be stunned
	else // this branch checks whether the insect was stunned here and has not moved since
		return !(m_previousLocation == current);
}

// returns false if dead and should doSomething should stop. else, returns true.
bool HungryActor::manageHealth()
{
	decrementHP();
	return getHealth() > 0;
}

// returns false and decrement moves inactive if insect is not supposed to move; else, return true.
bool Insect::manageDamage()
{
	if (getMovesInactive() > 0)
	{
		decrementMovesInactive();
		return false;
	}
	else
		return true;
}

void Ant::doSomething()
{
	updatePreviousLocation();
	if (!manageHealth())
		return;
	else if (!manageDamage())
		return;
	else
	{
		Compiler::Command cmd;
		int commandCounter = 0; // no commands have been executed yet
		Coord current(getX(), getY()); // used for passing in coordinate pairs to various functions
		while(commandCounter < 10) // does there need to be another check for if all instructions have been looked through?
		{
			// get the command from element ic of the vector
			if (!getCompiler()->getCommand(m_instructionCounter, cmd))
			{
				reduceHP(9000); // error - no such instruction!
				return;
			}
			switch (cmd.opcode)
			{
			case Compiler::moveForward:
			{
				++m_instructionCounter;
				if (moveInCurrentDirection())
				{
					m_wasBlocked = false;
					m_wasBitten = false;
				}
				else
				{
					m_wasBlocked = true; // might have been bitten while here so don't change the state of m_wasBitten
				}
				return;
			}
			case Compiler::generateRandomNumber:
				generateRandomNumberUpTo(cmd.operand1);
				++m_instructionCounter; // advance to next instruction
				break;
			case Compiler::faceRandomDirection:
			{
				setRandomDirection(); 
				++m_instructionCounter;
				return; // changes the state of the simulation so return
			}
			case Compiler::bite:
			{
				attackEnemy(getColonyNumber(), current, 15);
				++m_instructionCounter;
				return; // same comment as above
			}
			case Compiler::eatFood:
			{
				int healthToAdd = reduceHeldFood(100);
				++m_instructionCounter;
				if (healthToAdd > 0)
					addHealth(healthToAdd);
				return; // an attempt to eat food counts as a change in the state of the simulation
			}
			case Compiler::pickupFood:
			{
				pickupFood(current);
				++m_instructionCounter;
				return; // same comment as above for pickupFood
			}
			case Compiler::dropFood:
			{
				int amountToDrop = reduceHeldFood(1800); // reduces amount of food held by as much as possible
				++m_instructionCounter;
				if (amountToDrop > 0)
					getPtrToWorld()->addNewDuringGame(IID_FOOD, current, amountToDrop);
				return;
			}
			case Compiler::emitPheromone:
			{
				addPheromone(current);
				++m_instructionCounter;
				return;
			}
			case Compiler::rotateClockwise:
			{
				rotateClockwise();
				++m_instructionCounter;
				return;
			}
			case Compiler::rotateCounterClockwise:
			{
				rotateCounterClockwise();
				++m_instructionCounter;
				return;
			}
			case Compiler::goto_command:
				m_instructionCounter = stoi(cmd.operand1);
				break;
			case Compiler::if_command:
				if (conditionFulfilled(cmd, current))
					m_instructionCounter = stoi(cmd.operand2); // goto wherever was in second operand if true
				else
					++m_instructionCounter; // otherwise, just keep moving down instructions
				break;
			}
			commandCounter++;
		}
	}
}

// rotates the ant 90 degrees clockwise
void Ant::rotateClockwise()
{
	Direction dir = getDirection();
	switch (dir)
	{
	case up:
		setDirection(right);
		break;
	case right:
		setDirection(down);
		break;
	case down:
		setDirection(left);
		break;
	case left:
		setDirection(up);
		break;
	}
}

// rotates the ant 90 degrees counterclockwise
void Ant::rotateCounterClockwise()
{
	Direction dir = getDirection();
	switch (dir)
	{
	case up:
		setDirection(left);
		break;
	case right:
		setDirection(up);
		break;
	case down:
		setDirection(right);
		break;
	case left:
		setDirection(down);
		break;
	}
}

bool Ant::conditionFulfilled(Compiler::Command cmd, Coord location)
{
	int condition = stoi(cmd.operand1);
	switch (condition)
	{
	case 0: //i_smell_danger_in_front_of_me
	{ return getPtrToWorld()->dangerAhead(location, getColonyNumber(), getDirection()); }
	case 1: //i_smell_pheromone_in_front_of_me
	{ return getPtrToWorld()->pheromoneAhead(location, getColonyNumber(), getDirection()); }
	case 2: //i_was_bit
	{ return m_wasBitten; }
	case 3: //i_am_carrying_food
	{ return m_foodQuantityHeld != 0; }
	case 4: //i_am_hungry
	{ return getHealth() <= 25; }
	case 5: //i_am_standing_on_my_anthill
	{ return location == m_anthillCoords && getPtrToWorld()->howManyAreThereAtCurrentSquare(IID_ANT_HILL, location) != 0; }
	case 6: //i_am_standing_on_food
	{ return isThereFoodHere(location); }
	case 7: //i_am_standing_with_an_enemy
	{ return isThereEnemyHere(location);}
	case 8: //i_was_blocked_from_moving
	{ return m_wasBlocked; }
	case 9: //last_random_number_was_zero
	{ return m_lastRandInt == 0; }
	default: {return false; }
	}
}

bool Ant::isThereEnemyHere(Coord current)
{
	// what follows is an attempt to compensate for what dangerAhead does
	// it goes one square in the direction dictated; to allow it to check
	// location I want, I have to go one square in the opposite direction
	switch (getDirection())
	{
	case up: 
		current.setRow(current.getRow() - 1);
		break;
	case down:
		current.setRow(current.getRow() + 1);
		break;
	case left:
		current.setCol(current.getCol() + 1);
		break;
	case right:
		current.setCol(current.getCol() - 1);
		break;
	}
	return getPtrToWorld()->dangerAhead(current, getColonyNumber(), getDirection());
}

// if it's able to eat 1 unit of food there and then return it, there must be food there
bool Ant::isThereFoodHere(Coord current)
{
	bool isFood = (getPtrToWorld()->eatFoodAtCurrentSquare(current, 1, this) > 0);
	if (isFood)
	{
		reduceHP(1);
		getPtrToWorld()->addNewDuringGame(IID_FOOD, current, 1);
	}
	return isFood;
}

//reduce the quantity of food held unless or until it reaches 0
int Ant::reduceHeldFood(int amount)
{
	int maxReduced = m_foodQuantityHeld;
	if (amount < maxReduced)
	{
		m_foodQuantityHeld -= amount;
		return amount;
	}
	else
	{
		m_foodQuantityHeld = 0;
		return maxReduced;
	}
}

void Ant::pickupFood(Coord current)
{
	// this looks strange and needlessly complicated. to some degree it is, but it eliminates
	// the need for another function in StudentWorld.
	// so. eat as much food, up to 400 units.
	int amountToEat = 1800 - m_foodQuantityHeld; // only try to eat as much food as there is space for it
	int amountPickedUp;
	if (amountToEat < 400)
		amountPickedUp = getPtrToWorld()->eatFoodAtCurrentSquare(current, amountToEat, this);
	else
		amountPickedUp = getPtrToWorld()->eatFoodAtCurrentSquare(current, 400, this);
	// remove the health gained because that's not really where the food is supposed to end up
	reduceHP(amountPickedUp);
	// add it to quantity held
	addHeldFood(amountPickedUp);
}

// adds to the quantity of food held, until or unless it reaches the maximum limit
void Ant::addHeldFood(int amount)
{
	int maxToAdd = 1800 - m_foodQuantityHeld;
	if (maxToAdd < 0)
		maxToAdd = 0;
	amount < maxToAdd ? m_foodQuantityHeld += amount : m_foodQuantityHeld += maxToAdd;
}

void Ant::generateRandomNumberUpTo(std::string operand)
{
	int upTo = stoi(operand);
	if (upTo <= 0) // just to be safe and have all my bases covered
	{
		m_lastRandInt = 0;
		return;
	}
	m_lastRandInt = randInt(0, upTo);
}

void Ant::addPheromone(Coord current)
{
	getPtrToWorld()->addNewDuringGame(IID_PHEROMONE_TYPE0 + getColonyNumber(), current, 0, getColonyNumber());
}

bool BabyGrasshopper::doAdultOrImmatureThings()
{
	return evolve();
}

// returns true if it evolves
bool BabyGrasshopper::evolve()
{
	if (getHealth() >= 1600) // it's time to evolve
	{
		reduceHP(9000); // effectively sets baby grasshopper to dead.
		Coord currentLocation(getX(), getY());
		getPtrToWorld()->addNewDuringGame(IID_ADULT_GRASSHOPPER, currentLocation); // add adult grasshopper to map
		return true;
	}
	return false;
}

bool AdultGrasshopper::doAdultOrImmatureThings()
{
	Coord location(getX(), getY());
	if (attackEnemy(getColonyNumber(), location, 50)) // tries to bite enemy
		return true;
	else
		return fly(); // tries to fly
}

bool AdultGrasshopper::fly()
{
	if (randInt(0, 9) == 0) // one in ten chance
	{
		int numPossibleSquares = findNumberOfOpenSquares();
		if (numPossibleSquares == 0)
			return false; // can't fly if there's nowhere to fly to
		else
		{
			int squareNum = randInt(0, numPossibleSquares - 1); // pick a random square
			Coord toJump = findNthOpenSquare(squareNum);
			Coord outOfBounds;
			if (toJump == outOfBounds)
				return false;
			else
			{
				moveTo(toJump.getCol(), toJump.getRow()); // jump to that random square
				return true;
			}
		}
	}
	return false;
}

// n ranges from 0 to n-1 where n is number of open squares around
// returns out of bounds coordinate if it can't find the nth open square
Coord AdultGrasshopper::findNthOpenSquare(int n)
{
	int spaceCount = 0;
	for (int sum = 10; sum >= 0; sum--) // amounts to radius of circle
	{
		for (int movesX = -sum; movesX <= sum; movesX++) // distance along x
		{
			int posmovesY = sum - abs(movesX); // distance above y
			Coord xOne(getX() + movesX, getY() + posmovesY); // semicircle part one
			if (isValidSquare(xOne) && !getPtrToWorld()->pathBlocked(xOne))
			{
				if (spaceCount == n) // checks if it's the nth open square
					return xOne;
				spaceCount++;
			}
			int negmovesY = -sum + abs(movesX); // distance below y
			Coord xTwo(getX() + movesX, getY() + negmovesY); // semicircle part two
			if (isValidSquare(xTwo) && !getPtrToWorld()->pathBlocked(xTwo))
			{
				if (spaceCount == n)
					return xTwo;
				spaceCount++;
			}
		}
	}
	Coord outOfBounds;
	return outOfBounds;
}

int AdultGrasshopper::findNumberOfOpenSquares()
{
	int openSpaceCount = 0;
	for (int sum = 10; sum >= 0; sum--) // amounts to radius of circle
	{
		for (int movesX = -sum; movesX <= sum; movesX++) // distance along x
		{
			int posmovesY = sum - abs(movesX); // distance above y
			Coord xOne(getX() + movesX, getY() + posmovesY);
			if (isValidSquare(xOne) && !getPtrToWorld()->pathBlocked(xOne))
				openSpaceCount++;
			int negmovesY = -sum + abs(movesX); // distance below y
			Coord xTwo(getX() + movesX, getY() + negmovesY);
			if (isValidSquare(xTwo) && !getPtrToWorld()->pathBlocked(xTwo))
				openSpaceCount++;
		}
	}
	return openSpaceCount;
}

bool AdultGrasshopper::isValidSquare(Coord square)
{
	int x = square.getCol(), y = square.getRow();
	return x >= 0 && x < VIEW_WIDTH && y >= 0 && y < VIEW_HEIGHT;
}

// called if an adult grasshopper is bitten
void AdultGrasshopper::biteBack()
{
	if (randInt(0, 1)) // 50% chance it will want to bite back
	{
		Coord current(getX(), getY());
		attackEnemy(getColonyNumber(), current, 50);  // attacks a random enemy on the square
	}
}

// used in determining if something can be stunned or not
void Insect::updatePreviousLocation()
{
	Coord newLocation(getX(), getY());
	m_previousLocation = newLocation;
}

// this should make clear 
void Insect::updateLastStunnedLocation(const Coord &newStunLocation)
{
	m_locationLastStunned = newStunLocation;
	m_previousLocation = newStunLocation; // this step ensures that
}

bool Insect::isAttackable(int colonyNumber, Actor* self)
{
	if (isDead()) // no point biting something that's dead
		return false;
	else if (self == this) // don't bite yourself
		return false;
	else if (getColonyNumber() == -1) // can definitely bite a grasshopper
		return true;
	else
		return colonyNumber != getColonyNumber(); // can bite ants from other colonies
}

// returns true if it attacks an enemy. otherwise, returns false. 
bool Insect::attackEnemy(int colonyNumber, Coord location, int damage)
{
	return getPtrToWorld()->biteEnemy(colonyNumber, location, damage, this);
}

// performs grasshopper's functions - doAdultOrImmatureThings does different things for adults and babies
void Grasshopper::doSomething()
{
	Coord currentLocation(getX(), getY());
	updatePreviousLocation(); // should maybe get put in insect's doSomething
	if (!manageHealth()) // if it returns false, doSomething should also return.
		return;
	else if (!manageDamage())
		return;
	else if (doAdultOrImmatureThings()) // adult grasshoppers bite and hop. babies can evolve
	{
		if (!isDead())
			goToSleep();
		return;
	}
	else if (getPtrToWorld()->eatFoodAtCurrentSquare(currentLocation, 200, this) > 0 && randInt(0, 1)) // 50% chance it will want to rest after eating
	{
		// should go straight to the end if the above is true
	}
	else if (areWeThereYet()) // checks if there's no more distance to travel
	{
		addMoreDistance();
		setRandomDirection();
	}
	else if (!moveInCurrentDirection())
		doneTraveling();
	else
		oneStepCloser();
	goToSleep();
}

// the following two poison and stun, respectively, actors that happen to land on their squares
void Poison::attackAllActorsAtCurrentSquare()
{
	Coord current(getX(), getY());
	getPtrToWorld()->attackAllAtCurrentSquare(current, 'p');
}

void WaterPool::attackAllActorsAtCurrentSquare()
{
	Coord current(getX(), getY());
	getPtrToWorld()->attackAllAtCurrentSquare(current, 'w');
}