// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#include "Items/ItemBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}



UItemBase* UInventoryComponent::FindMatchingItem(UItemBase* ItemIn) const
{
	if (ItemIn)
	{
		if (InventoryContents.Contains(ItemIn))
		{
			return ItemIn;
		}
	}
	return nullptr;
}

UItemBase* UInventoryComponent::FindNextItemByID(UItemBase* ItemIn) const
{
	if (ItemIn)
	{
		if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result=InventoryContents.FindByKey(ItemIn))
		{
			return *Result;
		}
	}
	return nullptr;
}

UItemBase* UInventoryComponent::FindNextPartialStack(UItemBase* ItemIn) const
{
	
	if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result=
		// Lambda function
		InventoryContents.FindByPredicate([&ItemIn](const UItemBase* InventoryItem)
		{
			return InventoryItem->ID ==ItemIn->ID && !InventoryItem->IsFullItemStack();
		}
		))
	{
		return *Result;
	}
	return nullptr;
}

int32 UInventoryComponent::CalculateWeightAddAmount(UItemBase* ItemIn, int32 RequestedAddAmount)
{
	const int32 WeightMaxAddAmount=FMath::FloorToInt((GetWeightCapacity()-InventoryTotalWeight)/ItemIn->GetItemSingleWeight());
	if (WeightMaxAddAmount>=RequestedAddAmount)
	{
		return RequestedAddAmount;
	}
	return WeightMaxAddAmount;
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemBase* StackableItem, int32 InitialRequestedAddAmount)
{
	const int32 AddAmountToMakeFullStack=StackableItem->NumericData.MaxStackSize-StackableItem->Quantity;
	return FMath::Min(InitialRequestedAddAmount,AddAmountToMakeFullStack);
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemBase* ItemToMove)
{
	InventoryContents.RemoveSingle(ItemToMove);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemBase* ItemIn, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove,ItemIn->Quantity);
	ItemIn->SetQuantity(ItemIn->Quantity-ActualAmountToRemove);
	InventoryTotalWeight-=ActualAmountToRemove*ItemIn->GetItemSingleWeight();
	OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UInventoryComponent::SplitExistingStack(UItemBase* ItemIn, const int32 AmountToSplit)
{
	if (!(InventoryContents.Num()+1>InventorySlotsCapacity))
	{
		RemoveAmountOfItem(ItemIn,AmountToSplit);
		AddNewItem(ItemIn,AmountToSplit);
	}
}

FItemAddResult UInventoryComponent::HandleNonStackableItem(UItemBase* InputItem)
{
	// check if in the input item has valid weight
	if (FMath::IsNearlyZero(InputItem->GetItemSingleWeight())||InputItem->GetItemSingleWeight()<0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory,item has invalid weight value."),InputItem->TextData.Name));
	}
	// will the item weight overflow weight capacity
	if (InventoryTotalWeight+InputItem->GetItemSingleWeight()>GetWeightCapacity())
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory,item would overflow weight limit."),InputItem->TextData.Name));
	}
	// adding one more item would overflow slot capacity
	if (InventoryContents.Num()+1>InventorySlotsCapacity)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory,all inventory slots are full."),InputItem->TextData.Name));
	}

	AddNewItem(InputItem,1);
	return FItemAddResult::AddedAll(1,FText::Format(FText::FromString("Successfully added a single {0} to the inventory."),InputItem->TextData.Name));
}

