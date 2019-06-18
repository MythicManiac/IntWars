#include "Logger.h"
#include "Pathfinder.h"
#include "Map.h"
#include "AIMesh.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include "Logger.h"
#include "Minion.h"
#include "Champion.h"
#include <ctime>

Map * Pathfinder::chart = 0;
auto g_Clock = std::clock(); 

#define debugOutput() false //((std::clock() - g_Clock) > 4000)

int successes = 0 , oot = 0, empties = 0;

auto start_time = std::chrono::high_resolution_clock::now();
unsigned int totalDuration = 0, durations = 0;

Path Pathfinder::getPath(Vector2 from, Vector2 to, float boxSize)
{
	start_time = std::chrono::high_resolution_clock::now();
   Path path;
	PathJob job; 
	
	if ((std::clock() - g_Clock) > 4000 && (successes + oot + empties) > 0)
	{
		CORE_INFO("Pathfinding successrate: %f", (((float)successes / (float)(successes + oot + empties))*(100.0f)));
	}

   if (debugOutput())
   {
		CORE_INFO("Recording this minion movement.");
   } 

   if (chart == 0) CORE_FATAL("Tried to find a path without setting the map.");
   if (getMesh() == 0) CORE_FATAL("Can't start pathfinding without initialising the AIMesh");


   job.start = job.fromPositionToGrid(from); // Save start in grid info
   job.destination = job.fromPositionToGrid(to); // Save destination in grid info

	if (debugOutput())
	{
		CORE_INFO("Going from (%f, %f) to (%f, %f)", job.start.X, job.start.Y, job.destination.X, job.destination.Y);
	}

   job.insertObstructions(chart, getMesh()); // Ready the map.
          
   job.addToOpenList(job.start, 0); // Let's start at the start.
	
	int tries;
   for ( tries = 0; job.openList.size() != 0; tries++) // Go through the open list while it's not empty
	{
		if (debugOutput())
		{
			CORE_INFO("Going through openlist. Tries: %d | Objects on list: %d", tries, job.openList.size());
		}

      if (tries == MAX_PATHFIND_TRIES)
      {
         path.error = PATH_ERROR_OUT_OF_TRIES;
         oot++;
			//CORE_WARNING("PATH_ERROR_OUT_OF_TRIES");
         path.waypoints = job.reconstructUnfinishedPath();
         job.cleanPath(path);
         job.cleanLists();
         return path;
      }
      else if (job.traverseOpenList(tries==0))
      {
         path.error = PATH_ERROR_NONE;
         successes++;
         //CORE_INFO("We finished a path.");
         path.waypoints = job.reconstructPath();
         job.cleanPath(path);
         job.cleanLists();
         return path;
      }
   }

	if (debugOutput())
	{
		CORE_INFO("Going through openlist. Tries: %d | Objects on list: %d", tries, job.openList.size());
	}

	//CORE_WARNING("PATH_ERROR_OPENLIST_EMPTY");
   path.error = PATH_ERROR_OPENLIST_EMPTY;
   empties++;
   path.waypoints.push_back(from);
   job.cleanPath(path);
   job.cleanLists();
   return path;
}

