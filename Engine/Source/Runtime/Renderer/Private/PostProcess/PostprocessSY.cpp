// Copyright Epic Games, Inc. All Rights Reserved.


#include "PostProcess/PostprocessSY.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "ShaderCompilerCore.h"
#include "PixelShaderUtils.h"
#include "SceneRendering.h"
namespace
{

class FMyTestPassPS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FMyTestPassPS);
    SHADER_USE_PARAMETER_STRUCT(FMyTestPassPS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture01)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture02)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture03)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture04)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture05)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Color)
	SHADER_PARAMETER(FScreenTransform, SvPositionToColor)
	SHADER_PARAMETER(FVector4f, MyColor)
	SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
	RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};
//创建一个着色器使用时通过”FMyTestPassPS“这个去指定对应的着色器 示例：TShaderMapRef<FMyTestPassPS> PixelShader(View.ShaderMap);
IMPLEMENT_GLOBAL_SHADER(FMyTestPassPS, "/Engine/Private/PostProcessSY.usf", "MainPS", SF_Pixel);

} //! namespace

FScreenPassTexture AddMyTestPass(
    FRDGBuilder& GraphBuilder,
    const FViewInfo& View,
    const FMyTestInputs& Inputs)
{
	//检查场景颜色纹理是否有效
    check(Inputs.SceneColor.IsValid());
	//创建一个事件范围”SY“
    RDG_EVENT_SCOPE(GraphBuilder, "SY");
	//创建一个输出类型，用于存储输出，会先判断Output是否是有效的如果有效则跳过，无效则自己声明
    FScreenPassRenderTarget Output = Inputs.OverrideOutput;
    if (!Output.IsValid())
    {
    	Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Inputs.SceneColor, View.GetOverwriteLoadAction(), TEXT("MotionVectors.Visualize"));
    }
	//获取到场景颜色纹理的视口
    const FScreenPassTextureViewport Viewport(Inputs.SceneColor);
	//获取到当前视图的的视图family
    const FSceneViewFamily& ViewFamily = *(View.Family);
	//分配一个名为 PassParameters 的 FMyTestPassPS::FParameters 类型的实例，用于存储着色器参数。
    FMyTestPassPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FMyTestPassPS::FParameters>();
	//输入shader所需参数
    PassParameters->MyColor = FLinearColor(Inputs.MyColor);

	
    PassParameters->InputTexture = Inputs.SceneColor.Texture;
	PassParameters->InputTexture01 = Inputs.SceneColor01.Texture;
	PassParameters->InputTexture02 = Inputs.SceneColor02.Texture;
	PassParameters->InputTexture03 = Inputs.SceneColor03.Texture;
	PassParameters->InputTexture04 = Inputs.SceneColor04.Texture;
	PassParameters->InputTexture05 = Inputs.SceneColor05.Texture;
	//将shader输出的渲染目标绑定到着色器的输出目标上
	PassParameters->ColorSampler     = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
    PassParameters->RenderTargets[0] = Output.GetRenderTargetBinding();
	FScreenTransform SvPositionToViewportUV = FScreenTransform::SvPositionToViewportUV(Output.ViewRect);
	PassParameters->SvPositionToColor = (
		SvPositionToViewportUV *
		FScreenTransform::ChangeTextureBasisFromTo(Inputs.SceneColor, FScreenTransform::ETextureBasis::ViewportUV, FScreenTransform::ETextureBasis::TextureUV));
	//创建一个类型为”FMyTestPassPS“名为 PixelShader 的 TShaderMapRef 类型的实例，表示 SY 的像素着色器。
    TShaderMapRef<FMyTestPassPS> PixelShader(View.ShaderMap);
	//将SY的pass添加到Rendermap中
    FPixelShaderUtils::AddFullscreenPass(
        GraphBuilder,//渲染图生成器，用于构建渲染操作的依赖关系图
        View.ShaderMap,//这是一个着色器映射，包含了所有与当前视图相关的着色器。
        RDG_EVENT_NAME("SY %dx%d", Output.ViewRect.Width(), Output.ViewRect.Height()),//这是一个事件名称，用于在性能分析工具中标识这个渲染过程。
        PixelShader,//这是一个像素着色器的引用，每个像素会执行此操作
        PassParameters,//着色器参数的结构体
        Output.ViewRect);//这是一个矩形定义了全屏四边形的大小和位置

    return MoveTemp(Output);//将 MyTestPass 的输出纹理（FScreenPassTexture 类型）返回给调用者。
}