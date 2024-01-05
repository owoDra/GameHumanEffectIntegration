// Copyright (C) 2023 owoDra

#include "AnimNotify_HumanFootStep.h"

// This Plugin
#include "GameplayTag/GHFXITags_ContextEffect.h"

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
	TraceChannel = ECollisionChannel::ECC_Visibility;
	TraceStartOffset = FVector(0.0, 0.0, 4.0);
	TraceEndOffset = FVector(0.0, 0.0, -8.0);
	VFXScale = FVector::OneVector;
	VolumeMultiplier = 1.0f;
	PitchMultiplier = 1.0f;

#if WITH_EDITORONLY_DATA

	bPreviewInEditor = false;
	bPreviewPhysicalSurfaceAsContext = true;
	PreviewPhysicalSurface = EPhysicalSurface::SurfaceType_Default;

#endif
}


FString UAnimNotify_HumanFootStep::GetNotifyName_Implementation() const
{
	return bIsLeftFoot ? FString(TEXT("L Foot")) : FString("R Foot");
}

void UAnimNotify_HumanFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (auto* OwningActor{ MeshComp ? MeshComp->GetOwner() : nullptr})
	{
		const auto BoneName{ bIsLeftFoot ? ULocomotionHumanNameStatics::FootLeftBoneName() : ULocomotionHumanNameStatics::FootRightBoneName() };
		const auto FootPos{ MeshComp->GetSocketLocation(BoneName) };

		// Prepare Line Trace

		const auto TraceStart{ FootPos + TraceStartOffset };
		const auto TraceEnd{ FootPos + TraceEndOffset };

		auto QueryParams{ FCollisionQueryParams() };
		QueryParams.AddIgnoredActor(OwningActor);

		auto HitResult{ FHitResult(0) };

		// Call Line Trace, Pass in relevant properties

		const auto* World{ OwningActor->GetWorld() };
		const auto bHitSuccess
		{
			World ? World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams, FCollisionResponseParams::DefaultResponseParam) : false
		};

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
			FContextEffectGenericParameter Pram;
			Pram.AudioPitch = PitchMultiplier;
			Pram.AudioVolume = VolumeMultiplier;
			Pram.bHitSuccess = bHitSuccess;
			Pram.Bone = BoneName;
			Pram.Component = MeshComp;
			Pram.Contexts = FGameplayTagContainer::EmptyContainer;
			Pram.EffectTag = EffectTag;
			Pram.HitResult = HitResult;
			Pram.LocationOffset = FVector::ZeroVector;
			Pram.RotationOffset = FRotator::ZeroRotator;
			Pram.VFXScale = VFXScale;

			IContextEffectInterface::Execute_ContextEffectAction(ContextEffectImplementingObject, Pram);
		}

#if WITH_EDITORONLY_DATA
		// This is for Anim Editor previewing, it is a deconstruction of the calls made by the Interface and the Subsystem

		if (bPreviewInEditor)
		{
			// Get the world, make sure it's an Editor Preview World

			if (World && World->WorldType == EWorldType::EditorPreview)
			{
				// Add Preview contexts if necessary
				
				auto Contexts{ PreviewContexts };

				// Convert given Surface Type to Context and Add it to the Contexts for this Preview

				if (bPreviewPhysicalSurfaceAsContext)
				{
					auto PhysicalSurfaceType{ PreviewPhysicalSurface };

					const auto* DevSettings{ GetDefault<UEffectDeveloperSettings>() };

					if (const auto* FoundSurfaceContext{ DevSettings->SurfaceTypeToContextMap.Find(PhysicalSurfaceType) })
					{
						auto SurfaceContext{ *FoundSurfaceContext };

						Contexts.AddTag(SurfaceContext);
					}
				}

				// Libraries are soft referenced, so you will want to try to load them now
			
				if (auto* EffectsLibrariesObj{ PreviewContextEffectsLibrary.TryLoad() })
				{
					// Check if it is in fact a ULyraContextEffectLibrary type

					if (auto* EffectLibrary{ Cast<UContextEffectLibrary>(EffectsLibrariesObj) })
					{
						// Prepare Sounds and Niagara System Arrays

						TArray<USoundBase*> TotalSounds;
						TArray<UNiagaraSystem*> TotalNiagaraSystems;

						// Attempt to load the Effect Library content (will cache in Transient data on the Effect Library Asset)

						EffectLibrary->LoadEffects();

						// If the Effect Library is valid and marked as Loaded, Get Effects from it

						if (EffectLibrary && EffectLibrary->GetContextEffectLibraryLoadState() == EContextEffectLibraryLoadState::Loaded)
						{
							// Prepare local arrays

							TArray<USoundBase*> Sounds;
							TArray<UNiagaraSystem*> NiagaraSystems;

							// Get the Effects

							EffectLibrary->GetEffects(EffectTag, Contexts, Sounds, NiagaraSystems);

							// Append to the accumulating arrays

							TotalSounds.Append(Sounds);
							TotalNiagaraSystems.Append(NiagaraSystems);
						}

						// Cycle through Sounds and call Spawn Sound Attached, passing in relevant data

						for (auto Sound : TotalSounds)
						{
							UGameplayStatics::SpawnSoundAttached(
								Sound, 
								MeshComp, 
								BoneName, 
								FVector::ZeroVector, 
								FRotator::ZeroRotator, 
								EAttachLocation::KeepRelativeOffset,
								false, 
								VolumeMultiplier, 
								PitchMultiplier, 
								0.0f, 
								nullptr, 
								nullptr, 
								true);
						}

						// Cycle through Niagara Systems and call Spawn System Attached, passing in relevant data

						for (auto NiagaraSystem : TotalNiagaraSystems)
						{
							UNiagaraFunctionLibrary::SpawnSystemAttached(
								NiagaraSystem, 
								MeshComp, 
								BoneName, 
								FVector::ZeroVector,
								FRotator::ZeroRotator, 
								VFXScale, 
								EAttachLocation::KeepRelativeOffset, 
								true,
								ENCPoolMethod::None, 
								true, 
								true);
						}
					}
				}

			}
		}
#endif

	}
}
