#pragma once

#if _MSC_VER
#define FUNC_NAME    TEXT(__FUNCTION__)
#else // FIXME - GCC?
#define FUNC_NAME    TEXT(__func__)
#endif

#define NETMODE_WORLD(Context) (((GEngine == nullptr) || (Context == nullptr) || (Context->GetWorld() == nullptr)) \
						? TEXT("Invalid") \
						: (GEngine->GetNetMode(Context->GetWorld()) == NM_Client) ? ( EWorldType::PIE == Context->GetWorld()->WorldType ? *FString::Printf(TEXT("Client<%d>"), UE::GetPlayInEditorID()) : TEXT("Client") ) \
						: (GEngine->GetNetMode(Context->GetWorld()) == NM_ListenServer) ? TEXT("ListenServer") \
						: (GEngine->GetNetMode(Context->GetWorld()) == NM_DedicatedServer) ? TEXT("DedicatedServer") \
						: TEXT("Standalone"))


//#define AGP_LOG_FORMAT_OBJECT_NAME(Format, ...) FString::Printf(TEXT("%s : %s"), *GetNameSafe(this), FUNC_NAME, *FString::Printf(Format, ##__VA_ARGS__))

#define AGP_LOG_FORMAT(Format, ...) FString::Printf(TEXT("%s()\t%s"), FUNC_NAME, *FString::Printf(Format, ##__VA_ARGS__))

#define AGP_NET_LOG(Context, CategoryName, Verbosity, Format, ...) \
{ \
	UE_LOG(CategoryName, Verbosity, TEXT("[Net: %s]\t%s"), NETMODE_WORLD(Context), *AGP_LOG_FORMAT(Format, ##__VA_ARGS__)); \
}

#define AGP_LOG(CategoryName, Verbosity, Format, ...) \
{ \
	UE_LOG(CategoryName, Verbosity, TEXT("%s"), *AGP_LOG_FORMAT(Format, ##__VA_ARGS__)); \
}

#define AGP_SCREEN_DEBUG_MSG(Key, TimeToDisplay, DisplayColor, Format, ...) \
{ \
	if(GEngine) \
	{ \
		GEngine->AddOnScreenDebugMessage(Key, TimeToDisplay, DisplayColor, AGP_LOG_FORMAT(Format, ##__VA_ARGS__)); \
	}\
}