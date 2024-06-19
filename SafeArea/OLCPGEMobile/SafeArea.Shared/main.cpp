
//////////////////////////////////////////////////////////////////
// Pixel Game Engine Mobile Release 2.2.8                      //
// John Galvin aka Johnngy63: 18-Jun-2024                       //
// iOS Sensor NOT supported, coming soon                        //
// Please report all bugs to https://discord.com/invite/WhwHUMV //
// Or on Github: https://github.com/Johnnyg63					//
//////////////////////////////////////////////////////////////////

//
// Base Project
//


// Set up headers for the different platforms
#if defined (__ANDROID__)

#include "../SafeArea/pch.h"

//#include "pch.h"

#endif

#if defined (__APPLE__)

#include "ios_native_app_glue.h"
#include <memory>

#endif

//#define STBI_NO_SIMD // Removes SIMD Support
// SIMD greatly improves the speed of your game
#if defined(__arm__)||(__aarch64__)

// Use Advance SIMD NEON when loading images for STB Default is SSE2 (x86)
#define STBI_NEON

#endif

#include "olcUTIL_Geometry2D.h"
#include <numeric>
#include <vector>


#define OLC_PGE_APPLICATION
#define OLC_IMAGE_STB
#include "olcPixelGameEngine_Mobile.h"

#define OLC_PGEX_MINIAUDIO
#include "olcPGEX_MiniAudio.h"  // Checkout https://github.com/Moros1138/olcPGEX_MiniAudio Thanks Moros1138
#include <fstream> // Used for saving the savestate to a file
#include <cstdlib>


using namespace olc;
using namespace olc::utils;

/// <summary>
/// To ensure proper cross platform support keep the class name as PGE_Mobile
/// This will ensure the iOS can launch the engine correctly
/// If you change it make the required changes in GameViewController.mm in the iOS app to suit
/// </summary>
class PGE_Mobile : public olc::PixelGameEngine {

public:

	PGE_Mobile() {
		sAppName = "Safe Area Demo";
	}

	/* Vectors */
	std::vector<std::string> vecMessages;
	/* END Vectors*/

	int nFrameCount = 0;
	int nStep = 20;



public:
	//Example Save State Struct and Vector for when your app is paused
	struct MySaveState {
		std::string key;
		int value;
	};

	std::vector<MySaveState> vecLastState;

	std::vector<geom2d::rect<float>>pillars{
			{{100.0001f,100.0001f},{30,30}},
			{{250.0001f,400.0001f},{60,20}},
			{{230.0001f,150.0001f},{75,35}},
			{{320.0001f,240.0001f},{10,10}}
	};

	bool OnUserCreate() override
	{
		return true;
	}

	double gameTime;

	template<typename T>
	inline constexpr v_2d<T>middle(const geom2d::polygon<T>& poly)const {
		vf2d total = std::accumulate(poly.pos.begin(), poly.pos.end(), olc::v_2d<T>{}, [](const olc::v_2d<T>& middle, const olc::v_2d<T>& point) {return std::move(middle) + point; });
		return total / poly.pos.size();
	}

	float angle_difference(float angle_1, float angle_2)
	{
		angle_1 = fmod(angle_1, 2 * geom2d::pi);
		angle_2 = fmod(angle_2, 2 * geom2d::pi);
		float angle_diff = angle_1 - angle_2;

		if (angle_diff > geom2d::pi)
			angle_diff -= 2 * geom2d::pi;
		else if (angle_diff < -geom2d::pi)
			angle_diff += 2 * geom2d::pi;

		return -angle_diff;
	}

	vf2d pointTo(vf2d posFrom, vf2d posTo) {
		return geom2d::line(posFrom, posTo).vector().norm();
	}

	float area(std::vector<vf2d>points) {
		float totalArea{};
		size_t index{};
		while (index < points.size() - 2) {
			totalArea += geom2d::triangle<float>{points[index], points[index + 1], points[index + 2]}.area();
			index++;
		}
		return totalArea;
	}

	struct RenderPolygon {
		std::vector<vf2d>points;
		std::vector<vf2d>uvs;
		std::vector<Pixel>cols;
	};

