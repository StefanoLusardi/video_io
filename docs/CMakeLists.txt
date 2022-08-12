find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/icon.png")
    set(PROJECT_MAIN_PAGE README.md)

    set(VIDEO_IO_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include/${PROJECT_NAME}")
    set(VIDEO_IO_INCLUDE_FILES
        ${VIDEO_IO_HEADERS_PATH}/frame_queue.hpp
        ${VIDEO_IO_HEADERS_PATH}/raw_frame.hpp
        ${VIDEO_IO_HEADERS_PATH}/video_reader.hpp
        ${VIDEO_IO_HEADERS_PATH}/video_writer.hpp
    )

    list(APPEND DOXYGEN_INPUT_FILES "${CMAKE_SOURCE_DIR}/README.md" "${VIDEO_IO_INCLUDE_FILES}")
    string(REPLACE ";"  " " DOXYGEN_INPUT_FILES "${DOXYGEN_INPUT_FILES}")

    set(DOXYGEN_INPUTS ${DOXYGEN_INPUT_FILES})
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/)
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
        DEPENDS ${VIDEO_IO_INCLUDE_FILES}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating Doxygen documentation"
    )

    add_custom_target(video_io_docs ALL DEPENDS ${DOXYGEN_INDEX_FILE})
else()
    message(WARNING "Please install Doxygen to generate documentation.")
endif()