*Programmers provide value by making the right trade-offs*

Introduction: 

https://www.rfleury.com/p/the-marketplace-of-ideals

The insights PLAN:

1. Finding a mentor and it's value, or surrounding yourself with people better than you. How it changed the way I think and shaped this post itself
2. Decline in culture
What is the decline about? + examples
how this is a general process we find civilization naturally gravitates towards
    How did it come to be? (complexity from abstraction and "requirements" from industry +…)
Hard to notice to a beginner how doesn't have the context
    - ownership problems over time
    Why it is important?
3. Countering the decline
    - Why understanding deeply is a important rudiment you must follow, to counter it
    Learning about your platform aka the computer
    Using debugger to understand programs
    learning the known and available methods old people came up with and succeeded along with failed to not repeat the mistakes
    appreciating the things you have now by using "old" technologies
    - Language don't matter, until they do...
    Languages don't matter
    every experienced programmers knows more than one language and it become easy once you know the basics
    Languages are just tools to make your job easier, just like you won't buy a new car just because it has some minor improvement over other cars, expect to do the same with languages
    Languages matter
    not every language allows the same amount of control, to truly understand things deeply you need to also use a language that does not hide things away from you
    in the end languages are tools, if you don't understand how the tool works, what it can and can not do you will not use it effectively
    - Some word about abstractions
    Abstractions as a way of managing complexity
    Why they are bad
    Why they are good
    And what does it mean?
4. it all comes down to value
This is our job - to bring value to the people
How the right trade offs make value
don't take my word for it - jon carmak about value of programming
5. How it all comes down to trade-offs
    - Look into previous chapters for how some trade offs might get made + How all we learned comes down to help with making the right trade-offs
    Language - higher to lower abstraction
    Abstraction as method to manage complexity
    Code reuse - as an example of how trade offs are made
    Why do people reuse code?
    Some common options for code reuse
    trade-offs in code reuse
    - What makes a experienced programmer valuable? spoiler: makes the right trade-offs

i have been having thoughts on how at many of the things at school seem to not match my current reality. The things I learned from the internet seem to seek different kind of knowledge than I was getting. I was getting simple knowledge with a great lack of fundementals of computing and computers. And a sense of my doing things by learnding them by heart (rote) instead of understanding how the things I wam working with work. Classess are a classic example of 

Since then, every time I display fundamental knowledge I learned people seem perplexed that they don’t know this seemingly fundamental knowledge. They acknowledge the fact that it is important, that it help them understand the things they work with better yet, were never taught that.

This made me wonder, why is this so? Why do people who learn at school and some self learn from the internet, seem to lack so much. I now think that it is in part inevitable. At least with the current systems and institutions set in place. 

People usually subscribe to what others in their environment believe in. Just like children who believe what their parents believe. Knowledge is made from what we read, learn and interact with on a daily basis. We assume people we can trust will be correct.

On the one hand this means that if the networks of trust we built up are aligned with truth we can easily acquire new knowledge. On the other hand if there is any mistake, lie, uncertainty, we may be fed fallacies or part truths without knowing so. 

# Decline In Culture - 5/10

In the couple years I learned programming, there was a big shift in my views. A change in what I constitute as good programming. More specifically I found the Handmade Community. 

## Source Of Truth - 4/10

