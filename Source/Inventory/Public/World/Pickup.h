// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MyCharacter.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractionInterface.h"
#include "Pickup.generated.h"

class UItemBase;
class UDataTable;

UCLASS()
class INVENTORY_API APickup : public AActor,public IInteractionInterface
{
	GENERATED_BODY()
	
public:

	
	// Sets default values for this actor's properties
	APickup();

	void InitializePickup(const TSubclassOf<UItemBase> BaseClass,const int32 InQuantity);
	void InitializeDrop(UItemBase* ItemToDrop,const int32 InQuantity);
	FORCEINLINE UItemBase* GetItemData(){return ItemReference;}

	virtual void BeginFocus() override;
	virtual void EndFocus() override;

protected:                                                                             

	UPROPERTY(VisibleAnywhere,Category="Pickup | Components")
	UStaticMeshComponent* PickupMesh;
	UPROPERTY(VisibleInstanceOnly,Category="Pickup | Interaction Data")
	FInteractableData  InstanceInteractableData;
	UPROPERTY(VisibleAnywhere,Category="Pickup | Item Reference")
	UItemBase* ItemReference;
	UPROPERTY(EditInstanceOnly,Category="Pickup | Item Initializtion")
	int32 ItemQuantity;
	UPROPERTY(EditInstanceOnly,Category="Pickup | Item Initializtion")
	FDataTableRowHandle ItemRowHandle;
	
	virtual void BeginPlay() override;
	virtual void Interact(AMyCharacter* PlayerCharacter) override;
	void UpdateInteractableData();
	void TakePickup(const AMyCharacter* Taker);

#if WITH_EDITOR
	// when change DesiredItemID in editor ,the static mesh will change by follow the DesiredItemID
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
