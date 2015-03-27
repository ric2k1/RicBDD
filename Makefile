CSRCS     = $(wildcard *.cpp)
CHDRS     = $(wildcard *.h)
COBJS     = $(addsuffix .o, $(basename $(CSRCS)))

CXX       = g++
CFLAGS    = -g -Wall
CFLAGS    = -O3 -Wall
EXTINCDIR = 
LIBDIR    = 
ECHO      = /bin/echo

EXEC      = testBdd

.PHONY: depend

$(EXEC): $(COBJS)
	@$(ECHO) "> building: $@"
	@$(CXX) -o $@ $(CFLAGS) $(COBJS)

%.o : %.cpp
	@$(ECHO) "> compiling: $<"
	@$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(COBJS) $(EXEC)

depend: .depend.mak
.depend.mak: $(CSRCS) $(CHDRS)
	@$(ECHO) Making dependencies ...
	@$(CXX) -MM $(DEPENDDIR) $(CSRCS) > $@

include .depend.mak