int32 UInventoryComponent::HandleStackableItems(UItemBase* InputItem, int32 RequestedAddAmount)
{
	if (RequestedAddAmount<=0||FMath::IsNearlyZero(InputItem->GetItemStackWeight()))
	{
		// invalid item data
		return 0;
	}
	
	int32 AmountToDistribute=RequestedAddAmount;

	// check if the input item already exists in the inventory and is not a full stack 
	UItemBase* ExistingItemStack=FindNextPartialStack(InputItem);

	// distribute item stack over existing stacks
	while (ExistingItemStack)
	{
		// calculate how many of the existing item would be needed to make the next full stack
		const int32 AmountToMakeFullStack=CalculateNumberForFullStack(ExistingItemStack,AmountToDistribute);
		// calcalate how many of the AmountToMakeFullStack can actually be carried based on weight capacity
		const int32 WeightLimitAddAmount=CalculateWeightAddAmount(ExistingItemStack,AmountToMakeFullStack);

		// as long as the remaing amount of the item does not overflow weight capacity
		if (WeightLimitAddAmount>0)
		{
			// adjust the existing items stack quantity and inventory total weight
			ExistingItemStack->SetQuantity(ExistingItemStack->Quantity+WeightLimitAddAmount);
			InventoryTotalWeight+=ExistingItemStack->GetItemSingleWeight()*WeightLimitAddAmount;

			// adjust the count to be distributed
			AmountToDistribute -=WeightLimitAddAmount;
			
			InputItem->SetQuantity(AmountToDistribute);

			// TODO: Refine this logic since going over weight capacity should not ever be possible
			// if max weight capacity is reached,no need to run the loop again
			if (InventoryTotalWeight>=InventoryWeightCapacity)
			{
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount-AmountToDistribute;
			}
		}
		else if (WeightLimitAddAmount<=0)
		{
			if (AmountToDistribute!=RequestedAddAmount)
			{
				// this block will be reached if distributing an item across multiple stacks
				// and the weight limit is hit during that process
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount-AmountToDistribute;
			}
			return 0;
		}
		if (AmountToDistribute<=0)
		{
			// all of the input item was distributed across existing stacks
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount;
		}
		// check if there is still another valid partial stack of the input item
		ExistingItemStack=FindNextPartialStack(InputItem);
	}

	// no more partial stacks found,check if a new stack can be added
	if (InventoryContents.Num()+1 <=InventorySlotsCapacity)
	{
		// attempt to add as many from the remaining item quantity that can fit inventory weight capacity
		const int32 WeightLimitAddAmount=CalculateWeightAddAmount(InputItem,AmountToDistribute);

		if (WeightLimitAddAmount>0)
		{
			// if there is still more item to distribute,but weight limit has been reached
			if (WeightLimitAddAmount<AmountToDistribute)
			{
				// adjust the input item and add a new stack with as many as can be held
				AmountToDistribute-=WeightLimitAddAmount;
				InputItem->SetQuantity(AmountToDistribute);

				// create a copy since only a partial stack is being added
				AddNewItem(InputItem->CreateItemCopy(),WeightLimitAddAmount);
				return RequestedAddAmount-AmountToDistribute;
			}
			// otherwise, the full remainder of the stack can be added
			AddNewItem(InputItem,AmountToDistribute);
			return RequestedAddAmount;
		}
	}
	OnInventoryUpdated.Broadcast();
	return RequestedAddAmount-AmountToDistribute;
}

FItemAddResult UInventoryComponent::HandleAddItem(UItemBase* InputItem)
{
	if (GetOwner())
	{
		const int32 InitialRequestedAddAmount=InputItem->Quantity;

		// handle non-stackable items
		if (!InputItem->NumericData.bIsStackable)
		{
			return HandleNonStackableItem(InputItem);
		}

		// hanle stackable
		const int32 StackableAmountAdded=HandleStackableItems(InputItem,InitialRequestedAddAmount);
		if (StackableAmountAdded==InitialRequestedAddAmount)
		{
			return FItemAddResult::AddedAll(InitialRequestedAddAmount,FText::Format(FText::FromString("Successfully added {0} {1} to the inventory."),InitialRequestedAddAmount,InputItem->TextData.Name));
		}

		if (StackableAmountAdded<InitialRequestedAddAmount&&StackableAmountAdded>0)
		{
			return FItemAddResult::AddedPartial(StackableAmountAdded,FText::Format(FText::FromString("Partial amount of {0} added to the inventory.Number added = {1}"),InputItem->TextData.Name,StackableAmountAdded));
		}
		
		if (StackableAmountAdded<=0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to the inventory.No remaining inventory slots,or invalid item."),InputItem->TextData.Name));

		}
	}
	check(false);
	return FItemAddResult::AddedNone(FText::FromString("TryAddItem fallthrough error.GetOwner() check somehow failed."));
	
}

void UInventoryComponent::AddNewItem(UItemBase* Item, const int32 AmountToAdd)
{
	UItemBase* NewItem;
	if (Item->bIsCopy||Item->bIsPickup)
	{
		// if the item is already a copy,or is a world pickup;
		NewItem=Item;
		NewItem->ResetItemFlags();
	}
	else
	{
		// used when splitting or dragging to/from another inventory
		NewItem=Item->CreateItemCopy();
	}

	NewItem->OwningInventory=this;
	NewItem->SetQuantity(AmountToAdd);

	InventoryContents.Add(NewItem);
	InventoryTotalWeight+=NewItem->GetItemStackWeight();
	OnInventoryUpdated.Broadcast();
	
}


