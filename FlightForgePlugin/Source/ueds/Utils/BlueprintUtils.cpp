// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintUtils.h"

// yaml-cpp libs
#include "Node.h"
#include "Parsing.h"

void UBlueprintUtils::WriteGateTransformToCsv(const FString& Filename, const TArray<FString>& GateNames, const TArray<FVector>& Vectors, const TArray<float>& Heading, bool& bOutResult, FString& OutString)
{
	if (GateNames.Num() != Vectors.Num() || Vectors.Num() != Heading.Num())
	{
		bOutResult = false;
		OutString = "Array sizes do not match.";
		return;
	}

	// Combine all data into a sortable array


	TArray<FGateData> GateDataArray;
	for (int i = 0; i < GateNames.Num(); i++)
	{
		GateDataArray.Add({ GateNames[i], Vectors[i], Heading[i] });
	}

	// Sort by GateName
	GateDataArray.Sort([](const FGateData& A, const FGateData& B)
	{
		return A.GateName < B.GateName;
	});
	
	FString Csv2Print;
	Csv2Print.Appendf(TEXT("GATE_NAME, X[m], Y[m], Z[m], HEADING[deg] \r\n"));

	for (auto GateData : GateDataArray)
	{
		Csv2Print.Appendf(TEXT("%s, %.3lf, %.3lf, %.3lf, %.3lf\r\n"),
			//*GateNames[i], Vectors[i].X, Vectors[i].Y, Vectors[i].Z, Heading[i]);
			*GateData.GateName, GateData.Location.X, GateData.Location.Y, GateData.Location.Z, GateData.Heading);
	}

	FString FilePath = FPaths::ProjectContentDir() + Filename + ".csv";
	if(!FFileHelper::SaveStringToFile(Csv2Print,*FilePath))
	{
		bOutResult = false;
		OutString = "Failed to write to file.";
	}

	bOutResult = true;
	OutString = "Finished writing to file.";
}

void UBlueprintUtils::ReadYamlFromFile(const FString& FilePath)
{
	FYamlNode Node;
	UYamlParsing::LoadYamlFromFile(FPaths::ProjectContentDir() + "example1.yaml", Node);  // Load and Parse from 
	
	FString filePath = FString(FPaths::ProjectContentDir() + "example1.yaml");
	UE_LOG(LogTemp, Log, TEXT("Try load YAML Node from: %s"), *filePath);

	// Accessing and converting an attribute
	FString Message = Node["message"].As<FString>();
	int32 Answer =    Node["answer"].As<int32>();
	FVector Origin =  Node["origin"].As<FVector>(FVector::ForwardVector);  // User ForwardVector if the conversion is not possible

	// // Iterate over Fibonacci's
	// int32 Sum = 0;
	// for (const auto& Fib : Node["fibonacci"]) {
	// 	Sum += Fib.As<int32>(0);  // "The End!" cannot be converted, so 0 will be used
	// }
	//
	UE_LOG(LogTemp, Log, TEXT("%d"), Answer);
}

