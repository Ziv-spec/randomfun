202309131816
Status: #idea
Tags:

# GZAR Reflections
#### Pain points working on the project
I do see that I am suffering greatly from a poor base layer since 
everything else is being built on top of it. For that reason, it might make sense 
to advance the base layer before continuing on with the rest of the compiler infastructure. 

This might include introducing stb arrays since I have a great need for arrays and C arrays 
are kind of hard to work with (This is of course if I don't at some point switch to CPP).
 Of course threading will be crucial in the future, depending on the stage that I would want 
to introduce threads to the compiler. 

I also need a "platform" layer. I began noticing that certain OS operations have become 
more and more frequently used. These operations should not be platform specific as they 
are currently. For this reason I have concluded the need for a basic platform API. 
This is not without it's challenges. Especially because of my unfamiliarity with Linux API.

#### Changes base on pain
There are also different design that I could have went with in my Lexer & Parser & Codegen that I want to test against other implementations. This would require me to take the time to implement these different ways of doing what I already have achieved. But since it shouldn't take too much time I figure it will be fine to include. 

All these changes basically mean a complete rewrite of the compiler as a whole. It should 
also allow me to get a really solid design since I now understand the problem better.

This rewrite will use many of the currently existing components and pieces of code I have 
already written but should take a new refreshed form. It will include all my insights 
for when creating the current testing language "GZAR".

#### Insights into the development process
The general insight I gained from this project is the effort required to "know" 
the problem well. In this case the problem of compiling with all it's stages and design 
choices. In the future I can be sure to create significantly better designs more eaily and 
yet, when I began working on this compiler I found myself exploring as much as I can arriving 
at "okay" designs. These designs were of-course mostly taken from the internet since I was
not knowledgeable or familiar at all with this type of problem set. As I have familiarized 
myself with it though, the general intuition I gained feel SO MUCH stronger than before. 
  
This makes me affermative that truly knowing the problem and having an intuition for how 
to solve it, is what results in good design. Prototyping certain designs first as a 
exploratory project towards this end goal is at the end what makes good choices happen. 

On the micro level, some certain choices can be made based on patterns generally found in 
all software. These patterns are common and are easily identified by a good level programmer.
 On the macro level, the choices require a level of intuition and knowledge only someone 
already familiar with solving the problem can enlighten.
 
This is the case with Ryan Fleury with his UI series, tinygrad which is a refinement over
the amazing pytorch, stb which is a refinement of years and years of usage code Sean Barrat 
developed, and the final educational project the creator of worrydream.com has created after developing many sites and researching the topic over many years.

Are there exceptions to the rule? I think for the most part no. It is highly unlikely 
that someone not already familiar with a problem can solve it better than someone who 
is already familiar with it. That is not to say that when familiarizing yourself with 
a problem you will not change strategies. Changing strategies like I have outlines for 
my project is a vital sign of recognition for better ways to design and solve my problem. 

#### Translating insights into beginner problems

^604a28

These insights also explain beginners who want to design a "perfect" system 
on their first try are unsure of how that can be achieved and ultimately, fail. 
It is BECAUSE they do not already understand the problem they are trying to solve. 
Had they truly understood the problem implementing it would seem incredibly easy. Since
they would use their intuition around the problem to arrive a better start then otherwise
would be available to them.

This type of insight is very often already known by many long standing "giants" in the field.
Since they encounter both competent and incompetent people, they know what people lack. 
It is not technical skill. It is problem solving ability. They are simply not as used to 
think about and solve the true problems they have before them. This might be due to a myriad 
of factors. But, this fact alone stands true regardless of reason.

#### The hardness of testing
I have yet to find a compelling option for testing the language in a nice and scalable manner. Simple asserts and checks of "correctness" have proven a bad option in terms of scale. For any case you will need a test O(n). But what about the combination of features and their interactions? And the complex transformations that might happen in the IR to optimize code yet still require correctness. 
This proves to me that testing is hard. At least scaling testing is hard. And I might require a further 


---
# References