	// <summary>
	/// Draws a Target Pointer at the center position of Center Point
	/// </summary>
	/// <param name="vCenterPoint">Center Position of the target</param>
	/// <param name="nLineLenght">Length of lines</param>
	/// <param name="nCircleRadus">Center Circle radius</param>
	void DrawTargetPointer(olc::vi2d vCenterPoint, int32_t nLineLength, int32_t nCircleRadus, olc::Pixel p = olc::WHITE)
	{
		/*
						|
						|
					----O----
						|
						|


		*/
		FillCircle(vCenterPoint, nCircleRadus, p);
		DrawLine(vCenterPoint, { vCenterPoint.x, vCenterPoint.y + nLineLength }, p);
		DrawLine(vCenterPoint, { vCenterPoint.x, vCenterPoint.y - nLineLength }, p);
		DrawLine(vCenterPoint, { vCenterPoint.x + nLineLength, vCenterPoint.y }, p);
		DrawLine(vCenterPoint, { vCenterPoint.x - nLineLength, vCenterPoint.y }, p);

	}

	bool OnUserUpdate(float fElapsedTime) override {

		SetDrawTarget(nullptr);

		gameTime += fElapsedTime;

		Clear(PixelLerp({ 0,0,24 }, { 0,0,4 }, sin(geom2d::pi * gameTime / 2)));

		std::string sMessage = "OneLoneCoder.com";
		vecMessages.push_back(sMessage);

		sMessage = "PGE Mobile Release 2.2.8";
		vecMessages.push_back(sMessage);

		sMessage = "NOTE: Android FPS = CPU FPS, iOS = GPU FPS";
		vecMessages.push_back(sMessage);

		nFrameCount = GetFPS();

		sMessage = sAppName + " - FPS: " + std::to_string(nFrameCount);
		vecMessages.push_back(sMessage);

		DrawCircle(GetTouchPos(), 4, BLUE);

		std::vector<RenderPolygon>polygonList;
		for (geom2d::rect<float>pillar : pillars) 
		{

			if (geom2d::overlaps(GetTouchPos(), pillar))
				continue;

			//First draw 4 lines to the corners.
			const vf2d upperLeftCorner{ pillar.pos };
			const vf2d upperRightCorner{ pillar.pos.x + pillar.size.x,pillar.pos.y };
			const vf2d lowerLeftCorner{ pillar.pos.x,pillar.pos.y + pillar.size.y };
			const vf2d lowerRightCorner{ pillar.pos.x + pillar.size.x,pillar.pos.y + pillar.size.y };

			const std::vector<vf2d>corners{ upperLeftCorner,upperRightCorner,lowerLeftCorner,lowerRightCorner };

			using AngleDiff = float;
			using Corner = vf2d;
			std::pair<AngleDiff, Corner>largestCorner{};
			std::pair<AngleDiff, Corner>smallestCorner{};

			std::for_each(corners.begin(), corners.end(), 
				[&](const vf2d& corner) 
				{
					float angleToMiddle{ geom2d::line<float>{GetTouchPos(),pillar.middle()}.vector().polar().y };
					float angleToCorner{ geom2d::line<float>{GetTouchPos(),corner}.vector().polar().y };
					AngleDiff diff{ angle_difference(angleToCorner,angleToMiddle) };
					if (largestCorner.first < diff)largestCorner = { diff,corner };
					if (smallestCorner.first > diff)smallestCorner = { diff,corner };
				});

			DrawLineDecal(GetTouchPos(), largestCorner.second, RED);
			DrawLineDecal(GetTouchPos(), smallestCorner.second, RED);

			vf2d tmp_extendedLargestCornerPoint{ geom2d::line<float>{GetTouchPos(),largestCorner.second}.rpoint((100000.f)) };
			vf2d tmp_extendedSmallestCornerPoint{ geom2d::line<float>{GetTouchPos(),smallestCorner.second}.rpoint((100000.f)) };
			std::vector<vf2d>safeAreaPolygon{ largestCorner.second,tmp_extendedLargestCornerPoint,tmp_extendedSmallestCornerPoint,smallestCorner.second };
			safeAreaPolygon.reserve(8);
			vf2d& extendedLargestCornerPoint{ safeAreaPolygon[1] };
			vf2d& extendedSmallestCornerPoint{ safeAreaPolygon[2] };

			const vf2d screenUpperLeftCorner{};
			const vf2d screenUpperRightCorner{ float(ScreenWidth()),0.f };
			const vf2d screenLowerLeftCorner{ 0.0f,float(ScreenHeight()) };
			const vf2d screenLowerRightCorner{ float(ScreenWidth()),float(ScreenHeight()) };

			enum Side {
				TOP,
				LEFT,
				RIGHT,
				BOTTOM
			};

			const std::vector<geom2d::line<float>>screenClipLines
			{
				geom2d::line{screenUpperLeftCorner,screenUpperRightCorner}, //TOP
				geom2d::line{screenUpperLeftCorner,screenLowerLeftCorner}, //LEFT
				geom2d::line{screenUpperRightCorner,screenLowerRightCorner}, //RIGHT
				geom2d::line{screenLowerLeftCorner,screenLowerRightCorner}, //BOTTOM
			};

			std::array<uint8_t, 4>sidesClipped{};

			Side currentSide{ TOP };

			std::for_each(screenClipLines.begin(), screenClipLines.end(), 
				[&](const geom2d::line<float>& clipLine) 
				{
					std::vector<vf2d>largestCornerClipPoint{ geom2d::intersects(clipLine,geom2d::line<float>{largestCorner.second,extendedLargestCornerPoint}) };
					if (largestCornerClipPoint.size() > 0)extendedLargestCornerPoint = largestCornerClipPoint.at(0);

					std::vector<vf2d>smallestCornerClipPoint{ geom2d::intersects(clipLine,geom2d::line<float>{smallestCorner.second,extendedSmallestCornerPoint}) };
					if (smallestCornerClipPoint.size() > 0)extendedSmallestCornerPoint = smallestCornerClipPoint.at(0);

					if (largestCornerClipPoint.size() > 0 || smallestCornerClipPoint.size() > 0) {
						sidesClipped[currentSide]++;
					}

					currentSide = Side(currentSide + 1);
				});

			const uint8_t topBottomClipped = sidesClipped[TOP] + sidesClipped[BOTTOM];
			const uint8_t leftRightClipped = sidesClipped[LEFT] + sidesClipped[RIGHT];


			if (topBottomClipped + leftRightClipped == 1)
			{
				//This means we just add a midpoint, the shape is already complete. This is a no-op.
			}
			else
			{
				if (topBottomClipped == 1 && leftRightClipped == 1)
				{
					//This means we add a new point after index 1 to complete the shape. We still add a midpoint.
					vf2d newCorner{ screenUpperLeftCorner };

					if (sidesClipped[TOP] && sidesClipped[RIGHT])newCorner = screenUpperRightCorner;
					else if (sidesClipped[BOTTOM] && sidesClipped[RIGHT])newCorner = screenLowerRightCorner;
					else if (sidesClipped[BOTTOM] && sidesClipped[LEFT])newCorner = screenLowerLeftCorner;

					safeAreaPolygon.insert(safeAreaPolygon.begin() + 2, newCorner);
				}
				else
				{
					if (topBottomClipped == 2 || leftRightClipped == 2) {
						//This means we add two new points after index 1 to complete the shape. We still add a midpoint.
						vf2d angleToRectMiddle{ pointTo(GetTouchPos(),pillar.middle()) };

						//Assume top and bottom clipping.

						if (topBottomClipped)
						{
							const bool PlayerLeftOfPillar = angleToRectMiddle.x > 0;
							if (PlayerLeftOfPillar)
							{
								safeAreaPolygon.insert(safeAreaPolygon.begin() + 2, screenUpperRightCorner);
								safeAreaPolygon.insert(safeAreaPolygon.begin() + 3, screenLowerRightCorner);
							}
							else
							{
								safeAreaPolygon.insert(safeAreaPolygon.begin() + 2, screenLowerLeftCorner);
								safeAreaPolygon.insert(safeAreaPolygon.begin() + 3, screenUpperLeftCorner);
							}
						}
						else
							if (leftRightClipped)
							{
								const bool PlayerAbovePillar = angleToRectMiddle.y > 0;
								if (PlayerAbovePillar) {
									safeAreaPolygon.insert(safeAreaPolygon.begin() + 2, screenLowerRightCorner);
									safeAreaPolygon.insert(safeAreaPolygon.begin() + 3, screenLowerLeftCorner);
								}
								else {
									safeAreaPolygon.insert(safeAreaPolygon.begin() + 2, screenUpperLeftCorner);
									safeAreaPolygon.insert(safeAreaPolygon.begin() + 3, screenUpperRightCorner);
								}
							}
					}
					else
					{

						sMessage = "Something really bad happened! THIS SHOULD NOT BE HAPPENING!: ";
						vecMessages.push_back(sMessage);
						sMessage = "Top - Bottom Clip Count : " + std::to_string(topBottomClipped) + ", Left - Right Clip Count : " + std::to_string(leftRightClipped);
						vecMessages.push_back(sMessage);

						//std::cout << std::format("Something really bad happened! THIS SHOULD NOT BE HAPPENING! Top-Bottom Clip Count:{}, Left-Right Clip Count:{}", topBottomClipped, leftRightClipped);
						//throw;
					}
				}
			}

				

			safeAreaPolygon.insert(safeAreaPolygon.begin(), middle(geom2d::polygon<float>{safeAreaPolygon}));
			safeAreaPolygon.emplace_back(safeAreaPolygon[1]);

			std::vector<vf2d>uvs;
			uvs.resize(safeAreaPolygon.size());
			uvs[0] = { 1.f,1.f };
			std::vector<Pixel>cols;
			cols.resize(safeAreaPolygon.size(), DARK_GREEN);
			cols[0] = GREEN;

			polygonList.emplace_back(RenderPolygon{ safeAreaPolygon,uvs,cols });

			FillCircle(upperLeftCorner, 2, RED);
			FillCircle(upperRightCorner, 2, RED);
			FillCircle(lowerLeftCorner, 2, RED);
			FillCircle(lowerRightCorner, 2, RED);
		}

		std::sort(polygonList.begin(), polygonList.end(), 
			[&](const RenderPolygon& poly1, const RenderPolygon& poly2) 
			{
				return area(poly1.points) < area(poly2.points);
			});

		for (RenderPolygon& poly : polygonList)
		{
			DrawPolygonDecal(nullptr, poly.points, poly.uvs, poly.cols);
		}
			

		for (geom2d::rect<float>pillar : pillars)
		{
			FillRectDecal(pillar.pos, pillar.size);
		}
			

		
		nStep = 10;
		for (auto& s : vecMessages)
		{
			DrawString(20, nStep, s);
			nStep += 10;
		}
		vecMessages.clear();

		return true;
	}

