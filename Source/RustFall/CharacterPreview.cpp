#include "CharacterPreview.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#if WITH_EDITORONLY_DATA
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionOneMinus.h"
#endif

ACharacterPreview::ACharacterPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComp);
	PreviewMesh->SetRelativeLocation(FVector::ZeroVector);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetCastShadow(false);

	Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
	Capture->SetupAttachment(RootComp);
	Capture->SetRelativeLocation(FVector(250.0f, 0.0f, 95.0f));
	Capture->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Capture->FOVAngle = 30.0f;
	Capture->bCaptureEveryFrame = true;
	Capture->bCaptureOnMovement = false;
	Capture->bAlwaysPersistRenderingState = true;
	Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	Capture->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	Capture->PostProcessSettings.AutoExposureMinBrightness = 0.5f;
	Capture->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	Capture->PostProcessSettings.AutoExposureMaxBrightness = 0.5f;

	Capture->ShowFlags.SetAtmosphere(false);
	Capture->ShowFlags.SetFog(false);
	Capture->ShowFlags.SetVolumetricFog(false);

	KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(RootComp);
	KeyLight->SetRelativeLocation(FVector(180.0f, -160.0f, 220.0f));
	KeyLight->SetIntensity(30.0f);
	KeyLight->SetAttenuationRadius(2000.0f);
	KeyLight->SetCastShadows(false);

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComp);
	FillLight->SetRelativeLocation(FVector(-120.0f, 160.0f, 160.0f));
	FillLight->SetIntensity(15.0f);
	FillLight->SetAttenuationRadius(2000.0f);
	FillLight->SetCastShadows(false);
}

void ACharacterPreview::BeginPlay()
{
	Super::BeginPlay();

	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(384, 672);

	Capture->TextureTarget = RenderTarget;
	Capture->ShowOnlyActors.Add(this);

#if WITH_EDITORONLY_DATA
	UMaterial* CutoutMat = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
	CutoutMat->MaterialDomain = MD_Surface;
	CutoutMat->BlendMode = BLEND_Translucent;
	CutoutMat->SetShadingModel(MSM_Unlit);

	UMaterialExpressionTextureCoordinate* TexCoord = NewObject<UMaterialExpressionTextureCoordinate>(CutoutMat);
	CutoutMat->GetExpressionCollection().AddExpression(TexCoord);

	UMaterialExpressionTextureSampleParameter2D* TexSample = NewObject<UMaterialExpressionTextureSampleParameter2D>(CutoutMat);
	TexSample->ParameterName = TEXT("Src");
	TexSample->SamplerType = SAMPLERTYPE_Color;
	TexSample->Coordinates.Expression = TexCoord;
	TexSample->Coordinates.OutputIndex = 0;
	CutoutMat->GetExpressionCollection().AddExpression(TexSample);

	UMaterialExpressionOneMinus* OneMinus = NewObject<UMaterialExpressionOneMinus>(CutoutMat);
	OneMinus->Input.Expression = TexSample;
	OneMinus->Input.OutputIndex = 4;
	CutoutMat->GetExpressionCollection().AddExpression(OneMinus);

	UMaterialEditorOnlyData* Ed = CutoutMat->GetEditorOnlyData();
	Ed->EmissiveColor.Expression = TexSample;
	Ed->EmissiveColor.OutputIndex = 0;
	Ed->Opacity.Expression = OneMinus;
	Ed->Opacity.OutputIndex = 0;

	CutoutMat->PostEditChange();

	PreviewMaterial = UMaterialInstanceDynamic::Create(CutoutMat, this);
	if (PreviewMaterial)
	{
		PreviewMaterial->SetTextureParameterValue(TEXT("Src"), RenderTarget);
	}
#endif
}

void ACharacterPreview::SetPreviewMesh(USkeletalMesh* Mesh)
{
	if (PreviewMesh)
	{
		PreviewMesh->SetSkeletalMesh(Mesh);
	}
}

void ACharacterPreview::FollowPose(USkeletalMeshComponent* Leader)
{
	if (PreviewMesh && Leader)
	{
		PreviewMesh->SetRelativeRotation(Leader->GetRelativeRotation());
		PreviewMesh->SetLeaderPoseComponent(Leader);
	}
}
