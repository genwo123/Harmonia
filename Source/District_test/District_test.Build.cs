using UnrealBuildTool;

public class District_test : ModuleRules
{
    public District_test(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public dependencies (다른 모듈에서도 접근 가능)
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput", // Enhanced Input 모듈 추가
            "UMG",           // UMG(User Interface) 모듈 추가
            "Slate",         // Slate UI 프레임워크
            "SlateCore"      // Slate 핵심 컴포넌트
        });

        // Private dependencies (이 모듈 내부 전용)
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // 예시: "Slate", "SlateCore"
        });

        // Public Include Paths
        PublicIncludePaths.AddRange(new string[] {
            "District_test/Public"
        });

        // Private Include Paths
        PrivateIncludePaths.AddRange(new string[] {
            "District_test/Private"
        });
    }
}