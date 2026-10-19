#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterPreview.generated.h"

class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UPointLightComponent;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;

UCLASS()
class RUSTFALL_API ACharacterPreview : public AActor
{
	GENERATED_BODY()

public:
	ACharacterPreview();

	void SetPreviewMesh(USkeletalMesh* Mesh);

	void FollowPose(USkeletalMeshComponent* Leader);

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UMaterialInstanceDynamic* GetPreviewMaterial() const { return PreviewMaterial; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	USkeletalMeshComponent* PreviewMesh;

	UPROPERTY()
	USceneCaptureComponent2D* Capture;

	UPROPERTY()
	UPointLightComponent* KeyLight;

	UPROPERTY()
	UPointLightComponent* FillLight;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterial;
};
