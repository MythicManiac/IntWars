#include "SummonersRift.h"
#include "Turret.h"
#include "Game.h"
#include "CollisionHandler.h"
#include "LevelProp.h"
#include "Minion.h"

using namespace std;

const static vector<vector<Vector2> > laneWaypoints =
{
   { // blue top
      Vector2(917.f, 1725.f),
      Vector2(1170.f, 4041.f),
      Vector2(861.f, 6459.f),
      Vector2(880.f, 10180.f),
      Vector2(1268.f, 11675.f),
      Vector2(2806.f, 13075.f),
      Vector2(3907.f, 13243.f),
      Vector2(7550.f, 13407.f),
      Vector2(10244.f, 13238.f),
      Vector2(10947.f, 13135.f),
      Vector2(12511.f, 12776.f)
   },
   { // blue bot
      Vector2(1487.f, 1302.f),
      Vector2(3789.f, 1346.f),
      Vector2(6430.f, 1005.f),
      Vector2(10995.f, 1234.f),
      Vector2(12841.f, 3051.f),
      Vector2(13148.f, 4202.f),
      Vector2(13249.f, 7884.f),
      Vector2(12886.f, 10356.f),
      Vector2(12511.f, 12776.f)
   },
   { // blue mid
      Vector2(1418.f, 1686.f),
      Vector2(2997.f, 2781.f),
      Vector2(4472.f, 4727.f),
      Vector2(8375.f, 8366.f),
      Vector2(10948.f, 10821.f),
      Vector2(12511.f, 12776.f)
   },
   { // red top
      Vector2(12451.f, 13217.f),
      Vector2(10947.f, 13135.f),
      Vector2(10244.f, 13238.f),
      Vector2(7550.f, 13407.f),
      Vector2(3907.f, 13243.f),
      Vector2(2806.f, 13075.f),
      Vector2(1268.f, 11675.f),
      Vector2(880.f, 10180.f),
      Vector2(861.f, 6459.f),
      Vector2(1170.f, 4041.f),
      Vector2(1418.f, 1686.f)
   },
   { // red bot
      Vector2(13062.f, 12760.f),
      Vector2(12886.f, 10356.f),
      Vector2(13249.f, 7884.f),
      Vector2(13148.f, 4202.f),
      Vector2(12841.f, 3051.f),
      Vector2(10995.f, 1234.f),
      Vector2(6430.f, 1005.f),
      Vector2(3789.f, 1346.f),
      Vector2(1418.f, 1686.f)
   },
   { // red mid
      Vector2(12511.f, 12776.f),
      Vector2(10948.f, 10821.f),
      Vector2(8375.f, 8366.f),
      Vector2(4472.f, 4727.f),
      Vector2(2997.f, 2781.f),
      Vector2(1418.f, 1686.f)
   },
};


