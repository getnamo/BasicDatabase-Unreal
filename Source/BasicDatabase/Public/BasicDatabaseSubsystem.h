#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "BasicDatabaseNative.h"
#include "BasicDatabaseSubsystem.generated.h"

UCLASS(ClassGroup = "BasicDatabase")
class BASICDATABASE_API UBasicDataBaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	
	virtual	void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	TSharedPtr<FBasicDatabaseNative> DBNative;
};
