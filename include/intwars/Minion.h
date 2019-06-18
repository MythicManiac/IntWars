#ifndef _MINION_H
#define _MINION_H

#include "Unit.h"
#include "Vector2.h"

enum MinionSpawnPosition : uint32 {
	SPAWN_BLUE_TOP = 0xeb364c40,
	SPAWN_BLUE_BOT = 0x53b83640,
	SPAWN_BLUE_MID = 0xb7717140,
	SPAWN_RED_TOP = 0xe647d540,
	SPAWN_RED_BOT = 0x5ec9af40,
	SPAWN_RED_MID = 0xba00e840
};

enum MinionSpawnType : uint8 {
   MINION_TYPE_MELEE = 0x00,
   MINION_TYPE_CASTER = 0x03,
   MINION_TYPE_CANNON = 0x02,
   MINION_TYPE_SUPER = 0x01
};

class Minion : public Unit 
{
public:
	Minion(Map* map, uint32 id, MinionSpawnType type, MinionSpawnPosition position, const std::vector<Vector2>& mainWaypoints = std::vector<Vector2>());
	uint32 getSpawnPosition() const { return spawnPosition; }
	uint32 getType() const { return type; }

	virtual void update(int64 diff) override;

	void onCollision(Object * a_Collider);

	virtual bool isInDistress() const override { return distressCause!=0; }
protected:
	// AI tasks
	bool scanForTargets();
	void walkToDestination();
	void keepFocussingTarget();


   MinionSpawnPosition spawnPosition;
	MinionSpawnType type;
   
   /**
    * Const waypoints that define the minion's route
    */
   const std::vector<Vector2> mainWaypoints;
   unsigned int curMainWaypoint = 0;
};

#endif