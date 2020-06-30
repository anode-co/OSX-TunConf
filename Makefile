tunconf: tunconf.c
	gcc -framework CoreFoundation -framework SystemConfiguration -o tunconf ./tunconf.c