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
	vector<fish> vecFishs;
	fish* pSelectedFish = nullptr;

	vector<food> vecFoods;

	void AddFish(float x, float y, float r, float MaxSpeed, int Type, float Range, float Separation) {
		fish f(x, y, r, Type, vecFishs.size());
		f.vx = (float)rand() * 2 / RAND_MAX - 1.0f;
		if ((float)rand() / RAND_MAX > 0.5f)
			f.vy = sqrtf(1 - f.vx * f.vx);
		else
			f.vy = -sqrtf(1 - f.vx * f.vx);
		f.max_speed = MaxSpeed;
		f.range = Range;
		f.separation = Separation;
		vecFishs.emplace_back(f);

	}

public:
	bool OnUserCreate() {
		modelFish.push_back({ 0, 0 });
		int nPoints = 20;
		for (int i = 0; i < nPoints; i++)
			modelFish.push_back({ cosf(i / (float)(nPoints - 1) * 2 * 3.14159f), sinf(i / (float)(nPoints - 1) * 2 * 3.14159f) });

		int numFish = 200;
		float fDefaultRad = 1.0f;
		float fMaxSpeed = 60.0f;
		float fRange = 20.0f;
		float fSeparation = 4.0f;

		for (int i = 0; i < numFish; i++) {
			float x, y;
			x = (float)rand() / RAND_MAX * ScreenWidth();
			y = (float)rand() / RAND_MAX * ScreenHeight();
			if (vecFishs.size() != 0) {
				for (int j = 0; j < vecFishs.size(); j++) {
					if (DistSqu(x, y, vecFishs[j].px, vecFishs[j].py) < (2 * fDefaultRad) * (2 * fDefaultRad)) {
						x = (float)rand() / RAND_MAX * ScreenWidth();
						y = (float)rand() / RAND_MAX * ScreenHeight();
						j = 0;
					}
				}
			}
			AddFish(x, y, fDefaultRad, fMaxSpeed, 0, fRange, fSeparation);
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

		auto IsInRange = [](float x1, float y1, float x2, float y2, float r) {
			return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= (r * r);
		};

		auto IsInSightAngle = [](float vx, float vy, float x1, float y1, float x2, float y2) {
			float fDist = Dist(x1, y1, x2, y2);
			float fDirx = (x1 - x2) / fDist;
			float fDiry = (y1 - y2) / fDist;
			return (vx * fDirx) + (vy * fDiry) <= -cosf(3.14159f / 2);
		};

		auto IsPredator = [](int Type) {
			return Type >= 1;
		};

		if (m_mouse[1].bPressed) {
			float fPredatorRad = 10.0f;
			float fPredatorSpeed = 10.0f + (float)rand() / RAND_MAX * 20.0f;
			float fPredatorRange = 40.0f;
			float fPredatorSeparation = 30.0f;
			AddFish(m_mousePosX, m_mousePosY, fPredatorRad, fPredatorSpeed, 1, fPredatorRange, fPredatorSeparation);
		}

		if (m_mouse[0].bPressed) {
			pSelectedFish = nullptr;
			for (auto& a : vecFishs) {
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
		for (auto& a : vecFishs) {
			for (auto& b : vecFishs) {
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
		for (auto& a : vecFishs) {

			// random motion
			float fRandWeight = 3.0f;
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
			for (auto& b : vecFishs) {
				if (a.id != b.id) {
					if (IsInRange(a.px, a.py, b.px, b.py, a.range)) {
						if (IsInSightAngle(a.vx, a.vy, a.px, a.py, b.px, b.py)) {
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
			float fMinEscapeDistance = 20.0f;
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
						}

						if (DistSqu(a.px, a.py, b.px, b.py) < a.separation * a.separation) {
							fSeparationX += a.px - b.px;
							fSeparationY += a.py - b.py;
						}
					}

					if (a.type < b.type) {
						if (Dist(a.px, a.py, b.px, b.py) < fMinEscapeDistance) {
							fEscapeX += a.px - b.px;
							fEscapeY += a.py - b.py;
						}
					}

				}

				float fVMeanAbs = sqrtf(vx_mean * vx_mean + vy_mean * vy_mean);
				if (fVMeanAbs != 0.0f) {
					fAlignmentX = vx_mean / fVMeanAbs - a.vx;
					fAlignmentX = vy_mean / fVMeanAbs - a.vy;
				}

				x_mean /= (float)vecNeighbourFishs.size();
				y_mean /= (float)vecNeighbourFishs.size();
				if (x_mean != 0.0f && y_mean != 0.0f) {
					float fDistAbs = Dist(a.px, a.py, x_mean, y_mean);
					fCohesionX = -(a.px - x_mean) / fDistAbs - a.vx;
					fCohesionY = -(a.py - y_mean) / fDistAbs - a.vy;
				}

				float fSepAbs = sqrtf(fSeparationX * fSeparationX + fSeparationY * fSeparationY);
				if (fSepAbs != 0.0f) {
					fSeparationX /= fSepAbs;
					fSeparationY /= fSepAbs;
				}

				float fEscapeAbs = sqrtf(fEscapeX * fEscapeX + fEscapeY * fEscapeY);
				if (fEscapeAbs != 0.0f) {
					fEscapeX /= fEscapeAbs;
					fEscapeY /= fEscapeAbs;
				}
			}

			float fSepWeight = 50.0f;
			float fAliWeight = 30.0f;
			float fCohWeight = 10.0f;

			a.ax += fSeparationX * fSepWeight + fAlignmentX * fAliWeight + fCohesionX * fCohWeight;
			a.ay += fSeparationY * fSepWeight + fAlignmentY * fAliWeight + fCohesionY * fCohWeight;

			float fPredatorWeight = 10000000.0f;
			a.ax += fEscapeX * fPredatorWeight;
			a.ay += fEscapeY * fPredatorWeight;

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

		for (auto& a : vecFishs) {
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
	if (simulation.ConstructConsole(200, 120, 8, 8))
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