#include "ToggleMenu.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/ComboBoxString.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"

UToggleMenu::UToggleMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsFocusable = true;
}

void UToggleMenu::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (!bCurrentKeysInitialized)
	{
		CurrentMoveForwardKey = DefaultMoveForwardKey;
		CurrentMoveBackwardKey = DefaultMoveBackwardKey;
		CurrentMoveLeftKey = DefaultMoveLeftKey;
		CurrentMoveRightKey = DefaultMoveRightKey;
		CurrentJumpKey = DefaultJumpKey;

		bCurrentKeysInitialized = true;
	}

	if (MoveForwardButton)
	{
		MoveForwardButton->OnClicked.AddUniqueDynamic(
			this,
			&UToggleMenu::StartRebindMoveForward
		);
	}

	if (MoveBackwardButton)
	{
		MoveBackwardButton->OnClicked.AddUniqueDynamic(
			this,
			&UToggleMenu::StartRebindMoveBackward
		);
	}

	if (MoveLeftButton)
	{
		MoveLeftButton->OnClicked.AddUniqueDynamic(
			this,
			&UToggleMenu::StartRebindMoveLeft
		);
	}

	if (MoveRightButton)
	{
		MoveRightButton->OnClicked.AddUniqueDynamic(
			this,
			&UToggleMenu::StartRebindMoveRight
		);
	}

	if (JumpButton)
	{
		JumpButton->OnClicked.AddUniqueDynamic(
			this,
			&UToggleMenu::StartRebindJump
		);
	}
	
	SetupColorBlindnessDropdown();

	RefreshKeyTexts();
	SetStatusText(FText::GetEmpty());
}

FReply UToggleMenu::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
)
{
	if (PendingRebindTarget == ERebindTarget::None)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	const FKey PressedKey = InKeyEvent.GetKey();

	if (PressedKey == EKeys::Escape)
	{
		PendingRebindTarget = ERebindTarget::None;
		SetStatusText(FText::FromString(TEXT("Rebind cancelled.")));
		return FReply::Handled();
	}

	if (!PressedKey.IsValid() || PressedKey == EKeys::AnyKey)
	{
		SetStatusText(FText::FromString(TEXT("Invalid key. Press another key.")));
		return FReply::Handled();
	}

	if (
		PressedKey.IsGamepadKey() ||
		PressedKey.IsMouseButton() ||
		PressedKey.IsTouch() ||
		PressedKey.IsAnalog()
	)
	{
		SetStatusText(FText::FromString(TEXT("Only keyboard keys are supported for now.")));
		return FReply::Handled();
	}

	CommitRebind(PressedKey);
	return FReply::Handled();
}

void UToggleMenu::StartRebindMoveForward()
{
	BeginRebind(ERebindTarget::MoveForward);
}

void UToggleMenu::StartRebindMoveBackward()
{
	BeginRebind(ERebindTarget::MoveBackward);
}

void UToggleMenu::StartRebindMoveLeft()
{
	BeginRebind(ERebindTarget::MoveLeft);
}

void UToggleMenu::StartRebindMoveRight()
{
	BeginRebind(ERebindTarget::MoveRight);
}

void UToggleMenu::StartRebindJump()
{
	BeginRebind(ERebindTarget::Jump);
}

void UToggleMenu::BeginRebind(ERebindTarget Target)
{
	PendingRebindTarget = Target;

	SetKeyboardFocus();

	SetStatusText(
		FText::Format(
			FText::FromString(TEXT("Press a new key for {0}. Press Esc to cancel.")),
			GetTargetDisplayName(Target)
		)
	);
}

