202308281526
Status: #idea
Tags: 

# Memory Management Trade-Offs

In your language I assume you have 4 options you could choose to when it comes to memory management. 
1) don't manage it (use malloc/free style allocations which are basic abstractions of mmap/VirtualAlloc) and put it on the user (what C does) 
2) manage it in a GC style handling all allocations and tracking them (Java) 
3) create a temporary allocator convention which has implicit lifetimes backed (Like ODIN or JAI and so on...) 
4) Have a mash of all of the above which aside from option 3 (which you can do yourself but that would lose some of the inherit greatness) which is what c++ does The first case is pretty bad since you don't give your users any help. The second one is nice for the users until it becomes a major pain point, and also is incredibly complex to reason about. The third options allows for the most control, will usually lead to better library communication, and has the benefit of simplicity but expects some knowledge from the user. The last option is most likely the worst one since it is a mess (no convention is produced, everyone does whatever and so you can not have libraries communicate/assume certain things). In terms of implementation, the first option is the easiest. Of course if you don't manage anything yourself and let the OS do the work for you it will be as easy as giving bindings to those function calls and calling it a day. The second option is the hardest, since it will require for you to constantly and actively manage the data, allocate bits, free bits and to make it perform well you will have to use complex strategies like Java uses. The third option is sort of easy to implement. You will be able to implement it quite easily if you know what you are doing, and the implementation won't even be long. But making the design good will take some work. Especially if you want your users to use it cross libraries. The fourth options like I said is a mess and a will require to do basically most of the above so it is also very complex.

Also the choices you will have will not only determine the ease of use of your systems to the users of your language or the complexity of building such memory management systems. The choices will also determine the performance implications you are willing to let your users experiences on the worse side. So for example if you choose garbage collection, you can only do so much for understanding what your algorithm will do before you give up. When you use the malloc/free approach you will inevitably see people will build libraries to make their lives easier since most of the time you don't think of the things you are allocating as truly independent parts where you don't know their lifetimes. The problem here is mainly the somewhat chaotic development of libraries or the absolutely nightmare that is to assume the complete independent lifetimes of all allocation all the time (along with the performance implications). Since introspection into this system is of course the "easiest" because it has a very thin layer above the OS. You don't have the same type of oh no I don't know when the GC will turn on or how to turn it off for this time. Which of course means that programmers can really optimize the performance of their programs if they put a lot of work into it. The third option (temporary allocator) has the best balance (if you assume a certain level of competence). A knowledgeable programmer is able to group lifetimes of the things he is using correctly. Since you create the convention it means people can assume that your temporary allocator will handle the lifetimes they choose. Since you are assuming a certain lifetime for allocations you will have better performance and usability for "free". And since the system is less complex you are able to reason about the assumptions made more easily.

To complete with the fourth option of "just have all the options". Well we can see what it does in c++. Much added complexity, reduced usability, and a bunch of effort for features people who don't know how to use use incorrectly leading to horrible programs.

TL;DR there are a couple of options for how you can do memory management and I try to lay the trad-offs and decisions you are making by taking on some approach. It should help you think maybe more clearly about whether you want to continue in your way or try to see whether different approaches are better for your case 

Also now that I think about it I might be able to think about a fifth option for how you might build your system in a more dynamic way (essentially leaving room for completely replacing parts of the underlying assumptions you are making by allowing the user to extend and modify parts of the basic blocks you use). But this is way too much for now. And I have not implemented anything remotely close to that so this is just an idea.

---
# References


https://www.youtube.com/watch?v=5l2wMgm7ZOk
[[Avoiding Unnecessary Complexity]] 
https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
https://www.gingerbill.org/article/2019/02/01/memory-allocation-strategies-001/


How it might be handled in UI 
https://www.rfleury.com/p/ui-part-3-the-widget-building-language 
https://halt.software/dead-simple-layouts/