#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "BasicDatabaseNative.h"
#include "BasicDatabaseSubsystem.generated.h"

UCLASS(ClassGroup = "WRCoreAI")
class BASICDATABASE_API UBasicDataBaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual	void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	TSharedPtr<FBasicDatabaseNative> DBNative;
};
