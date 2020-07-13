CXX=g++
CXXFLAGS=-O3 -fPIC -Wall
CFLAGS=`root-config --cflags`
LDFLAGS=`root-config --ldflags --glibs` -lRooFit -lRooFitCore -lMinuit

ROODCBIR=../RooFit-pdfs
PDFSDIR=./pdfs

EXEC=lifetime_study
MACRO1=fit_jpsik
MACRO2=plot_bkg_sub
MACRO3=fit_MC_lifetime

all: $(EXEC)

$(EXEC): main.cpp $(MACRO1).cpp $(MACRO2).cpp $(MACRO3).cpp $(PDFSDIR)/src/pdfs.cpp
	@echo "---> Making executable..."
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -I $(PDFSDIR)/include  $^ -o $(EXEC).exe


clean:
	rm -f $(EXEC).exe