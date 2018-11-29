#include <iostream>
#include <SDL.h>
#include <cmath>
#include <stdlib.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <fstream>
#include "Classes.h";

using namespace std;

SDL_Window *WindowMain;
SDL_Surface *SurfaceMain;
SDL_Renderer *RendererMain;

vector<Particle> Particles;
vector<Spring> Connections;

const int RADIUS_PARTICLE = 5;
const int NUMBER_CIRCLE = 20;
const int RADIUS_CIRCLE = 100;
const double G = 100000 * 5;
const double DT = 1e-4;
const double CONSTANT_SPRING = 1000000;
const double VELOCITY_WHEEL_FACTOR = 10000;
const double CIRCLE_D = 200;
double VELOCITY_WHEEL;

double Distance(int i1, int i2);

double xbar, ybar, zbar;

void Initialize_SDL();
void Initialize_Values();
void Draw_Circle(int x, int y, int r);
void Draw();
void Move();

double Momentum();
double AngularVelocity();

enum View
{
	XY,
	XZ,
	YZ
} view;

template <class T>
T min(T a, T b)
{
	if (a > b)
		return b;
	else
		return a;
}

template <class T>
T max(T a, T b)
{
	if (a >= b)
		return a;
	else
		return b;
}

int main(int arg, char *args[])
{
	cout << "This program simulates the motion of a gyroscope. The lines represent a spring with a massive constant.\nThe circles are tiny masses representing the gyroscope.\nIn order to view the gyroscope from different angles, press the keys 1, 2, and 3.\nWritten by: Soroush Khoubyarian" << endl;

	cout << "***************************\n\n";

	double a = -1;
	while (a < 0 || a > 5)
	{
		cout << "Please select the angular velocity of the gyroscope(It can be any real number from 0 to 5), then press enter: ";
		cin >> a;

		if (a < 0 || a > 5)
		{
			cout << "\"" << a << "\" is not in the interval 0-5. Please Insert a valid number." << endl;
		}
	}
	VELOCITY_WHEEL = VELOCITY_WHEEL_FACTOR * a;

	Initialize_SDL();
	Initialize_Values();

	//ofstream fout("output.txt");

	int index = 0;
	SDL_Event e;
	while (true)
	{
		SDL_PollEvent(&e);

		Move();
		if (index % 1 == 0)
		{
			Draw();
		}
		if (++index % 10 == 0)
		{
			index = 0;
			//fout << AngularVelocity()	 << endl;
		}

		if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
			break;

		if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_1:
				view = XY;
				break;
			case SDLK_2:
				view = XZ;
				break;
			case SDLK_3:
				view = YZ;
				break;
			}
		}
	}

	//fout.close();
	SDL_Quit();

	int aa;
	cin >> aa;
		
	return EXIT_SUCCESS;
}

void Initialize_SDL()
{
	SDL_Init(SDL_INIT_VIDEO);

	WindowMain = SDL_CreateWindow("Gyroscope Simulation", 20, 50, 1500, 750, SDL_WINDOW_SHOWN);
	SurfaceMain = SDL_GetWindowSurface(WindowMain);
	RendererMain = SDL_CreateRenderer(WindowMain, -1, SDL_RENDERER_ACCELERATED);
}

