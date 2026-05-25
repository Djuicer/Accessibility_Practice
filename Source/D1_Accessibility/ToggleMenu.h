// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "ToggleMenu.generated.h"

class UButton;
class UTextBlock;
class UInputAction;
class UInputMappingContext;
class UComboBoxString;
class UMaterialParameterCollection;


UENUM(BlueprintType)
enum class EColorBlindnessOption : uint8
{
	NoDeficiency,
	Deutanopia,
	Protanopia,
	Tritanopia
};

UENUM(BlueprintType)
enum class ERebindTarget : uint8
{
	None,
	MoveForward,
	MoveBackward,
	MoveLeft,
	MoveRight,
	Jump
};

/**
 * 
 */
UCLASS()
class D1_ACCESSIBILITY_API UToggleMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	explicit UToggleMenu(const FObjectInitializer& ObjectInitializer);
	void SetInputMappingContext(UInputMappingContext* NewInputMappingContext);
	
protected:
	virtual void NativeConstruct() override;

	// Preview is useful because it can catch keys before focused child widgets consume them.
	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;
	
	// Assign your IMC_Default / player input mapping context here.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// Usually IA_Move, value type Axis2D.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input")
	TObjectPtr<UInputAction> MoveAction;

	// Usually IA_Jump, value type Boolean.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input")
	TObjectPtr<UInputAction> JumpAction;

	// Defaults should match the keys currently set in your Input Mapping Context.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input|Default Keys")
	FKey DefaultMoveForwardKey = EKeys::W;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input|Default Keys")
	FKey DefaultMoveBackwardKey = EKeys::S;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input|Default Keys")
	FKey DefaultMoveLeftKey = EKeys::A;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input|Default Keys")
	FKey DefaultMoveRightKey = EKeys::D;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Input|Default Keys")
	FKey DefaultJumpKey = EKeys::SpaceBar;
	
	// Material Parameter Collection used by the world master material.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toggle Menu|Accessibility")
	TObjectPtr<UMaterialParameterCollection> AccessibilityMaterialParameterCollection;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ColorBlindnessComboBox;

	// Optional: name your widgets exactly like this in the Widget Blueprint.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MoveForwardButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MoveBackwardButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MoveLeftButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MoveRightButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> JumpButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoveForwardKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoveBackwardKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoveLeftKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoveRightKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> JumpKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

private:
	UPROPERTY()
	ERebindTarget PendingRebindTarget = ERebindTarget::None;

	FKey CurrentMoveForwardKey;
	FKey CurrentMoveBackwardKey;
	FKey CurrentMoveLeftKey;
	FKey CurrentMoveRightKey;
	FKey CurrentJumpKey;

	bool bCurrentKeysInitialized = false;

private:
	UFUNCTION()
	void StartRebindMoveForward();

	UFUNCTION()
	void StartRebindMoveBackward();

	UFUNCTION()
	void StartRebindMoveLeft();

	UFUNCTION()
	void StartRebindMoveRight();

	UFUNCTION()
	void StartRebindJump();

	void BeginRebind(ERebindTarget Target);
	bool CommitRebind(const FKey& NewKey);

	bool ReplaceMappingKey(
		const UInputAction* Action,
		const FKey& OldKey,
		const FKey& NewKey
	);

	bool IsKeyAlreadyUsed(
		const FKey& NewKey,
		const UInputAction* ActionBeingChanged,
		const FKey& CurrentKeyBeingChanged
	) const;

	UInputAction* GetActionForTarget(ERebindTarget Target) const;
	FKey* GetCurrentKeyForTarget(ERebindTarget Target);

	FText GetTargetDisplayName(ERebindTarget Target) const;
	static FText KeyToText(const FKey& Key);

	void RefreshKeyTexts();
	void SetStatusText(const FText& Message);
	
	UFUNCTION()
	void OnColorBlindnessSelectionChanged(
		FString SelectedItem,
		ESelectInfo::Type SelectionType
	);

	void SetupColorBlindnessDropdown();
	void ApplyColorBlindnessOption(EColorBlindnessOption Option);

	void SetColorBlindnessMaterialParameters(
		const FLinearColor& TintColor,
		const FLinearColor& ChannelMultiplier,
		float TintStrength,
		float DesaturationStrength,
		float Brightness,
		float Contrast
	);
	
	void ApplyTintToPlayerCharacter(
		const FLinearColor& CharacterTintColor,
		float CharacterTintStrength
	);

	EColorBlindnessOption GetColorBlindnessOptionFromString(const FString& SelectedItem) const;
};	
