#include "CollisionHandler.h"
#include "Map.h"
#include "AIMesh.h"
#include "Unit.h"
#include "Pathfinder.h"
#include "Minion.h"
#include "Champion.h"
#include "Logger.h"
#include "Object.h"
#include <iostream>

CollisionHandler::CollisionHandler(Map*map) : chart(map)
{
   Pathfinder::setMap(map);
	// Initialise the pathfinder.

   divisionCount = -1;
	// We have no divisions yet. Waiting for the AIMesh to initialise.

	if (simple)
		CORE_WARNING("Using simple collision. This could impact performance with larger amounts of minions.");
}

void CollisionHandler::init(int divisionsOverWidth)
{
	// Called after AIMesh works.
   width = chart->getAIMesh()->getWidth();
   height = chart->getAIMesh()->getHeight();

	// Get the square root of the division count. This is why it requires to be squared. (It's a map of 3x3 by default)
	//CORE_INFO("CollisionHandler has %d divisions.", divisionsOverWidth*divisionsOverWidth);
	divisionCount = divisionsOverWidth;

	// Setup the division dimensions
   for (int y = 0; y < divisionsOverWidth; y++)
   {
      for (int x = 0; x < divisionsOverWidth; x++)
      {
         managedDivisions[y*divisionsOverWidth + x].min.X = x*(width / divisionsOverWidth);
         managedDivisions[y*divisionsOverWidth + x].min.Y = y*(height / divisionsOverWidth);

         managedDivisions[y*divisionsOverWidth + x].max.X = (x + 1)*(width / divisionsOverWidth);
         managedDivisions[y*divisionsOverWidth + x].max.Y = (y + 1)*(height / divisionsOverWidth);

			managedDivisions[y*divisionsOverWidth + x].objectCount = 0;
      }
	}
}

void CollisionHandler::checkForCollisions(int pos)
{
	// Check for collisions in the managed division
   auto curDiv = managedDivisions[pos];

   for (int i = 0; i < curDiv.objects.size(); i++) // for each object in the current division
   {
		auto o1 = curDiv.objects[i];
		
		for (int j = 0; j < curDiv.objects.size(); j++) if (j != i)
      {
			auto o2 = curDiv.objects[j];

         auto displ = (o2->getPosition() - o1->getPosition());
         if (displ.SqrLength() < (o1->getCollisionRadius() + o2->getCollisionRadius())*(o1->getCollisionRadius() + o2->getCollisionRadius()))
         {
            o1->onCollision(o2);
            //o2->onCollision(o1); // Is being done by the second iteration.
         }  
      }
	}
}

void CollisionHandler::update(float deltatime)
{	
	if (!simple) // Faster
	{
		// Correct the unmanaged division (minions outside the map)
		//correctUnmanagedDivision();

		// For every managed division
		for (int i = 0; i < divisionCount*divisionCount; i++)
		{
			// Correct the divisions it should be in
			correctDivisions(i);

			// Check for collisions inside this division
			checkForCollisions(i);
		}
	}
	else // Slower, but works 
	{ 
		for (auto& objectRef : chart->getObjects())
		{
			auto o1 = objectRef.second;
			if (dynamic_cast<Minion*>(o1) == 0 && dynamic_cast<Champion*>(o1) == 0)
				continue;

			for (auto& objectRef2 : chart->getObjects()) if (objectRef != objectRef2)
			{
				auto o2 = objectRef2.second;
				if (dynamic_cast<Minion*>(o2) == 0 && dynamic_cast<Champion*>(o2) == 0)
					continue;

				auto displ = (o2->getPosition() - o1->getPosition());
				if (displ.SqrLength() < (o1->getCollisionRadius() + o2->getCollisionRadius())*(o1->getCollisionRadius() + o2->getCollisionRadius()))
				{
					o1->onCollision(o2);
					//o2->onCollision(o1); // Is being done by the second iteration.
				}
			}
		}
	}
}