	bool OnUserDestroy() override {
		return true;
	}

	void OnSaveStateRequested() override
	{
		// Fires when the OS is about to put your game into pause mode
		// You have, at best 30 Seconds before your game will be fully shutdown
		// It depends on why the OS is pausing your game tho, Phone call, etc
		// It is best to save a simple Struct of your settings, i.e. current level, player position etc
		// NOTE: The OS can terminate all of your data, pointers, sprites, layers can be freed
		// Therefore do not save sprites, pointers etc 

		// Example 1: vector
		vecLastState.clear();
		vecLastState.push_back({ "MouseX", 55 });
		vecLastState.push_back({ "MouseY", 25 });
		vecLastState.push_back({ "GameLevel", 5 });

#if defined(__ANDROID__)
		// You can save files in the android Internal app storage
		const char* internalPath = app_GetInternalAppStorage(); //Android protected storage
#endif
#if defined(__APPLE__)
		// For iOS the internal app storage is read only, therefore we use External App Storage
		const char* internalPath = app_GetExternalAppStorage(); // iOS protected storage AKA /Library
#endif

		std::string dataPath(internalPath);

		// internalDataPath points directly to the files/ directory                                  
		std::string lastStateFile = dataPath + "/lastStateFile.bin";

		std::ofstream file(lastStateFile, std::ios::out | std::ios::binary);

		if (file)
		{
			float fVecSize = vecLastState.size();
			file.write((char*)&fVecSize, sizeof(long));
			for (auto& vSS : vecLastState)
			{
				file.write((char*)&vSS, sizeof(MySaveState));
			}

			file.close();
		}


	}