SummonersRift::SummonersRift(Game* game) : Map(game, 5*1000000, 30*1000000, 90*1000000, true)
{
   mesh.load("LEVELS/Map1/AIPath.aimesh");
   collisionHandler->init(3); // Needs to be initialised after AIMesh

   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_R_03_A", 10097.62f, 808.73f, 2550, 156, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_R_02_A", 6512.53f, 1262.62f, 2550, 170, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_07_A", 3747.26f, 1041.04f, 2550, 190, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_R_03_A", 13459.f, 4284.f, 2550, 156, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_R_02_A", 12920.f, 8005.f, 2550, 170, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_R_01_A", 13205.f, 10474.f, 2550, 190, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_05_A", 5448.02f, 6169.10f, 2550, 156, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_04_A", 4657.66f, 4591.91f, 2550, 170, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_03_A", 3233.99f, 3447.24f, 2550, 190, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_01_A", 1341.63f, 2029.98f, 2425, 180, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_02_A", 1768.19f, 1589.47f, 2425, 180, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_C_05_A", 8548.f, 8289.f, 2550, 156, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_C_04_A", 9361.f, 9892.f, 2550, 170, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_C_03_A", 10743.f, 11010.f, 2550, 190, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_C_01_A", 12662.f, 12442.f, 2425, 180, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_C_02_A", 12118.f, 12876.f, 2425, 180, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_OrderTurretShrine_A", -236.05f, -53.32f, 9999, 999, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_ChaosTurretShrine_A", 14157.f, 14456.f, 9999, 999, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_L_03_A", 574.66f, 10220.47f, 2550, 156, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_L_02_A", 1106.26f, 6485.25f, 2550, 170, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T1_C_06_A", 802.81f, 4052.36f, 2550, 190, 0));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_L_03_A", 3911.f, 13654.f, 2550, 156, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_L_02_A", 7536.f, 13190.f, 2550, 170, 1));
   addObject(new Turret(this, GetNewNetID(), "@Turret_T2_L_01_A", 10261.f, 13465.f, 2550, 190, 1));

   addObject(new LevelProp(this, GetNewNetID(), 12465.f, 14422.257f, 101.f, 0.f, 0.f, 0.f, 0.f, 0.f, "LevelProp_Yonkey", "Yonkey"));
   addObject(new LevelProp(this, GetNewNetID(), -76.f, 1769.1589f, 94.f, 0.f, 0.f, 0.f, 0.f, 0.f, "LevelProp_Yonkey1", "Yonkey"));
   addObject(new LevelProp(this, GetNewNetID(), 13374.17f, 14245.673f, 194.9741f, 224.f, 33.33f, 0.f, 0.f, -44.44f, "LevelProp_ShopMale", "ShopMale"));
   addObject(new LevelProp(this, GetNewNetID(), -99.5613f, 855.6632f, 191.4039f, 158.f, 0.f, 0.f, 0.f, 0.f, "LevelProp_ShopMale1", "ShopMale"));

   // Start at xp to reach level 1
   expToLevelUp = { 0, 280, 660, 1140, 1720, 2400, 3180, 4060, 5040, 6120, 7300, 8580, 9960, 11440, 13020, 14700, 16480, 18360 };

   // Announcer events
   announcerEvents.push_back(std::make_pair(false, make_tuple(30 * 1000000, 119, true))); // Welcome to SR
   if (firstSpawnTime - 30 * 1000000 >= 0.0f) announcerEvents.push_back(std::make_pair(false, make_tuple(firstSpawnTime - 30 * 1000000, 120, true))); // 30 seconds until minions spawn
   announcerEvents.push_back(std::make_pair(false, make_tuple(firstSpawnTime, 127, false))); // Minions have spawned (90 * 1000000)
   announcerEvents.push_back(std::make_pair(false, make_tuple(firstSpawnTime, 118, false))); // Minions have spawned [2] (90 * 1000000)

   fountain->setHealLocations(this);
}

void SummonersRift::update(long long diff) {
   Map::update(diff);

   if(gameTime >= 120*1000000){
      setKillReduction(false);
   }
}

std::pair<int, Vector2> SummonersRift::getMinionSpawnPosition(uint32 spawnPosition) const
{
	switch (spawnPosition)
	{
	case SPAWN_BLUE_TOP:
		return std::make_pair(0, Vector2(907, 1715));
	case SPAWN_BLUE_BOT:
		return std::make_pair(0, Vector2(1533, 1321));
	case SPAWN_BLUE_MID:
		return std::make_pair(0, Vector2(1443, 1663));
	case SPAWN_RED_TOP:
		return std::make_pair(1, Vector2(14455, 13159));
	case SPAWN_RED_BOT:
		return std::make_pair(1, Vector2(12967, 12695));
	case SPAWN_RED_MID:
		return std::make_pair(1, Vector2(12433, 12623));
	}
	return std::make_pair(0, Vector2(0, 0));
}

