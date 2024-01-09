# Combat Design in a 3D Open World Videogame - Raffael Scagnetti 2023/24
## Introduction to the Work
In the last decades, the videogame industry has grown enormously. Today, a high number of games get released by large studios every year trying to satisfy the huge demand for games. There is also a large number of small teams or even solo creators developing and releasing their own game. This leads to a market where players have a wide choice. That means that a game must meet the expectations of a player exactly and provide a promising game experience to be successful. Creating such a game is time-consuming and difficult even for large studios and generally requires massive effort. There are however differences in the level of difficulty and complexity depending on the type of the game to be created: creating a massive multiplayer online game (MMO) for example requires significantly more time and effort than creating a simple offline puzzle game like Tetris. One of the main reasons for this is that a MMO requires a much higher amount of mechanics and functionalities than a puzzle game.

### Combat Design in a 3D Open World Videogame
An open world game is another example of a game type requiring a massive work to create, since the premise of such a game is that the player can make their own decisions on how to move through the game's world. Because I am personally fascinated by open world games, I decided to work on such a game as my Maturaarbeit. Of course, creating a complete game doesn't fit in the limited time of a Maturaarbeit. Thus this work focuses on one key aspect that significantly influences the overall gameplay experience of an open world videogame: the combat design.

Combat design focuses on how the player experiences encounters with opponents inside the game world. Although combat is not necessarily a part of every open world videogame, most games include this mechanic. Often, the goal of combat is to reinforce the feeling of a real world and at the same time test the player's skills by challenging them. Required skills often include timing and strategy. Combat can also be used to give the player a certain feeling, like Santa Monica Studio's "God Of War" games, which want the player to feel powerful and therefore encourage offensive actions. Other videogames like FromSoftware's "Dark Souls" games want the player to think more strategically and thus discourage overly offensive behavior and rewards a considered style of play.

### The goal of this Maturaarbeit
The overall goal of this work was to create the combat elements of a 3D open world videogame. The combat system should be easy to understand and get used to and make the player feel powerful while also allowing skill expression. Making the work playable, it should also include a small world to test the mechanics in. This means in addition to creating a small test world with different terrains, the goal of the work is to create a combat system that:
 * is easy to understand (intuitive for averagely experienced gamers)
 * gives the player the required support to maintain control and confidence in their abilities
 * consists of mechanics and behaviors that feel natural
 * makes the player feel powerful
 * allows players to improve their gameplay when playing more skillfully
 * facilitates varied gameplay by allowing the player to execute a number of actions
 * includes various opponents which interact with the player and can attack them
 * makes combat interesting by having distinct opponent characters with a variety of different attacks

-- Extract of "Combat Design In A 3D Open World Videogame" (R. Scagnetti [2024](CombatDesignInA3DOpenWorldVideogame.pdf))

## Get The Game Running
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

> <picture>
>   <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/light-theme/info.svg">
>   <img alt="Info" src="https://raw.githubusercontent.com/Mqxx/GitHub-Markdown/main/blockquotes/badge/dark-theme/info.svg">
> </picture><br>
>
> The code editor used in this work is jetbrain's [rider](https://www.jetbrains.com/rider/). However, as this software only has a paid version, the functionally equivalent Visual Studio is reccomended in these instructions.



Software requirements:
 * Install Unreal Engine 5.2 from the [Epic Games launcher](https://store.epicgames.com/en-US/download)
 * Install the [GitHub desktop app](https://desktop.github.com)
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
> Every time the local folder is synchronized with the GitHub, especially if any changes have been made to the project's "Source", the above steps must be repeated.



## The Project's Folder Structure
<pre>
├───.idea
├───Config
├───Content
├───Platforms
├───Plugins
└───Source
</pre>

### .idea
Editor generated files;

### Config
In-Engine Settings; Can be modified through toggles and dropdowns in-engine.

### Content
Assets used in the Engine; This is also the "Content" folder in the engine filebrowser.
All files whose content is not part of the "Used Assets" as mentionned in the work contain custom logic and configurations.
Some of the assets are inspired by tutorials or resources. All of those resources used directly as tutorials or inspiration
are listed in the references of this work under Combat, Artificial Intelligence, Visual FX, Sound FX or Various Topics depending on the subject.

### Platforms
Auto generated engine content;

### Plugins
Third party plugins relevant for the basic functionality of the game; Currently contains only the GenericGraph plugin from the [GenericGraph github](https://github.com/jinyuliao/GenericGraph/tree/master) as stated in the references.

### Source
All files concerning user added functionality using C++; This is also the "C++" folder in the engine
filebrowser. The .cs files are automatically generated by UE5 as a list of the modules used in compilation.
While as all .h and .cpp files contain custom logic and setup. As such all files contained in this folder
can be assumed to be content created by Raffael Scagnetti when not explicitly stated otherwise. 
An exception are the files located unter Source/MAProject/Private/Utility/Tools/pcg-cpp.
These are downloaded form the [pcg-cpp github](https://github.com/imneme/pcg-cpp) as mentioned in the references.
