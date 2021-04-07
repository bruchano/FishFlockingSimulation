#include <iostream>
#include <vector>
#include <random>
#include "fish.h"
#include "olcConsoleGameEngine.h"

using namespace std;

float Dist(float, float, float, float); // return the distance
float DistSqu(float, float, float, float); // return the square of distance

struct food
{
	float px, py;
};

class FishPond : public olcConsoleGameEngine
{
public:
	FishPond() {
		m_sAppName = L"Flocking Fish";
	}

private:
	vector<pair<float, float>> modelFish;
	vector<fish> vecFishes;
	fish* pSelectedFish = nullptr;

	vector<food> vecFoods;

	void AddFish(float x, float y, float r, float MaxSpeed, int Type, float Range, float Separation, float SightAngle) {
		fish f(x, y, r, Type, vecFishes.size());
		f.vx = (float)rand() * 2 / RAND_MAX - 1.0f;
		if ((float)rand() / RAND_MAX > 0.5f)
			f.vy = sqrtf(1 - f.vx * f.vx);
		else
			f.vy = -sqrtf(1 - f.vx * f.vx);
		f.max_speed = MaxSpeed;
		f.range = Range;
		f.separation = Separation;
		f.sight_angle = SightAngle;
		vecFishes.emplace_back(f);
	}

private:
	int nFishes = 200;
	float fDefaultRad = 1.0f;
	float fMaxSpeed = 50.0f;
	float fRange = 15.0f;
	float fSightAngle = 2.0f / 3;
	float fSeparation = 4.0f;

	float fPredatorMinRad = 9.0f;
	float fPredatorRadRange = 3.0f;
	float fPredatorMinSpeed = 15.0f;
	float fPredatorSpeedRange = 5.0f;
	float fPredatorRange = 40.0f;
	float fPredatorSightAngle = 8.0f / 9;
	float fPredatorSeparation = 30.0f;
	float fPredatorSepMassDifference = 10.0f;
	
	float fRandWeight = 2.0f;

	float fSepWeight = 15.0f;
	float fAliWeight = 10.0f;
	float fCohWeight = 5.0f;

	float fMinEscapeDistance = 6.0f;
	float fPredatorWeight = 1000.0f;

	float fMinDistFromBoundary = 5.0f;
	float fTurnWeight = 50.0f;

public:
	bool OnUserCreate() {
		modelFish.push_back({ 0, 0 });
		int nPoints = 20;
		for (int i = 0; i < nPoints; i++)
			modelFish.push_back({ cosf(i / (float)(nPoints - 1) * 2 * 3.14159f), sinf(i / (float)(nPoints - 1) * 2 * 3.14159f) });

		

		for (int i = 0; i < nFishes; i++) {
			float x, y;
			x = (float)rand() / RAND_MAX * ScreenWidth();
			y = (float)rand() / RAND_MAX * ScreenHeight();
			if (vecFishes.size() != 0) {
				for (int j = 0; j < vecFishes.size(); j++) {
					if (DistSqu(x, y, vecFishes[j].px, vecFishes[j].py) < (2 * fDefaultRad) * (2 * fDefaultRad)) {
						x = (float)rand() / RAND_MAX * ScreenWidth();
						y = (float)rand() / RAND_MAX * ScreenHeight();
						j = 0;
					}
				}
			}
			AddFish(x, y, fDefaultRad, fMaxSpeed, 0, fRange, fSeparation, fSightAngle);
		}

		/*food snack;
		snack.px = (float)ScreenWidth() * 0.4;
		snack.py = (float)ScreenHeight() * 0.5;
		vecFoods.push_back(snack);*/

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) {
		auto Overlap = [](float x1, float y1, float r1, float x2, float y2, float r2) {
			return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= (r1 + r2) * (r1 + r2);
		};

		auto IsPointInFish = [](float x, float y, float px, float py, float r) {
			return (x - px) * (x - px) + (y - py) * (y - py) <= (r * r);
		};

		auto IsInRange = [](float x1, float y1, float x2, float y2, float radius, float range) {
			return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) - radius <= range;
		};

		auto IsInSightAngle = [](float vx, float vy, float x1, float y1, float x2, float y2, float SightAngle) {
			float fDist = Dist(x1, y1, x2, y2);
			float fDirx = (x1 - x2) / fDist;
			float fDiry = (y1 - y2) / fDist;
			return (vx * fDirx) + (vy * fDiry) <= -cosf(3.14159f * SightAngle);
		};

		auto IsPredator = [](int Type) {
			return Type >= 1;
		};

		auto IsAtBoundary = [](float x, float y, float bx, float by, float r) {
			if (x < r || bx - x < r || y < r || by - y < r)
				return true;
			else
				return false;
		};

