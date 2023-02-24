#include "BasicDatabaseNative.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CUFileSubsystem.h"
#include "SIOJConvert.h"

FBasicDatabaseNative::FBasicDatabaseNative(const FString& InRCDomain)
{
	FileSystem	= GEngine->GetEngineSubsystem<UCUFileSubsystem>();
	FileDomain = FileDomainFromRC(InRCDomain);
	DirectoryPath = FileSystem->ProjectSavedDirectory();

	PrimaryKeyHandler = MakeShareable(new FPrimaryKeyIndexHandler(DirectoryPath));
}

FBasicDatabaseNative::~FBasicDatabaseNative()
{
	FileSystem = nullptr;
}

bool FBasicDatabaseNative::ReadIndicesToCache()
{
	bool bValidPK = PrimaryKeyHandler->ReadPrimaryKeyFile();
	
	//First time we're reading this!
	if (!bValidPK)
	{
		//Flush an empty file
		PrimaryKeyHandler->SavePrimaryKeyFile();

		//re-read it
		PrimaryKeyHandler->ReadPrimaryKeyFile();
	}

	//bIndexIsCached is set from reading the primary key file
	return PrimaryKeyHandler->IsCached();
}

FString FBasicDatabaseNative::FileDomainFromRC(const FString& InRCDomain /*= TEXT("default")*/)
{
	//if we didn't pass in a domain, return a default domain
	if (InRCDomain.IsEmpty())
	{
		return TEXT("Default");
	}
	//Copy for mutation
	FString MutableDomain = InRCDomain;

	//Capitalize the first letter
	CapitalizeLetterAtIndex(MutableDomain, 0);

	//Capitalize each letter after .
	FRegexPattern Pattern = FRegexPattern(TEXT("\\."));	//matches any period

	FRegexMatcher Matcher(Pattern, MutableDomain);

	while (Matcher.FindNext())
	{
		int32 BeginIndex = Matcher.GetMatchBeginning();
		int32 EndIndex = Matcher.GetMatchEnding();

		CapitalizeLetterAtIndex(MutableDomain, EndIndex);
		//UE_LOG(LogTemp, Log, TEXT("Indices: %d, %d"), BeginIndex, EndIndex);
	}
	//replace . to -
	MutableDomain = MutableDomain.Replace(TEXT("."), TEXT("-"));

	return MutableDomain;
}

bool FBasicDatabaseNative::SaveStructToPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct)
{
	//SIOJConvert already has a utility that does the base functionality we need, wrap it directly in our api
	return USIOJConvert::ToJsonFile(Path, Struct, StructPtr, bIsBlueprintStruct);
}

bool FBasicDatabaseNative::LoadStructFromPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct)
{
	//SIOJConvert already has a utility that does the base functionality we need, wrap it directly in our api
	return USIOJConvert::JsonFileToUStruct(Path, Struct, StructPtr, bIsBlueprintStruct);
}

//BEGIN TODO: implement
FString FBasicDatabaseNative::AddStructToDatabase(UStruct* Struct, void* StructPtr)
{
	//Request a new index
	int32 NextPK= PrimaryKeyHandler->NextPrimaryKey();
	if (NextPK == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid key received, database indices have not been read/cached. Adding struct data failed."));
		return TEXT("Invalid");
	}

	//Update index cache
	//PrimaryKeyHandler->Add

	//Store struct
	return TEXT("Invalid");
}

bool FBasicDatabaseNative::RemoveStructFromDatabase(const FString& Index)
{
	//remove struct index

	//delete struct data
	return false;
}

bool FBasicDatabaseNative::ReadStructAtIndex(UStruct* Struct, void* StructPtr, const FString& Index)
{
	return LoadStructFromPath(Struct, StructPtr, Index);
}

bool FBasicDatabaseNative::AddSpatialIndex(const FString& Index, const FVector& Location)
{
	return false;
}

