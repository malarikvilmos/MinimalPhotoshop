#pragma once
#include "application.h"
#include "image.h"

enum class State
{
	None,
	Invert,
	GammaTransform,
	LogarithmicTransform,
	Grayscale,
	Histogram,
	BoxFilter,
	GaussFilter,
	SobelEdge,
	LaplaceEdge,
	FeatureDetection
};

class GUI
{
public:
	inline static Application* app = nullptr;
	inline static Image* image = nullptr;

	static void Init(Application* app);
	static void Shutdown();
	static void Begin();
	static void End();
	static void DrawMenuBar();
	static void DrawEffectsBar();
	static void DrawOptionsBar();
	static void DrawTimeTaken();
	static void DrawMultithreaded();

private:
	inline static bool multithreading = false;
	inline static State state = State::None;
	inline static int scalarLogarithmic = 50;
	inline static float gammaGamma = 1.0f;
	inline static float histogramLeft[256] = { 0.0f };
	inline static float histogramRight[256] = {0.0f};
	inline static int threshold = 75;
	inline static int thresholdMoravec = 10;
};
