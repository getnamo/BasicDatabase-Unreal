#pragma once

class UCUFileSubsystem;

/**
* Core class for loading databases, wrapped as a non-uobject base for more flexible integration (e.g. component).
*/
class FBasicDatabaseNative
{
public:
	FBasicDatabaseNative();
	~FBasicDatabaseNative();

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

protected:
	//Will get initialized before constructor
	UCUFileSubsystem* FileSystem;
};