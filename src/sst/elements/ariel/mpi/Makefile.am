all-local: mpilauncher

mpilauncher: mpilauncher.cc
	 $(ARIEL_MPICXX) $(AM_CFLAGS) mpilauncher.cc -o mpilauncher
fakepin: fakepin.cc
	 $(ARIEL_MPICXX) fakepin.cc -o fakepin

bin_SCRIPTS = mpilauncher

clean-local:
	rm -f mpilauncher fakepin