	void OnRestoreStateRequested() override
	{
		// This will fire every time your game launches 
		// OnUserCreate will be fired again as the OS may have terminated all your data

#if defined(__ANDROID__)
		// You can save files in the android Internal app storage
		const char* internalPath = app_GetInternalAppStorage(); //Android protected storage
#endif
#if defined(__APPLE__)
		// For iOS the internal app storage is read only, therefore we use External App Storage
		const char* internalPath = app_GetExternalAppStorage(); // iOS protected storage AKA /Library
#endif

		std::string dataPath(internalPath);
		std::string lastStateFile = dataPath + "/lastStateFile.bin";

		vecLastState.clear();

		std::ifstream file(lastStateFile, std::ios::in | std::ios::binary);

		MySaveState saveState;

		if (file)
		{
			float fVecSize = 0.0f;
			file.read((char*)&fVecSize, sizeof(long));
			for (long i = 0; i < fVecSize; i++)
			{
				file.read((char*)&saveState, sizeof(MySaveState));
				vecLastState.push_back(saveState);
			}

			file.close();
			// Note this is a temp file, we must delete it
			std::remove(lastStateFile.c_str());

		}


	}

};


#if defined (__ANDROID__)
/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
* This is now what drives the engine, the thread is controlled from the OS
*/
void android_main(struct android_app* initialstate) {

	/*
		initalstate allows you to make some more edits
		to your app before the PGE Engine starts
		Recommended just to leave it at its defaults
		but change it at your own risk
		to access the Android/iOS directly in your code
		android_app* pMyAndroid = this->pOsEngine.app;;

	*/

	PGE_Mobile demo;

	/*
		Note it is best to use HD(1280, 720, ? X ? pixel, Fullscreen = true) the engine can scale this best for all screen sizes,
		without affecting performance... well it will have a very small affect, it will depend on your pixel size
		Note: cohesion is currently not working
	*/
	demo.Construct(1280, 720, 2, 2, true, false, false);

	demo.Start(); // Lets get the party started


}