void Initialize_Values()
{
	Particles.push_back(Particle(SurfaceMain->w / 2, 10, 0, 1, 0, 255, 0, true));
	Particles.push_back(Particle(SurfaceMain->w / 2, SurfaceMain->h / 2, 0, 1, 0, 255, 0, true));
	Particles.push_back(Particle(Particles[1].x + CIRCLE_D, SurfaceMain->h / 2, 0, 1, 0, 255, 0, false));

	Connections.push_back(Spring(0, 1, Distance(0, 1), CONSTANT_SPRING));
	Connections.push_back(Spring(1, 2, Distance(1, 2), CONSTANT_SPRING));

	for (int i = 0; i < NUMBER_CIRCLE; i++)
	{
		Particles.push_back(Particle(Particles[2].x, SurfaceMain->h / 2 + RADIUS_CIRCLE * cos((double)i / (double)NUMBER_CIRCLE * 2 * M_PI), RADIUS_CIRCLE * sin((double)i / (double)NUMBER_CIRCLE * 2 * M_PI), 1, (i <= 2) ? 0 : 255, 0, (i <= 2) ? 255 : 0, false));
	}

	for (int i = 0; i < NUMBER_CIRCLE; i++)
	{
		Connections.push_back(Spring(3 + i, (i + 1) % NUMBER_CIRCLE + 3, Distance(3 + i, (i + 1) % NUMBER_CIRCLE + 3), CONSTANT_SPRING));
		for (int j = 7; j <= NUMBER_CIRCLE / 2; j++)
		{
			Connections.push_back(Spring(3 + i, (i + j) % NUMBER_CIRCLE + 3, Distance(3 + i, (i + j) % NUMBER_CIRCLE + 3), CONSTANT_SPRING));
		}

		Connections.push_back(Spring(2, 3 + i, Distance(2, 3 + i), 5 * CONSTANT_SPRING));
		Connections.push_back(Spring(1, 3 + i, Distance(1, 3 + i), 5 * CONSTANT_SPRING));
	}

	for (int i = 0; i < NUMBER_CIRCLE; i++)
	{
		Particles[3 + i].vy = -VELOCITY_WHEEL * sin((double)i / (double)NUMBER_CIRCLE * 2 * M_PI);
		Particles[3 + i].vz = VELOCITY_WHEEL * cos((double)i / (double)NUMBER_CIRCLE * 2 * M_PI);
	}

	xbar = ybar = zbar = 0;
	for (int i = 0; i < Particles.size(); i++)
	{
		xbar += Particles[i].x;
		ybar += Particles[i].y;
		zbar += Particles[i].z;
	}
	xbar /= Particles.size();
	ybar /= Particles.size();
	zbar /= Particles.size();

	xbar -= 200;
}

double Distance(int i1, int i2)
{
	return hypot(Particles[i1].z - Particles[i2].z, hypot(Particles[i1].x - Particles[i2].x, Particles[i1].y - Particles[i2].y));
}

void Draw_Circle(int x, int y, int r)
{
	SDL_Rect rect;

	for (int i = max(0.0, (double)y - (double)r); i < min((double)SurfaceMain->h, (double)y + (double)r); i++)
	{
		for (int j = max(0.0, (double)x - sqrt((double)r * (double)r - pow((double)i - (double)y, 2))); j <= min((double)SurfaceMain->w, (double)x + sqrt((double)r * (double)r - pow((double)i - (double)y, 2))); j++)
		{
			SDL_RenderDrawPoint(RendererMain, j, i);
		}
	}
}

