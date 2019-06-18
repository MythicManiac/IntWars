#ifndef TARGET_H_
#define TARGET_H_

#include "Vector2.h"

class Target {

public:
   
   virtual ~Target() { }
	Target(float x, float y) : x(x), y(y) { }
	Target(Vector2 vec) : x(vec.X), y(vec.Y) { }

   float distanceWith(Target* target);
   float distanceWith(float xtarget, float ytarget);

   float distanceWithSqr(Target* target);
   float distanceWithSqr(float xtarget, float ytarget);

   float getX() const { return x; }
   float getY() const { return y; }

   void setPosition(float x, float y) { this->x = x; this->y = y; }
   Vector2 getPosition() { return Vector2(x, y); }

   virtual bool isSimpleTarget() { return true; }

protected:
	
   float x, y;
};

#endif
