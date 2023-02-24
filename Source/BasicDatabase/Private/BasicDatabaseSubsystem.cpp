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