bool UToggleMenu::CommitRebind(const FKey& NewKey)
{
	if (PendingRebindTarget == ERebindTarget::None)
	{
		return false;
	}

	UInputAction* Action = GetActionForTarget(PendingRebindTarget);
	FKey* CurrentKey = GetCurrentKeyForTarget(PendingRebindTarget);

	if (!InputMappingContext)
	{
		SetStatusText(FText::FromString(TEXT("Rebind failed: Input Mapping Context is not assigned.")));
		PendingRebindTarget = ERebindTarget::None;
		return false;
	}

	if (!Action)
	{
		SetStatusText(FText::FromString(TEXT("Rebind failed: Input Action is not assigned.")));
		PendingRebindTarget = ERebindTarget::None;
		return false;
	}

	if (!CurrentKey)
	{
		SetStatusText(FText::FromString(TEXT("Rebind failed: current key is invalid.")));
		PendingRebindTarget = ERebindTarget::None;
		return false;
	}

	const ERebindTarget TargetBeingChanged = PendingRebindTarget;
	const FKey OldKey = *CurrentKey;

	if (OldKey == NewKey)
	{
		SetStatusText(
			FText::Format(
				FText::FromString(TEXT("{0} is already bound to {1}.")),
				GetTargetDisplayName(TargetBeingChanged),
				KeyToText(NewKey)
			)
		);

		PendingRebindTarget = ERebindTarget::None;
		return true;
	}

	if (IsKeyAlreadyUsed(NewKey, Action, OldKey))
	{
		SetStatusText(
			FText::Format(
				FText::FromString(TEXT("{0} is already in use. Press another key.")),
				KeyToText(NewKey)
			)
		);

		// Keep listening so the player can press another key.
		return false;
	}

	if (!ReplaceMappingKey(Action, OldKey, NewKey))
	{
		SetStatusText(FText::FromString(TEXT("Rebind failed: mapping was not found.")));
		PendingRebindTarget = ERebindTarget::None;
		return false;
	}

	*CurrentKey = NewKey;
	PendingRebindTarget = ERebindTarget::None;

	RefreshKeyTexts();

	SetStatusText(
		FText::Format(
			FText::FromString(TEXT("{0} rebound to {1}.")),
			GetTargetDisplayName(TargetBeingChanged),
			KeyToText(NewKey)
		)
	);

	return true;
}

bool UToggleMenu::ReplaceMappingKey(
	const UInputAction* Action,
	const FKey& OldKey,
	const FKey& NewKey
)
{
	if (!InputMappingContext || !Action || !OldKey.IsValid() || !NewKey.IsValid())
	{
		return false;
	}

	FEnhancedActionKeyMapping OldMappingCopy;
	bool bFoundMapping = false;

	for (const FEnhancedActionKeyMapping& Mapping : InputMappingContext->GetMappings())
	{
		if (Mapping.Action == Action && Mapping.Key == OldKey)
		{
			OldMappingCopy = Mapping;
			bFoundMapping = true;
			break;
		}
	}

	if (!bFoundMapping)
	{
		return false;
	}

	InputMappingContext->UnmapKey(Action, OldKey);

	FEnhancedActionKeyMapping& NewMapping = InputMappingContext->MapKey(Action, NewKey);

	NewMapping.Modifiers = OldMappingCopy.Modifiers;
	NewMapping.Triggers = OldMappingCopy.Triggers;

	return true;
}

bool UToggleMenu::IsKeyAlreadyUsed(
	const FKey& NewKey,
	const UInputAction* ActionBeingChanged,
	const FKey& CurrentKeyBeingChanged
) const
{
	if (!InputMappingContext)
	{
		return false;
	}

	for (const FEnhancedActionKeyMapping& Mapping : InputMappingContext->GetMappings())
	{
		const bool bIsCurrentMapping =
			Mapping.Action == ActionBeingChanged &&
			Mapping.Key == CurrentKeyBeingChanged;

		if (!bIsCurrentMapping && Mapping.Key == NewKey)
		{
			return true;
		}
	}

	return false;
}

UInputAction* UToggleMenu::GetActionForTarget(ERebindTarget Target) const
{
	switch (Target)
	{
	case ERebindTarget::MoveForward:
	case ERebindTarget::MoveBackward:
	case ERebindTarget::MoveLeft:
	case ERebindTarget::MoveRight:
		return MoveAction.Get();

	case ERebindTarget::Jump:
		return JumpAction.Get();

	default:
		return nullptr;
	}
}

