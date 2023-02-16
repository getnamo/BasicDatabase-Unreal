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

	//Save/Load
	bool SaveBytesToFile(const TArray<uint8>& Bytes, const FString& Directory, const FString& FileName, bool bLogSave);
	bool ReadBytesFromFile(const FString& Directory, const FString& FileName, TArray<uint8>& OutBytes);

protected:
	//Will get initialized before constructor
	UCUFileSubsystem* FileSystem;
};