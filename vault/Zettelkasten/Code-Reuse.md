202309071928
Status: #idea
Tags:

# Code-Reuse

Seemingly good practices like code-reuse are over-used irresponsibly, with people including huge libraries for the use of a single function. Sometimes it is in search of a high quality library that implements something you are not an expert on.

It can be the community around the library along with the increase transfer-ability of skills across teams. Maybe you just don't want to maintain that part. These are the reasons one might turn to reusing code instead of coding it from scratch.

On the other hand, using other people's code means conforming to the constraints that the people who created the library had. Bigger libraries impose more constraints, and more libraries impose more constraints. If the constraints don't meet yours, you might end-up spending more time on adjusting your/other's code to make it work than if you had wrote it from scratch. And if you have invested too much into a bad library... too bad.

> it's fine if a library isn't completely perfect, none is, it's a time saver. you sacrifice a little bit of personal peace for the sake of getting some shit done or not needing to have the mental overhead of dealing with another person's crap. especially not a person or people who you can't directly contact...the maintainers are simply not my friends, they're not my colleagues, if i need LLVM fixed i cry, if i need any huge library fixed i need to go learn their BS along with the original problem space. if i need a feature added, i also cry. 
> — NeGate, on [handmade-network fishbowl](https://handmade.network/fishbowl/libraries/#chatlog__message-container-1003003441322397797)

Worst are the changing constraints. Usually this happens with package managers when updating dependencies. In general, package managers allow for quick download-and-use type of behavior with regard to dependencies, and easily updating dependencies. We can notice problems when a library author has decided to change the behavior or api of his library. The update will cause many programs dependent on that old version's behavior or api to break. For this reason, explicit upgrades to stable versions of software is preferred, rather than the more convenient automatic option.

We can see this in the [https://handmade.network/](https://handmade.network/)">handmade-community where single-header libraries are popular. e.g. [https://github.com/nothings/stb](https://github.com/nothings/stb)`stb_image` and `stb_freetype`. They require explicit upgrades, the ability to modify source code, and they usually do not have dependencies themselves. Along with the fact that single-header libraries are usually smaller, they dodge to some degree the more destructive nature of large managed dependencies while still bringing great value.

Sometimes you just need more than what people fit into a single-header library or you don't want to change the source code and instead just extend what is already there. This will touch on library design a bit. But it is important to know because if a library is [well designed](https://www.youtube.com/watch?v=5l2wMgm7ZOk/), then it will meet a much wider range of people's needs and won't become a large time-sink down the line if maintenance is needed in the code-base.



The important take a way here is when you buy into a big library, you sell some personal responsibility away which can be good... until things go wrong


---
# References