		if (m_mouse[1].bPressed) {
			
			AddFish(m_mousePosX, m_mousePosY, fPredatorMinRad + fPredatorRadRange * (float)rand() / RAND_MAX, fPredatorMinSpeed + fPredatorSpeedRange * (float)rand() / RAND_MAX, 1, fPredatorRange, fPredatorSeparation, fPredatorSightAngle);
		}

		if (m_mouse[0].bPressed) {
			pSelectedFish = nullptr;
			for (auto& a : vecFishes) {
				if (IsPointInFish(m_mousePosX, m_mousePosY, a.px, a.py, a.radius)) {
					pSelectedFish = &a;
					break;
				}
			}
		}

		if (m_mouse[0].bHeld) {
			if (pSelectedFish != nullptr) {
				pSelectedFish->px = m_mousePosX;
				pSelectedFish->py = m_mousePosY;
			}
		}

		if (m_mouse[0].bReleased) {
			pSelectedFish = nullptr;
		}

		// handle static collision
		for (auto& a : vecFishes) {
			for (auto& b : vecFishes) {
				if (a.id != b.id || a.type != b.type) {
					if (Overlap(a.px, a.py, a.radius, b.px, b.py, b.radius)) {
						float fDist = Dist(a.px, a.py, b.px, b.py);
						float fOverlapDist = a.radius + b.radius - fDist;
						a.px += fOverlapDist * (a.px - b.px) / fDist * (b.mass / (a.mass + b.mass));
						a.py += fOverlapDist * (a.py - b.py) / fDist * (b.mass / (a.mass + b.mass));
						b.px -= fOverlapDist * (a.px - b.px) / fDist * (a.mass / (a.mass + b.mass));
						b.py -= fOverlapDist * (a.py - b.py) / fDist * (a.mass / (a.mass + b.mass));

					}
				}
			}
		}

