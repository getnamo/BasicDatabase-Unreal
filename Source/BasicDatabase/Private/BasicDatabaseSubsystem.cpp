#include "BasicDatabaseSubsystem.h"
#include "CUFileSubsystem.h"

void UBasicDataBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//We depend on file system being available for our derivative functions
	Collection.InitializeDependency(UCUFileSubsystem::StaticClass());

	DBNative = MakeShareable(new FBasicDatabaseNative());
}

void UBasicDataBaseSubsystem::Deinitialize()
{
	DBNative = nullptr;
}