FKey* UToggleMenu::GetCurrentKeyForTarget(ERebindTarget Target)
{
	switch (Target)
	{
	case ERebindTarget::MoveForward:
		return &CurrentMoveForwardKey;

	case ERebindTarget::MoveBackward:
		return &CurrentMoveBackwardKey;

	case ERebindTarget::MoveLeft:
		return &CurrentMoveLeftKey;

	case ERebindTarget::MoveRight:
		return &CurrentMoveRightKey;

	case ERebindTarget::Jump:
		return &CurrentJumpKey;

	default:
		return nullptr;
	}
}

FText UToggleMenu::GetTargetDisplayName(ERebindTarget Target) const
{
	switch (Target)
	{
	case ERebindTarget::MoveForward:
		return FText::FromString(TEXT("Move Forward"));

	case ERebindTarget::MoveBackward:
		return FText::FromString(TEXT("Move Backward"));

	case ERebindTarget::MoveLeft:
		return FText::FromString(TEXT("Move Left"));

	case ERebindTarget::MoveRight:
		return FText::FromString(TEXT("Move Right"));

	case ERebindTarget::Jump:
		return FText::FromString(TEXT("Jump"));

	default:
		return FText::FromString(TEXT("None"));
	}
}

FText UToggleMenu::KeyToText(const FKey& Key)
{
	return Key.GetDisplayName(false);
}

void UToggleMenu::RefreshKeyTexts()
{
	if (MoveForwardKeyText)
	{
		MoveForwardKeyText->SetText(KeyToText(CurrentMoveForwardKey));
	}

	if (MoveBackwardKeyText)
	{
		MoveBackwardKeyText->SetText(KeyToText(CurrentMoveBackwardKey));
	}

	if (MoveLeftKeyText)
	{
		MoveLeftKeyText->SetText(KeyToText(CurrentMoveLeftKey));
	}

	if (MoveRightKeyText)
	{
		MoveRightKeyText->SetText(KeyToText(CurrentMoveRightKey));
	}

	if (JumpKeyText)
	{
		JumpKeyText->SetText(KeyToText(CurrentJumpKey));
	}
}

void UToggleMenu::SetStatusText(const FText& Message)
{
	if (StatusText)
	{
		StatusText->SetText(Message);
	}
}

void UToggleMenu::SetInputMappingContext(UInputMappingContext* NewInputMappingContext)
{
	InputMappingContext = NewInputMappingContext;
}

void UToggleMenu::SetupColorBlindnessDropdown()
{
	if (!ColorBlindnessComboBox)
	{
		return;
	}

	ColorBlindnessComboBox->ClearOptions();

	ColorBlindnessComboBox->AddOption(TEXT("No deficiency"));
	ColorBlindnessComboBox->AddOption(TEXT("Deutanopia"));
	ColorBlindnessComboBox->AddOption(TEXT("Protanopia"));
	ColorBlindnessComboBox->AddOption(TEXT("Tritanopia"));

	ColorBlindnessComboBox->SetSelectedOption(TEXT("No deficiency"));

	ColorBlindnessComboBox->OnSelectionChanged.AddUniqueDynamic(
		this,
		&UToggleMenu::OnColorBlindnessSelectionChanged
	);

	ApplyColorBlindnessOption(EColorBlindnessOption::NoDeficiency);
}

void UToggleMenu::OnColorBlindnessSelectionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType
)
{
	const EColorBlindnessOption SelectedOption =
		GetColorBlindnessOptionFromString(SelectedItem);

	ApplyColorBlindnessOption(SelectedOption);
}


EColorBlindnessOption UToggleMenu::GetColorBlindnessOptionFromString(
	const FString& SelectedItem
) const
{
	if (SelectedItem == TEXT("Deutanopia"))
	{
		return EColorBlindnessOption::Deutanopia;
	}

	if (SelectedItem == TEXT("Protanopia"))
	{
		return EColorBlindnessOption::Protanopia;
	}

	if (SelectedItem == TEXT("Tritanopia"))
	{
		return EColorBlindnessOption::Tritanopia;
	}

	return EColorBlindnessOption::NoDeficiency;
}

