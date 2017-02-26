#include "Actor.h"
#include "StudentWorld.h"
#include "GraphObject.h"
//#include <iostream> // remove this after use
#include <cstdlib>

int Actor::getCurrentMove() const
{ 
	return ptrToWorld->getTicksElapsed(); 
}
/*
// need to have this in the base class because ptrToWorld is a private member of Actor
// intended for ants and grasshoppers
// should not exist
bool Actor::die()
{
	if (!isDead())
		return false;
	else
	{
		Coord currentLocation(this->getX(), this->getY());
		ptrToWorld->addNewDuringGame(IID_FOOD, currentLocation, 100); // add food to map
		return true;
	}
}
bool EnergeticActor::die() // everything that has energy
{ 
	return isDead(); 
}*/

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

/*bool ImmortalActor::die() // pebble, poison, water pool
{
	return false;
}

bool SuperAnt::die()
{
	if (!canBePoisoned()) // pheromone and anthill
		return EnergeticActor::die();
	else // ant
		return Insect::die();
}*/

/*// should only be called if it is dead and can die. if it can't, return false.
bool Insect::die() // wait should this be a thing?
{
	if (!hasEnergyOrHealth() || !isDead()) // something should only die if it's dead
		return false;
	else
	{
		reduceHP(9000);
		if (!canEat()) // pheromone
			return true;
		else if (!canBePoisoned()) // anthill
			return true;
		else
		{

			return true; // not totally correct at this point, but oh well.
			// add Food to square
		}
	}
}*/

/*void Actor::numberTimesCalled() // delete this after confirming that everything works properly.
{
	if (getCurrentMove() != m_timesCalled)
	{
		std::cerr << "IMAGE ID: " << getActorID() << std::endl;
		std::cerr << "Ticks elapsed: " << getCurrentMove() << std::endl;
		std::cerr << "DoSomething calls: " << m_timesCalled << std::endl;
	}
}*/

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
			int maxToAdd = m_maxLimit - amount;
			if (maxToAdd < 0)
				maxToAdd = 0;
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

/*// decreases amount of food left. if amount of food desired is more than amount remaining,
// amount of food left will be set to 0 (effectively dead)
bool Food::beEaten(int amount)
{
	if (isDead)
		return false;
	else
	{
		reduceHP(amount); // don't need to check to see if the amount exceeds 
		return true;
	}
}*/

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
// may not need this.
bool Grasshopper::manageHealth() // move this to EnergeticActor
{
	decrementHP();
	if (getHealth() <= 0)
	{
		die();
		return false;
	}
	return true;
}

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

// so the format above suggests that I should have a pure virtual isStunned common among the insect class
// should ants and adult grasshoppers inherit from baby grasshoppers?

bool BabyGrasshopper::doAdultOrImmatureThings()
{
	return evolve();
}

// returns true if it evolves
bool BabyGrasshopper::evolve()
{
	if (getHealth() >= 1600)
	{
		reduceHP(9000); // effectively sets baby grasshopper to dead.
		die();
		Coord currentLocation(getX(), getY());
		getPtrToWorld()->addNewDuringGame(IID_ADULT_GRASSHOPPER, currentLocation); // add adult grasshopper to map
		return true;
	}
	return false;
}

bool AdultGrasshopper::doAdultOrImmatureThings()
{
	Coord location(getX(), getY());
	if (attackEnemy(getColonyNumber(), location, 50))
		return true;
	else
		return fly();
}

bool AdultGrasshopper::fly()
{
	if (randInt(0, 9) == 0)
	{
		int numPossibleSquares = findNumberOfOpenSquares();
		if (numPossibleSquares == 0)
			return false;
		else
		{
			int squareNum = randInt(0, numPossibleSquares - 1);
			Coord toJump = findNthOpenSquare(squareNum);
			Coord outOfBounds;
			if (toJump == outOfBounds)
				return false;
			else
			{
				moveTo(toJump.getCol(), toJump.getRow());
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
			Coord xOne(getX() + movesX, getY() + posmovesY);
			if (isValidSquare(xOne) && !getPtrToWorld()->pathBlocked(xOne))
			{
				if (spaceCount == n)
					return xOne;
				spaceCount++;
			}
			int negmovesY = -sum + abs(movesX); // distance below y
			Coord xTwo(getX() + movesX, getY() + negmovesY);
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

void AdultGrasshopper::biteBack()
{
	if (randInt(0, 1))
	{
		Coord current(getX(), getY());
		attackEnemy(getColonyNumber(), current, 50);  // need to add info on where it is.
	}
}

void Insect::updatePreviousLocation()
{
	Coord newLocation(getX(), getY());
	m_previousLocation = newLocation;
}

void Insect::updateLastStunnedLocation(const Coord &newStunLocation)
{
	m_locationLastStunned = newStunLocation;
	// not sure if this is safe, but it ensures that the function that checks for if an insect can be stunned will return false
	m_previousLocation = newStunLocation;
}

// also it seems like there's some issue with isDead()
// fix function below.

bool Insect::isAttackable(int colonyNumber, Actor* self)
{
	if (isDead())
		return false;
	else if (self == this)
		return false;
	else if (getColonyNumber() == -1)
		return true;
	else
		return colonyNumber != getColonyNumber(); 
}

// returns true if it attacks an enemy. otherwise, returns false. 
// make sure that baby grasshoppers never call this.
bool Insect::attackEnemy(int colonyNumber, Coord location, int damage)
{
	return getPtrToWorld()->biteEnemy(colonyNumber, location, damage, this);
}

// HEY! ADULT GRASSHOPPERS NEED A BITE BACK FUNCTION! so in StudentWorld function that does damage, if it's an adult grasshopper that gets bitten, it needs to respond.

// need evolve function
// performs the baby grasshopper's specified actions
void Grasshopper::doSomething() // move a lot of this to Grasshopper
{
	/*incrementTimesCalled();
	numberTimesCalled();*/
	Coord currentLocation(getX(), getY());
	updatePreviousLocation(); // should maybe get put in insect's doSomething
	if (!manageHealth()) // if it returns false, doSomething should also return.
		return;
	else if (!manageDamage())
		return;
	else if (doAdultOrImmatureThings()) // adult grasshoppers and ants need findEnemy(int colonyNumber, int location). used to be evolve
	{
		if (!isDead())
			goToSleep();
		return;
	}
	else if (getPtrToWorld()->eatFoodAtCurrentSquare(currentLocation, 200, this) && randInt(0, 1))
	{                                        // split things up into void performMovement
		/*goToSleep();
		return;*/
		// should go straight to the end
	}
	else if (areWeThereYet()) // checks if there's no more distance to travel
	{
		addMoreDistance();
		setDirection(getRandomDirection()); //this is how it should ultimately be implemented. the line below is purely for testing.
	}											//setDirection(up);
	else if (!moveInCurrentDirection())
		doneTraveling();
	else
		oneStepCloser();
	goToSleep();
		/*if (getMovesInactive() > 0)
		{
			decrementMovesInactive();
			return;
		}
		if (isStunned())
		{
			decrementMovesStunned();
			return;
		}
		else if (isAsleep())
		{
			decrementMovesAsleep();
			return;
		}*/
}

void Poison::attackAllActorsAtCurrentSquare()
{
	Coord current(getX(), getY());
	getPtrToWorld()->poisonAllAtCurrentSquare(current);
}

void WaterPool::attackAllActorsAtCurrentSquare()
{
	Coord current(getX(), getY());
	getPtrToWorld()->stunAllAtCurrentSquare(current);
}