bool FBasicDatabaseNative::RemoveSpatialIndex(const FString& Index)
{
	return false;
}

bool FBasicDatabaseNative::FindInSpatialIndex(const FVector& AtSpatial, TArray<FString>& OutResults, float Radius /*= 1.f*/)
{
	return false;
}
//END TODO

bool FBasicDatabaseNative::SaveBytesToPath(const TArray<uint8>& Bytes, const FString& Path, bool bLogSave /*= false*/)
{
	return FileSystem->SaveBytesToPath(Bytes, Path, bLogSave);
}

bool FBasicDatabaseNative::ReadBytesFromPath(const FString& Path, TArray<uint8>& OutBytes)
{
	return FileSystem->ReadBytesFromPath(Path, OutBytes);
}

void FBasicDatabaseNative::CapitalizeLetterAtIndex(FString& StringToModify, int32 Index)
{
	StringToModify[Index] = FChar::ToUpper(StringToModify[Index]);
}

FString FBasicDatabaseNative::RootPath()
{
	return FPaths::Combine(DirectoryPath, FileDomain);
}

FString FBasicDatabaseNative::PrimaryKeyPath()
{
	return FPaths::Combine(RootPath(), PrimaryKeyFileName);
}

FString FBasicDatabaseNative::StructPathForIndex(const FString& Index)
{
	return FPaths::Combine(RootPath(), Index + TEXT(".json"));
}

//Testing suite
void FBasicDatabaseNative::RunRCTest(const FString& InRCDomain)
{
	UE_LOG(LogTemp, Log, TEXT("rc domain %s"), *InRCDomain);

	//Test the RCDomain
	FileDomain = FileDomainFromRC(InRCDomain);

	UE_LOG(LogTemp, Log, TEXT("file domain %s"), *FileDomain);
}

//PrimaryKeyIndex
FPrimaryKeyIndexHandler::FPrimaryKeyIndexHandler(const FString& InPath, const FString& PrimaryKeyFileName)
{
	PrimaryKeyFilePath = FPaths::Combine(InPath, PrimaryKeyFileName);;
	bIsDirty = false;
	bIndexIsCached = false;
}

void FPrimaryKeyIndexHandler::AddNewEntry(int32 Key, const FString& FileKey)
{
	PrimaryIndex.PrimaryKeyMap.Add(Key, FileKey);
	bIsDirty = true;
}

void FPrimaryKeyIndexHandler::RemoveEntry(int32 Key)
{
	PrimaryIndex.PrimaryKeyMap.Remove(Key);
	bIsDirty = true;
}

void FPrimaryKeyIndexHandler::GetKeys(TArray<int32>& OutKeys)
{
	PrimaryIndex.PrimaryKeyMap.GetKeys(OutKeys);
}

int32 FPrimaryKeyIndexHandler::NextPrimaryKey()
{
	//We're 0 indexed, return 0 as first key
	int32 NextKey = PrimaryIndex.MaxPrimaryKey;
	PrimaryIndex.MaxPrimaryKey++;
	bIsDirty = true;
	return PrimaryIndex.MaxPrimaryKey;
}

bool FPrimaryKeyIndexHandler::ReadPrimaryKeyFile()
{
	bIndexIsCached = USIOJConvert::JsonFileToUStruct(PrimaryKeyFilePath, FBDBPrimaryKeyIndex::StaticStruct(), &PrimaryIndex);
	return bIndexIsCached;
}

bool FPrimaryKeyIndexHandler::SavePrimaryKeyFile()
{
	bool bSaveSuccessful = USIOJConvert::ToJsonFile(PrimaryKeyFilePath, FBDBPrimaryKeyIndex::StaticStruct(), &PrimaryIndex);
	bIsDirty = !bSaveSuccessful;
	return bSaveSuccessful;
}

bool FPrimaryKeyIndexHandler::IsCached()
{
	return bIndexIsCached;
}
