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

class FPrimaryKeyIndexHandler
{
public:
	FPrimaryKeyIndexHandler(const FString& InPath, const FString& PrimaryKeyFileName = TEXT("PrimaryKeyIndex.json"));

	void AddNewEntry(int32 Key, const FString& FileKey);
	void RemoveEntry(int32 Key);

	void GetKeys(TArray<int32>& OutKeys);

	int32 NextPrimaryKey();

	//Read into cache
	bool ReadPrimaryKeyFile();
	bool SavePrimaryKeyFile();

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
class FBasicDatabaseNative
{
public:
	//RC domain is reverse comlike e.g. entity.npc
	FBasicDatabaseNative(const FString& InRCDomain = TEXT("default"));
	~FBasicDatabaseNative();


	//Main API
	bool ReadIndicesToCache();

	//Save/Load struct wrapper, BP fieldpaths require custom thunk wrapper
	bool SaveStructToPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);
	bool LoadStructFromPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);

	//Main Index
	FString AddStructToDatabase(UStruct* Struct, void* StructPtr);
	bool RemoveStructFromDatabase(const FString& Index);

	bool ReadStructAtIndex(UStruct* Struct, void* StructPtr, const FString& Index);
	

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

protected:
	//The file domain that this db uses e.g. Default or Entity-Npc, converrted from reverse comlike
	FString FileDomain;

	//Root path for all databases
	FString DirectoryPath;

	//Primary key file
	FString PrimaryKeyFileName;

	//Will get initialized before constructor
	UCUFileSubsystem* FileSystem;

	TSharedPtr<FPrimaryKeyIndexHandler> PrimaryKeyHandler;


	//Path utility
	FString PrimaryKeyPath();
	FString RootPath();
	FString StructPathForIndex(const FString& Index);

	//Utility
	void RunRCTest(const FString& InRCDomain);
};