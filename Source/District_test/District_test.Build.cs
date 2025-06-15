using UnrealBuildTool;

public class District_test : ModuleRules
{
    public District_test(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public dependencies (�ٸ� ��⿡���� ���� ����)
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput", // Enhanced Input ��� �߰�
            "UMG",           // UMG(User Interface) ��� �߰�
            "Slate",         // Slate UI �����ӿ�ũ
            "SlateCore"      // Slate �ٽ� ������Ʈ
        });

        // Private dependencies (�� ��� ���� ����)
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // ����: "Slate", "SlateCore"
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