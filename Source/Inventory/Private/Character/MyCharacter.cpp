// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyCharacter.h"

#include "EnhancedInputComponent.h"
#include "Components/InventoryComponent.h"
#include "UserInterface/MyHUD.h"
#include "World/Pickup.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InteractionCheckFrequency = 0.1;
	InteractionCheckDistance = 225.0f;
	BaseEyeHeight = 74.0f;

	PlayerInventory=CreateDefaultSubobject<UInventoryComponent>(TEXT("PlayerInventory"));
	PlayerInventory->SetSlotsCapacity(20);
	PlayerInventory->SetWeightCapacity(50.0f);
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	MyHUD=Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
}


// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		//Interacting
		EnhancedInputComponent->BindAction(InterAction,ETriggerEvent::Triggered,this,&AMyCharacter::BeginInteract);
		EnhancedInputComponent->BindAction(InterAction,ETriggerEvent::Completed,this,&AMyCharacter::EndInteract);

		//ToggleMenu
		EnhancedInputComponent->BindAction(ToggleMenuAction,ETriggerEvent::Completed,this,&AMyCharacter::ToggleMenu);
	}
}

void AMyCharacter::ToggleMenu()
{
	MyHUD->ToggleMenu();
}

void AMyCharacter::DropItem(UItemBase* ItemToDrop, const int32 QuantityToDrop)
{
	if (PlayerInventory->FindMatchingItem(ItemToDrop))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner=this;
		SpawnParameters.bNoFail=true;
		SpawnParameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		
		const FVector SpawnLocation{GetActorLocation()+(GetActorForwardVector()*50.0f)};
		const FTransform SpawnTransform(GetActorRotation(),SpawnLocation);
		
		const int32 RemovedQuantiy=PlayerInventory->RemoveAmountOfItem(ItemToDrop,QuantityToDrop);
		APickup* Pickup=GetWorld()->SpawnActor<APickup>(APickup::StaticClass(),SpawnTransform,SpawnParameters);
		Pickup->InitializeDrop(ItemToDrop,RemovedQuantiy);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("Item to drop was somehow null!"))
	}
}

void AMyCharacter::PerformInteractionCheck()
{
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	// Sets default values
	FVector TraceStart{GetPawnViewLocation()};
	FVector TraceEnd{TraceStart + GetViewRotation().Vector() * InteractionCheckDistance};

	float LookDirection = FVector::DotProduct(GetActorForwardVector(), GetViewRotation().Vector());
	if (LookDirection > 0)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 2.0f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		FHitResult TraceHit;
		if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			if (TraceHit.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				if (TraceHit.GetActor() != InteractionData.CurrentInteractable )
				{
					FoundInteractable(TraceHit.GetActor());
					return;
				}
				if (TraceHit.GetActor() == InteractionData.CurrentInteractable)
				{
					return;
				}
			}
		}
		NoInteractableFound();
	}
}

void AMyCharacter::FoundInteractable(AActor* NewInteractable)
{
	// pre_Check
	if (IsInteracting())
	{
		EndInteract();
	}
	if (InteractionData.CurrentInteractable)
	{
		TargetInteractable=InteractionData.CurrentInteractable;
		TargetInteractable->EndFocus();
	}

	InteractionData.CurrentInteractable=NewInteractable;
	TargetInteractable=NewInteractable;

	// update interaction widget
	MyHUD->UpdateInteractionWidget(&TargetInteractable->InteractableData);
	
	TargetInteractable->BeginFocus();
	
}

void AMyCharacter::NoInteractableFound()
{
	if (IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	}
	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->EndFocus();
	}

	// hide interaction widget on the HUD
	MyHUD->HideInteractionWidget();
	
	InteractionData.CurrentInteractable=nullptr;
	TargetInteractable=nullptr;
}

void AMyCharacter::BeginInteract()
{
	// verify nothing has changed with the interactable state since begining interaction
	PerformInteractionCheck();

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->BeginInteract();

			if (FMath::IsNearlyZero(TargetInteractable->InteractableData.InteractionDuration,0.1f))
			{
				Interact();
			}
			else
			{
				GetWorldTimerManager().SetTimer(TimerHandle_Interaction,this,&AMyCharacter::Interact,TargetInteractable->InteractableData.InteractionDuration,false);
			}
		}
	}
}

void AMyCharacter::EndInteract()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->EndInteract();
	}
}

void AMyCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->Interact(this);
	}
}

void AMyCharacter::UpdateInteractionWidget() const
{
	if (IsValid(TargetInteractable.GetObject()))
	{
		MyHUD->UpdateInteractionWidget(&TargetInteractable->InteractableData);
	}
}