Up until then my main source of information was school. Knowledge I gained was restricted to the wants and needs of my education system. Since my teachers were an [authority I had trust in](https://www.jsanilac.com/trust/), I accepted their teachings like a sponge. This made my view of programming limited and the experience was dreadful.

In hindsight I should have thought critically before accepting their imparted knowledge, looking in other directions searching the web. 

> Some of my friends agree the programming profession hasn’t matured yet because it’s at a pre-scientific stage of its existence. I claim many of us are subconsciously subscribing to traditional sources of knowledge that have proven to be unreliable, and as far as I can tell NASA has done well to guard themselves against them
> 
>    — [Abner Coimbre](https://observer.com/2017/07/a-look-into-nasa-coding-philosophy-kennedy-space-center-programming/)
> 

NASA is a institution known for knowledge and competence. How does it maintain knowledge for designing and testing the extremely complex and dangerous systems they make? 

The answer is *mentorship* and seeking *deep understanding*. If you have someone you know has deep insights into the field you want to study, him being your mentor will result in your competence within the field (assuming you want to learn). If you are in a place where everyone has a mentor that teaches you fundamental knowledge to absorb you will find that you have better tools and information of how to deal with your daily trade-offs you have to consider leading to better produced software.

My first mentor was not someone I was close to or even knew. It was by accident that I discovered Handmade Hero. It was fascinating for me how everything clicked, as I rediscovered my love for programming I was improving the things I was creating at a rate I was not able before. The resulting shift came in the form of 2 things I had to understand and apply.

1. I lack understanding so I have to acquire it
2. How can I provide more value by making the right trade-offs

 I will talk about each of these in separate, then show how they are all connected. 

Usually people subscribe to different ideas and cultures when they are surrounded by them. When a culture advocates for the production of high-quality good products, people will produce high-quality products, of course the opposite is true.

Today as far as I can see, the software culture seems to advocate for fast development time at the expense of the user experience. You don’t have to do much to see the degree to which we normalized bad software. [Bugs, crashes](https://danluu.com/everything-is-broken/), slow load times, or just the [ridiculous effort put into ads](https://how-i-experience-web-today.com/) are common place. 

Linux is a many million lines of code monolith, adobe after effects [has grown so complex, speed and stability seems to be a dream](https://www.youtube.com/watch?v=aVNCgmAsI8c). Creating a simple website now “requires” frameworks with many libraries, and languages are [slow to compile](https://fasterthanli.me/articles/why-is-my-rust-build-so-slow).

The problem is the systems we handle today are too complex. We are having a hard time managing their complexity, which leads to a worse end product. But it doesn't have to be this way. It most definitely wasn't the case just a couple decades ago. 

> About 25 years ago, an interactive text editor could be designed with as little as 8,000 bytes of storage. (Modern program editors request 100 times that much!) An operating system had to manage with 8,000 bytes, and a compiler had to fit into 32 Kbytes, whereas their modern descendants require megabytes. Has all this inflated software become any faster? On the contrary. Were it not for a thousand times faster hardware, modern software would be utterly unusable.
   [— Nicklaus Wirth, A plea for lean software](https://cr.yp.to/bib/1995/wirth.pdf)
> 

## Decline is natural for institutions

History has taught us that every civilization, no matter how great, eventually declines. The cost of maintaining their power increases immensely over time. While the short-term effects may not seem significant, the long-term effects are detrimental and leads to a costly yet necessary relapse.

How do we know that the times we are experiencing are that of decline? Since education is a key factor that influences the rest of our equation, let us see how it holds up. 

Ray dalio in his book Changing World Order had education as a prime factor he tracks in all countries. This is because the powerful determinants which change large states fundamentally come from the peoples education, which leads to many positive or negative determenants based on this single metric. 

> Avoiding collapse is so difficult because succession failure is often opaque. If the Institute of Pottery lost the ability to make good pots—to mold people into skilled pot makers—would they declare it to the world? Of course not—institutions are very rarely self-abolishing
  [— Samo Burja, Why Civilizations Collapse](https://thesideview.co/journal/why-civilizations-collapse/)

Knowledge a school college or university teaches, no longer guarantees your competence in the field. We can see this is the case since more and more places are hiring people without degrees. Because the market still needs competent people, a better screening process is preferred over the usual qualifications one.

This is the case at tesla, and more places…

We can explore the reasoning behind this phenomenon with a simple exercise. Imagine dear reader, you are a politician. To win the elections you say that you will will increase taxes on businesses to finance better healthcare for disabled children. The people find your proposition attractive and vote for you. You keep your promise and raise the taxes no businesses. Since taxes were high for businesses, 4 years later this cased the economy to slow down. 

After these 4 years there are again elections. Now, you decide to lower taxes on businesses at the at the expense of disabled children’s healthcare since it is needed. You are not re-elected. Instead, someone who says he will keep the taxes high replaces you. Just like you expected after 4 years the economy slowed down hurting everyone, including disabled children’s healthcare.

Since the politician had to save face, he raised taxes again on businesses to make disabled children’s healthcare rise to the same level before. Putting a plaster instead of treating the disease. This again made the economy slow down hurting disabled children’s healthcare. 

As you can see, in this case people prefer when things are given to them instead of taken away. System are usually to rigid to accommodate hard changes so important yet hard decisions are not made. Because once these decisions were good, it is assumed that they will continue to be good. But old decisions pile up and once enough rust accumulates everything falls down.

## Decline is natural for programming

> Human history as a whole has been characterized by a seemingly inexorable trend toward higher levels of complexity, specialization, and sociopolitical control, processing of greater quantities of energy and information, formation of ever larger settlements, and development of more complex and capable technologies.
> 
> 
>   [—JOSEPH A. TAINTER, The Collapse of Complex Societies](http://risk.princeton.edu/img/Historical_Collapse_Resources/Tainter_The_Collapse_of_Complex_Societies_ch_1_2_5_6.pdf)
> 

As I see it, current trends in the software world are derived from larger processes. If you know how it turns out, you will see how rudimentary it is to believe that our challenge will always be the handling of complexity.

With the rapid rise in power of the United States, it’s chip and software industries developed to become a standard others had to match. With superior education, innovation and economic interests were on the rise. With more economic interest and funding, innovation continued, leading to greater funding and the feverish cycle continued. This cycle caused two major processes to come to life. 

1. Creation of faster chips
2. Rapid increase of complexity

Improved chip design and process nodes have led to faster chips. This, in turn, has enabled software to utilize the excess processing power to provide users with more features. However, this has also caused the complexity of problems faced by programmers to increase significantly.

> It was slowly recognized that programming was a difficult task, and that mastering complex problems was non-trivial, even when – or because – computers were so powerful.
  [— Nicklaus Wirth, A Brief History of Software Engineering](https://people.inf.ethz.ch/wirth/Miscellaneous/IEEE-Annals.pdf)
> 

At some point chip designers saw that they are reaching limits with their single core design leading multi-core machines to eventually become a way for further improvement of processor speed. Yet the difficulty of taming this beast was on a level never seen before. 

> The new, hot topics were multiprocessing and concurrent programming. The difficulties brought big companies to the brink of collapse. In 1968 a conference sponsored by NATO was dedicated to the topic (1968 at Garmisch-Partenkirchen, Germany). Although critical comments had occasionally been voiced earlier, it was not before that conference that the difficulties were openly discussed and confessed with unusual frankness, and the terms *software engineering* and *software crisis* were coined.
  [— Nicklaus Wirth, A Brief History of Software Engineering](https://people.inf.ethz.ch/wirth/Miscellaneous/IEEE-Annals.pdf)
> 

So what might hold us back? 

## Layman ignorance and herd mentality - 1/10

The result of history is what we are currently seeing. 

Someone who’s not involved with the production of software might not consider all this to be a problem. A beginner to the field can't tell how fast something should go. He can only tell when it is *slow.* 

Imagine dear reader, you are a child trying to use a computer. In this instance you want to play a game. You turn on your computer and begin waiting for the computer to boot. A minute or so passed and you can now enter your password. Now, after entering you password you click on the game you want to play. You wait a couple seconds, then a couple more and you are in. You can now play as you like. 

This was my experience no so long ago. Since then I replaced my old computer to a faster one. Clearly the speed of my machine had bothered me. Yet my only option was buying a new machine. 

Games are to a degree unique in that the speed of the game is extremely important to its users. No player would want their game to freeze of lag behind. So game are usually made to be fast with the resources they have. 

But was computer slow? No, by no means it was slow. Instead the software it was running was slow.

Some might say that gamers are willing to splurge on beefy machines just to run their games faster. So are game just slow then? No, not everything is slow.

In the above case you, the child, couldn’t see how fast what you are using really is, since the software it runs is slow. Now how is this applicable to our case? Well, you might be surprised to find that your computer the one you are reading this from is a super-fast computer, much more powerful than the super computers of the past. This is a case to such a extreme degree that many will find it surprising when a [old computer running old software feels significantly faster than today's computers running modern software](https://jmmv.dev/2023/06/fast-machines-slow-machines.html). With how much we have improved, wouldn’t it make sense to not have certain classes of bugs? I certainly think so. Yet they persist. 

Now we see that unless one knows what something is capable of doing, he will never know it is achievable. He is a fog in a well. The lack of understanding of what is possible is what limits people what they are able to create. Essentially this would be me a couple years ago lacking in fundamental understanding of computers. My teachers have not shown me this since they themselves were ignorant and lacking. But all of this made me wonder *why is this the case?*


## Complexity - 1/10

*Complexity is the devil we should eradicate, simplicity is the angel to bring forth salvation.*

As we have seen, the improvement of hardware users have available to them does not translate to a faster better experience. With ever faster CPU’s, more RAM, and faster GPU’s programmers utilize the resources with a different mindset then before. As we have more resources we can use, the concern of efficiency and size has dropped, and in it's stead, we care about *programmers efficiency* and more *features.*

It is easy to see why this happened if you turn to history (insert thinking from wirth here)

From fast hand crafted assembly routines to slow bloated software. Throughout the years companies have created new software that chews the new resources the hardware companies produce. A common phrase that describes this phenomena you might hear is [“What Intel giveth, Microsoft taketh away”](http://exo-blog.blogspot.com/2007/09/what-intel-giveth-microsoft-taketh-away.html). 

Programs are becoming more and more complex since we want more out of the things we use. Any program that does anything must have some complexity associated with it. This complexity will inevitably result in hidden costs to be discovered later usually called technical debt. 

Usually programmers deal with complexity with a tool called abstraction. Most programmers when referring to abstraction mean black box abstraction.

 Black box abstraction tries it's best to hide complexity by giving the users a clean API to hide the implementation behind a “black box”. 

Because abstraction on its own is ambiguous. From this point on, I will use the word abstraction to mean black box abstraction.

Basically hiding things behind a black box that you hope works. If the user of the black box uses it for the exact case it was made for, it works well. When the user uses the black box for a use case it was not designed specifically for, all heaven breaks loose. 

abstraction might mean meta abstraction. This is a case where you are providing a black box abstraction interface with meta interface that can change some of the decisions made by the regular black box abstraction interface. 

This allows the programmer to both hide complexity while being able to deal with mapping problems using the meta interface. 


***TODO(ziv): CONTINUE THIS!!!!!!!***

These might be some of the issues. But we have to remember [we haven’t had computers for that long](https://people.inf.ethz.ch/wirth/Miscellaneous/ComputersAndComputing.pdf). We research many ideas to finally separate the [true kernels of nourishment from the chaff](https://people.inf.ethz.ch/wirth/Articles/GoodIdeas.pdf). Unless a good culture is maintained with time I fear this knowledge might turn into [intellectual dark matter](https://medium.com/the-long-now-foundation/intellectual-dark-matter-2e5890aa8d8f).

As software engineers we need to make the right trade-offs which requires deep knowledge of tools we use, and ideas we believe. Which can also come from insights into old technology.

h[ttps://floooh.github.io/2018/06/02/one-year-of-c.html](https://floooh.github.io/2018/06/02/one-year-of-c.html)

You need to find the best balance between every axis of simplicity you can find as you work on projects. But remember to also let go when needed. Don't obsess over a irrelevant axis forgetting about the rest, look for the best trade-offs you can find. The better trade-offs you make the better results you will see.

## solving problems that don’t exist

Some people try to generalize their software from the get go. This usually results in poor brittle software with many edge cases that are hard to check and so on. Writing the simple thing that solves your problem and then generalizing if needed is the correct way. 

NOTE(ziv): This is a extention to the abstraction thingy

## Why should I care? - 2/10

An important question to ask now is "but why do we care?" computers are getting faster every year, we have so many programmers to put this burden of understanding complex systems. Why is this a problem?
Well... computers are not getting that much faster!! Today the trend for general purpose chips is the increase the number of cores along with some slight improvements in IPC - Instructions Per Clock.  The slowdown might be caused because of lack of <a href="https://www.theregister.com/2022/07/18/electrical_engineers_extinction/">engineers</a> along with technical difficulties we experience. <br /> The traditional way chips have gotten faster is by the exploitation of <a href="https://youtu.be/Nb2tebYAaOA?t=220">parallelism within code programmers write</a> better process nodes and higher clock speeds. Whether it is by predicting branches, executing multiple instructions at different locations on the hardware, running other code while the cpu waits for memory using hyperthreading or clock speed and node process improvements, today the performance gains from these techniques reached a plateau where any improvement requires more hard work than ever before and give diminishing returns.
While hardware improvements can be made with time, software improvements can be made easily with each new software created. Because I advocate for a culture where quality should be regarded as a high priority I am bringing this topic up. Part of the value we can bring to users is the speed and bug free user experience, so I spend a considerable amount of time talking about it right now. As my advice is partially connected to the future I want to see.

# Countering The Decline

## Code Reuse - 6/10

Seemingly good practices like code-reuse are over-used irresponsibly, with people including huge libraries for the use of a single function. Sometimes it is in search of a high quality library that implements something you are not an expert on. 

It can be the community around the library along with the increase transfer-ability of skills across teams. Maybe you just don't want to maintain that part. These are the reasons one might turn to reusing code instead of coding it from scratch.

On the other hand, using other people's code means conforming to the constraints that the people who created the library had. Bigger libraries impose more constraints, and more libraries impose more constraints. If the constraints don't meet yours, you might end-up spending more time on adjusting your/other's code to make it work than if you had wrote it from scratch. And if you have invested too much into a bad library... too bad.

> it's fine if a library isn't completely perfect, none is, it's a time saver. you sacrifice a little bit of personal peace for the sake of getting some shit done or not needing to have the mental overhead of dealing with another person's crap.  especially not a person or people who you can't directly contact...the maintainers are simply not my friends, they're not my colleagues, if i need LLVM fixed i cry, if i need any huge library fixed i need to go learn their BS along with the original problem space. if i need a feature added, i also cry. 
— NeGate, on [handmade-network fishbowl](https://handmade.network/fishbowl/libraries/#chatlog__message-container-1003003441322397797)
> 

Worst are the changing constraints. Usually this happens with package managers when updating dependencies. In general, package managers allow for quick download-and-use type of behavior with regard to dependencies, and easily updating dependencies. We can notice problems when a library author has decided to change the behavior or api of his library. The update will cause many programs dependent on that old version's behavior or api to break. For this reason, explicit upgrades to stable versions of software is preferred, rather than the more convenient automatic option.

We can see this in the <a href="https://handmade.network/">handmade-community</a> where single-header libraries are popular. e.g. <a href="https://github.com/nothings/stb">`stb_image` and `stb_freetype`.</a> They require explicit upgrades, the ability to modify source code, and they usually do not have dependencies themselves. Along with the fact that single-header libraries are usually smaller, they dodge to some degree the more destructive nature of large managed dependencies while still bringing great value.

Sometimes you just need more than what people fit into a single-header library or you don't want to change the source code and instead just extend what is already there.  This will touch on library design a bit. But it is important to know because if a library is <a href="https://www.youtube.com/watch?v=5l2wMgm7ZOk/">well designed</a>, then it will meet a much wider range of people's needs and won't become a large time-sink down the line if maintenance is needed in the code-base.

The important take a way here is when you buy into a big library, you sell some personal responsibility away which can be good... until things go wrong
