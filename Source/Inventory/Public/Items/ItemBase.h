// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructs.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UItemBase : public UObject
{
	GENERATED_BODY()

#pragma region Properties&Variables
	UPROPERTY(VisibleAnywhere,Category="Item")
	int32 Quantity;
	UPROPERTY(VisibleAnywhere, Category="Item")
	FName ID;
	UPROPERTY(VisibleAnywhere, Category="Item")
	EItemQuality ItemQuality;
	UPROPERTY(VisibleAnywhere, Category="Item")
	EItemType ItemType;
	UPROPERTY(VisibleAnywhere, Category="Item")
	FItemStatistics ItemStatistics;
	UPROPERTY(VisibleAnywhere, Category="Item")
	FItemTextData TextData;
	UPROPERTY(VisibleAnywhere, Category="Item")
	FItemNumericData NumericData;
	UPROPERTY(VisibleAnywhere, Category="Item")
	FItemAssetData AssetData;
#pragma endregion

#pragma region Functions
	UItemBase();
	
	UItemBase* CreateItemCopy();

	UFUNCTION(Category="Item")
	FORCEINLINE float GetItemStackWeight() const{return  Quantity * NumericData.Weight; }
	UFUNCTION(Category="Item")
	FORCEINLINE float GetItemSingleWeight() const{return NumericData.Weight;}
	UFUNCTION(Category="Item")
	FORCEINLINE bool IsFullItemStack() const{return Quantity == NumericData.MaxStackSize;}
	UFUNCTION(Category="Item")
	void SetQuantity(const int32 NewQuantity);
	UFUNCTION(Category="Item")
	virtual void Use(AMyCharacter* Character);

#pragma endregion

protected:
	bool operator ==(const FName& OtherID) const{return this->ID==OtherID;}
};