void CollisionHandler::correctDivisions(int pos)
{
   CollisionDivision curDiv = managedDivisions[pos];
	for (int j = 0; j < curDiv.objects.size(); j++) // For all objects inside this division
   {
      Object* o = curDiv.objects[j];

		if (o)
		//if (o->isMovementUpdated())  // Only check if they moved around.
		{
			while (o->getMap() != chart)
			{
				CORE_WARNING("I have found an object that is not healthy. His map pointer is %x (not %x). Removing it from the database (%d/%d in div %d).", o->getMap(), chart, j, curDiv.objects.size(), pos);
				removeObject(o);
				if (j < curDiv.objects.size())
					o = curDiv.objects[j];
				else break;
			}

			// If they are no longer in this division..
         if ((o->getPosition().X - o->getCollisionRadius() > curDiv.max.X || o->getPosition().Y - o->getCollisionRadius() > curDiv.max.Y ||
            o->getPosition().X + o->getCollisionRadius() < curDiv.min.X || o->getPosition().Y + o->getCollisionRadius() < curDiv.min.Y))
         {
            removeFromDivision(o, pos); // Remove them from it.
            addObject(o); // Reset in what divisions this object is. 
         }

			// If they've entered another division, but not left this one yet..
         else if ((o->getPosition().X + o->getCollisionRadius() > curDiv.max.X || o->getPosition().Y + o->getCollisionRadius() > curDiv.max.Y ||
            o->getPosition().X - o->getCollisionRadius() < curDiv.min.X || o->getPosition().Y - o->getCollisionRadius() < curDiv.min.Y))
         {
            addObject(o); // Reset in what divisions this object is.
         }
      }
	}
}

void CollisionHandler::correctUnmanagedDivision()
{
   CollisionDivision curDiv = unmanagedDivision;
	for (int j = 0; j < curDiv.objects.size(); j++) // For everything outside the map
   {
      Object* o = (curDiv.objects[j]);

      Vector2 center = curDiv.min + ((curDiv.max - curDiv.min)*0.5f);

      //if (o->isMovementUpdated()) // if they moved
      {
			// If they're inside the map.
         if ((o->getPosition().X - o->getCollisionRadius() > width || o->getPosition().Y - o->getCollisionRadius() > height ||
            o->getPosition().X + o->getCollisionRadius() < 0 || o->getPosition().Y + o->getCollisionRadius() < 0))
         {
				removeFromDivision(o, -1);
				//CORE_INFO("Minion moving from unmanaged!");
            addObject(o);
         }
      }
	}
}

void CollisionHandler::addObject(Object *object)
{
	if (divisionCount == -1) // If we have not initialised..
   {
      CORE_ERROR("Tried to add an object before we initialised the CollisionHandler!");
		return;
   }

	if (object->getMap() != chart)
	{
		CORE_INFO("Map is adding an object that is not healthy. His map pointer is %x (not %x). Not adding it.", object->getMap(), chart);
		return;
	}

   float divX = object->getPosition().X / (float)(width / divisionCount); // Get the division position.
   float divY = object->getPosition().Y / (float)(height / divisionCount);

   int divi = (int)divY*divisionCount + (int)divX;

   if (divX < 0 || divX > divisionCount || divY < 0 || divY > divisionCount)  // We're not inside the map! Add to the unmanaged objects.
   {
      CORE_ERROR("Object spawned outside of map. (%f, %f)", object->getPosition().X, object->getPosition().Y);
		//addUnmanagedObject(object);
   }
   else
   {
      addToDivision(object, (int)divX, (int)divY);
      CollisionDivision curDiv = managedDivisions[divi];

      bool a = false, b = false;
		if (abs(object->getPosition().X - curDiv.max.X) < object->getCollisionRadius()) // Are we in the one to the right?
			addToDivision(object, (int)divX + 1, (int)divY);									  // Add it there too.
      if (abs(object->getPosition().X - curDiv.min.X) < object->getCollisionRadius()) // Maybe on the left?
         a = true, addToDivision(object, (int)divX - 1, (int)divY);

      if (abs(object->getPosition().Y - curDiv.max.Y) < object->getCollisionRadius()) // Are we touching below us?
         addToDivision(object, (int)divX, (int)divY + 1);									  
      if (abs(object->getPosition().Y - curDiv.min.Y) < object->getCollisionRadius()) // Or above?
         b = true, addToDivision(object, (int)divX, (int)divY - 1);

      if (a && b)																							  // If we are touching all four, add the left-upper one.
         b = true, addToDivision(object, (int)divX - 1, (int)divY - 1);
	}
}

