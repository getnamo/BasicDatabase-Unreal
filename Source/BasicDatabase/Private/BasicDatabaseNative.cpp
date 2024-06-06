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
	DataDirectory = TEXT("Data");
	bOutputUpdatesToLog = true;

	PrimaryKeyHandler = MakeShareable(new FPrimaryKeyIndexHandler(DirectoryPath));

	bInstantSave = true; //unoptimized path
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
	/*if (bOutputUpdatesToLog)
	{
		UE_LOG(LogTemp, Log, TEXT("Saved struct to %s"), *FPaths::ConvertRelativePathToFull(Path));
	}*/
	return USIOJConvert::ToJsonFile(Path, Struct, StructPtr, bIsBlueprintStruct);
}

bool FBasicDatabaseNative::SaveStringToPath(const FString& JsonString, const FString& Path)
{
	FTCHARToUTF8 Utf8String(*JsonString);

	TArray<uint8> Bytes;
	Bytes.Append((uint8*)Utf8String.Get(), Utf8String.Length());

	//flush to disk
	return FFileHelper::SaveArrayToFile(Bytes, *Path);
}

bool FBasicDatabaseNative::LoadStructFromPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct)
{
	//SIOJConvert already has a utility that does the base functionality we need, wrap it directly in our api
	return USIOJConvert::JsonFileToUStruct(Path, Struct, StructPtr, bIsBlueprintStruct);
}

bool FBasicDatabaseNative::LoadStringFromPath(const FString& Path, FString& OutJsonString)
{
	//Read bytes from file
	TArray<uint8> OutBytes;
	if (!FFileHelper::LoadFileToArray(OutBytes, *Path))
	{
		return false;
	}

	//Convert to json string
	FFileHelper::BufferToString(OutJsonString, OutBytes.GetData(), OutBytes.Num());

	return true;
}

FString FBasicDatabaseNative::AddStructToDatabase(UStruct* Struct, void* StructPtr, bool bIsBlueprintStruct)
{
	//Request a new index
	int32 NextPK= PrimaryKeyHandler->NextPrimaryKey();
	if (NextPK == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid key received, database indices have not been read/cached. Adding struct data failed."));
		return TEXT("Invalid");
	}

	//Update index cache
	FString KeyString = PrimaryKeyHandler->AddNewEntry(NextPK);
	bool bSuccessfulQueue = false;

	//Simple path, will cause hitches
	if (bInstantSave)
	{
		bSuccessfulQueue = SaveStructToPath(Struct, StructPtr, EntryPathForPrimaryKey(KeyString), bIsBlueprintStruct);
	}
	else
	{
		//We use queuing and flush on separate threads if appropriate
		FStructSaveCommand SaveCommand;
		SaveCommand.Struct = Struct;
		SaveCommand.StructPtr = StructPtr;
		SaveCommand.Path = EntryPathForPrimaryKey(KeyString);
		SaveCommand.bIsBlueprintStruct = bIsBlueprintStruct;
		QueueSave(SaveCommand);
		bSuccessfulQueue = true;
	}

	//Store struct
	if (!bSuccessfulQueue)
	{
		UE_LOG(LogTemp, Error, TEXT("Struct save failed."));
		return TEXT("Invalid");
	}
	return KeyString;
}

FString FBasicDatabaseNative::AddJsonStringToDatabase(const FString& JsonString)
{
	//Request a new index
	int32 NextPK = PrimaryKeyHandler->NextPrimaryKey();
	if (NextPK == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid key received, database indices have not been read/cached. Adding struct data failed."));
		return TEXT("Invalid");
	}

	FString KeyString = PrimaryKeyHandler->AddNewEntry(NextPK);
	
	//NB: debug bypass for now
	return KeyString;

	/*bool bSuccessfulQueue = false;

	//Simple path, will cause hitches
	if (bInstantSave)
	{
		bSuccessfulQueue = SaveStringToPath(JsonString, EntryPathForPrimaryKey(KeyString));
	}

	//Store struct
	if (!bSuccessfulQueue)
	{
		UE_LOG(LogTemp, Error, TEXT("Struct save failed."));
		return TEXT("Invalid");
	}
	return KeyString;*/
}

bool FBasicDatabaseNative::RemoveEntryFromDatabase(const FString& PrimaryKey)
{
	int32 Key = PrimaryKeyHandler->KeyFromString(PrimaryKey);

	//remove struct index
	PrimaryKeyHandler->RemoveEntry(Key);

	//obtain correct path then delete struct data
	FString Path = EntryPathForPrimaryKey(PrimaryKey);
	
	//Todo: validate that path is part of our directory (i.e. no risk of deleting file outside of desired folder)

	return FileSystem->DeleteFileAtPath(Path);
}

