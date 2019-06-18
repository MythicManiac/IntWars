#include "Target.h"
#include <cmath>

float Target::distanceWith(Target* target) {
   return distanceWith(target->getX(), target->getY());
}

float Target::distanceWith(float xtarget, float ytarget) {
   return std::sqrt((x - xtarget)*(x - xtarget) + (y - ytarget)*(y - ytarget));
}

float Target::distanceWithSqr(Target* target) {
   return distanceWithSqr(target->getX(), target->getY());
}

float Target::distanceWithSqr(float xtarget, float ytarget) {
   return ((x - xtarget)*(x - xtarget) + (y - ytarget)*(y - ytarget));
}
