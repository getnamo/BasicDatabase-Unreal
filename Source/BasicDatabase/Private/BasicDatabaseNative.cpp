#include "BasicDatabaseNative.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FBasicDatabaseNative::FBasicDatabaseNative()
{

}

FBasicDatabaseNative::~FBasicDatabaseNative()
{

}


bool FBasicDatabaseNative::SaveBytesToFile(const TArray<uint8>& Bytes, const FString& Directory, const FString& FileName, bool bLogSave)
{
	//bool AllowOverwriting = false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.CreateDirectoryTree(*Directory))
	{
		FString AbsoluteFilePath;

		// Get absolute file path
		if (Directory.EndsWith(TEXT("/")))
		{
			AbsoluteFilePath = FPaths::ConvertRelativePathToFull(Directory + FileName);
		}
		else
		{
			AbsoluteFilePath = FPaths::ConvertRelativePathToFull(Directory + "/" + FileName);
		}

		// Allow overwriting or file doesn't already exist
		bool bSaveSuccesful = FFileHelper::SaveArrayToFile(Bytes, *AbsoluteFilePath);

		if (bLogSave)
		{
			if (bSaveSuccesful)
			{
				UE_LOG(LogTemp, Log, TEXT("Saved: %s with %d bytes"), *AbsoluteFilePath, Bytes.Num());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Failed to save: %s"), *AbsoluteFilePath);
			}
		}

		return bSaveSuccesful;
	}

	return false;
}

bool FBasicDatabaseNative::ReadBytesFromFile(const FString& Directory, const FString& FileName, TArray<uint8>& OutBytes)
{
	// Get absolute file path
	FString AbsoluteFilePath = Directory + "/" + FileName;

	// Allow overwriting or file doesn't already exist
	return FFileHelper::LoadFileToArray(OutBytes, *AbsoluteFilePath);
}