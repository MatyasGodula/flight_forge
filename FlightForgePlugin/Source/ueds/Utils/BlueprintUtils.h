// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtils.generated.h"

/**
 * 
 */
USTRUCT()
struct FGateData
{
	GENERATED_BODY()
	
	FString GateName;
	FVector Location;
	float Heading;
};

USTRUCT(BlueprintType)
struct FObjectPosition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float X;

	UPROPERTY(BlueprintReadWrite)
	float Y;

	UPROPERTY(BlueprintReadWrite)
	float Z;
};

USTRUCT(BlueprintType)
struct FObjectOrientation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Roll;

	UPROPERTY(BlueprintReadWrite)
	float Pitch;

	UPROPERTY(BlueprintReadWrite)
	float Yaw;
};

USTRUCT(BlueprintType)
struct FObjectDataYaml
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FObjectPosition Position;

	UPROPERTY(BlueprintReadWrite)
	FObjectOrientation Orientation;
};


UCLASS()
class UEDS_API UBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Csv")
	static void WriteGateTransformToCsv(const FString& Filename, const TArray<FString>& GateNames, const TArray<FVector>& Vectors, const TArray<float>& Heading, bool& bOutResult, FString& OutString); 

	UFUNCTION(BlueprintCallable, Category="Yaml")
	static void ReadYamlFromFile(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category="Yaml")
	static void ReadObjectsTransformFromYaml(const FString& FilePath, TArray<FObjectDataYaml>& OutGatesData);

	UFUNCTION(BlueprintCallable, Category="Coordinates")
	static void TransformToUECoord(const FVector& RightHandLocation, const FVector& RightHandOrientation, const FVector& WorldOrigin, FTransform& Out_UETransform);
};
