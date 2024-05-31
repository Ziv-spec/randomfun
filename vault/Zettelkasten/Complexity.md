202308250929
Status: #idea
Tags: 

# Complexity

A systems complexity might be characterized in a number of ways. One would be 
the Kolmogorov complexity. The definition is the minimum program which required 
to represent a string. From this we can inquire that a more complex system is
one who's parts are all unique. For if they are not unique they can be 
compressed and represented more compactly. This is the main argument for the 
"essential complexity" of software as Brooks defined is No Silver Bullet.
While it is correct I feel like some aspect is forgotten. Some software while 
seemingly significantly smaller are more complex than their large counterparts.
Why might this be the case? I believe that it is because of how inter-wind the
parts that make up the system are.

Think of the economic market we live in. Each person is making seemingly simple
decisions regarding his daily life buys and sells. And yet the markets is so 
complex there is almost no hope for ever being able to accurately predict it's 
chaotic behaviour. The interactions themselves might be simple but the sheer 
amount of even simple decisions and tendencies make the economic of a country 
seem like it's own entity and require very high level theories of how it works.
It is just impossible for us to understand the economy on a high level without
approximating some of it's behaviour. This approximation is what abstractions 
are all about in software and in theories in general. 

There are popular approaches for thinking about complex systems. The holistic 
approach says that you must look at the whole in-order to understand how it 
works. The reductionistic approach says that you can break the system into 
parts until the system is understandable. These approaches try to approximate 
the general system by ignoring the details. This seems to be a natural human 
behaviour. We are not capable of reasoning about all of the details while 
looking at the bit picture, for this reason we just ignore them. Sometimes the 
details leak through our abstracted view of the system which break our 
understanding. 


Biological systems seem to contain endless amounts of complexity inside their 
inner workings and yet they are capable of changing and adapting better than 
any system we humans can design.

## Software complexity

Today software can be characterized by it's never ending complexity.
Software seems to know no bounds when it comes to features. Features are what 
sell software and they are to be included at any price. Some of this price is 
the endless growth of complexity of software. For some this complexity is 
manifested through bloat. Applications that take gigabytes of storage space to 
install, and hog available resources. While for some applications much of its 
size and resource utilization is high due to its nature, for most I would say, 
this is not the case. Complexity also harms the cost of maintenance. Adobe has 
made maintaining and adding features it's software a technical nightmare. Old 
bugs and crashes haunt Adobe Premiere to this day[^1] with no saving grace in 
sight. 






Since the word complexity can be used to mean many terms, I will define some terms to make it easier to distinguish the different kinds of complexity I will talk about. 

**System complexity** - The amount of inter connections and reliance's the systems parts have one on each other. The more inter connections and reliance's the more states the program has to handle correctly. This implies that a simple system is one with independent parts. If you understand how a single part works, you don't have to worry about it any more, your responsibility of knowing how it works is over[^1]. There is also another aspect to system complexity. It is the randomness of the system. If the system is random and large, the amount of unique information alone is enough to categorize something as more complex than the other.

**Easy/Hard** - How much energy it would take to do something. If something takes more energy to do, it is harder to do. The reverse is true.

**Readability** - How much of the system complexity is forwarded to the programmer to comprehend and manage at the average case. If you only have to handle the large parts of the system that don't contain much of the system's complexity, as they were hidden/not presented you wouldn't need to care about that complexity until you would need to change a part of the system. 


Now that we have defined some terms we can begin thinking about the problem. The problem of complexity is one of dependence and states.


Programmers handle the problem of system complexity by refactoring/rewriting code. The problem of readability is handled using abstraction. The problem of a hard/easy program is handled by correct understanding of a problem. 


NOTE(Ziv): Great example to showcase complexity is the braid problem of bread. When you don't braid the doe it looks simple. When the lines interwind because you braid them, you end up with a more complex looking bread.[^2]
Another term which is useful to think about when talking about complexity is "tangle". Since when two things are tangled you have to think not only about them as a sperate entity but also how each and every one of them responds, and their how their interactions make way for a good/bad emergent behavior.


How do we handle the complexity of the programs that we create? 
    - Refactoring
    - Rewriting
    - Abstraction 

Refactoring is essentially taking the code and modifying it to solve the problem we face better. 
Rewriting the code is completely abandoning the effort made since we recognize that refactoring the existing code will take more effort than rewriting it with the new insights we gained. 
Abstraction is a tool for handling complexity where we hide some of the details of how something works, to not constantly have to worry about these details for when we want to use a basic block of our program. 

Each and every of these ways we handle complexity is of-course used in different times and for different reasons or usecases. 


---
# References

[^1]: adobe link to people talknig about how much it suck but it also can't change

[^1]: https://kristoff.it/blog/simple-not-just-easy/
[^2]: https://www.youtube.com/watch?v=LKtk3HCgTa8

https://en.wikipedia.org/wiki/Kolmogorov_complexity
