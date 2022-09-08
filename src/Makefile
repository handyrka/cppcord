CXX=c++
CXX_FLAGS=-Wall -std=c++17 -g
CXX_LIBS=
CXX_FILES=main.cc resources.cc helper.cc
CXX_TARGET=../bin/cppcord.exe

# Source files
CXX_FILES += gui/window.cc
CXX_FILES += core/cache.cc core/config.cc core/http.cc core/notification.cc core/websocket.cc
CXX_OFILES=$(patsubst %.cc, %.o, $(CXX_FILES))

# Add libs
CXX_FLAGS += $(shell pkg-config --cflags gtkmm-3.0)
CXX_FLAGS += $(shell pkg-config --cflags gio-2.0)
CXX_FLAGS += $(shell pkg-config --cflags libcurl)
CXX_FLAGS += $(shell pkg-config --cflags jsoncpp)

CXX_LIBS += $(shell pkg-config --libs gtkmm-3.0)
CXX_LIBS += $(shell pkg-config --libs gio-2.0)
CXX_LIBS += $(shell pkg-config --libs libcurl)
CXX_LIBS += $(shell pkg-config --libs jsoncpp)
CXX_LIBS += -lixwebsocket -lcrypto -lssl -lz -lpthread

ifeq ($(OS),Windows_NT)
CXX_LIBS += -lws2_32 -lcrypt32 -lshlwapi
endif

.PHONY:all

all:$(CXX_OFILES) $(CXX_TARGET)

clean:
	@rm -f $(CXX_OFILES)

%.o: %.cc
	@echo " >"$<
	@$(CXX) -c $< -o $@ $(CXX_FLAGS)
	
$(CXX_TARGET): $(CXX_OFILES)
	@echo ":: Linking..."
	@$(CXX) $(CXX_OFILES) -o $@ $(CXX_LIBS)
	
