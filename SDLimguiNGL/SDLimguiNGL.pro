# This specifies the exe name
TARGET=SDLNGL
# where to put the .o files
OBJECTS_DIR=obj
# core Qt Libs to use add more here if needed.
QT+=gui opengl core

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
	cache()
	DEFINES +=QT5BUILD
}
# this demo uses SDL so add the paths using the sdl2-config tool
QMAKE_CXXFLAGS+=$$system(sdl2-config  --cflags)
message(output from sdl2-config --cflags added to CXXFLAGS= $$QMAKE_CXXFLAGS)

LIBS+=$$system(sdl2-config  --libs)
message(output from sdl2-config --libs added to LIB=$$LIBS)



# where to put moc auto generated files
MOC_DIR=moc
# on a mac we don't create a .app bundle file ( for ease of multiplatform use)
CONFIG-=app_bundle
# Auto include all .cpp files in the project src directory (can specifiy individually if required)
SOURCES+= $$PWD/src/NGLDraw.cpp    \
          $$PWD/../imgui/src/imgui.cpp \
          $$PWD/../imgui/src/imgui_draw.cpp \
          $$PWD/src/imgui_impl_sdl_gl3.cpp \
          $$PWD/../imgui/src/ColourPicker.cpp \
          $$PWD/src/main.cpp
# same for the .h files
HEADERS+= $$PWD/include/NGLDraw.h \
          $$PWD/../imgui/include/imgui.h \
          $$PWD/../imgui/include/stb_rect_pack.h \
          $$PWD/../imgui/include/stb_truetype.h \
          $$PWD/../imgui/include/imconfig.h \
          $$PWD/../imgui/include/imgui_internal.h \
          $$PWD/../imgui/include/stb_textedit.h \
          $$PWD/include/imgui_impl_sdl_gl3.h

# and add the include dir into the search path for Qt and make
INCLUDEPATH +=./include
INCLUDEPATH +=../imgui/include

# where our exe is going to live (root of project)
DESTDIR=./
# add the glsl shader files
OTHER_FILES+= shaders/*.glsl  \
              README.md \
              imgui.ini
# were are going to default to a console app
CONFIG += console
# note each command you add needs a ; as it will be run as a single line
# first check if we are shadow building or not easiest way is to check out against current
!equals(PWD, $${OUT_PWD}){
	copydata.commands = echo "creating destination dirs" ;
	# now make a dir
	copydata.commands += mkdir -p $$OUT_PWD/shaders ;
	copydata.commands += echo "copying files" ;
	# then copy the files
	copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/ ;
	# now make sure the first target is built before copy
	first.depends = $(first) copydata
	export(first.depends)
	export(copydata.commands)
	# now add it as an extra target
	QMAKE_EXTRA_TARGETS += first copydata
}
NGLPATH=$$(NGLDIR)
isEmpty(NGLPATH){ # note brace must be here
	message("including $HOME/NGL")
	include($(HOME)/NGL/UseNGL.pri)
}
else{ # note brace must be here
	message("Using custom NGL location")
	include($(NGLDIR)/UseNGL.pri)
}
