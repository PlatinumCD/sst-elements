
CXX=$(ARCH)-linux-musl-g++

CXXFLAGS=-O3
LDFLAGS=-static

PROG=affinity-change

$(PROG) : $(PROG).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(ARCH)/$@ $<

clean:
	rm -r $(ARCH)/$(PROG)
