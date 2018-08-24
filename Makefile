CXX=g++
CXXFLAGS=-std=c++14 -Wall -g -pedantic

CPPFLAGS=-Isrc \
         -I $(BOOST_INC) \
         -I $(CANVAS_INC) \
         -I $(CETLIB_INC) \
         -I $(FHICLCPP_INC) \
         -I $(GALLERY_INC) \
         -I $(LARCOREOBJ_INC) \
         -I $(LARDATAOBJ_INC) \
         -I $(NUSIMDATA_INC) \
	 -I $(UBOONECODE_INC) \
         -I $(ROOT_INC)

#UBOONECODE_LIB="/uboone/app/users/mastbaum/uboonecode-v06_26_01_12/localProducts_larsoft_v06_26_01_10_e10_prof/uboonecode/v06_26_01_12/slf6.x86_64.e10.prof/lib"

LDFLAGS=$(shell root-config --libs) \
        -L $(CANVAS_LIB) -l canvas_Utilities -l canvas_Persistency_Common -l canvas_Persistency_Provenance \
        -L $(CETLIB_LIB) -l cetlib \
        -L $(GALLERY_LIB) -l gallery \
        -L $(NUSIMDATA_LIB) -l nusimdata_SimulationBase \
        -L $(LARCOREOBJ_LIB) -l larcoreobj_SummaryData \
        -L $(LARDATAOBJ_LIB) -l lardataobj_RecoBase \
        -L $(UBOONECODE_LIB) -l EventWeight 


all: lib/libcov.so bin/make_cov bin/make_reco_tree bin/arborist

lib/libcov.so: src/Covariance.cxx
	@echo Building $@
	@mkdir -p lib
	@$(CXX) -shared -fPIC $(CXXFLAGS) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) 

bin/make_cov: src/make_cov.cpp
	@echo Building $@
	@mkdir -p bin
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) -Llib -lcov

bin/make_reco_tree: src/make_reco_tree.cpp
	@echo Building $@
	@mkdir -p bin
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

bin/arborist: src/arborist.cpp
	@echo Building $@
	@mkdir -p bin
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	@rm -f lib/libcov.so bin/make_cov bin/make_reco_tree bin/arborist