bool PathJob::traverseOpenList(bool first)
{
	if (openList.size() == 0)
	{
		if (debugOutput())
		{
			CORE_INFO("In OpenList, but failed, we are empty.");
		}
		return false;
	}
	else if (debugOutput())
	{
		CORE_INFO("TraverseOpenList! First: %d", first);
	}

   
   // This sorts every iteration, which means that everything but the last couple of elements are sorted.
   // TODO: That means, this can probably be optimised. Sort only the last elements and add them into the vector where they belong.
   // But honestly, it's running pretty fast so why bother
   std::sort(openList.begin(), openList.end(), SortByF);

   PathNode * currentNode = openList.back();
   openList.pop_back();

	bool atDestination = (abs(currentNode->x - (int)destination.X) <= 1 && abs(currentNode->y-(int)destination.Y) <= 1);

	if (debugOutput() && atDestination)
	{
		CORE_INFO("We're at our destination! (%d, %d)", currentNode->x, currentNode->y);
	}
	else if (debugOutput())
	{
		CORE_INFO("Difference: (%d, %d)", (int)destination.X - currentNode->x, (int)destination.Y-currentNode->y);
	}

   if (!atDestination) // While we're not there
   {      
      for (int dx = -1; dx <= 1; dx++)
      {
         if (currentNode->x + dx >= 0 && currentNode->x + dx < GRID_WIDTH) // Search in 8 directions, but we're supposed to stay in map
         {
            for (int dy = -1; dy <= 1; dy++)
            {
               if (!(dx == 0 && dy == 0)) // in all 8 directions, ignore the x==y==0 where we dont move
               {
                  if (!isGridNodeOccupied(currentNode->x + dx, currentNode->y + dy)) // Is something here?
                  {
                     PathNode* conflictingNode = isNodeOpen(currentNode->x + dx, currentNode->y + dy); // Nothing is here, did we already add this to the open list?
                     if (!conflictingNode) // We did not, add it
                     {
								if (debugOutput())
								{
									CORE_INFO("Adding to open list: (%f, %f)", (float)(currentNode->x + dx), (float)(currentNode->y + dy));
								}
                        addToOpenList(Vector2((float)(currentNode->x + dx), (float)(currentNode->y + dy)), currentNode);
                     }
                     else if (conflictingNode->g > CALC_G(currentNode->g)) // I found a shorter route to this node.
                     {
                        conflictingNode->setParent(currentNode); // Give it a new parent
                        conflictingNode->setScore((int)CALC_H(conflictingNode->x, conflictingNode->y, destination.X, destination.Y), CALC_G(currentNode->g)); // set the new score.
                     }
                  }
						else if (debugOutput())
						{
							CORE_INFO("Occupied: (%d, %d)", currentNode->x + dx, currentNode->y + dy);
						}
               }
            }
         }
      }
   }


   closedList.push_back(currentNode);
   return atDestination;
}

std::vector<Vector2> Path::getWaypoints()
{
	return waypoints;
}

std::vector<Vector2> PathJob::reconstructPath( ) // Make a std::vector of the waypoints
{
   std::vector<Vector2> ret;
   auto i = (closedList.back());

   while (i)
   {
      ret.push_back(fromGridToPosition(Vector2((float)i->x, (float)i->y)));
      i = i->parent;
   }

   return ret;
}

std::vector<Vector2> PathJob::reconstructUnfinishedPath() // Let's go over the closed list and go back to the start, create a path from the best choice.
{
   std::vector<Vector2> ret;
   
   auto a = closedList.back();
   int lowestG = 9e7;
   for (auto i = closedList.begin(); i != closedList.end(); i++)
      if ((*i)->g < lowestG)
      {
         lowestG = (*i)->g;
         a = (*i);
      }

   while (a)
   {
      ret.push_back(fromGridToPosition(Vector2((float)a->x, (float)a->y)));
      a = a->parent;
   }

   return ret;
}

void PathJob::cleanPath(Path &path)
{
	return;
   if (path.waypoints.size() < 2) return;
   int startSize = path.waypoints.size();
   CORE_WARNING("Cleaning path.. Current size is %d", startSize);

   int dirX = 0, dirY = 0;
   auto prevPoint = path.waypoints.begin();
   for (auto i = path.waypoints.begin()+1; i!=path.waypoints.end(); i++)
   {
      if (((*i).X - (*prevPoint).X == dirX) &&
         ((*i).Y - (*prevPoint).Y == dirY))
      {
         path.waypoints.erase(prevPoint);
         CORE_WARNING("Erased a waypoint");
      }
      else
      {
         dirX = ((*i).X - (*prevPoint).X);
         dirY = ((*i).Y - (*prevPoint).Y);
      }

      prevPoint = i;
   }

   CORE_WARNING("Done cleaning. New size is %d", path.waypoints.size());
   if (startSize != path.waypoints.size())
      CORE_WARNING("Removed %d nodes", startSize-path.waypoints.size());
}

