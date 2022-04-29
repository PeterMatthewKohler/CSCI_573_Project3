CXX ?= g++
CFLAGS = -std=c++11
SHVER = 2

all: RAD HOD HJPD SVMF useSVM

lib: svm.o
	if [ "$(OS)" = "Darwin" ]; then \
		SHARED_LIB_FLAG="-dynamiclib -Wl,-install_name,libsvm.so.$(SHVER)"; \
	else \
		SHARED_LIB_FLAG="-shared -Wl,-soname,libsvm.so.$(SHVER)"; \
	fi; \
	$(CXX) $${SHARED_LIB_FLAG} svm.o -o libsvm.so.$(SHVER)

RAD: rad.cpp
	$(CXX) $(CFLAGS) rad.cpp -o RAD

HJPD: hjpd.cpp
	$(CXX) $(CFLAGS) hjpd.cpp -o HJPD

HOD: hod.cpp
	$(CXX) $(CFLAGS) hod.cpp -o HOD

SVMF: svmFormat.cpp
	$(CXX) $(CFLAGS) svmFormat.cpp -o SVMF

useSVM: useSVM.cpp svm.o
	$(CXX) $(CFLAGS) useSVM.cpp svm.o -o useSVM

svm.o: svm.cpp svm.h
	$(CXX) $(CFLAGS) -c svm.cpp

clean:
	rm RAD HJPD HOD SVMF useSVM

clean_data:
	rm rad_d1 rad_d1.t hjpd_d1 hjpd_d1.t hod_d1 hod_d1.t rad_d2 rad_d2.t hjpd_d2 hjpd_d2.t hod_d2 hod_d2.t

clean_all:
	rm RAD HJPD HOD SVMF useSVM rad_d1 rad_d1.t hjpd_d1 hjpd_d1.t hod_d1 hod_d1.t rad_d2 rad_d2.t hjpd_d2 hjpd_d2.t hod_d2 hod_d2.t

