* analyze this simple interpreter that has a built-in transpiler (it is only partially complete.) currently, the exception handling for the interpreter usually has the form 'expr.eval(); CHECK_ANY_BO;' which basically is doing the exception handling manually (ie, no use of c++ facilities like try/catch). the transpiler throws exceptions in the compiled code to provide the same functionality. however, there is a wrinkle: if transpiled code runs plain interpreter code, there has to be a translation between the two styles at some point. the goal is to change the interpreter to use try/catch so it interoperates right with transpiled code.

* to do this refactor, we are going to remove the following functionality from the interpreter: exchandle, exchandle2, exc-set-handler-ret, mainly because it is too complicated. there will not longer be a way to resume at the raise call. the vcexcctx.* files should be unnecessary after the refactor.

* we are going to keep the functionality provided by dotry()/doexcraise() with string matching for finding the exception context during backouts. however, the string matching should be simple. there is no longer any significance of the first two character as in the past.

* we also need to maintain the functionality provided by doexcbackout(). backout expressions are stored in the function context, and evaluated  in reverse order as you would expect during the exception unwind operation. the evaluation should be done in the context in which the backout expression was set.

* excdhandle should be implemented as a handler that can be evaluated if no other handler matches after an excraise call. after the default handler is run, the program exits.

* be sure to keep dbg_backouts working. it is useful during production for tracking down bugs. NOTE: dbg_backout is a misnomer and is not related to "backout expressions" implemented by doexcbackout().  dbg_backout might be better named "dbg_backtrace"

* leave CHECK_ANY_BO and similar macro invokations in the source code for now. just make the macros empty  as part of the refactor. this might help with debugging if there is a problem with the refactor.

* leave all comments, unless they refer directly to old exception code. they contain items that might need to be addressed in the future.

* assume LHOBJ, PERFHACKS, and FUNCACHE are defined

* corner cases: if an exception is raised while executing a backout expression, the program should terminate with a message. if an exception is thrown while executing a catch expression, the current exception processing is terminated and the exception is thrown to the next enclosing try block.

* to build the interpreter (the results are in ~/git/build-lh/vccmd): (cd ~/git/dwyco;sh vccmd/build.sh)
* to run tests (returns 0 if ok): (sh ~/test-exc.sh)
