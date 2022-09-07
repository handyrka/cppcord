VER=1.0a
MAKEFLAGS += --no-print-directory

build:
	@echo ":: Starting build for $(shell uname -omr)"
	@echo ":: Compiling resources..."
	$(shell glib-compile-resources --target src/resources.cc --generate-source gresource.xml)
	@cd src && $(MAKE)
	@echo ":: Output saved to bin/cppcord.exe!"

clean:
	@cd src && $(MAKE) clean
	@echo ":: make clean"	
