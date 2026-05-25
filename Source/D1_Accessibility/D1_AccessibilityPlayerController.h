// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1_AccessibilityPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UToggleMenu;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AD1_AccessibilityPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;
	

	/** Input Action used to open/close the rebind menu. Map this to Tab. */
	UPROPERTY(EditAnywhere, Category = "Input|Input Actions")
	TObjectPtr<UInputAction> ToggleMenuAction;

	/** Rebind menu widget to spawn */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ToggleMenuWidgetClass;

	/** Pointer to the rebind menu widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> ToggleMenuWidget;
	

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TObjectPtr<UInputMappingContext> RebindSourceMappingContext;
	
	UPROPERTY()
	TObjectPtr<UInputMappingContext> RuntimeRebindMappingContext;
	
	UPROPERTY()
	TArray<TObjectPtr<UInputMappingContext>> RuntimeMappingContexts;
	

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;


	
	void ToggleMenu();
	void OpenToggleMenu();
	void CloseToggleMenu();
	

};