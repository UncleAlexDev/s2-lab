function(add_translations TARGET_NAME TS_FILES_LIST)
    # 1. Задаем варианты имен утилит для Qt5 / Qt6
    if(Qt6_FOUND OR Qt6_DIR)
        set(LUPDATE_NAMES "lupdate-qt6" "lupdate6" "lupdate")
        set(LRELEASE_NAMES "lrelease-qt6" "lrelease6" "lrelease")
        if(Qt6_DIR)
            get_filename_component(QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
        endif()
    else()
        set(LUPDATE_NAMES "lupdate-qt5" "lupdate5" "lupdate")
        set(LRELEASE_NAMES "lrelease-qt5" "lrelease5" "lrelease")
        if(Qt5_DIR)
            get_filename_component(QT_BIN_DIR "${Qt5_DIR}/../../../bin" ABSOLUTE)
        endif()
    endif()

    # 2. Ищем утилиты в папке Qt и в системе (Windows / Linux / macOS)
    find_program(LUPDATE_EXE NAMES ${LUPDATE_NAMES} HINTS "${QT_BIN_DIR}")
    find_program(LRELEASE_EXE NAMES ${LRELEASE_NAMES} HINTS "${QT_BIN_DIR}")

    if(NOT LUPDATE_EXE OR NOT LRELEASE_EXE)
        message(WARNING "Linguist tools (lupdate/lrelease) не найдены!")
        return()
    endif()

    message("lupdate:  ${LUPDATE_EXE}")
    message("lrelease: ${LRELEASE_EXE}")

    list(GET TS_FILES_LIST 0 TS_FILE)
    get_filename_component(TS_DIR "${TS_FILE}" DIRECTORY)
    if(NOT EXISTS "${TS_DIR}")
        message("${TS_DIR} not found. Skip translations...")
        return()
    endif()

    # 3. Обрабатываем каждый .ts файл из переданного списка
    foreach(TS_FILE IN LISTS TS_FILES_LIST)
        message("File: ${TS_FILE}")
        get_filename_component(TS_DIR "${TS_FILE}" DIRECTORY)
        if(NOT EXISTS "${TS_DIR}")
            message(STATUS "${TS_DIR} not found. Skip...")
            continue()
        endif()
        get_filename_component(TSFILE "${TS_FILE}" NAME)
        string(REPLACE ".ts" ".qm" QMFILE "${TSFILE}")
        set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${QMFILE}")
        message("${TS_FILE} => ${QMFILE}")

        # 4. Вешаем прямой вызов консольных утилит на POST_BUILD таргета
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMENT "-- translation: ${TS_FILE} => ${QM_FILE}"
            COMMAND "${LUPDATE_EXE}" "${CMAKE_CURRENT_SOURCE_DIR}/src" -ts "${TS_FILE}"
            COMMAND "${LRELEASE_EXE}" "${TS_FILE}" -qm "${QM_FILE}"
        )
    endforeach()
endfunction()
