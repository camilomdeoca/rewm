
add_executable(${PROJECT_NAME}
    xapp.cpp
    client.cpp
    managed_window.cpp
    rewm.cpp
    window_wrapper.cpp
    drawable_window_wrapper.cpp
    wm.cpp
    wm_config.cpp
    titlebar.cpp
    assets.cpp
    border.cpp
    button.cpp
    container.cpp
    vertical_array.cpp
    horizontal_array.cpp
    border_container.cpp
    taskbar.cpp
    writable_window_wrapper.cpp
    monitor.cpp
    padding.cpp
    workspace.cpp
    primary_container.cpp
    client_component.cpp
    wm_root_window.cpp
)

add_executable(rewmsh
    xapp.cpp
    rewmsh.cpp
)

target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${JPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${X11_Xrender_LIB})
target_link_libraries(${PROJECT_NAME} ${X11_Xft_LIB})
target_link_libraries(${PROJECT_NAME} ${X11_Xpm_LIB})
target_link_libraries(${PROJECT_NAME} ${X11_Xinerama_LIB})
target_link_libraries(${PROJECT_NAME} ${IMLIB2_LIBRARY})

target_link_libraries(rewmsh ${X11_LIBRARIES})
target_link_libraries(rewmsh ${X11_Xrender_LIB})

