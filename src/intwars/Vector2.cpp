// Vector2.cpp

#include "stdafx.h"
#include "Vector2.h"
#include "Object.h"

Vector2::operator MovementVector()
{
	return MovementVector(X, Y);
}

// Returns the length of the vector
float Vector2::Length(){
   return sqrt(X * X + Y * Y);
}

// Returns the length of the vector
float Vector2::SqrLength(){
   return (X * X + Y * Y);
}

// Normalizes the vector
Vector2 Vector2::Normalize(){
   Vector2 vector;
   float length = this->Length();

   if (length != 0){
      vector.X = X / length;
      vector.Y = Y / length;
   }

   return vector;
}

Vector2::~Vector2(void)
{
}