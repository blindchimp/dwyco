* analyze this simple interpreter (named LH).

* we are going to change the language to allow caching variable references, essentially removing the dynamic binding properties of the language. this is for performance reasons mostly. 

* the most common form of variable binding in the syntax tree is <x>. we'll change the language so that x must be bound (using lbind or gbind) before it is referenced. when it is referenced, that location in the syntax tree caches the looked up value, and it remains bound to that slot for the remainder of program run. future lbind(x foo) calls refer to that slot as well. references to x are specific to the function it lives in, not the function context during a run (recursion is a special case, described below.) so <x> in function A is different than <x> in functions B if both functions issue an lbind for x.

* to build the interpreter (the results are in ~/git/build-lh/vccmd): (cd ~/git/dwyco;sh vccmd/build.sh)
* to run tests (returns 0 if ok): (sh ~/test-exc.sh)