void CollisionHandler::getDivisions(Object *object, CollisionDivision *divs[], int &divCount)
{
   for (int i = 0; i < 4; i++) // Prepare the divisions
   {
      divs[i] = 0;
   }

   divCount = 0; // How many divisions the object is in.
   float divX = object->getPosition().X / (float)(width / divisionCount);
   float divY = object->getPosition().Y / (float)(height / divisionCount);

	int divi = (int)divY*divisionCount + (int)divX; // Current division index

   if (divY >= 0 && divY < divisionCount) // If we're inside the map
   {
      divs[divCount] = &managedDivisions[divi];
      divCount++;
   }

	// Below is same principle from addObject.
   bool a = false, b = false;
   auto curDiv = managedDivisions[divi];
   if (abs(object->getPosition().X - curDiv.max.X) < object->getCollisionRadius() && divX + 1 >= 0 && divX + 1 < divisionCount)
   {
      divs[divCount] = &managedDivisions[(int)divY*divisionCount + (int)divX + 1];
      divCount++;
   }
   else if (abs(object->getPosition().X - curDiv.min.X) < object->getCollisionRadius() && divX - 1 >= 0 && divX - 1 < divisionCount)
   {
      divs[divCount] = &managedDivisions[(int)divY*divisionCount + (int)divX - 1];
      divCount++;
      a = true;
   }
   if (abs(object->getPosition().Y - curDiv.max.Y) < object->getCollisionRadius() && divY + 1 >= 0 && divY + 1 < divisionCount)
   {
      divs[divCount] = &managedDivisions[(int)divY*divisionCount + (int)divX + 1];
      divCount++;
   }
   else if (abs(object->getPosition().Y - curDiv.min.Y) < object->getCollisionRadius() && divY - 1 >= 0 && divY - 1 < divisionCount)
   {
      divs[divCount] = &managedDivisions[(int)divY*divisionCount + (int)divX + 1];
      divCount++;
      b = true;
   }

   if (a && b && divX + 1 >= 0 && divX + 1 < divisionCount)
   {
      divs[divCount] = &managedDivisions[(int)divY*divisionCount + (int)divX + 1];
      divCount++;
   }
}

void CollisionHandler::addToDivision(Object* object, int x, int y)
{
	// If we're inside the map.
   if (y >= 0 && y < divisionCount && x >= 0 && x < divisionCount)
   {
      int pos = y*divisionCount + x; // get the division position
      if (managedDivisions[pos].find(object)==-1) // Are we not in this division?
      {
         managedDivisions[pos].push(object); // Add it
			//CORE_INFO("MINION %d ADDED TO DIVISION %d", managedDivisions[pos].objects.size() - 1, pos);
      }
   }
}

void CollisionHandler::addUnmanagedObject(Object* object)
{
   //if(y < 0 || y >= divisionCount || x < 0 || x >= divisionCount))
   {
		if (unmanagedDivision.find(object)==-1)
      {
			unmanagedDivision.push(object);
			//CORE_INFO("MINION %d ADDED TO UNMANAGED DIVISION", unmanagedDivision.objects.size() - 1);
      }
	}
}


void CollisionHandler::removeObject(Object* object)
{
   CollisionDivision * curDiv;
   for (int i = -1; i < divisionCount*divisionCount; i++)
   {
      if (i == -1) curDiv = &unmanagedDivision;
      else curDiv = &managedDivisions[i];

      auto j = curDiv->find(object);			
      while (j != -1)
		{
			curDiv->remove(j);
			j = curDiv->find(object);
      }
	}
}

void CollisionHandler::removeFromDivision(Object* object, int i)
{
   CollisionDivision * curDiv;
   if (i == -1) curDiv = &unmanagedDivision;
   else curDiv = &managedDivisions[i];

   curDiv->remove(curDiv->find(object));
//	auto j = curDiv->find(object);
//   while (j != -1)
//	{
//		curDiv->remove(j);
//
//		j = curDiv->find(object);
//   }
}

int CollisionDivision::find(Object* a)
{
	for (int i = 0; i <objects.size(); i++)
		if (a == objects[i])
			return i;

	return -1;
}

void CollisionDivision::clear()
{
	for (int i = 0; i < objects.size(); i++)
		objects[i] = 0;
}

void CollisionDivision::push(Object * a)
{
	objects.push_back(a);
}

void CollisionDivision::remove(unsigned int i)
{
	if (i >= 0 && i < objects.size())
	{

		objects.erase(objects.begin() + i);
	}
}