void UToggleMenu::ApplyColorBlindnessOption(EColorBlindnessOption Option)
{
	switch (Option)
	{
	case EColorBlindnessOption::NoDeficiency:
		SetColorBlindnessMaterialParameters(
			FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
			FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		ApplyTintToPlayerCharacter(
			FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
			0.0f
		);
		break;

	case EColorBlindnessOption::Deutanopia:
		SetColorBlindnessMaterialParameters(
			FLinearColor(0.90f, 1.00f, 1.10f, 1.0f),
			FLinearColor(1.00f, 0.95f, 1.10f, 1.0f),
			0.20f,
			0.05f,
			1.00f,
			1.10f
		);

		ApplyTintToPlayerCharacter(
			FLinearColor(0.30f, 0.85f, 1.00f, 1.0f),
			0.35f
		);
		break;

	case EColorBlindnessOption::Protanopia:
		SetColorBlindnessMaterialParameters(
			FLinearColor(1.10f, 0.95f, 0.85f, 1.0f),
			FLinearColor(1.10f, 1.00f, 0.95f, 1.0f),
			0.20f,
			0.05f,
			1.00f,
			1.10f
		);

		ApplyTintToPlayerCharacter(
			FLinearColor(1.00f, 0.75f, 0.25f, 1.0f),
			0.35f
		);
		break;

	case EColorBlindnessOption::Tritanopia:
		SetColorBlindnessMaterialParameters(
			FLinearColor(1.10f, 0.90f, 1.00f, 1.0f),
			FLinearColor(1.05f, 0.95f, 1.10f, 1.0f),
			0.20f,
			0.05f,
			1.00f,
			1.10f
		);

		ApplyTintToPlayerCharacter(
			FLinearColor(1.00f, 0.35f, 0.85f, 1.0f),
			0.35f
		);
		break;

	default:
		break;
	}
}


void UToggleMenu::SetColorBlindnessMaterialParameters(
	const FLinearColor& TintColor,
	const FLinearColor& ChannelMultiplier,
	float TintStrength,
	float DesaturationStrength,
	float Brightness,
	float Contrast
)
{
	if (!AccessibilityMaterialParameterCollection)
	{
		SetStatusText(
			FText::FromString(
				TEXT("Accessibility Material Parameter Collection is not assigned.")
			)
		);

		return;
	}

	UObject* WorldContextObject = GetWorld();

	if (!WorldContextObject)
	{
		return;
	}

	UKismetMaterialLibrary::SetVectorParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_TintColor")),
		TintColor
	);

	UKismetMaterialLibrary::SetVectorParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_ChannelMultiplier")),
		ChannelMultiplier
	);

	UKismetMaterialLibrary::SetScalarParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_TintStrength")),
		TintStrength
	);

	UKismetMaterialLibrary::SetScalarParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_DesaturationStrength")),
		DesaturationStrength
	);

	UKismetMaterialLibrary::SetScalarParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_Brightness")),
		Brightness
	);

	UKismetMaterialLibrary::SetScalarParameterValue(
		WorldContextObject,
		AccessibilityMaterialParameterCollection,
		FName(TEXT("CB_Contrast")),
		Contrast
	);
	
	
}


void UToggleMenu::ApplyTintToPlayerCharacter(
	const FLinearColor& CharacterTintColor,
	float CharacterTintStrength
)
{
	APlayerController* PlayerController = GetOwningPlayer();

	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (!PlayerPawn)
	{
		return;
	}

	USkeletalMeshComponent* PlayerMesh =
		PlayerPawn->FindComponentByClass<USkeletalMeshComponent>();

	if (!PlayerMesh)
	{
		return;
	}

	const int32 MaterialCount = PlayerMesh->GetNumMaterials();

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; MaterialIndex++)
	{
		UMaterialInstanceDynamic* DynamicMaterial =
			Cast<UMaterialInstanceDynamic>(
				PlayerMesh->GetMaterial(MaterialIndex)
			);

		if (!DynamicMaterial)
		{
			DynamicMaterial = PlayerMesh->CreateDynamicMaterialInstance(MaterialIndex);
		}

		if (!DynamicMaterial)
		{
			continue;
		}

		DynamicMaterial->SetVectorParameterValue(
			FName(TEXT("LocalTintColor")),
			CharacterTintColor
		);

		DynamicMaterial->SetScalarParameterValue(
			FName(TEXT("LocalTintStrength")),
			CharacterTintStrength
		);
	}
}