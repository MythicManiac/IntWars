

#ifndef VECTOR2_H
#define	VECTOR2_H


#pragma once

#include <math.h>

struct MovementVector;
class Vector2
{
public:
   Vector2() : Vector2(0, 0) { }
   Vector2(float X, float Y) : X(X), Y(Y) { }
   Vector2(const Vector2& v) : X(v.X), Y(v.Y) { }
   ~Vector2(void);
   float Length();
   float SqrLength();
   Vector2 Normalize();
   float X, Y;

	operator MovementVector();

   void operator += (const Vector2& a_V) { X += a_V.X; Y += a_V.Y; }
   void operator += (Vector2* a_V) { X += a_V->X; Y += a_V->Y; }
   void operator -= (const Vector2& a_V) { X -= a_V.X; Y -= a_V.Y; }
   void operator -= (Vector2* a_V) { X -= a_V->X; Y -= a_V->Y; }
   void operator *= (const float f) { X *= f; Y *= f;  }
   void operator *= (const Vector2& a_V) { X *= a_V.X; Y *= a_V.Y; }
   void operator *= (Vector2* a_V) { X *= a_V->X; Y *= a_V->Y; }

   Vector2 operator- () const { return Vector2(-X, -Y); }
   friend Vector2 operator + (const Vector2& v1, const Vector2& v2) { return Vector2(v1.X + v2.X, v1.Y + v2.Y); }
   friend Vector2 operator + (const Vector2& v1, Vector2* v2) { return Vector2(v1.X + v2->X, v1.Y + v2->Y); }
   friend Vector2 operator - (const Vector2& v1, const Vector2& v2) { return Vector2(v1.X - v2.X, v1.Y - v2.Y); }
   friend Vector2 operator - (const Vector2& v1, Vector2* v2) { return Vector2(v1.X - v2->X, v1.Y - v2->Y); }
   friend Vector2 operator - (const Vector2* v1, Vector2& v2) { return Vector2(v1->X - v2.X, v1->Y - v2.Y); }
   // friend Vector2 operator - ( const Vector2* v1, Vector2* v2 ) { return Vector2( v1->X - v2->X, v1->Y - v2->Y, v1->z - v2->z ); }
   friend Vector2 operator * (const Vector2& v, const float f) { return Vector2(v.X * f, v.Y * f); }
   friend Vector2 operator * (const Vector2& v1, const Vector2& v2) { return Vector2(v1.X * v2.X, v1.Y * v2.Y); }
   friend Vector2 operator * (const float f, const Vector2& v) { return Vector2(v.X * f, v.Y * f); }
   friend Vector2 operator / (const Vector2& v, const float f) { return Vector2(v.X / f, v.Y / f); }
   friend Vector2 operator / (const Vector2& v1, const Vector2& v2) { return Vector2(v1.X / v2.X, v1.Y / v2.Y); }
   friend Vector2 operator / (const float f, const Vector2& v) { return Vector2(v.X / f, v.Y / f); }

};
#endif	/* VECTOR2_H */

