#include "AIMesh.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include "RAFManager.h"

#define FRACPOS(x)		((x)-((int)(x)))


// TODO: Clean this junk up

AIMesh::AIMesh() :fileStream(0), heightMap(0)
{
}


AIMesh::~AIMesh()
{
   if (heightMap!=0)
      delete[] heightMap;
}

bool AIMesh::load(std::string inputFile)
{
   // Old direct file loading code
	/*std::ifstream t_FileStream(inputFile, std::ios::binary);
	t_FileStream.seekg(0, std::ios::end);
	std::streamsize size = t_FileStream.tellg();
	t_FileStream.seekg(0, std::ios::beg);

	buffer.resize((unsigned)size);

	if (t_FileStream.read(buffer.data(), size))
	{
		fileStream = (__AIMESHFILE*)buffer.data();
		outputMesh(1024, 1024);
		return true;
	}*/

   // LEVELS/Map1/AIPath.aimesh

   buffer.clear();
   std::cout << "Before reading" << std::endl;
   if (RAFManager::getInstance()->readFile(inputFile, buffer)) // Our file exists. Load it.
   {
      std::cout << "After reading" << std::endl;
      for (int i = 0; i < 1024; i++) // For every scanline for the triangle rendering
      {
         scanlineLowest[i].u = scanlineLowest[i].v = scanlineLowest[i].x = scanlineLowest[i].y = scanlineLowest[i].z = 1e10f; // Init to zero
         scanlineHighest[i].u = scanlineHighest[i].v = scanlineHighest[i].x = scanlineHighest[i].y = scanlineHighest[i].z = -1e10f;
      }

      heightMap = new float[1024 * 1024]; // Shows occupied or not
      fileStream = (__AIMESHFILE*)buffer.data();
      outputMesh(1024, 1024);

      std::cout << "Opened AIMesh file for this map." << std::endl;
      return true;
   }
   return false;
}

bool AIMesh::outputMesh(unsigned width, unsigned height)
{
   mapHeight = mapWidth = 0;
	for (unsigned i = 0; i < width*height; i++) heightMap[i] = 0.0f; // Clear the map

   lowX = 9e9, lowY = 9e9, highX = 0, highY = 0; // Init triangle

	for (unsigned i = 0; i < fileStream->triangle_count; i++) 
   // Need to find the absolute values.. So we can map it to 1024x1024 instead of 13000x15000
	{
      // Triangle low X check
		if (fileStream->triangles[i].Face.v1[0] < lowX)
			lowX = fileStream->triangles[i].Face.v1[0];
		if (fileStream->triangles[i].Face.v2[0] < lowX)
			lowX = fileStream->triangles[i].Face.v2[0];
		if (fileStream->triangles[i].Face.v3[0] < lowX)
			lowX = fileStream->triangles[i].Face.v3[0];		

      // Triangle low Y check
		if (fileStream->triangles[i].Face.v1[2] < lowY)
			lowY = fileStream->triangles[i].Face.v1[2];
		if (fileStream->triangles[i].Face.v2[2] < lowY)
			lowY = fileStream->triangles[i].Face.v2[2];
		if (fileStream->triangles[i].Face.v3[2] < lowY)
			lowY = fileStream->triangles[i].Face.v3[2];

      // Triangle high X check
		if (fileStream->triangles[i].Face.v1[0] > highX)
			highX = fileStream->triangles[i].Face.v1[0];
		if (fileStream->triangles[i].Face.v2[0] > highX)
			highX = fileStream->triangles[i].Face.v2[0];
		if (fileStream->triangles[i].Face.v3[0] > highX)
			highX = fileStream->triangles[i].Face.v3[0];
		
      // Triangle high Y check
		if (fileStream->triangles[i].Face.v1[2] > highY)
			highY = fileStream->triangles[i].Face.v1[2];
		if (fileStream->triangles[i].Face.v2[2] > highY)
			highY = fileStream->triangles[i].Face.v2[2];
		if (fileStream->triangles[i].Face.v3[2] > highY)
			highY = fileStream->triangles[i].Face.v3[2];
	}

   mapWidth = (highX - lowX);
   mapHeight = (highY - lowY);
   
   // If the width or width larger?
   if ((highY - lowY) < (highX - lowX))
   {
      highX = 1.0f / (highX - lowX)*width; // We're wider than we're high, map on width
      highY = highX; // Keep aspect ratio Basically, 1 y should be 1 x.
      lowY = 0; // Though we need to project this in the middle, no offset
   }
   else
   {
      highY = 1.0f / (highY - lowY)*height; // We're higher than we're wide, map on width
      highX = highY; // Keep aspect ratio Basically, 1 x should be 1 y.
      // lowX = 0; // X is already in the middle? ??????
   }

   for (unsigned i = 0; i <fileStream->triangle_count; i++) // For every triangle
	{
      Triangle t_Triangle; // Create a triangle that is warped to heightmap coordinates
      t_Triangle.Face.v1[0] = (fileStream->triangles[i].Face.v1[0] - lowX)*highX;
      t_Triangle.Face.v1[1] = fileStream->triangles[i].Face.v1[1];
      t_Triangle.Face.v1[2] = (fileStream->triangles[i].Face.v1[2] - lowY)*highY;

      t_Triangle.Face.v2[0] = (fileStream->triangles[i].Face.v2[0] - lowX)*highX;
      t_Triangle.Face.v2[1] = fileStream->triangles[i].Face.v2[1];
      t_Triangle.Face.v2[2] = (fileStream->triangles[i].Face.v2[2] - lowY)*highY;

      t_Triangle.Face.v3[0] = (fileStream->triangles[i].Face.v3[0] - lowX)*highX;
      t_Triangle.Face.v3[1] = fileStream->triangles[i].Face.v3[1];
      t_Triangle.Face.v3[2] = (fileStream->triangles[i].Face.v3[2] - lowY)*highY;

      /*
      // Draw just the wireframe.
      drawLine(t_Triangle.Face.v1[0], t_Triangle.Face.v1[2], t_Triangle.Face.v2[0], t_Triangle.Face.v2[2], t_Info, width, height);
      drawLine(t_Triangle.Face.v2[0], t_Triangle.Face.v2[2], t_Triangle.Face.v3[0], t_Triangle.Face.v3[2], t_Info, width, height);
      drawLine(t_Triangle.Face.v3[0], t_Triangle.Face.v3[2], t_Triangle.Face.v1[0], t_Triangle.Face.v1[2], t_Info, width, height);
      */

      // Draw this triangle onto the heightmap using an awesome triangle drawing function
      drawTriangle(t_Triangle, heightMap, width, height);
	}

	//writeFile(t_Info, width, height);
	return true;
}

