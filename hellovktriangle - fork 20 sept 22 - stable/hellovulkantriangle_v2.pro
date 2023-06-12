QT += widgets concurrent

HEADERS += \
    #vulkanwindow.h \
    mainwindow.h \
    camera.h \
#    ../shared/trianglerenderer.h \
#    ../shared/tiny_obj_loader.h \
    trianglerenderer.h \
    tiny_obj_loader.h \
    lecture_csv_qt.h \
    struct_vertex.h


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    #vulkanwindow.cpp\
    camera.cpp \
#    ../shared/trianglerenderer.cpp
    trianglerenderer.cpp

RESOURCES += hellovulkantriangle.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/vulkan/hellovulkantriangle
INSTALLS += target
