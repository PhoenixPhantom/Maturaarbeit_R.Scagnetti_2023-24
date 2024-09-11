using UnrealBuildTool;

public class MAProjectEditor : ModuleRules
{
    public MAProjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.AddRange(new string[]{ "MAProject/Public", "MAProjectEditor/Public"});
        PrivateIncludePaths.AddRange(new string[]{ "MAProjectEditor/Private"});

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "MAProject"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "GenericGraphRuntime",
                "MAProjectEditor"
            }
        );
    }
}