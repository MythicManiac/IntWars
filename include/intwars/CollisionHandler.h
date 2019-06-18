#ifndef _COLLISIONHANDLER_H
#define _COLLISIONHANDLER_H

#include "stdafx.h"
#include "Map.h"
#include "Vector2.h"
#include "Logger.h"

class Object;
struct CollisionDivision
{
   Vector2 min, max;
	std::vector<Object*> objects;
	unsigned int objectCount = 0;

	void push(Object * a);
	void remove(unsigned int i);
	int find(Object* a);
	void clear();
};

class CollisionHandler
{
public:
   CollisionHandler(Map * map);
   void init(int divisionsOverWidth);
	~CollisionHandler(){}

   void addObject(Object *object);
   void removeObject(Object* object);
   void getDivisions(Object *object, CollisionDivision *divisionArray[], int &a_DivisionCount);

   void update(float a_DT);

private:
   void checkForCollisions(int pos);
   void correctDivisions(int pos);
   void correctUnmanagedDivision();
   void addToDivision(Object* object, int x, int y);
   void addUnmanagedObject(Object* object);
   
   void removeFromDivision(Object* object, int i);
   int width, height;
   CollisionDivision managedDivisions[3*3];
   CollisionDivision unmanagedDivision;
   int divisionCount;
   Map* chart;
   bool simple = false;
};

#endif