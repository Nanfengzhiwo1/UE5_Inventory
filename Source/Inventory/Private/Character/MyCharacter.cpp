// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyCharacter.h"

#include "Components/InventoryComponent.h"
#include "UserInterface/MyHUD.h"

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

	PlayerInputComponent->BindAction("Interact",IE_Pressed,this,&AMyCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact",IE_Released,this,&AMyCharacter::EndInteract);

	PlayerInputComponent->BindAction("ToggleMenu",IE_Pressed,this,&AMyCharacter::ToggleMenu);
	
}

void AMyCharacter::ToggleMenu()
{
	MyHUD->ToggleMenu();
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