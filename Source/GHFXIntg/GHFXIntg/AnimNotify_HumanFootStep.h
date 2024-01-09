// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"

#include "GameplayTagContainer.h"

#include "AnimNotify_HumanFootStep.generated.h"

class UAnimMontage;


/**
 * AnimNotify class to notify the occurrence of FootStep effects
 */
UCLASS(BlueprintType, meta = (DisplayName = "AN Human Foot Step"))
class UAnimNotify_HumanFootStep : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_HumanFootStep(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep")
	bool bIsLeftFoot;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep", meta = (Categories = "ContextEffect"))
	FGameplayTag EffectTag;

	UPROPERTY(AdvancedDisplay, EditAnywhere, Category = "PreviewProperties", meta = (EditCondition = "bPreviewInEditor"))
	bool bPhysicalSurfaceAsContext;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|Trace")
	FVector TraceStartOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|Trace")
	FVector TraceEndOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|VFX")
	FVector VFXScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|SFX")
	float VolumeMultiplier;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FootStep|SFX")
	float PitchMultiplier;


#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category = "PreviewProperties")
	bool bPreviewInEditor;

	UPROPERTY(EditAnywhere, Category = "PreviewProperties", meta = (EditCondition = "bPreviewInEditor"))
	TEnumAsByte<EPhysicalSurface> PreviewPhysicalSurface;

	UPROPERTY(EditAnywhere, Category = "PreviewProperties", meta = (AllowedClasses = "/Script/LyraGame.LyraContextEffectsLibrary", EditCondition = "bPreviewInEditor"))
	FSoftObjectPath PreviewContextEffectsLibrary;

	UPROPERTY(EditAnywhere, Category = "PreviewProperties", meta = (EditCondition = "bPreviewInEditor"))
	FGameplayTagContainer PreviewContexts;

#endif // WITH_EDITORONLY_DATA

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

protected:
	void TryPlayFootStepEffect(USkeletalMeshComponent* MeshComp);

#if WITH_EDITORONLY_DATA

	void TryPlayFootStepEffect_EditorPreview(USkeletalMeshComponent* MeshComp);

#endif // WITH_EDITORONLY_DATA

};