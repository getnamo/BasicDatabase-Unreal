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

	RunRCTest(TEXT("group.default"));
	RunRCTest(TEXT("some.thing.is.good"));
	UE_LOG(LogTemp, Log, TEXT("++++Done++++"));
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

FString TransformString(FString InputString)
{
	if (InputString.IsEmpty())
	{
		return FString("Default");
	}

	FString TransformedString = InputString.Replace(TEXT("."), TEXT("-"));
	TransformedString[0] = FChar::ToUpper(TransformedString[0]);

	for (int i = 1; i < TransformedString.Len(); i++)
	{
		if (TransformedString[i - 1] == '-')
		{
			TransformedString[i] = FChar::ToUpper(TransformedString[i]);
		}
	}

	return TransformedString;
}

//Testing suite
void FBasicDatabaseNative::RunRCTest(const FString& InRCDomain)
{
	UE_LOG(LogTemp, Log, TEXT("rc domain %s"), *InRCDomain);

	//Test the RCDomain
	FileDomain = FileDomainFromRC(InRCDomain);

	UE_LOG(LogTemp, Log, TEXT("file domain %s"), *FileDomain);

	FString GPTTestString = TransformString(InRCDomain);

	UE_LOG(LogTemp, Log, TEXT("file domain2 %s"), *GPTTestString);
}