float AIMesh::getY(float argX, float argY)
{
   unsigned pos = (unsigned)((argX - lowX)*highX * 1024.0f) + (argY - lowY)*highY;
   return heightMap[pos];
}

void AIMesh::drawLine(float x1, float y1, float x2, float y2, char *heightInfo, unsigned width, unsigned height)
{
   if ((x1 < 0) || (y1 < 0) || (x1 >= width) || (y1 >= height) ||
      (x2 < 0) || (y2 < 0) || (x2 >= width) || (y2 >= height))
   {
      return;
   }
   float b = x2 - x1;
   float h = y2 - y1;
   float l = fabsf(b);
   if (fabsf(h) > l) l = fabsf(h);
   int il = (int)l;
   float dx = b / (float)l;
   float dy = h / (float)l;
   for (int i = 0; i <= il; i++)
   {
      heightInfo[(unsigned)x1 + (unsigned)y1 * width] = (char)255;
      x1 += dx, y1 += dy;
   }
}

bool AIMesh::writeFile(float *pixelInfo, unsigned width, unsigned height)
{
   std::ofstream imageFile("maps/test.tga", std::ios::binary);
   if (!imageFile) return false;

   // The image header
   unsigned char header[18] = { 0 };
   header[2] = 1;  // truecolor
   header[12] = width & 0xFF;
   header[13] = (width >> 8) & 0xFF;
   header[14] = height & 0xFF;
   header[15] = (height >> 8) & 0xFF;
   header[16] = 24;  // bits per pit_Xel

   imageFile.write((const char*)header, 18);

   // The image data is stored bottom-to-top, left-to-right
   for (int y = height - 1; y >= 0; y--)
      //for (int y = 0; y<height; y++)
   for (int x = 0; x < width; x++)
   {
      imageFile.put((char)(pixelInfo[(y * width) + x]));
      imageFile.put((char)(pixelInfo[(y * width) + x]));
      imageFile.put((char)(pixelInfo[(y * width) + x]));
   }

   // The file footer. This part is totally optional.
   static const char footer[26] =
      "\0\0\0\0"  // no et_XTension area
      "\0\0\0\0"  // no developer directort_Y
      "TRUEVISION-XFILE"  // Yep, this is a TGA file
      ".";
   imageFile.write(footer, 26);

   imageFile.close();
   return true;
}

