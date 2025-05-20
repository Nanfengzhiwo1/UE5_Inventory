// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryToolTip.h"

#include "Items/ItemBase.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"

void UInventoryToolTip::NativeConstruct()
{
	Super::NativeConstruct();
	const UItemBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference();

	switch (ItemBeingHovered->ItemType)
	{
		case EItemType::Armor:
			ItemType->SetText(FText::FromString("Armor"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Weapon:
			ItemType->SetText(FText::FromString("Weapon"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Shield:
			ItemType->SetText(FText::FromString("Shield"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Spell:
			ItemType->SetText(FText::FromString("Spell"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Consumable:
			ItemType->SetText(FText::FromString("Consumable"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case EItemType::Mundane:
			ItemType->SetText(FText::FromString("Mundane"));
			DamageValue->SetVisibility(ESlateVisibility::Collapsed);
			ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
			UsageText->SetVisibility(ESlateVisibility::Collapsed);
			break;
		default:;
	}
	
	ItemName->SetText(ItemBeingHovered->TextData.Name);
	DamageValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.DamageValue));
	ArmorRating->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.ArmorRating));
	UsageText->SetText(ItemBeingHovered->TextData.UsageText);
	ItemDescription->SetText(ItemBeingHovered->TextData.Description);
	
    const FString WeightInfo{"Weight: "+FString::SanitizeFloat(ItemBeingHovered->GetItemStackWeight())};
	StackWeight->SetText(FText::FromString(WeightInfo));
	
	if (ItemBeingHovered->NumericData.bIsStackable)
	{
		const FString StackInfo{"Max stack size: "+FString::FromInt(ItemBeingHovered->NumericData.MaxStackSize)};
		MaxStackSize->SetText(FText::FromString(StackInfo));
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
	}
}
