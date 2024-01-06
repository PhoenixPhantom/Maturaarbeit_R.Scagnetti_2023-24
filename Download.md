# Get the Game Running

> <picture>
>   <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/light-theme/info.svg">
>   <img alt="Info" src="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/dark-theme/info.svg">
> </picture><br>
>
> The current instructions were only verified on devices using Microsoft Windows 11, but should also work on Windows 10 and newer versions of MacOS

> <picture>
>   <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/light-theme/info.svg">
>   <img alt="Info" src="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/dark-theme/info.svg">
> </picture><br>
>
> Visual Studio 2022 does not support MacOS as of August 2023. Alternatively the [Visual Studio 2022 for Mac Preview](https://visualstudio.microsoft.com/vs/mac/preview/) can be used.

> Every time the local folder is synchronized with the GitHub, especially if any changes have been made to the project's "Source", the above steps must be repeated.
> <picture>
>   <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/light-theme/info.svg">
>   <img alt="Info" src="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/dark-theme/info.svg">
> </picture><br>
>
> The code editor used in this work is jetbrain's [rider](https://www.jetbrains.com/rider/). However, as this software only has a paid version, the functionally equivalent Visual Studio is reccomended in these instructions.



Software requirements:
 * Install Unreal Engine 5.2 from the [Epic Games launcher](https://store.epicgames.com/en-US/download)
 * Install the [GitHub desktop app](https://desktop.github.com)
 * Download the [Git Large File Storage](https://git-lfs.com) and install it by running "git lfs install" in the command line
 * Download [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) and install "Game development with C++" in the "Workloads" tab.
 * Install [.NET 6.0](https://dotnet.microsoft.com/en-us/download) and the newest version of the [.NET framework developer pack (currently 4.8.1)](https://dotnet.microsoft.com/en-us/download/dotnet-framework/thank-you/net481-developer-pack-offline-installer)



After downloading the repository:
 1. Right-click on "MAProject.uproject", choose "Generate Visual Studio project files" and wait for the process to complete
 2. Double-click "MAProject.sln" to open the project in Visual Studio 2022
 3. Do not touch any of the compilation settings
 4. Click F5 or "Run" the project (green play button)
 5. From now on the project can be run by simply double-clicking "MAProject.uproject"
 6. After the shaders are compiled, the game will launch to the Unreal Engine editor
 7. click the green play button in the top left corner or the editor window

> <picture>
>   <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/light-theme/warning.svg">
>   <img alt="Warning" src="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/dark-theme/warning.svg">
> </picture><br>
>


[return to README](README.md)
