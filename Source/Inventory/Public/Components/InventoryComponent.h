// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

class UItemBase;

UENUM(BlueprintType)
enum class EItemAddResult:uint8
{
	IAR_NoItemAdded UMETA(DisplayName="No item added"),
	IAR_PartialAmountItemAdded UMETA(DisplayName="Partial amount of item added"),
	IAR_ALLItemAdded UMETA(DisplayName="All of item added")
};

USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_USTRUCT_BODY()

	FItemAddResult():
	ActualAmountAdded(0),
	OperationResult(EItemAddResult::IAR_NoItemAdded),
	ResultMessage(FText::GetEmpty())
	{
		
	};
	// Actual amount of item that was added to the inventory
	UPROPERTY(BlueprintReadOnly,Category="Item Add Result")
	int32 ActualAmountAdded;
	// Enum representing the end of state of an add item operation
	UPROPERTY(BlueprintReadOnly,Category="Item Add Result")
	EItemAddResult OperationResult;
	// Informational message that can be passed with the result
	UPROPERTY(BlueprintReadOnly,Category="Item Add Result")
	FText ResultMessage;


	// helper function
	static FItemAddResult AddedNone(const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult;
		AddedNoneResult.ActualAmountAdded=0;
		AddedNoneResult.OperationResult=EItemAddResult::IAR_NoItemAdded;
		AddedNoneResult.ResultMessage=ErrorText;
		return AddedNoneResult;
	};
	static FItemAddResult AddedPartial(const int32 PartialAmountAdded,const FText& ErrorText)
	{
		FItemAddResult AddedPartialResult;
		AddedPartialResult.ActualAmountAdded=0;
		AddedPartialResult.OperationResult=EItemAddResult::IAR_PartialAmountItemAdded;
		AddedPartialResult.ResultMessage=ErrorText;
		return AddedPartialResult;
	};
	static FItemAddResult AddedAll(const int32 AllAdded,const FText& Message)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded=0;
		AddedAllResult.OperationResult=EItemAddResult::IAR_ALLItemAdded;
		AddedAllResult.ResultMessage=Message;
		return AddedAllResult;
	};
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	FOnInventoryUpdated OnInventoryUpdated;
	
	// Sets default values for this component's properties
	UInventoryComponent();
	UFUNCTION(Category="Inventory")
    UItemBase* FindMatchingItem(UItemBase* ItemIn)const;
	UFUNCTION(Category="Inventory")
	UItemBase* FindNextItemByID(UItemBase* ItemIn)const;
	UFUNCTION(Category="Inventory")
	UItemBase* FindNextPartialStack(UItemBase* ItemIn)const;
	UFUNCTION(Category="Inventory")
	FItemAddResult HandleAddItem(UItemBase* InputItem);
	UFUNCTION(Category="Inventory")
	void RemoveSingleInstanceOfItem(UItemBase* ItemToMove);
	UFUNCTION(Category="Inventory")
	int32 RemoveAmountOfItem(UItemBase* ItemIn,int32 DesiredAmountToRemove);
	UFUNCTION(Category="Inventory")
	void SplitExistingStack(UItemBase* ItemIn,const int32 AmountToSplit);

	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetInventoryTotalWeight()const{return InventoryTotalWeight;};
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetWeightCapacity()const{return InventoryWeightCapacity;};
	UFUNCTION(Category="Inventory")
	FORCEINLINE int32 GetSlotsCapacity()const{return InventorySlotsCapacity;};
	UFUNCTION(Category="Inventory")
	FORCEINLINE TArray<UItemBase*>GetInventoryContents()const{return InventoryContents;};

	UFUNCTION(Category="Inventory")
	FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity){InventorySlotsCapacity=NewSlotsCapacity;};
	UFUNCTION(Category="Inventory")
	FORCEINLINE void SetWeightCapacity(const float NewWeightCapacity){InventoryWeightCapacity=NewWeightCapacity;};
	
	
protected:
	UPROPERTY(VisibleAnywhere,Category="Inventory")
	float InventoryTotalWeight;
	UPROPERTY(EditInstanceOnly,Category="Inventory")
	int32 InventorySlotsCapacity;
	UPROPERTY(EditInstanceOnly,Category="Inventory")
	float InventoryWeightCapacity;
	UPROPERTY(VisibleAnywhere,Category="Inventory")
	TArray<TObjectPtr<UItemBase>> InventoryContents;
	
	// Called when the game starts
	virtual void BeginPlay() override;
	FItemAddResult HandleNonStackableItem(UItemBase* InputItem);
	int32 HandleStackableItems(UItemBase* InputItem,int32 RequestedAddAmount);
	int32 CalculateWeightAddAmount(UItemBase* ItemIn,int32 RequestedAddAmount);
	int32 CalculateNumberForFullStack(UItemBase* StackableItem,int32 InitialRequestedAddAmount);

	void AddNewItem(UItemBase* Item,int32 AmountToAdd);
};