void AIMesh::drawTriangle(Triangle triangle, float *heightInfo, unsigned width, unsigned height)
{
#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)>(b))?(a):(b))
   // The heart of the rasterizer

   Vertex tempVertex[3];

   tempVertex[0].x = triangle.Face.v1[0];
   tempVertex[0].y = triangle.Face.v1[2];
   tempVertex[0].z = triangle.Face.v1[1];

   tempVertex[1].x = triangle.Face.v2[0];
   tempVertex[1].y = triangle.Face.v2[2];
   tempVertex[1].z = triangle.Face.v2[1];

   tempVertex[2].x = triangle.Face.v3[0];
   tempVertex[2].y = triangle.Face.v3[2];
   tempVertex[2].z = triangle.Face.v3[1];

   fillScanLine(tempVertex[0], tempVertex[1]);
   fillScanLine(tempVertex[1], tempVertex[2]);
   fillScanLine(tempVertex[2], tempVertex[0]);

   float tempWidth = width;
   float tempHeight = height;
   // target width and width

   int startY = (int)MIN(tempVertex[0].y, MIN(tempVertex[1].y, tempVertex[2].y));
   int endY = (int)MAX(tempVertex[0].y, MAX(tempVertex[1].y, tempVertex[2].y));
   // Get the scanline where we start drawing and where we stop.

   endY = MIN(endY, height - 1);
   startY = MAX(0, startY);

   for (int y = startY; y <= endY; y++) // for each scanline
   {
      if (scanlineLowest[y].x<scanlineHighest[y].x) // If we actually have something filled in this scanline
      {
         int yw = y * height;

         float z = scanlineLowest[y].z;
         float u = scanlineLowest[y].u;
         float v = scanlineLowest[y].v;
         // The start of the Z, U, and V coordinate.

         float deltaX = 1.f / (scanlineHighest[y].x - scanlineLowest[y].x);
         // Interpolation over X (change in X between the two, then reverse it so it's usable as multiplication
         // in divisions

         float deltaZ = (scanlineHighest[y].z - scanlineLowest[y].z) * deltaX;
         float deltaU = (scanlineHighest[y].u - scanlineLowest[y].u) * deltaX;
         float deltaV = (scanlineHighest[y].v - scanlineLowest[y].v) * deltaX;
         // The interpolation in Z, U and V in respect to the interpolation of X	

         // Sub-texel correction
         int x = (int)scanlineLowest[y].x;
         int tx = x + 1;
         int distInt = (int)(scanlineHighest[y].x) - (int)(scanlineLowest[y].x);
         if (distInt > 0.0f)
         {
            u += (((float)(tx)) - tx) * deltaU;
            v += (((float)(tx)) - tx) * deltaV;
            z += (((float)(tx)) - tx) * deltaZ;
         }

         if (!(scanlineHighest[y].x<0 || x >= height))
         for (; x<(int)scanlineHighest[y].x; x++) // for each piece of the scanline
         {
            if (x >= height) break; // If we're out of screen, break out this loop

            if (x<0)
            {
               int inverseX = abs(x);
               z += deltaZ * inverseX;
               u += deltaU * inverseX;
               v += deltaV * inverseX;
               x = 0;
            }        


            {
               // Get the point on the texture that we need to draw. It basically picks a pixel based on the uv.

               //a_Target->GetRenderTarget()->Plot(x, tempHeight - y, 255);
               heightInfo[height-x + y * height] = z;
            }

            z += deltaZ;
            u += deltaU;
            v += deltaV;
            // interpolate
         }
      }

      scanlineLowest[y].x = 1e10f;
      scanlineHighest[y].x = -1e10f;
   }
}

void AIMesh::fillScanLine(Vertex vertex1, Vertex vertex2)
{
   // Fills a scanline structure with information about the triangle on this y scanline.

   if (vertex1.y > vertex2.y)
      return fillScanLine(vertex2, vertex1);
   // We need to go from low to high so switch if the other one is higher

   if (vertex2.y < 0 || vertex1.y >= 1024)
      return;
   // There's nothing on this line

   Vertex deltaPos;
   
   deltaPos.x = vertex2.x - vertex1.x;
   deltaPos.y = vertex2.y - vertex1.y;
   deltaPos.z = vertex2.z - vertex1.z;

   float tempWidth = 1024;
   float tempHeight = 1024;

   float t_DYResp = deltaPos.y == 0 ? 0 : 1.f / deltaPos.y;
   int startY = (int)vertex1.y, endY = (int)vertex2.y;

   float x = vertex1.x;
   float z = vertex1.z;

   float deltaX = deltaPos.x * t_DYResp;
   float deltaZ = deltaPos.z * t_DYResp;

   float t_Inc = 1.f - FRACPOS(vertex1.y);

   // subpixel correction
   startY++;
   x += deltaX * t_Inc;
   z += deltaZ * t_Inc;

   if (startY < 0)
   {
      x -= deltaX * startY;
      z -= deltaZ * startY;
      startY = 0;
   }

   // Small fix
   if (endY >= 1024) endY = 1024 - 1;

   // For each scanline that this triangle uses
   for (int y = startY; y <= endY; y++)
   {
      if (x<scanlineLowest[y].x) // If the x is lower than our lowest x
      {
         scanlineLowest[y].x = (x); // Fill the scanline struct with our info
         scanlineLowest[y].y = (float)y;
         scanlineLowest[y].z = z;
      }
      if (x>scanlineHighest[y].x) // If the x is higher than our highest x
      {
         scanlineHighest[y].x = (x); // Fill the scanline struct with our info
         scanlineHighest[y].y = (float)y;
         scanlineHighest[y].z = z;
      }

      // Interpolate
      // Or going to the part of the triangle on the next scanline
      x += deltaX;
      z += deltaZ;
   }
}