PathNode* PathJob::isNodeOpen(int x, int y)
{
   // TODO: Optimise? This is where the application is residing in 96% of the time during pathfinding.

   // It checks if we've already added this x and y to the openlist. If we did, return it. 
   for (auto i = openList.begin(); i != openList.end(); i++)
   {
      if ((*i)->x == x && (*i)->y == y)
         return (*i);
   }

   return 0;
}

bool SortByF(const PathNode* first, const PathNode* second) // Sort function for finding best score
{
   return (first->g + first->h > second->g + second->h); // Greater than because we want the lower scores in the back (first)
}

Vector2 PathJob::fromGridToPosition(Vector2 position)
{
   AIMesh* mesh = Pathfinder::getMesh();
   if (mesh == 0) CORE_FATAL("Tried to get a grid location without an initialised AIMesh!");
   
   return position*PATH_DEFAULT_BOX_SIZE(mesh->getSize());
}

Vector2 PathJob::fromPositionToGrid(Vector2 position)
{
   AIMesh* mesh = Pathfinder::getMesh();
   if (mesh == 0) CORE_FATAL("Tried to get a position without an initialised AIMesh!");

   return (position / (float)PATH_DEFAULT_BOX_SIZE(mesh->getSize()));
}

bool PathJob::isGridNodeOccupied(int x, int y)
{
   if ((x >= 0 && x < GRID_SIZE) &&
       (y >= 0 && y < GRID_SIZE))
   {
      return map[x][y].isOccupied();
   }
   else return true;
}

#include <fstream>
#include <iostream>


void PathJob::insertObstructions(Map * chart, AIMesh *mesh) // insert all objects into the map
{
   std::memset(map, false, sizeof(Grid)*GRID_WIDTH*GRID_HEIGHT); // Empty map

	if (mesh != NULL)
	{
		// Now to draw the mesh onto the thing.
		if (mesh->isLoaded()) // if we have loaded the mesh
			for (int x = 0; x < GRID_WIDTH; x++) // for every grid piece
				for (int y = 0; y < GRID_HEIGHT; y++)
				{
					Vector2 translated = fromGridToPosition(Vector2((float)x, (float)y));
					if (!mesh->isWalkable(translated.X, translated.Y)) // If there's nothing at this position
						map[x][y].occupied = true; // This is obstructed
				}
	}

	if (chart != NULL)
	{
		const std::map<uint32, Object*>& objects = chart->getObjects();
		for (auto i = objects.begin(); i != objects.end(); i++) // For every object
		{
			if (dynamic_cast<Minion*>(i->second) == 0 && dynamic_cast<Champion*>(i->second) == 0)
				continue;

			Vector2 gridPos = fromPositionToGrid(i->second->getPosition()); // get the position in grid size

			int radius = ((int)ceil((float)i->second->getCollisionRadius() / (float)PATH_DEFAULT_BOX_SIZE(mesh->getSize()))) / 2; // How many boxes does the radius of this object cover?

			if (debugOutput())
			{
				CORE_INFO("We're drawing a minion with a width of (%d/%f)/2=%d pixels at (%f, %f)", i->second->getCollisionRadius(), (float)PATH_DEFAULT_BOX_SIZE(mesh->getSize()), radius, gridPos.X, gridPos.Y);
			}

			for(int dx = -radius; dx < radius; dx++) // For the whole radius in the width
				if (gridPos.X + dx >= 0 && gridPos.X + dx < GRID_WIDTH) // As long as we're in the map (x)
					for (int dy = -radius; dy < radius; dy++) // for the whole radius in the y
						if (gridPos.Y + dy >= 0 && gridPos.Y + dy < GRID_HEIGHT) // As long as we're in the map (y)
							map[(int)gridPos.X + dx][(int)gridPos.Y + dy].occupied = true; // Occupy this piece of the map.
		}
	}

	if (debugOutput())
	{
		auto width = GRID_WIDTH;
		auto height = GRID_HEIGHT;
#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)>(b))?(a):(b))
		std::ofstream imageFile("..\\..\\test.tga", std::ios::out | std::ios::binary);
		if (!imageFile) return;

		// The image header
		unsigned char header[18] = { 0 };
		header[2] = 1;  // truecolor
		header[12] = width & 0xFF;
		header[13] = (width >> 8) & 0xFF;
		header[14] = height & 0xFF;
		header[15] = (height >> 8) & 0xFF;
		header[16] = 24;  // bits per pixel

		imageFile.write((const char*)header, 18);

		//for (int y = 0; y < height; y++)
		for (int y = height - 1; y >= 0; y--)
			for (int x = 0; x < width; x++)
			{
			/* blue */  imageFile.put(map[x][y].occupied * 128);
			/* green */ imageFile.put(map[x][y].occupied * 128);
			/* red */   imageFile.put(map[x][y].occupied * 128);
			}

		// The file footer. This part is totally optional.
		static const char footer[26] =
			"\0\0\0\0"  // no extension area
			"\0\0\0\0"  // no developer directory
			"TRUEVISION-XFILE"  // Yep, this is a TGA file
			".";
		imageFile.write(footer, 26);

		imageFile.close();
	}
}

