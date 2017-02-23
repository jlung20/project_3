#include "Actor.h"
#include "StudentWorld.h"
#include "GraphObject.h"
#include <iostream> // remove this after use

int Actor::getCurrentMove() const
{ 
	return ptrToWorld->getTicksElapsed(); 
}

// need to have this in the base class because ptrToWorld is a private member of Actor
// intended for ants and grasshoppers
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
}

bool Insect::die() // all insects
{
	return Actor::die();
}

bool ImmortalActor::die() // pebble, poison, water pool
{
	return false;
}

bool SuperAnt::die()
{
	if (!canBePoisoned()) // pheromone and anthill
		return EnergeticActor::die();
	else // ant
		return Insect::die();
}

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
bool Insect::canMove(Coord attemptedLocation)
{
	//Actor* uselessPtr;
	return getPtrToWorld()->howManyAreThereAtCurrentSquare(IID_ROCK, attemptedLocation) == 0; // remove all of the things of this form.
}

// the following move functions are pretty self-explanatory
// it is worth noting, however, that these functions don't decrement number of moves
// in a certain direction for grasshoppers and the like
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
	Coord desiredLocation(this->getX(), this->getY() + 1);
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
	Coord desiredLocation(this->getX(), this->getY() - 1);
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
	Coord desiredLocation(this->getX() - 1, this->getY());
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
	Coord desiredLocation(this->getX() + 1, this->getY());
	if (canMove(desiredLocation))
	{
		moveTo(desiredLocation.getCol(), desiredLocation.getRow());
		return true;
	}
	else
		return false;
}

// performs the baby grasshopper's specified actions
void BabyGrasshopper::doSomething()
{
	/*incrementTimesCalled();
	numberTimesCalled();*/
	decrementHP();
	if (getHealth() <= 0) // in my implementation, it's not necessary to do anything special to check if it's dead (handled by isDead fn)
	{
		Coord currentLocation(this->getX(), this->getY());
		getPtrToWorld()->addNewDuringGame(IID_FOOD, currentLocation, 100); // add food to map
		return;
	}
	else
	{
		if (isStunned())
		{
			decrementMovesStunned();
			return;
		}
		else if (isAsleep())
		{
			decrementMovesAsleep();
			return;
		}
		else if (getHealth() >= 1600)
		{
			reduceHP(9000); // not pretty, but it gets the job done. effectively sets baby grasshopper to dead.
			Coord currentLocation(this->getX(), this->getY());
			getPtrToWorld()->addNewDuringGame(IID_FOOD, currentLocation, 100); // add food to map
			getPtrToWorld()->addNewDuringGame(IID_ADULT_GRASSHOPPER, currentLocation); // add adult grasshopper to map
			return;
		}
		Coord currentLocation(this->getX(), this->getY());
		if (getPtrToWorld()->eatFoodAtCurrentSquare(currentLocation, 200, this) && randInt (0, 1))
		{
			goToSleep();
			return;
		}
		else
		{
			if (areWeThereYet())
			{
				addMoreDistance();
				setDirection(getRandomDirection()); //this is how it should ultimately be implemented. the line below is purely for testing.
				//setDirection(up);
			}

			if (!moveInCurrentDirection())
				doneTraveling();
			else
				oneStepCloser();
		}
		goToSleep();
	}
}