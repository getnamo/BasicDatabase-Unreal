#pragma once

class UCUFileSubsystem;

#include "BasicDatabaseNative.generated.h"

USTRUCT()
struct FBDBPrimaryKeyIndex
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() 
	TMap<int32, FString> PrimaryKeyMap;	

	//This property will decouple from index key length on add/remove
	UPROPERTY()
	int32 MaxPrimaryKey;

	FBDBPrimaryKeyIndex()
	{
		MaxPrimaryKey = 0;
	}
};

/*
USTRUCT()
struct FWaitableStructResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UStruct* Struct;

	FThreadSafeBool bReadComplete;
	void* StructPtr;

	//Todo: add a function that converts the result to TFieldAnyStruct

	FWaitableStructResult()
	{
		bReadComplete = false;
		StructPtr = nullptr;
	}
};
*/

struct FStructSaveCommand 
{
	UStruct* Struct;
	void* StructPtr;
	FString Path;
	bool bIsBlueprintStruct;
};

class FPrimaryKeyIndexHandler
{
public:
	FPrimaryKeyIndexHandler(const FString& InPath, const FString& PrimaryKeyFileName = TEXT("PrimaryKeyIndex.json"));

	FString AddNewEntry(int32 Key);
	void RemoveEntry(int32 Key);
	bool HasKey(int32 Key);

	//While this is trivial for current setup, the wrapper is used for testing availability for index
	FString StringFromKey(int32 Key);
	int32 KeyFromString(const FString& InKeyString);

	void GetKeys(TArray<int32>& OutKeys);

	int32 NextPrimaryKey();

	//Read into cache
	bool ReadPrimaryKeyFile();
	bool SavePrimaryKeyFile();

	//returns true if flushed (or already flushed)
	bool SaveIfDirty();

	bool IsCached();

protected:
	FString PrimaryKeyFilePath;
	FBDBPrimaryKeyIndex PrimaryIndex;
	bool bIndexIsCached;
	bool bIsDirty;
};

/**
* Core class for loading databases, wrapped as a non-uobject base for more flexible integration (e.g. component).
*/
class BASICDATABASE_API FBasicDatabaseNative
{
public:
	//RC domain is reverse comlike e.g. entity.npc
	FBasicDatabaseNative(const FString& InRCDomain = TEXT("default"));
	~FBasicDatabaseNative();

	bool bOutputUpdatesToLog;

	//Main API
	bool ReadIndicesToCache();

	//Main Index

	FString AddStructToDatabase(UStruct* Struct, void* StructPtr, bool bIsBlueprintStruct = false);
	FString AddJsonStringToDatabase(const FString& JsonString);
	bool RemoveEntryFromDatabase(const FString& PrimaryKey);

	

	//If struct didn't exist, returns new PK
	FString UpdateStructAtPrimaryIndex(UStruct* Struct, void* StructPtr, const FString& PrimaryKey, bool bIsBlueprintStruct = false);
	FString UpdateJsonStringAtPrimaryIndex(const FString& JsonString, const FString& Index);

	bool ReadStructAtIndex(UStruct* Struct, void* StructPtr, const FString& Index, bool bIsBlueprintStruct = false);
	bool ReadJsonStringAtIndex(const FString& Index, FString& OutJsonString);
	

	//Spatial index
	bool AddSpatialIndex(const FString& Index, const FVector& Location);
	bool RemoveSpatialIndex(const FString& Index);

	//returns true if any results were found. OutResults is a list of main indices
	bool FindInSpatialIndex(const FVector& AtSpatial, TArray<FString>& OutResults, float Radius = 1.f);

	//Auto-populate files, update indices

	//Caching functions and lazy flush, pre-fetch?

	//Save/Load - local overrides for convenience
	bool SaveBytesToPath(const TArray<uint8>& Bytes, const FString& Path, bool bLogSave = false);
	bool ReadBytesFromPath(const FString& Path, TArray<uint8>& OutBytes);


	//Utility
	//Obtain a file domain from a reverse comlike domain e.g. entity.npc -> Entity-Npc
	FString FileDomainFromRC(const FString& InDomain);
	void CapitalizeLetterAtIndex(FString& StringToModify, int32 Index);

	//Queue up saves and flushes
	void QueueSave(const FStructSaveCommand& SaveCommand);
	void FlushPendingCachedData();

	static bool SaveStringToPath(const FString& JsonString, const FString& Path);
	static bool LoadStringFromPath(const FString& Path, FString& OutJsonString);
	static bool SaveStructToPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);
	static bool LoadStructFromPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);

protected:
	//Save/Load struct wrapper, BP fieldpaths require custom thunk wrapper


	//The file domain that this db uses e.g. Default or Entity-Npc, converrted from reverse comlike
	FString FileDomain;

	//Root path for all databases
	FString DirectoryPath;
	FString DataDirectory;

	//Primary key file
	FString PrimaryKeyFileName;

	//Will get initialized before constructor
	UCUFileSubsystem* FileSystem;

	TSharedPtr<FPrimaryKeyIndexHandler> PrimaryKeyHandler;
	TQueue<FStructSaveCommand> SaveQueue;
	bool bInstantSave;


	//Path utility
	FString PrimaryKeyPath();
	FString RootPath();
	FString EntryPathForPrimaryKey(const FString& Index);

	//Utility
	void RunRCTest(const FString& InRCDomain);
};