void Draw()
{
	SDL_SetRenderDrawColor(RendererMain, 0, 0, 0, 255);
	SDL_RenderClear(RendererMain);

	for (int i = 0; i < Connections.size(); i++)
	{
		SDL_SetRenderDrawColor(RendererMain, 255, 255, 255, 255);

		switch (view)
		{
		case XY:
			if (Particles[Connections[i].first].z < zbar || Particles[Connections[i].second].z < zbar)
			{
				SDL_SetRenderDrawColor(RendererMain, 125, 125, 125, 255);
			}

			SDL_RenderDrawLine(RendererMain, Particles[Connections[i].first].x - xbar + SurfaceMain->w / 2, Particles[Connections[i].first].y - ybar + SurfaceMain->h / 2, Particles[Connections[i].second].x - xbar + SurfaceMain->w / 2, Particles[Connections[i].second].y - ybar + SurfaceMain->h / 2);
			break;
		case XZ:
			if (Particles[Connections[i].first].y < ybar || Particles[Connections[i].second].y < ybar)
			{
				SDL_SetRenderDrawColor(RendererMain, 255, 255, 255, 125);
			}

			SDL_RenderDrawLine(RendererMain, Particles[Connections[i].first].x - xbar + SurfaceMain->w / 2, Particles[Connections[i].first].z - zbar + SurfaceMain->h / 2, Particles[Connections[i].second].x - xbar + SurfaceMain->w / 2, Particles[Connections[i].second].z - zbar + SurfaceMain->h / 2);
			break;
		case YZ:
			if (Particles[Connections[i].first].x > xbar || Particles[Connections[i].second].x > xbar)
			{
				SDL_SetRenderDrawColor(RendererMain, 255, 255, 255, 125);
			}

			SDL_RenderDrawLine(RendererMain, Particles[Connections[i].first].z - zbar + SurfaceMain->w / 2, Particles[Connections[i].first].y - ybar + SurfaceMain->h / 2, Particles[Connections[i].second].z - zbar + SurfaceMain->w / 2, Particles[Connections[i].second].y - ybar + SurfaceMain->h / 2);
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < Particles.size(); i++)
	{
		SDL_SetRenderDrawColor(RendererMain, Particles[i].red, Particles[i].green, Particles[i].blue, 255);

		switch (view)
		{
		case XY:
			if (Particles[i].z < zbar && i > 2)
				continue;

			Draw_Circle(Particles[i].x - xbar + SurfaceMain->w / 2, Particles[i].y - ybar + SurfaceMain->h / 2, RADIUS_PARTICLE);
			break;
		case XZ:
			if (Particles[i].y < ybar && i > 2)
				continue;

			Draw_Circle(Particles[i].x - xbar + SurfaceMain->w / 2, Particles[i].z - zbar + SurfaceMain->h / 2, RADIUS_PARTICLE);
			break;
		case YZ:
			if (Particles[i].x > xbar && i > 2)
				continue;

			Draw_Circle(Particles[i].z - zbar + SurfaceMain->w / 2, Particles[i].y - ybar + SurfaceMain->h / 2, RADIUS_PARTICLE);
			break;
		default:
			break;
		}
	}

	SDL_RenderPresent(RendererMain);
}

void Move()
{
	double ax, ay, az;

	for (int i = 0; i < Particles.size(); i++)
	{
		if (!Particles[i].fixed)
		{
			ax = ay = az = 0;
			ay += G;

			Particles[i].vx += ax * DT;
			Particles[i].vy += ay * DT;
			Particles[i].vz += az * DT;
		}
	}

	double force;
	for (int i = 0; i < Connections.size(); i++)
	{
		force = CONSTANT_SPRING * (Connections[i].l0 - Distance(Connections[i].first, Connections[i].second));

		if (!Particles[Connections[i].first].fixed)
		{
			ax = ay = az = 0;

			ax = force / Particles[Connections[i].first].m * (Particles[Connections[i].first].x - Particles[Connections[i].second].x) / Distance(Connections[i].first, Connections[i].second);
			ay = force / Particles[Connections[i].first].m * (Particles[Connections[i].first].y - Particles[Connections[i].second].y) / Distance(Connections[i].first, Connections[i].second);
			az = force / Particles[Connections[i].first].m * (Particles[Connections[i].first].z - Particles[Connections[i].second].z) / Distance(Connections[i].first, Connections[i].second);

			Particles[Connections[i].first].vx += ax * DT;
			Particles[Connections[i].first].vy += ay * DT;
			Particles[Connections[i].first].vz += az * DT;
		}
		if (!Particles[Connections[i].second].fixed)
		{
			ax = ay = az = 0;

			ax = force / Particles[Connections[i].second].m * (Particles[Connections[i].second].x - Particles[Connections[i].first].x) / Distance(Connections[i].first, Connections[i].second);
			ay = force / Particles[Connections[i].second].m * (Particles[Connections[i].second].y - Particles[Connections[i].first].y) / Distance(Connections[i].first, Connections[i].second);
			az = force / Particles[Connections[i].second].m * (Particles[Connections[i].second].z - Particles[Connections[i].first].z) / Distance(Connections[i].first, Connections[i].second);

			Particles[Connections[i].second].vx += ax * DT;
			Particles[Connections[i].second].vy += ay * DT;
			Particles[Connections[i].second].vz += az * DT;
		}
	}

	for (int i = 0; i < Particles.size(); i++)
	{
		Particles[i].x += Particles[i].vx * DT;
		Particles[i].y += Particles[i].vy * DT;
		Particles[i].z += Particles[i].vz * DT;
	}
}

double Momentum()
{
	double x, y, z;
	double vx, vy, vz;
	double M;
	double mx, my, mz;

	mx = my = mz = 0;
	for (int i = 2; i < 3 + NUMBER_CIRCLE; i++)
	{
		x = Particles[i].x - Particles[1].x;
		y = Particles[i].y - Particles[1].y;
		z = Particles[i].z - Particles[1].z;

		vx = Particles[i].vx;
		vy = Particles[i].vy;
		vz = Particles[i].vz;

		double M = Particles[i].m;

		mx += M * (y * vz - z * vy);
		my += M * (z * vx - x * vz);
		mz += M * (x * vy - y * vx);
	}
	
	return hypot(mx, hypot(my, mz));
}

double AngularVelocity()
{
	double vx, vy, vz, v;
	v = 0;

	for (int i = 3; i < 3 + NUMBER_CIRCLE; i++)
	{
		vx = Particles[i].vx - Particles[2].vx;
		vy = Particles[i].vy - Particles[2].vy;
		vz = Particles[i].vz - Particles[2].vz;

		v += hypot(vx, hypot(vy, vz));
	}
	v /= NUMBER_CIRCLE;

	return v / RADIUS_CIRCLE;
}