#ifdef USE_OPTIMISATION
char PathNode::tableInitialised = -1;
std::vector<PathNode*> PathNode::nodeTable;

// Create a table of items to make sure we don't new/delete during runtime
void PathNode::InitTable()
{
   if (tableInitialised!=-1) return; // We have already initialised it or we're busy doing it.
   tableInitialised = 0;
   nodeTable.reserve(TABLE_SIZE);
   for (int i = 0; i < TABLE_SIZE; i++)
   {
      nodeTable.push_back(new PathNode());
   }

   tableInitialised = 1;
}

void* PathNode::operator new(size_t size) // catch news
{
   PathNode *ret = NULL;
   switch(tableInitialised)
   {
   case -1: // Should start
      InitTable();

   case 1: // Already cached
      //if (nodeTable.size() == 0) 0;
         //__debugbreak(); // Tried to get a new node while we're all out of nodes.
      ret = nodeTable.back();
      ret->Init(0, 0, 0, 0, 0);
      nodeTable.pop_back();
      break;

   case 0: // creating cache
      ret = (PathNode*)std::malloc(size);
      //if (ret == 0) 0;//__debugbreak(); // No memory?
   }
   return ret;
}

void PathNode::operator delete(void * object)
{
   if (tableInitialised == 2) std::free(object);
   else nodeTable.push_back((PathNode*)object);
}
#endif


void PathJob::cleanLists()
{
	for (auto i = openList.begin(); i != openList.end(); i++)
		delete (*i);
	openList.clear();
	for (auto i = closedList.begin(); i != closedList.end(); i++)
		delete (*i);
	closedList.clear();

	auto end_time = std::chrono::high_resolution_clock::now();

	totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	durations++;

	if ((std::clock() - g_Clock) > 4000)
	{
		CORE_INFO("%f milliseconds, %d paths.", (float)totalDuration/(float)durations, durations);
		g_Clock = std::clock();
	}
}

void Pathfinder::setMap(Map * map)
{ 
   chart = map; 
}

AIMesh* Pathfinder::getMesh()
{
   if (!chart) CORE_FATAL("The map hasn't been set but the mesh was requested.");
   return chart->getAIMesh();
}

Path Pathfinder::getPath(Vector2 from, Vector2 to)
{ 
   if (!chart->getAIMesh()) CORE_FATAL("Can't get path because of a missing AIMesh."); 
   return getPath(from, to, PATH_DEFAULT_BOX_SIZE(getMesh()->getSize())); 
}