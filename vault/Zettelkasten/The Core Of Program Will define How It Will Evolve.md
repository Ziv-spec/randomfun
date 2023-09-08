202308282217
Status: #idea
Tags:

# The Core Of Program Will define How It Will Evolve

IMPORTANT(ziv): This ties nicely into a bigger concept of Understanding The Problem


This is more of an observation than anything else. The simple observation that it seems like if you design a good core for a program, the end result of it will look significantly better. 

Let's take tinygrad as an example[^1]. Tinygrad took micrograds api with pytorch's and created a clean and lean high level api that was iterated many times. This formed the core of the library to which, once tinygrad gained more traction, additional contributors make additions. 

The core is very familiar still making the abstractions very nice to use. This allows you to not feel the growing complexity as a end user. The growing complexity is of course due to the need for speed. But again it is important to stress that the good CORE made the additions more manageable. 

You may look at it like a foundation. Where a building with a strong foundation can carry much more weight than one with a weak foundation. So much so that even if you place the weight unevenly, it may still prove better holding that weight. 




NOTE(ziv): IS THIS TRUE?
#### Key observation on degeneracy[^2]

**degeneracy** occurs when structurally dissimilar components/pathways can perform similar functions (i.e. are effectively interchangeable) under certain conditions, but perform distinct functions in other conditions


We want degeneracy in our programs. Essentially this could be done by having a 3 level system like in tinygrad/rfleury UI lib where you have low level ops -> medium level -> high level (sugar syntax). Or a more general design of ESC where you have components that you can mix and match to create entities (no need for sugar syntax). 

Since a core is going to be relatively small in terms of complexity and stable (no fast growth in loc), the core provides the core resilience and stability of the system. It also provides ways to fundamentally not have unwanted redundancy (duplication of code) which would lead usually to bad abstractions.

**Why is this the case?** 

Even if you don't know what types of features or needs we will have in the future, we can build systems that will gradually be able to adapt to accommodate them. You can think of it as a lifetime of changes that the program goes through. Usually programs are worked on until "done" there is a complex type of growth that happens. This is a dynamic growth that happens as a project grows and requirements change. 

Building a system where the you couple some of the properties into the components you use will make adding properties easy but changing them altogether hard. If you have the ability to change the structure of your program (on a high level) while keeping the same meaning with ease (rapidly adapting) without the requirement of a rewrite/refactor which is so commonly associated with it, you have created a system with the property of degeneracy. 

There is another benefit to such systems. Traceability of bugs, and performance issues become easier to reason about when you think about a certain feature your entities have instead of about how each entity implements the feature (ESC vs OOP approaches). Since you know what property is the problem usually easily, the traceability of it (since it is separate) is significantly easier than the opposite.  

It is also interesting that to a complex system, anytime you want to trace a bug (essentially trace a unwanted behavior) you will notice that you can't do that easily[^3]. One change might break some other part of your code that relies on that. Since people don't like to do these types of changes regardless (even when it is necessary) these complex systems can "collapse" in terms of positive growth and change. An good example is Adobe programs like After Effects and Premere Pro. And because such systems have not allowed for future adaptation, we can see that the ability of people to reason for why a certain bug is caused is decreased over time. 


One example is UI and how you have a renderer system to render pixels, but on top of it you have your UI system which uses the renderer. The rendering system is robust and will not change much when compare to the things you might want to render (aka a combination of the rendering calls). 
https://macroresilience.substack.com/p/redundancy-degeneracy-and-resilience
You can break this even more like Ryan Fleury did... by understanding the fundamental problem better.


---
# References

[^1]: https://www.github.com/tinygrad/tinygrad
[^2]: https://macroresilience.substack.com/p/redundancy-degeneracy-and-resilience
[^3]: https://macroresilience.substack.com/p/complex-systems-need-combination
