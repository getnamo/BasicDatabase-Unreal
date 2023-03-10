#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "BasicDatabaseNative.h"
#include "BasicDatabaseSubsystem.generated.h"

UCLASS(ClassGroup = "BasicDatabase")
class BASICDATABASE_API UBasicDataBaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	
	//Startup function
	UFUNCTION(BlueprintCallable, Category=BasicDatabaseFunctions)
	void ConnectDatabase(const FString& Domain);

	//Add or update struct at primary key, if key is 'new' it will create a new key instead and return that
	UFUNCTION(BlueprintCallable, Category = BasicDatabaseFunctions, CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	void AddStructToDatabase(FString& OutPrimaryKey, const FString& InDomain, TFieldPath<FProperty> AnyStruct, const FString& AtPrimaryKey = TEXT("New"));

	//Custom thunk to get bp struct data in c++ accessible form
	DECLARE_FUNCTION(execAddStructToDatabase)
	{
		Stack.MostRecentProperty = nullptr;
		FString Domain;
		FString PrimaryKey;
		FString* OutKey = nullptr;
		UStruct* Struct;

		Stack.StepCompiledIn<FStrProperty>(OutKey);
		Stack.StepCompiledIn<FStrProperty>(&Domain);
		
		//Determine wildcard property
		Stack.Step(Stack.Object, NULL);
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;
		Struct = StructProperty->Struct;

		Stack.StepCompiledIn<FStrProperty>(&PrimaryKey);

		P_FINISH;
		P_NATIVE_BEGIN;
		UBasicDataBaseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UBasicDataBaseSubsystem>();
		TSharedPtr<FBasicDatabaseNative> Database = Subsystem->DatabaseForDomain(Domain);
		if (Database)
		{
			*OutKey = Database->UpdateStructAtPrimaryIndex(Struct, StructPtr, PrimaryKey, true);
		}
		P_NATIVE_END;
	}

	virtual	void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//Helper function for custom thunk
	TSharedPtr<FBasicDatabaseNative> DatabaseForDomain(const FString& Domain);

protected:
	TMap<FString, TSharedPtr<FBasicDatabaseNative>> Databases;
};
