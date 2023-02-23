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
	RootPath = FileSystem->ProjectSavedDirectory();
}

FBasicDatabaseNative::~FBasicDatabaseNative()
{
	FileSystem = nullptr;
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
	return TEXT("Invalid");
}

bool FBasicDatabaseNative::RemoveStructFromDatabase(const FString& Index)
{
	return false;
}

void FBasicDatabaseNative::ReadStructAtIndex(UStruct* Struct, void* StructPtr)
{

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

//Testing suite
void FBasicDatabaseNative::RunRCTest(const FString& InRCDomain)
{
	UE_LOG(LogTemp, Log, TEXT("rc domain %s"), *InRCDomain);

	//Test the RCDomain
	FileDomain = FileDomainFromRC(InRCDomain);

	UE_LOG(LogTemp, Log, TEXT("file domain %s"), *FileDomain);
}
