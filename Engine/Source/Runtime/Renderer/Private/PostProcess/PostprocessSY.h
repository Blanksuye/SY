#pragma once

#include "ScreenPass.h"

struct FMyTestInputs
{
	FScreenPassRenderTarget OverrideOutput;
	FScreenPassTexture SceneColor;
	FScreenPassTexture SceneColor01;
	FScreenPassTexture SceneColor02;
	FScreenPassTexture SceneColor03;
	FScreenPassTexture SceneColor04;
	FScreenPassTexture SceneColor05;


	FVector4 MyColor;
};

FScreenPassTexture AddMyTestPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const FMyTestInputs& Inputs);