void UBlueprintUtils::ReadObjectsTransformFromYaml(const FString& FilePath, TArray<FObjectDataYaml>& OutObjectsData)
{
	FString FullPath = FPaths::ProjectContentDir() + FilePath;

	if (!FPaths::FileExists(FullPath))
	{
		UE_LOG(LogTemp, Error, TEXT("YAML file not found: %s"), *FullPath);
		return;
	}

	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *FullPath))
	{
		YAML::Node RootNode = YAML::Load(TCHAR_TO_UTF8(*FileContent));

		if (RootNode["objects"])
		{
			for (const auto& ObjectNode : RootNode["objects"])
			{
				bool ReadSuccess = true;
				FObjectDataYaml Object;
				YAML::Node nameNode = ObjectNode["name"];
				if (nameNode && nameNode.IsDefined()) 
				{
					// yaml-cpp does not natively support reading as FString directly, therefore we need to convert std::string to FString
					Object.Name = UTF8_TO_TCHAR(nameNode.as<std::string>().c_str());
				} 
				else 
				{
					ReadSuccess = false;
					UE_LOG(LogTemp, Warning, TEXT("Name key not found in YAML."));
				}

				YAML::Node positionNode = ObjectNode["position"];
                if (positionNode && positionNode.IsDefined())
                {
                    YAML::Node posX = positionNode["x"];
                    YAML::Node posY = positionNode["y"];
                    YAML::Node posZ = positionNode["z"];
                    if (posX && posY && posZ && posX.IsDefined() && posY.IsDefined() && posZ.IsDefined())
                    {
                        Object.Position.X = posX.as<float>();
                        Object.Position.Y = posY.as<float>();
                        Object.Position.Z = posZ.as<float>();
                    }
                    else
                    {
                        ReadSuccess = false;
                        UE_LOG(LogTemp, Warning, TEXT("Incomplete position data in YAML for an object"));
                    }
                }
                else
                {
                    ReadSuccess = false;
                    UE_LOG(LogTemp, Warning, TEXT("Position key not found in YAML for one of the gates."));
                }

                YAML::Node orientationNode = ObjectNode["orientation"];
                if (orientationNode && orientationNode.IsDefined())
                {
                    YAML::Node rollNode = orientationNode["roll"];
                    YAML::Node pitchNode = orientationNode["pitch"];
                    YAML::Node yawNode = orientationNode["yaw"];
                    if (rollNode && pitchNode && yawNode && rollNode.IsDefined() && pitchNode.IsDefined() && yawNode.IsDefined())
                    {
                        Object.Orientation.Roll = rollNode.as<float>();
                        Object.Orientation.Pitch = pitchNode.as<float>();
                        Object.Orientation.Yaw = yawNode.as<float>();
                    }
                    else
                    {
                        ReadSuccess = false;
                        UE_LOG(LogTemp, Warning, TEXT("Incomplete orientation data in YAML for an object"));
                    }
                }
                else
                {
                    ReadSuccess = false;
                    UE_LOG(LogTemp, Warning, TEXT("Orientation key not found in YAML for one of the gates."));
                }

				// UE_LOG(LogTemp, Warning, TEXT("Loaded position:  [%lf,%lf,%lf] Orientation: [%lf,%lf,%lf]"),
				// 	Object.Position.X,Object.Position.Y,Object.Position.Z, Object.Orientation.Roll, Object.Orientation.Pitch, Object.Orientation.Yaw);

                if (ReadSuccess)
                {
                    OutObjectsData.Add(Object);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Skipping invalid gate data for an object"));
                }

			} // for (const auto& ObjectNode : RootNode["gates"])

			UE_LOG(LogTemp, Log, TEXT("Successfully loaded %d gates"), OutObjectsData.Num());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No 'gates' section found in YAML file"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read YAML file"));
	}
}

void UBlueprintUtils::TransformToUECoord(const FVector& RightHandLocation, const FVector& RightHandOrientation, const FVector& WorldOrigin, FTransform& Out_UETransform)
{
	float PlayerStartOffset = 92.12;
	
	FVector LocationUE;
	LocationUE.X = WorldOrigin.X + RightHandLocation.X * 100;
	LocationUE.Y = WorldOrigin.Y - RightHandLocation.Y * 100;
	LocationUE.Z = WorldOrigin.Z + RightHandLocation.Z * 100 - PlayerStartOffset;
	
	FRotator OrientationUE;
	OrientationUE.Roll = 180 * (-RightHandOrientation.X / PI);
	OrientationUE.Pitch = 180 * (RightHandOrientation.Y / PI);
	OrientationUE.Yaw = 180 * (-RightHandOrientation.Z / PI);

	// UE_LOG(LogTemp, Warning, TEXT("Heading: %lf [right hand], %lf [UE]"), RightHandOrientation.Z, OrientationUE.Yaw);

	Out_UETransform.SetLocation(LocationUE);
	Out_UETransform.SetRotation(OrientationUE.Quaternion());
}
