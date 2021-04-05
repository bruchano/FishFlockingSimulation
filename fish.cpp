#include "fish.h"

fish::fish(float x, float y, float r, int Type, int ObjID) {
	px = x;
	py = y;
	vx = 0;
	vy = 0;
	ax = 0;
	ay = 0;
	radius = r;
	mass = r * r;
	max_speed = 0;
	range = 0;
	separation = 0;
	type = Type;
	id = ObjID;
}

fish::~fish() {

}
