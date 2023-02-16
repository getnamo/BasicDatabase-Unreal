#include "BasicDatabaseNative.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CUFileSubsystem.h"

FBasicDatabaseNative::FBasicDatabaseNative()
{
	FileSystem	= GEngine->GetEngineSubsystem<UCUFileSubsystem>();
}

FBasicDatabaseNative::~FBasicDatabaseNative()
{

}


bool FBasicDatabaseNative::SaveBytesToFile(const TArray<uint8>& Bytes, const FString& Directory, const FString& FileName, bool bLogSave)
{
	return FileSystem->SaveBytesToFile(Bytes, Directory, FileName, bLogSave);
}

bool FBasicDatabaseNative::ReadBytesFromFile(const FString& Directory, const FString& FileName, TArray<uint8>& OutBytes)
{
	return FileSystem->ReadBytesFromFile(Directory, FileName, OutBytes);
}