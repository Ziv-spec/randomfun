202308250929
Status: #idea
Tags: 

# Complexity

Since the word complexity can be used to mean many terms, I will define some terms to make it easier to distinguish the different kinds of complexity I will talk about. 

**System complexity** - The amount of inter connections and reliance's the systems parts have one on each other. The more inter connections and reliance's the more states the program has to handle correctly. This implies that a simple system is one with independent parts. If you understand how a single part works, you don't have to worry about it any more, your responsibility of knowing how it works is over[^1]. There is also another aspect to system complexity. It is the randomness of the system. If the system is random and large, the amount of unique information alone is enough to categorize something as more complex than the other.

**Easy/Hard** - How much energy it would take to do something. If something takes more energy to do, it is harder to do. The reverse is true.

**Readability** - How much of the system complexity is forwarded to the programmer to comprehend and manage at the average case. If you only have to handle the large parts of the system that don't contain much of the system's complexity, as they were hidden/not presented you wouldn't need to care about that complexity until you would need to change a part of the system. 


Now that we have defined some terms we can begin thinking about the problem. The problem of complexity is one of dependence and states.


Programmers handle the problem of system complexity by refactoring/rewriting code. The problem of readability is handled using abstraction. The problem of a hard/easy program is handled by correct understanding of a problem. 


NOTE(Ziv): Great example to showcase complexity is the braid problem of bread. When you don't braid the doe it looks simple. When the lines interwind because you braid them, you end up with a more complex looking bread.[^2]
Another term which is useful to think about when talking about complexity is "tangle". Since when two things are tangled you have to think not only about them as a sperate entity but also how each and every one of them responds, and their how their interactions make way for a good/bad emergent behavior.




---
# References

[^1]: https://kristoff.it/blog/simple-not-just-easy/
[^2]: https://www.youtube.com/watch?v=LKtk3HCgTa8

https://en.wikipedia.org/wiki/Kolmogorov_complexity