		// perform stepping
		for (auto& a : vecFishes) {

			// random motion
			float fRandAx = ((float)rand() * 2 / RAND_MAX - 1) * fRandWeight;
			float fRandAy = ((float)rand() * 2 / RAND_MAX - 1) * fRandWeight;
			a.ax = fRandAx;
			a.ay = fRandAy;
			
			// food attraction
			/*float fFoodAttracX, fFoodAttracY;
			fFoodAttracX = 0.0f;
			fFoodAttracY = 0.0f;
			if (!IsPredator(a.type)) {
				if (vecFoods.size() != 0) {
					int FoodClosest = 0;
					for (int i = 0; i < vecFoods.size(); i++) {
						if (DistSqu(a.px, a.py, vecFoods[i].px, vecFoods[i].py) < DistSqu(a.px, a.py, vecFoods[FoodClosest].px, vecFoods[FoodClosest].py))
							FoodClosest = i;
					}
					float fFoodDist = Dist(a.px, a.py, vecFoods[FoodClosest].px, vecFoods[FoodClosest].py);
					fFoodAttracX = -(a.px - vecFoods[FoodClosest].px) / fFoodDist - a.vx;
					fFoodAttracY = -(a.py - vecFoods[FoodClosest].py) / fFoodDist - a.vy;
					float fFoodAttracAbs = sqrtf(fFoodAttracX * fFoodAttracX + fFoodAttracY * fFoodAttracY);
					if (fFoodAttracAbs != 0.0f) {
						fFoodAttracX /= fFoodAttracAbs;
						fFoodAttracY /= fFoodAttracAbs;
					}
				}
			}
			float fFoodWeight = 1000.0f;
			a.ax += fFoodAttracX * fFoodWeight;
			a.ay += fFoodAttracY * fFoodWeight;*/

			// flocking behaviour
			vector<fish> vecNeighbourFishs;
			for (auto& b : vecFishes) {
				if (a.id != b.id) {
					if (IsInRange(a.px, a.py, b.px, b.py, b.radius, a.range)) {
						if (IsInSightAngle(a.vx, a.vy, a.px, a.py, b.px, b.py, a.sight_angle)) {
							vecNeighbourFishs.push_back(b);
						}
					}
				}
			}
			
			float fSeparationX, fSeparationY, fAlignmentX, fAlignmentY, fCohesionX, fCohesionY;
			fSeparationX = 0.0f;
			fSeparationY = 0.0f;
			fAlignmentX = 0.0f;
			fAlignmentY = 0.0f;
			fCohesionX = 0.0f;
			fCohesionY = 0.0f;

			float fEscapeX, fEscapeY;
			fEscapeX = 0.0f;
			fEscapeY = 0.0f;

			if (vecNeighbourFishs.size() != 0) {
				float x_mean = 0.0f;
				float y_mean = 0.0f;
				float vx_mean = 0.0f;
				float vy_mean = 0.0f;
				for (auto& b : vecNeighbourFishs) {
					if (a.type == b.type) {
						if (!IsPredator(a.type)) {
							x_mean += b.px;
							y_mean += b.py;
							vx_mean += b.vx;
							vy_mean += b.vy;

							if (DistSqu(a.px, a.py, b.px, b.py) < a.separation * a.separation) {
								fSeparationX += a.px - b.px;
								fSeparationY += a.py - b.py;
							}
						}
						else {
							if (DistSqu(a.px, a.py, b.px, b.py) < a.separation * a.separation) {
								if (a.mass - b.mass < fPredatorSepMassDifference) {
									fSeparationX += a.px - b.px;
									fSeparationY += a.py - b.py;
								}
							}
						}
						
					}

					if (a.type < b.type) {
						if (Dist(a.px, a.py, b.px, b.py) - b.radius < fMinEscapeDistance) {
							fEscapeX += a.px - b.px;
							fEscapeY += a.py - b.py;
						}
					}

				}

				if (vx_mean != 0.0f || vy_mean != 0.0f) {
					float fVMeanAbs = sqrtf(vx_mean * vx_mean + vy_mean * vy_mean);
					fAlignmentX = vx_mean / fVMeanAbs;
					fAlignmentX = vy_mean / fVMeanAbs;
				}

				
				if (x_mean != 0.0f && y_mean != 0.0f) {
					x_mean /= (float)vecNeighbourFishs.size();
					y_mean /= (float)vecNeighbourFishs.size();
					float fDistAbs = Dist(a.px, a.py, x_mean, y_mean);
					fCohesionX = -(a.px - x_mean) / fDistAbs;
					fCohesionY = -(a.py - y_mean) / fDistAbs;
				}

				if (fSeparationX != 0.0f || fSeparationY != 0.0f) {
					float fSepAbs = sqrtf(fSeparationX * fSeparationX + fSeparationY * fSeparationY);
					fSeparationX /= fSepAbs;
					fSeparationY /= fSepAbs;
				}

				if (fEscapeX != 0.0f || fEscapeY != 0.0f) {
					float fEscapeAbs = sqrtf(fEscapeX * fEscapeX + fEscapeY * fEscapeY);
					fEscapeX /= fEscapeAbs;
					fEscapeY /= fEscapeAbs;
				}
			}

			

			a.ax += fSeparationX * fSepWeight + fAlignmentX * fAliWeight + fCohesionX * fCohWeight;
			a.ay += fSeparationY * fSepWeight + fAlignmentY * fAliWeight + fCohesionY * fCohWeight;

			a.ax += fEscapeX * fPredatorWeight;
			a.ay += fEscapeY * fPredatorWeight;

			/*if (IsAtBoundary(a.px, a.py, (float)ScreenWidth(), (float)ScreenHeight(), fMinDistFromBoundary)) {
				float fTurnX, fTurnY;
				if (a.px < fMinDistFromBoundary || ScreenWidth() - a.px < fMinDistFromBoundary) {
					fTurnX = -a.vx;
					if (a.vy > 0) 
						fTurnY = 1.0f;
					else
						fTurnY = -1.0f;
				}
				else {
					fTurnY = -a.vy;
					if (a.vx > 0)
						fTurnX = 1.0f;
					else
						fTurnX = -1.0f;
				}
				a.ax += fTurnX * fTurnWeight;
				a.ay += fTurnY * fTurnWeight;
			}*/

			float fVAbs = sqrtf((a.vx + a.ax * fElapsedTime) * (a.vx + a.ax * fElapsedTime) + (a.vy + a.ay * fElapsedTime) * (a.vy + a.ay * fElapsedTime));
			a.vx = (a.vx + a.ax * fElapsedTime) / fVAbs;
			a.vy = (a.vy + a.ay * fElapsedTime) / fVAbs;
			a.px += a.vx * a.max_speed * fElapsedTime;
			a.py += a.vy * a.max_speed * fElapsedTime;

			if (a.px > ScreenWidth()) a.px -= (float)ScreenWidth();
			if (a.px < 0) a.px += (float)ScreenWidth();
			if (a.py > ScreenHeight()) a.py -= (float)ScreenHeight();
			if (a.py < 0) a.py += (float)ScreenHeight();
				
		}


		Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');

		for (auto& a : vecFishes) {
			if (!IsPredator(a.type)) 
				DrawWireFrameModel(modelFish, a.px, a.py, atan2f(a.vy, a.vx), a.radius, FG_WHITE);
			else
				DrawWireFrameModel(modelFish, a.px, a.py, atan2f(a.vy, a.vx), a.radius, FG_RED);
		}

		for (auto& snack : vecFoods) {
			DrawWireFrameModel(modelFish, snack.px, snack.py, 0.0f, 1.0f, FG_YELLOW);
		}

		return true;
	}

};


void main() {
	FishPond simulation;
	if (simulation.ConstructConsole(220, 120, 8, 8))
		simulation.Start();
	else
		wcout << L"Failed Constructing Console" << endl;

	
}

float Dist(float x1, float y1, float x2, float y2) {
	return	sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

float DistSqu(float x1, float y1, float x2, float y2) {
	return	(x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}