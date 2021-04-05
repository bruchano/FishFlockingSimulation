#pragma once
class fish
{
public:
	float px, py;
	float vx, vy;
	float ax, ay;
	float radius;
	float mass;
	float max_speed;
	float range;
	float separation;
	int id;
	int type;

public:
	fish(float, float, float, int, int);
	~fish();
};

