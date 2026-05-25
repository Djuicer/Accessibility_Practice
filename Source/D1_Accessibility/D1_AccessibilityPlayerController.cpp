// Copyright Epic Games, Inc. All Rights Reserved.

#include "D1_AccessibilityPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "D1_Accessibility.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "ToggleMenu.h"

void AD1_AccessibilityPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogD1_Accessibility, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AD1_AccessibilityPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			RuntimeMappingContexts.Empty();
			RuntimeRebindMappingContext = nullptr;

			auto AddRuntimeMappingContext = [this, Subsystem](UInputMappingContext* SourceContext)
			{
				if (!SourceContext)
				{
					return;
				}

				// This is the important part:
				// duplicate the IMC so runtime rebinding does not modify the original asset.
				UInputMappingContext* RuntimeContext = DuplicateObject<UInputMappingContext>(
					SourceContext,
					this
				);

				if (!RuntimeContext)
				{
					return;
				}

				RuntimeMappingContexts.Add(RuntimeContext);
				Subsystem->AddMappingContext(RuntimeContext, 0);

				if (SourceContext == RebindSourceMappingContext)
				{
					RuntimeRebindMappingContext = RuntimeContext;
				}
			};

			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				AddRuntimeMappingContext(CurrentContext);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					AddRuntimeMappingContext(CurrentContext);
				}
			}
		}

		// ---------- Toggle Menu addition ----------
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (ToggleMenuAction)
			{
				EnhancedInputComponent->BindAction(
					ToggleMenuAction,
					ETriggerEvent::Started,
					this,
					&AD1_AccessibilityPlayerController::ToggleMenu
				);
			}
		}
		// ---------- End Toggle Menu addition ----------
	}
}

bool AD1_AccessibilityPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

// ---------- Toggle Menu additions ----------

void AD1_AccessibilityPlayerController::ToggleMenu()
{
	if (ToggleMenuWidget && ToggleMenuWidget->IsInViewport())
	{
		CloseToggleMenu();
	}
	else
	{
		OpenToggleMenu();
	}
}

void AD1_AccessibilityPlayerController::OpenToggleMenu()
{
	if (!ToggleMenuWidgetClass)
	{
		UE_LOG(LogD1_Accessibility, Error, TEXT("ToggleMenuWidgetClass is not assigned."));
		return;
	}

	if (!ToggleMenuWidget)
	{
		ToggleMenuWidget = CreateWidget<UUserWidget>(this, ToggleMenuWidgetClass);

		if (UToggleMenu* ToggleMenu = Cast<UToggleMenu>(ToggleMenuWidget))
		{
			ToggleMenu->SetInputMappingContext(RuntimeRebindMappingContext);
		}
	}

	if (!ToggleMenuWidget)
	{
		UE_LOG(LogD1_Accessibility, Error, TEXT("Could not create ToggleMenuWidget."));
		return;
	}

	if (!ToggleMenuWidget->IsInViewport())
	{
		ToggleMenuWidget->AddToViewport(100);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ToggleMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Optional: stop player movement/look while menu is open.
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
}

void AD1_AccessibilityPlayerController::CloseToggleMenu()
{
	if (ToggleMenuWidget && ToggleMenuWidget->IsInViewport())
	{
		ToggleMenuWidget->RemoveFromParent();
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
}

// ---------- End Toggle Menu additions ----------