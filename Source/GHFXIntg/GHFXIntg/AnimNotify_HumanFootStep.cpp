// Copyright (C) 2023 owoDra

#include "AnimNotify_HumanFootStep.h"

// This Plugin
#include "GameplayTag/GHFXITags_ContextEffect.h"
#include "GHFXIntgLogs.h"

// Game Chracter Extension
#include "CharacterMeshAccessorInterface.h"

// Game Effect Extesnion
#include "ContextEffectComponent.h"
#include "ContextEffectInterface.h"
#include "EffectDeveloperSettings.h"

// Game Locomotion: Human Addon
#include "LocomotionHumanNameStatics.h"

// Engine Features
#include "Components/SkeletalMeshComponent.h"

///////////////////////////////////////////////////////////

#if WITH_EDITORONLY_DATA

// Game Effect Extesnion
#include "ContextEffectLibrary.h"
#include "ActiveContextEffectLibrary.h"

// Engine Features
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#endif // WITH_EDITORONLY_DATA

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotify_HumanFootStep)


UAnimNotify_HumanFootStep::UAnimNotify_HumanFootStep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsLeftFoot = false;
	EffectTag = TAG_ContextEffect_FootStep;
	bPhysicalSurfaceAsContext = true;
	TraceChannel = ECollisionChannel::ECC_Visibility;
	TraceStartOffset = FVector(0.0, 0.0, 5.0);
	TraceEndOffset = FVector(0.0, 0.0, -20.0);
	VFXScale = FVector::OneVector;
	VolumeMultiplier = 1.0f;
	PitchMultiplier = 1.0f;

#if WITH_EDITORONLY_DATA

	bPreviewInEditor = false;
	PreviewPhysicalSurface = EPhysicalSurface::SurfaceType_Default;

#endif // WITH_EDITORONLY_DATA
}


FString UAnimNotify_HumanFootStep::GetNotifyName_Implementation() const
{
	return bIsLeftFoot ? FString(TEXT("L Foot")) : FString("R Foot");
}

void UAnimNotify_HumanFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

#if WITH_EDITORONLY_DATA

	if (auto* World{ MeshComp->GetWorld() })
	{
		if (World->WorldType == EWorldType::EditorPreview)
		{
			TryPlayFootStepEffect_EditorPreview(MeshComp);
		}
		else
		{
			TryPlayFootStepEffect(MeshComp);
		}
	}

#else

	TryPlayFootStepEffect(MeshComp);

#endif // WITH_EDITORONLY_DATA
}

void UAnimNotify_HumanFootStep::TryPlayFootStepEffect(USkeletalMeshComponent* MeshComp)
{
	if (auto* OwningActor{ MeshComp ? MeshComp->GetOwner() : nullptr})
	{
		// If is not main mesh, skip

		if (OwningActor->Implements<UCharacterMeshAccessorInterface>())
		{
			if (MeshComp != ICharacterMeshAccessorInterface::Execute_GetMainMesh(OwningActor))
			{
				return;
			}
		}

		// Prepare Line Trace

		const auto BoneName{ bIsLeftFoot ? ULocomotionHumanNameStatics::FootLeftBoneName() : ULocomotionHumanNameStatics::FootRightBoneName() };
		const auto FootPos{ MeshComp->GetSocketLocation(BoneName) };

		const auto TraceStart{ FootPos + TraceStartOffset };
		const auto TraceEnd{ FootPos + TraceEndOffset };

		auto QueryParams{ FCollisionQueryParams() };
		QueryParams.AddIgnoredActor(OwningActor);
		QueryParams.bReturnPhysicalMaterial = true;

		auto HitResult{ FHitResult(0) };

		// Call Line Trace, Pass in relevant properties

		const auto* World{ OwningActor->GetWorld() };
		const auto bHitSuccess
		{
			World ? World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams, FCollisionResponseParams::DefaultResponseParam) : false
		};

		if (bHitSuccess)
		{
			// Convert PhysicalSurface to Context

			auto Contexts{ FGameplayTagContainer::EmptyContainer };

			if (bPhysicalSurfaceAsContext)
			{
				GetDefault<UEffectDeveloperSettings>()->ConvertHitResultToContext(HitResult, Contexts);
			}

			// Find Context Effect Implementing Object

			UObject* ContextEffectImplementingObject{ nullptr };

			if (OwningActor->Implements<UContextEffectInterface>())
			{
				ContextEffectImplementingObject = OwningActor;
			}
			else
			{
				ContextEffectImplementingObject = OwningActor->FindComponentByClass<UContextEffectComponent>();
			}

			// Call Context Effect Action

			if (ContextEffectImplementingObject)
			{
				IContextEffectInterface::Execute_PlayEffects(
					ContextEffectImplementingObject
					, EffectTag
					, Contexts
					, MeshComp
					, BoneName
					, FVector::ZeroVector
					, FRotator::ZeroRotator
					, EAttachLocation::KeepRelativeOffset
					, VolumeMultiplier
					, PitchMultiplier
					, VFXScale
				);
			}
		}
	}
}

#if WITH_EDITORONLY_DATA

void UAnimNotify_HumanFootStep::TryPlayFootStepEffect_EditorPreview(USkeletalMeshComponent* MeshComp)
{
	// Check if it is in fact a ULyraContextEffectLibrary type

	if (const auto* EffectLibrary{ Cast<UContextEffectLibrary>(PreviewContextEffectsLibrary.TryLoad()) })
	{
		// Load Effects

		auto* ActiveEffectLibrary{ NewObject<UActiveContextEffectLibrary>(this) };
		ActiveEffectLibrary->LoadContextEffectLibrary(EffectLibrary);

		// Cache Foot Pos

		const auto BoneName{ bIsLeftFoot ? ULocomotionHumanNameStatics::FootLeftBoneName() : ULocomotionHumanNameStatics::FootRightBoneName() };
		const auto FootPos{ MeshComp->GetSocketLocation(BoneName) };

		// Convert PhysicalSurface to Context

		auto Contexts{ FGameplayTagContainer::EmptyContainer };

		if (bPhysicalSurfaceAsContext)
		{
			GetDefault<UEffectDeveloperSettings>()->ConvertPhysicalSurfaceToContext(PreviewPhysicalSurface, Contexts);
		}

		// Play Effect

		ActiveEffectLibrary->PlayEffects(
			Contexts
			, MeshComp
			, BoneName
			, FVector::ZeroVector
			, FRotator::ZeroRotator
			, EAttachLocation::KeepRelativeOffset
			, VolumeMultiplier
			, PitchMultiplier
			, VFXScale
		);
	}
}

#endif // WITH_EDITORONLY_DATA
