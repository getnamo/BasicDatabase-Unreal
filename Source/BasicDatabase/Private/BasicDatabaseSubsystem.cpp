#include "BasicDatabaseSubsystem.h"

void UBasicDataBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	DBNative = MakeShareable(new FBasicDatabaseNative());
}

void UBasicDataBaseSubsystem::Deinitialize()
{
	DBNative = nullptr;
}

