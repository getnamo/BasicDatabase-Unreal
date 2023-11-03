#include "BasicDatabaseSubsystem.h"
#include "CUFileSubsystem.h"

void UBasicDataBaseSubsystem::ConnectDatabase(const FString& Domain)
{
	if (!Databases.Contains(Domain)) 
	{
		TSharedPtr<FBasicDatabaseNative> Database = MakeShareable(new FBasicDatabaseNative(Domain));
		Databases.Add(Domain, Database);
	}
}

void UBasicDataBaseSubsystem::AddJsonStringToDatabase(FString& OutPrimaryKey, const FString& InDomain, const FString& JsonString, const FString& AtPrimaryKey)
{
	UBasicDataBaseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UBasicDataBaseSubsystem>();
	TSharedPtr<FBasicDatabaseNative> Database = Subsystem->DatabaseForDomain(InDomain);
	if (Database)
	{
		//Update the struct and pass back the primary key (new PK if new)
		OutPrimaryKey = Database->UpdateJsonStringAtPrimaryIndex(JsonString, AtPrimaryKey);
	}
}

void UBasicDataBaseSubsystem::JsonStringForPrimaryKey(bool& bOutDidRead, const FString& Domain, const FString& PrimaryKey, FString& OutJsonString)
{
	UBasicDataBaseSubsystem* Subsystem = GEngine->GetEngineSubsystem<UBasicDataBaseSubsystem>();
	TSharedPtr<FBasicDatabaseNative> Database = Subsystem->DatabaseForDomain(Domain);

	if (Database)
	{
		//Run the lookup function
		bOutDidRead = Database->ReadJsonStringAtIndex(PrimaryKey, OutJsonString);
	}
}

void UBasicDataBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//We depend on file system being available for our derivative functions
	Collection.InitializeDependency(UCUFileSubsystem::StaticClass());

	UE_LOG(LogTemp, Log, TEXT("UBasicDataBaseSubsystem initiliazed."));
}

void UBasicDataBaseSubsystem::Deinitialize()
{
	for (const TPair<FString, TSharedPtr<FBasicDatabaseNative>>& Pair : Databases)
	{
		Pair.Value->FlushPendingCachedData();
	}

	Databases.Empty();
}

TSharedPtr<FBasicDatabaseNative> UBasicDataBaseSubsystem::DatabaseForDomain(const FString& Domain)
{
	//Whenever this is accessed, auto-connect
	ConnectDatabase(Domain);

	return Databases[Domain];
}