#endif

#if defined(__APPLE__)

/*
* The is the calling point from the iOS Objective C, called during the startup of your application
* Use the objects definded in IOSNativeApp to pass data to the Objective C
* By Default you must at minmum pass the game construct vars, pIOSNatvieApp->SetPGEConstruct
*
* iOS runs in its own threads, with its own
* event loop for receiving input events and doing other things.
* This is now what drives the engine, the thread is controlled from the OS
*/
int ios_main(IOSNativeApp* pIOSNatvieApp)
{
	// The iOS will instance your app differnetly to how Android does it
	// In the iOS it will automatically create the required classes and pointers
	// to get the PGE up and running successfull.

	// IMPORTANT: You must set your class name to PGE_Mobile (see above) always for iOS
	// Don't worry it will not conflict with any other apps that use the same base class name of PGE_Mobile
	// I got your back

	// Finally just like the Android you can access any avialble OS options using pIOSNatvieApp
	// Please note options will NOT be the same across both platforms
	// It is best to use the build in functions for File handling, Mouse/Touch events, Key events, Joypad etc

	//
	// To access the iOS directly in your code
	// auto* pMyApple = this->pOsEngine.app;
	//

	/*
		Note it is best to use HD(0, 0, ? X ? pixel, Fullscreen = true) the engine can scale this best for all screen sizes,
		without affecting performance... well it will have a very small affect, it will depend on your pixel size
		Note: cohesion is currently not working
		Note: It is best to set maintain_aspect_ratio to false, Fullscreen to true and use the olcPGEX_TransformView.h to manage your world-view
		in short iOS does not want to play nice, the screen ratios and renta displays make maintaining a full screen with aspect radio a pain to manage
	*/
	pIOSNatvieApp->SetPGEConstruct(0, 0, 2, 2, true, true, false);


	// We now need to return SUCCESS or FAILURE to get the party stated!!!!
	return EXIT_SUCCESS;
}

#endif


