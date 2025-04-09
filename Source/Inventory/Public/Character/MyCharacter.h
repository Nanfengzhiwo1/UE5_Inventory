// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/InteractionInterface.h"
#include "Inventory/InventoryCharacter.h"
#include "MyCharacter.generated.h"

class UInventoryComponent;
class AMyHUD;

USTRUCT()
struct FInteractionData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	AActor* CurrentInteractable;
	UPROPERTY()
	float LastInteractionCheckTime;

	FInteractionData():
	CurrentInteractable(nullptr),
	LastInteractionCheckTime(0.0f)
	{
		
	};
};


UCLASS()
class INVENTORY_API AMyCharacter : public AInventoryCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE bool IsInteracting() const{return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction);};

	FORCEINLINE UInventoryComponent* GetInventory()const{return PlayerInventory;};

	void UpdateInteractionWidget()const;
protected:

	UPROPERTY()
	AMyHUD* MyHUD;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category="Character|Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

	UPROPERTY(VisibleAnywhere,Category="Character|Inventory")
	UInventoryComponent* PlayerInventory;

	float InteractionCheckFrequency;

	float InteractionCheckDistance;

	FTimerHandle TimerHandle_Interaction;

	FInteractionData InteractionData;

	void PerformInteractionCheck();
	void FoundInteractable(AActor* NewInteractable);
	void NoInteractableFound();
	void BeginInteract();
	void EndInteract();
	void Interact();
	

};
