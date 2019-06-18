#include "Minion.h"
#include "MinionStats.h"
#include "Map.h"
#include "Game.h"
#include "Pathfinder.h"
#include "Logger.h"

using namespace std;

Minion::Minion(Map* map, uint32 id, MinionSpawnType type, MinionSpawnPosition position, const vector<Vector2>& mainWaypoints) : Unit(map, id, "", new MinionStats(), 40, 0, 0, 1100), type(type), spawnPosition(position), mainWaypoints(mainWaypoints), curMainWaypoint(0)
{
	// Setup spawn position and team.
	auto spawnSpecifics = map->getMinionSpawnPosition(spawnPosition);
	setTeam(spawnSpecifics.first);
	setPosition(spawnSpecifics.second.X, spawnSpecifics.second.Y);

	map->setMinionStats(this); // Let the map decide how strong this minion has to be.
   
   std::string minionModel;
	if (spawnSpecifics.first==0) // If we're the blue side
		minionModel += "Blue_Minion_"; // make it a blue minion
	else minionModel += "Red_Minion_"; // otherwise make it a red minion
   
	// Finish model name with type
   if (type == MINION_TYPE_MELEE) minionModel += "Basic";
   else if (type == MINION_TYPE_CASTER)  minionModel += "Wizard"; 
   else minionModel += "MechCannon";

	// Set model
   setModel(minionModel);
   

	if (mainWaypoints.size() > 0)										// If we have lane path instructions from the map
		setWaypoints({ mainWaypoints[0], mainWaypoints[0] }); // Follow these instructions
	else setWaypoints({ Vector2(x, y), Vector2(x, y) });		// Otherwise path to own position. (Stand still)

   setMoveOrder(MOVE_ORDER_ATTACKMOVE);
}

void Minion::update(int64 diff) 
{
	Unit::update(diff);

	if (!isDead())
	{
		if ( dashing )
			return;
		else if (scanForTargets())     // returns true if we have a target
			keepFocussingTarget(); // fight target
		else walkToDestination(); // walk to destination (or target)
	}
}

bool Minion::scanForTargets()
{
	Unit* nextTarget = 0;
	unsigned int nextTargetPriority = 9e5;

	const std::map<uint32, Object*>& objects = map->getObjects();
	for (auto& it : objects)
	{
		Unit* u = dynamic_cast<Unit*> (it.second);

		// Targets have to be:
		if (!u ||												 // a unit
			 u->isDead() ||									 // alive
			 u->getTeam() == getTeam() ||					 // not on our team
			 distanceWith(u) > DETECT_RANGE ||			 // in range
			 !getMap()->teamHasVisionOn(getTeam(), u)) // visible to this minion
			continue; // If not, look for something else

		auto priority = classifyTarget(u); // get the priority.
		if (priority < nextTargetPriority) // if the priority is lower than the target we checked previously
		{
			nextTarget = u;					  // make him a potential target.
			nextTargetPriority = priority;
		}
	}

	if (nextTarget) // If we have a target
	{
		setTargetUnit(nextTarget); // Set the new target and refresh waypoints
		map->getGame()->notifySetTarget(this, nextTarget);
		return true;
	}
	return false;
}

void Minion::keepFocussingTarget()
{
	if (isAttacking && (!targetUnit || distanceWith(targetUnit) > stats->getRange()))
	// If target is dead or out of range
	{
		map->getGame()->notifyStopAutoAttack(this);
		isAttacking = false;
	}
}

void Minion::walkToDestination()
{ 
	if ((waypoints.size() == 1) || (curWaypoint == 2 && ++curMainWaypoint < mainWaypoints.size()))
	{
		//CORE_INFO("Minion reached a point! Going to %f; %f", mainWaypoints[curMainWaypoint].X, mainWaypoints[curMainWaypoint].Y);
		vector<Vector2> newWaypoints = { Vector2(x, y), mainWaypoints[curMainWaypoint] };
		setWaypoints(newWaypoints);
   }
}

void Minion::onCollision(Object * a_Collider)
{
	if (a_Collider == targetUnit) // If we're colliding with the target, don't do anything.
		return;
	
	if (targetUnit != 0)
	{
		// Thread this?
		//Path newPath = Pathfinder::getPath(getPosition(), targetUnit->getPosition());
		//if (newPath.error == PATH_ERROR_NONE)
			//setWaypoints(newPath.getWaypoints());
	}
}