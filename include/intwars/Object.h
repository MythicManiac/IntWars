/**
 * An object is an abstract entity.
 * This is the base class for units and projectiles.
 */

#ifndef OBJECT_H_
#define OBJECT_H_
#pragma warning(disable:4458)

#include <vector>
#include "Target.h"
#include "stdafx.h"
#include "Vector2.h"

#pragma STDC FENV_ACCESS ON

class Map;

#define MAP_WIDTH (13982 / 2)
#define MAP_HEIGHT (14446 / 2)

struct MovementVector {
    int16 x;
    int16 y;
    
    MovementVector() : x(0), y(0){ }
    MovementVector(int16 x, int16 y) : x(x), y(y) { }
    MovementVector(float x, float y) : x(targetXToNormalFormat(x)), y(targetYToNormalFormat(y)) { }
	 Target* toTarget() { return new Target(2.0f*x + MAP_WIDTH, 2.0f*y + MAP_HEIGHT); }
	 operator Vector2() { return Vector2(2.0f*x + MAP_WIDTH, 2.0f*y + MAP_HEIGHT); }
    
    static int16 targetXToNormalFormat(float _x){
		 return (int16)(((_x) - MAP_WIDTH )/2.0f);
    }
    static int16 targetYToNormalFormat(float _y){
		 return (int16)(((_y) - MAP_HEIGHT)/2.0f);
    }
    
};

class Unit;

class Object : public Target {
protected:
  	uint32 id;

	float xvector, yvector;
   
   /**
    * Current target the object running to (can be coordinates or an object)
    */
	Target* target;

   std::vector<Vector2> waypoints;
   uint32 curWaypoint;
   Map* map;

   unsigned int team;
   bool movementUpdated;
   bool toRemove;
   uint32 attackerCount;
   
   uint32 collisionRadius;
   Vector2 direction;
   uint32 visionRadius;
   
   bool dashing;
   float dashSpeed;
   
   bool visibleByTeam[2];
      
public:
   virtual ~Object();
   Object(Map* map, uint32 id, float x, float y, uint32 collisionRadius, uint32 visionRadius = 0);

   virtual void onCollision(Object *collider) {}

   /**
   * Moves the object depending on its target, updating its coordinate.
   * @param diff the amount of milliseconds the object is supposed to move
   */
   void Move(int64 diff);
   
   Vector2 getDirection() { return direction; }

   void calculateVector(float xtarget, float ytarget);

   /**
   * Sets the team of the object
   * @param team the new team
   */
	void setTeam(unsigned int team) { this->team = team; }
	unsigned int getTeam() { return team; }

   virtual void update(int64 diff);
   virtual float getMoveSpeed() const = 0;

   virtual bool isSimpleTarget() { return false; }

   Target* getTarget() { return target; }
	void setTarget(Target* target);
	void setWaypoints(const std::vector<Vector2>& waypoints);

   const std::vector<Vector2>& getWaypoints() const { return waypoints; }
   uint32 getCurWaypoint() const { return curWaypoint; }
   bool isMovementUpdated() { return movementUpdated; }
   void clearMovementUpdated() { movementUpdated = false; }
   bool isToRemove() { return toRemove; }
   virtual void setToRemove() { toRemove = true; }

   uint32 getNetId() const { return id; }
   Map* getMap() const { return map; }

   void setPosition(float x, float y);
   float getZ();


   void setCollisionRadius(uint32 collisionRadius) { this->collisionRadius = collisionRadius; }
   uint32 getCollisionRadius() const { return collisionRadius; }
   virtual float getLargestRadius() { return collisionRadius; }

   uint32 getVisionRadius() const { return visionRadius; }
   bool collide(Object* o);
   
   uint32 getAttackerCount() const { return attackerCount; }
   void incrementAttackerCount() { ++attackerCount; }
   void decrementAttackerCount() { --attackerCount; }

   bool isVisibleByTeam(uint32 team);
   void setVisibleByTeam(uint32 team, bool visible);
   
   void dashTo(float x, float y, float dashSpeed);
   bool isDashing() const { return dashing; }
};

#endif /* OBJECT_H_ */
