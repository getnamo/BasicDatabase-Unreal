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

	UFUNCTION(BlueprintCallable, Category = BasicDatabaseFunctions)
	void AddJsonStringToDatabase(FString& OutPrimaryKey, const FString& InDomain, const FString& JsonString, const FString& AtPrimaryKey = TEXT("New"));

	UFUNCTION(BlueprintCallable, Category = BasicDatabaseFunctions)
	void JsonStringForPrimaryKey(bool& bOutDidRead, const FString& Domain, const FString& PrimaryKey, FString& OutJsonString);

	//Add or update struct at primary key, if key is 'new' it will create a new key instead and return that
	UFUNCTION(BlueprintCallable, Category = BasicDatabaseFunctions, CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
	void AddStructToDatabase(FString& OutPrimaryKey, const FString& InDomain, TFieldPath<FProperty> AnyStruct, const FString& AtPrimaryKey = TEXT("New"));

	//Custom thunk to get bp struct data in c++ accessible form
	DECLARE_FUNCTION(execAddStructToDatabase)
	{
		Stack.MostRecentProperty = nullptr;
		FString Domain;
		FString PrimaryKey;

		UStruct* Struct;

		PARAM_PASSED_BY_REF(OutKey, FStrProperty, FString);	//magic for FString& OutKey = Stack.StepCompiledInRef<FStrProperty>(&OutKey);
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
			//Update the struct and pass back the primary key (new PK if new)
			OutKey = Database->UpdateStructAtPrimaryIndex(Struct, StructPtr, PrimaryKey, true);
		}
		P_NATIVE_END;
	}

	//NB: This should be latent or have a 'Is Ready' promise type return wrapper.
	UFUNCTION(BlueprintCallable, Category = BasicDatabaseFunctions, CustomThunk, meta = (CustomStructureParam = "OutStruct"))
	void StructForPrimaryKey(bool& bOutDidRead, const FString& Domain, const FString& PrimaryKey, TFieldPath<FProperty> OutStruct);

	//Custom thunk to get bp struct data in c++ accessible form
	DECLARE_FUNCTION(execStructForPrimaryKey)
	{
		Stack.MostRecentProperty = nullptr;
		FString InDomain;
		FString InPrimaryKey;

		UStruct* Struct;

		PARAM_PASSED_BY_REF(bDidReadSuccessfully, FBoolProperty, bool);
		Stack.StepCompiledIn<FStrProperty>(&InDomain);
		Stack.StepCompiledIn<FStrProperty>(&InPrimaryKey);

		//Determine wildcard property
		Stack.Step(Stack.Object, NULL);
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;
		Struct = StructProperty->Struct;

		P_FINISH;
		P_NATIVE_BEGIN;
		UBasicDataBaseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UBasicDataBaseSubsystem>();
		TSharedPtr<FBasicDatabaseNative> Database = Subsystem->DatabaseForDomain(InDomain);
		
		if (Database)
		{
			//Run the lookup function
			bDidReadSuccessfully = Database->ReadStructAtIndex(Struct, StructPtr, InPrimaryKey, true);
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
