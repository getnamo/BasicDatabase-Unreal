#pragma once

class UCUFileSubsystem;

/**
* Core class for loading databases, wrapped as a non-uobject base for more flexible integration (e.g. component).
*/
class FBasicDatabaseNative
{
public:
	//RC domain is reverse comlike e.g. entity.npc
	FBasicDatabaseNative(const FString& InRCDomain = TEXT("default"));
	~FBasicDatabaseNative();

	//Root path for all databases
	FString RootPath;


	//Main API

	//Save/Load struct wrapper, BP fieldpaths require custom thunk wrapper
	bool SaveStructToPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);
	bool LoadStructFromPath(UStruct* Struct, void* StructPtr, const FString& Path, bool bIsBlueprintStruct = false);

	//Generate indices

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

	//Will get initialized before constructor
	UCUFileSubsystem* FileSystem;

	//Utility
	void RunRCTest(const FString& InRCDomain);
};