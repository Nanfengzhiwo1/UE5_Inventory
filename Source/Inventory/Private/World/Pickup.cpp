// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Pickup.h"

#include "Components/InventoryComponent.h"
#include "Items/ItemBase.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetSimulatePhysics(true);
	SetRootComponent(PickupMesh);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	InitializePickup(UItemBase::StaticClass(),ItemQuantity);
}

void APickup::InitializePickup(const TSubclassOf<UItemBase> BaseClass, const int32 InQuantity)
{
	if (!ItemRowHandle.IsNull())
	{
		const FItemData* ItemData=ItemRowHandle.GetRow<FItemData>(ItemRowHandle.RowName.ToString());
		ItemReference=NewObject<UItemBase>(this,BaseClass);

		ItemReference->ID=ItemData->ID;
		ItemReference->ItemType=ItemData->ItemType;
		ItemReference->ItemQuality=ItemData->ItemQuality;
		ItemReference->NumericData=ItemData->NumericData;
		ItemReference->TextData=ItemData->TextData;
		ItemReference->AssetData=ItemData->AssetData;

		ItemReference->NumericData.bIsStackable=ItemData->NumericData.MaxStackSize>1;
		ItemQuantity<=0?ItemReference->SetQuantity(1):ItemReference->SetQuantity(InQuantity);
		PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);

		UpdateInteractableData();
	}
	
}

void APickup::InitializeDrop(UItemBase* ItemToDrop, const int32 InQuantity)
{
	ItemReference=ItemToDrop;
	InQuantity<=0?ItemReference->SetQuantity(1):ItemReference->SetQuantity(InQuantity);
	ItemReference->NumericData.Weight=ItemToDrop->GetItemSingleWeight();
	ItemReference->OwningInventory=nullptr;
	PickupMesh->SetStaticMesh(ItemToDrop->AssetData.Mesh);

	UpdateInteractableData();
}

void APickup::UpdateInteractableData()
{
	InstanceInteractableData.InteractableType=EInteractableType::Pickup;
	InstanceInteractableData.Action=ItemReference->TextData.InteractionText;
	InstanceInteractableData.Name=ItemReference->TextData.Name;
	InstanceInteractableData.Quantity=ItemReference->Quantity;
	InteractableData=InstanceInteractableData;
}

void APickup::BeginFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(true);
	}

}

void APickup::EndFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}

}

void APickup::Interact(AMyCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		TakePickup(PlayerCharacter);
	}
}


void APickup::TakePickup(const AMyCharacter* Taker)
{
	if (!IsPendingKillPending())
	{
		if (ItemReference)
		{
			if (UInventoryComponent* PlayerInventory=Taker->GetInventory())
			{
				const FItemAddResult AddResult=PlayerInventory->HandleAddItem(ItemReference);

				switch (AddResult.OperationResult)
				{
				case EItemAddResult::IAR_NoItemAdded:
					break;
				case EItemAddResult::IAR_PartialAmountItemAdded:
					UpdateInteractableData();
					Taker->UpdateInteractionWidget();
					break;
				case EItemAddResult::IAR_ALLItemAdded:
					Destroy();
					break;
				}
				UE_LOG(LogTemp,Warning,TEXT("%s"),*AddResult.ResultMessage.ToString());
			}
			else
			{
				UE_LOG(LogTemp,Warning,TEXT("Player inventory component is null!"));
			}
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("Pickup internal item reference was somehow null!"))
		}
	}
}

#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName ChangedPropertyName=PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName():NAME_None;

	if (ChangedPropertyName==GET_MEMBER_NAME_CHECKED(FDataTableRowHandle,RowName))
	{
		if (!ItemRowHandle.IsNull())
		{
				const FItemData* ItemData=ItemRowHandle.GetRow<FItemData>(ItemRowHandle.RowName.ToString());
				PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
			
		}
	}
}
#endif