FString FBasicDatabaseNative::UpdateStructAtPrimaryIndex(UStruct* Struct, void* StructPtr, const FString& PrimaryKey, bool bIsBlueprintStruct)
{
	int32 Key = PrimaryKeyHandler->KeyFromString(PrimaryKey);

	if (PrimaryKey == TEXT("New") || !PrimaryKeyHandler->HasKey(Key))
	{
		return AddStructToDatabase(Struct, StructPtr, bIsBlueprintStruct);
	}
	else
	{
		SaveStructToPath(Struct, StructPtr, EntryPathForPrimaryKey(PrimaryKey), bIsBlueprintStruct);
		return PrimaryKey;
	}
}

FString FBasicDatabaseNative::UpdateJsonStringAtPrimaryIndex(const FString& JsonString, const FString& PrimaryKey)
{
	int32 Key = PrimaryKeyHandler->KeyFromString(PrimaryKey);

	if (PrimaryKey == TEXT("New") || !PrimaryKeyHandler->HasKey(Key))
	{
		return AddJsonStringToDatabase(JsonString);
	}
	else
	{
		SaveStringToPath(JsonString, EntryPathForPrimaryKey(PrimaryKey));
		return PrimaryKey;
	}
}

bool FBasicDatabaseNative::ReadStructAtIndex(UStruct* Struct, void* StructPtr, const FString& PrimaryKey, bool bIsBlueprintStruct)
{
	return LoadStructFromPath(Struct, StructPtr, EntryPathForPrimaryKey(PrimaryKey), bIsBlueprintStruct);
}

bool FBasicDatabaseNative::ReadJsonStringAtIndex(const FString& Index, FString& OutJsonString)
{
	return LoadStringFromPath(EntryPathForPrimaryKey(Index), OutJsonString);
}


//TODO: Add spatial query
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

void FBasicDatabaseNative::QueueSave(const FStructSaveCommand& SaveCommand)
{
	SaveQueue.Enqueue(SaveCommand);
}

void FBasicDatabaseNative::FlushPendingCachedData()
{
	while (!SaveQueue.IsEmpty())
	{
		FStructSaveCommand SaveCommand;
		SaveQueue.Dequeue(SaveCommand);
		SaveStructToPath(SaveCommand.Struct, SaveCommand.StructPtr, SaveCommand.Path, SaveCommand.bIsBlueprintStruct);
	}
}

FString FBasicDatabaseNative::RootPath()
{
	return FPaths::Combine(DirectoryPath, FileDomain);
}

FString FBasicDatabaseNative::PrimaryKeyPath()
{
	return FPaths::Combine(RootPath(), PrimaryKeyFileName);
}

FString FBasicDatabaseNative::EntryPathForPrimaryKey(const FString& Index)
{
	return FPaths::Combine(RootPath(), DataDirectory,  Index + TEXT(".json"));
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

FString FPrimaryKeyIndexHandler::AddNewEntry(int32 Key)
{
	//NB: might be more appropriate to use a TSet, but for now using map for future flexibility of remapping indices
	const FString KeyString = FString::Printf(TEXT("%d"), Key);
	PrimaryIndex.PrimaryKeyMap.Add(Key, KeyString);
	bIsDirty = true;
	return KeyString;
}

void FPrimaryKeyIndexHandler::RemoveEntry(int32 Key)
{
	PrimaryIndex.PrimaryKeyMap.Remove(Key);
	bIsDirty = true;
}

bool FPrimaryKeyIndexHandler::HasKey(int32 Key)
{
	return PrimaryIndex.PrimaryKeyMap.Contains(Key);
}

FString FPrimaryKeyIndexHandler::StringFromKey(int32 Key)
{
	if (!PrimaryIndex.PrimaryKeyMap.Contains(Key)) 
	{
		return TEXT("Invalid");
	}
	else
	{
		return PrimaryIndex.PrimaryKeyMap[Key];
	}
}

int32 FPrimaryKeyIndexHandler::KeyFromString(const FString& InKeyString)
{
	return FCString::Atoi(*InKeyString);
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

bool FPrimaryKeyIndexHandler::SaveIfDirty()
{
	if (bIsDirty)
	{
		return SavePrimaryKeyFile();
	}
	return true;
}

bool FPrimaryKeyIndexHandler::IsCached()
{
	return bIndexIsCached;
}
