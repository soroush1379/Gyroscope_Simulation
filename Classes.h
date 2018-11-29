#pragma once

class Particle
{
public:
	double x, y, z, vx, vy, vz, m;
	int red, green, blue;
	bool fixed;

	Particle()
	{
		vx = vy = vz = 0;
	}

	Particle(double x, double y, double z, double m, int red, int green, int blue, bool fixed) :
		x(x), y(y), z(z), m(m), red(red), green(green), blue(blue), fixed(fixed)
	{
		vx = vy = vz = 0;
	}
};

class Spring
{
public:
	int first, second;
	double l0, k;

	Spring()
	{
		l0 = 0;
	}

	Spring(int first, int second, double l0, double k) :
		first(first), second(second), l0(l0), k(k)
	{
		l0 = 0;
	}
};