void SummonersRift::setMinionStats(Minion * minion) const
{
	// Same for all minions
	minion->getStats().setMovementSpeed(325.f);
	minion->getStats().setBaseAttackSpeed(0.625f);
	minion->getStats().setAttackSpeedMultiplier(1.0f);

	switch (minion->getType())
	{
	case MINION_TYPE_MELEE:
		minion->getStats().setCurrentHealth(475.0f + ((20.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setMaxHealth(475.0f + ((20.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setBaseAd(12.0f + ((1.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setRange(180.f);
		minion->getStats().setBaseAttackSpeed(1.250f);
		minion->setAutoAttackDelay(11.8f / 30.f);
		minion->setMelee(true);
		break;
	case MINION_TYPE_CASTER:
		minion->getStats().setCurrentHealth(279.0f + ((7.5f)*(int)(gameTime / (float)(90 * 1000000))));
		minion->getStats().setMaxHealth(279.0f + ((7.5f)*(int)(gameTime / (float)(90 * 1000000))));
		minion->getStats().setBaseAd(23.0f + ((1.f)*(int)(gameTime / (float)(90 * 1000000))));
		minion->getStats().setRange(600.f);
		minion->getStats().setBaseAttackSpeed(0.670f);
		minion->setAutoAttackDelay(14.1f / 30.f);
		minion->setAutoAttackProjectileSpeed(650.f);
		break;
	case MINION_TYPE_CANNON:
		minion->getStats().setCurrentHealth(700.0f + ((27.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setMaxHealth(700.0f + ((27.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setBaseAd(40.0f + ((3.f)*(int)(gameTime / (float)(180 * 1000000))));
		minion->getStats().setRange(450.f);
		minion->getStats().setBaseAttackSpeed(1.0f);
		minion->setAutoAttackDelay(9.f / 30.f);
		minion->setAutoAttackProjectileSpeed(1200.f);
		break;
	}
}

bool SummonersRift::spawn() {

   static const vector<MinionSpawnPosition> positions = {
                                                      SPAWN_BLUE_TOP,
                                                      SPAWN_BLUE_BOT,
                                                      SPAWN_BLUE_MID,
                                                      SPAWN_RED_TOP,
                                                      SPAWN_RED_BOT,
                                                      SPAWN_RED_MID,
                                                   };

   if(waveNumber < 3) {
      for(uint32 i = 0; i < positions.size(); ++i) {
         Minion* m = new Minion(this, GetNewNetID(), MINION_TYPE_MELEE, positions[i], laneWaypoints[i]);
         addObject(m);
      }
      return false;
   }

   if(waveNumber == 3) {
      for(uint32 i = 0; i < positions.size(); ++i) {
         Minion* m = new Minion(this, GetNewNetID(), MINION_TYPE_CANNON, positions[i], laneWaypoints[i]);
         addObject(m);
      }
      return false;
   }

   if(waveNumber < 7) {
      for(uint32 i = 0; i < positions.size(); ++i) {
         Minion* m = new Minion(this, GetNewNetID(), MINION_TYPE_CASTER, positions[i], laneWaypoints[i]);
         addObject(m);
      }

      return false;
   }

   return true;
}

const Target SummonersRift::getRespawnLocation(int team) const {
   switch(team) {
   case 0:
      return Target(25.90f, 280);
   case 1:
      return Target(14119, 14063);
   }

   return Target(25.90f, 280);
}

float SummonersRift::getGoldFor(Unit* u) const {
   Minion* m = dynamic_cast<Minion*>(u);

  if(!m) {
      Champion* c = dynamic_cast<Champion*>(u);

      if(!c){
         return 0.f;
      }

      float gold = 300.f; //normal gold for a kill

      if(c->getKillDeathCounter() < 5 && c->getKillDeathCounter() >= 0){
         if(c->getKillDeathCounter() == 0){
            return gold;
         }

         for(int i = c->getKillDeathCounter(); i > 1; --i){
            gold += gold*0.165f;
         }

         return gold;
      }

      if(c->getKillDeathCounter() >= 5){
         return 500.f;
      }

      if(c->getKillDeathCounter() < 0){
         float firstDeathGold = gold-gold*0.085f;

         if(c->getKillDeathCounter() == -1){
            return firstDeathGold;
         }

         for(int i = c->getKillDeathCounter(); i < -1; ++i){
            firstDeathGold -= firstDeathGold*0.2f;
         }

         if(firstDeathGold < 50){
            firstDeathGold = 50;
         }

         return firstDeathGold;
      }

      return 0.f;
   }

   switch(m->getType()) {
      case MINION_TYPE_MELEE:
         return 19.f + ((0.5f)*(int)(gameTime / (180 * 1000000)));
      case MINION_TYPE_CASTER:
         return 14.f + ((0.2f)*(int)(gameTime / (90 * 1000000)));
      case MINION_TYPE_CANNON:
         return 40.f + ((1.f)*(int)(gameTime / (180 * 1000000)));
   }

   return 0.f;
}

float SummonersRift::getExperienceFor(Unit* u) const {
	Minion* m = dynamic_cast<Minion*>(u);

	if (!m) {
		return 0.f;
	}

	switch (m->getType()) {
	case MINION_TYPE_MELEE:
		return 58.88f;
	case MINION_TYPE_CASTER:
		return 	29.44f;
	case MINION_TYPE_CANNON:
		return 92.f;
	}

	return 0.f;
}
