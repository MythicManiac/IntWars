#include "Object.h"
#include <cmath>
#include <algorithm>
#include "Vector2.h"
#include "Map.h"
#include "CollisionHandler.h"

using namespace std;

Object::Object(Map* map, uint32 id, float x, float y, uint32 collisionRadius, uint32 visionRadius) : Target(x, y), map(map), id(id), target(0), collisionRadius(collisionRadius),
                                                                                                     visionRadius(visionRadius), team(0), movementUpdated(false), toRemove(false), attackerCount(0),
                                                                                                     dashing(false), visibleByTeam{false, false} 
{
}

Object::~Object() 
{
}

void Object::calculateVector(float xtarget, float ytarget) {
   xvector = xtarget-x;
   yvector = ytarget-y;

   if(xvector == 0 && yvector == 0)
    return;

   float toDivide = abs(xvector) + abs(yvector);
   xvector /= toDivide;
   yvector /= toDivide;
}

void Object::setTarget(Target* target) {
   if(this->target == target)
      return;

   if(this->target && this->target->isSimpleTarget()) {
      delete this->target;
   }
      
   this->target = target;

}

void Object::Move(int64 diff) {

   if (!target)
   {
      direction = Vector2();
      return;
   }

   Vector2 to(target->getX(), target->getY());
   Vector2 cur(x, y);
   
   Vector2 goingTo (to - cur);
	direction = goingTo.Normalize();

   float moveSpeed = dashing ? dashSpeed : getMoveSpeed();
   double deltaMovement = (double)(moveSpeed) * 0.000001f*diff;

   float xx = direction.X * deltaMovement;
   float yy = direction.Y * deltaMovement;

   x+= xx;
   y+= yy;

	/* If the target was a simple point, stop when it is reached */
   if(target->isSimpleTarget() && distanceWith(target) < deltaMovement*2) {
      if(dashing) {
         dashing = false;
         setTarget(0);
      } else if(++curWaypoint >= waypoints.size()) {
         setTarget(0);
      } else {
         setTarget(new Target(waypoints[curWaypoint]));
      }
   }
}

void Object::update(int64 diff) {
   Move(diff);
}

void Object::setWaypoints(const std::vector<Vector2>& newWaypoints) {
   waypoints = newWaypoints;
   
   setPosition(waypoints[0].X, waypoints[0].Y);
   movementUpdated = true;
   if(waypoints.size() == 1) {
      setTarget(0);
      return;
   }
   
   setTarget(new Target(waypoints[1]));
   curWaypoint = 1;
}

void Object::setPosition(float x, float y) {

   this->x = x;
   this->y = y;

   setTarget(0);
}

bool Object::collide(Object* o) {
   return distanceWithSqr(o) < (getCollisionRadius() + o->getCollisionRadius())*(getCollisionRadius() + o->getCollisionRadius());
}

bool Object::isVisibleByTeam(uint32 team) {
	if(team > 1) 
   {
		return false;
	}

	return (team == getTeam() || visibleByTeam[team]);
}

void Object::setVisibleByTeam(uint32 team, bool visible) {
	visibleByTeam[team] = visible;
}

float Object::getZ() {
   return map->getHeightAtLocation(x, y);
}

void Object::dashTo(float x, float y, float dashSpeed) {
   dashing = true;
   this->dashSpeed = dashSpeed;
   setTarget(new Target(x, y));
   waypoints.clear();
}