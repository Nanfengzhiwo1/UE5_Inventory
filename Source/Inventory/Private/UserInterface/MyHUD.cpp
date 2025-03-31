// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/MyHUD.h"
#include "UserInterface/MainMenu.h"
#include "UserInterface/Interaction/InteractionWidget.h"
#include "Blueprint/UserWidget.h"

AMyHUD::AMyHUD()
{
}

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();
	if (MainMenuClass)
	{
		MainMenu=CreateWidget<UMainMenu>(GetWorld(),MainMenuClass);
		MainMenu->AddToViewport();
		MainMenu->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (InteractionWidgetClass)
	{
		InteractionWidget=CreateWidget<UInteractionWidget>(GetWorld(),InteractionWidgetClass);
		InteractionWidget->AddToViewport();
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

void AMyHUD::DisplayMenu()
{
	if (MainMenu)
	{
		bIsMenuVisiable=true;
		MainMenu->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMyHUD::HideMenu()
{
	if (MainMenu)
	{
		bIsMenuVisiable=false;
		MainMenu->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMyHUD::DisplayInteractionWidget() const
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMyHUD::HideInteractionWidget() const
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMyHUD::UpdateInteractionWidget(const FInteractableData* InteractableData) const
{
	if (InteractionWidget)
	{
		if (InteractionWidget->GetVisibility()==ESlateVisibility::Collapsed)
		{
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}

		InteractionWidget->UpdateWidget(InteractableData);
	}
}


