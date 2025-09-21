using UnrealBuildTool;

public class District_test : ModuleRules
{
    public District_test(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "AIModule",
            "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {

        });

        PublicIncludePaths.AddRange(new string[]
        {
            "District_test/Public"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "District_test/Private"
        });
    }
}