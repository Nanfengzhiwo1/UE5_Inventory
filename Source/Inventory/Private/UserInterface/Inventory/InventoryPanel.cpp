// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryPanel.h"

#include "Character/MyCharacter.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"
#include "Components/InventoryComponent.h"
#include "Items/ItemBase.h"
#include "UserInterface/Inventory/ItemDragDropOperation.h"


void UInventoryPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter=Cast<AMyCharacter>(GetOwningPlayerPawn());
	if (PlayerCharacter)
	{
		InventoryReference=PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.AddUObject(this,&UInventoryPanel::RefreshInventory);
			SetInfoText();
		}
		
	}

}

void UInventoryPanel::SetInfoText() const
{
	const FString WeightInfoValue{FString::SanitizeFloat(InventoryReference->GetInventoryTotalWeight())+"/"+FString::SanitizeFloat(InventoryReference->GetWeightCapacity())};
	const FString CapacityInfoValue{FString::FromInt(InventoryReference->GetInventoryContents().Num())+ "/"+FString::FromInt(InventoryReference->GetSlotsCapacity())};
	
	WeightInfo->SetText(FText::FromString(WeightInfoValue));
	CapacityInfo->SetText(FText::FromString(CapacityInfoValue));
}


void UInventoryPanel::RefreshInventory()
{
	if (InventoryReference && InventorySlotClass)
	{
		InventoryWrapBox->ClearChildren();

		for (UItemBase* const& InventoryItem: InventoryReference->GetInventoryContents())
		{
			UInventoryItemSlot* ItemSlot=CreateWidget<UInventoryItemSlot>(this,InventorySlotClass);
			ItemSlot->SetItemReference(InventoryItem);
			InventoryWrapBox->AddChildToWrapBox(ItemSlot);
		}
		SetInfoText();
	}
}


bool UInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop=Cast<UItemDragDropOperation>(InOperation);

	if (ItemDragDrop->SourceItem&&InventoryReference)
	{
		UE_LOG(LogTemp,Warning,TEXT("Detected an item drop on InventoryPanel."))
		// returning true will stop the drop operation at this widget
		return true;
	}
	// returning false will cause the drop operation to fall through to underlying widgets(if any)
	return false;
}
