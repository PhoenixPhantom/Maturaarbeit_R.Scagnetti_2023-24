# Combat Design in a 3D Open World Videogame - Raffael Scagnetti 2023/24
## The Contents of This Work: Creating a Videogame
To create a “real” videogame – meaning one that people would voluntarily play - many seperate parts are crucial. That includes the construction of the levels where the game is played, creating the assets used therein. Coding responses and logic are only some examples. And most of these crucial snippets of a game require significant effort as well as different skillsets to do right.

But the required effort to go into depth on all these topics, makes it very difficult to be achieved a "Maturarbeit". So as the newly suggested article implies, this work will mainly focus on one part of a video game: _combat design_.

Nevertheless, a videogame cannot simply be made of only one of its crucial parts if any sort of playability should be achieved. Therefore, this work will, while focused on combat design, still include some of the above mentionned aspects. 

This would make it possible to create a playable game with a very basic game loop, while still going into depth in a single aspect. 

To provide the most essential functionalities, this work will make use of the current version of Epic Games’ Unreal Engine 5 (UE5) and incorporate some assets from the Unreal Engine Marketplace which are all listed [here](#UsedAssets).


## Requirements
[Get the game running.](Download.md)

## References
 * Description of the repository's [folder Structure](Folders.md)

<a name="UsedAssets"></a>
### Used Assets
* Starter Content – Epic Games Inc: One of the in-engine project setup options 
* Quadruped Fantasy Creatures - PROTOFACTOR INC: https://www.unrealengine.com/marketplace/en-US/product/7f7775996f7442b187f6fa510ec9d289
* GenericGraph - jinyuliao: https://github.com/jinyuliao/GenericGraph/tree/master (accessed: 5.10.2023)
* PCG random generator - imneme: https://github.com/imneme/pcg-cpp (accessed: 18.10.2023)
* Deep Star Maps - NASA: https://svs.gsfc.nasa.gov/3895 (accessed: 4.12.2023)
* CGI Moon Kit - NASA: https://svs.gsfc.nasa.gov/cgi-bin/details.cgi?aid=4720 (accessed: 4.12.2023)
* Wolf's Howl Gives You Goosebumps (the good kind) - Wolf Conservation Center: https://www.youtube.com/watch?v=JQUHDWHa7WQ (accessed: 16.11.2023)
* Desert3(Royalty Free Music) - PeriTune: https://soundcloud.com/sei_peridot/desert3?in=sei_peridot/sets/peritunematerial (accessed: 15.12.2023)
* Desert6(Royalty Free Music) - PeriTune: https://soundcloud.com/sei_peridot/desert6?in=sei_peridot/sets/peritunematerial (accessed: 15.12.2023)
* Labyrinth(Royalty Free Music) - PeriTune: https://soundcloud.com/sei_peridot/labyrinth?in=sei_peridot/sets/peritunematerial (accessed: 15.12.2023)
* WildField(Royalty Free Music) - PeriTune: https://soundcloud.com/sei_peridot/wildfield?in=sei_peridot/sets/peritunematerial (accessed: 15.12.2023)
* Dunkler Wald - Matteo Scagnetti (created: 2023)
* Leichter Wald - Matteo Scagnetti (created: 2023)
* Wald Wind - Matteo Scagnetti (created: 2023)

<a name="Combat"></a>
### Combat
* Evolving Combat in 'God of War' for a New Perspective - GDC (by Santa Monica Studio's Mihir Sheth): https://www.youtube.com/watch?v=hE5tWF-Ou2k
* Dreamscaper: Killer Combat on an Indie Budget - GDC (by Afterburner Studios' Ian Cofino): https://www.youtube.com/watch?v=3Omb5exWpd4
* Attack representation inspired by: "How to make combo attacks - Beginner tutorial Unreal Engine 5" - LeafBranchGames: https://www.youtube.com/watch?v=rKbb4PBmFUA (accessed: 3.10.2023)

<a name="AI"></a>
### Artificial Intelligence
* Unreal Engine 5 Tutorial - AI Part 9: Prediction Sense - Ryan Laley: https://www.youtube.com/watch?v=-EU8EedeVi4
* All about BTTasks in C++ - The Games Dev: https://www.thegames.dev/?p=70

<a name="FX"></a>
### Effects
#### Visual FX
* Animal Bite FX in UE4 Niagara Tutorial - CGHOW: https://www.youtube.com/watch?v=geHrMbvA5HA with Inspiration from Olivier Deplanque (https://www.artstation.com/artwork/b5XEod)
* Unreal Engine Impact Effect in Niagara Tutorial - CGHOW: https://www.youtube.com/watch?v=8UThnwfEwng
* How to Add Camera Shake in Unreal Engine 5 - GorkaGames: https://www.youtube.com/watch?v=rTVb7nEhKv8 (accessed: 18.10.2023)
* Unreal Engine Tutorial [UE5]: Aura Effect From Scratch - PART 1-3 [NIAGARA] - Rimaye [ Assets and Tutorials - NIAGARA ]: https://www.youtube.com/watch?v=S_pA2uLhZWc&list=PLvYJUGYt8abFzLte-jehECt2LHWGLfM1X&pp=iAQB (accessed: 13.11.2023)
#### Sound FX
* Unreal Engine 5 | MetaSound Footsteps - The Sound FX Guy: https://www.youtube.com/watch?v=SXBEbs69sZ8
* Unreal Engine 5 | Dynamic Footsteps with MetaSounds - The Sound FX Guy: https://www.youtube.com/watch?v=pYxocPdtHBw
* Unreal Engine 5.1 | Creating Soundscapes - The Sound FX Guy: https://www.youtube.com/watch?v=nS3leRwZbNk (accessed: 12.12.2023)

<a name="Varia"></a>
### Various Topics
* UE4: How to fix translucent materials - Tech Art Aid: https://www.youtube.com/watch?v=ieHpTG_P8Q0

