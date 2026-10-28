function(make_dist TARGET_NAME)
    # 1. Инсталлируем сам .exe строго в корень папки dist
    install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ".")

    # 2. Прямой вызов windeployqt в момент выполнения 'cmake --install'
    if(WIN32)
            # Находим путь к windeployqt на основе вашей установленной Qt6
            if(Qt6_DIR)
                get_filename_component(QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
            elseif(Qt5_DIR)
                get_filename_component(QT_BIN_DIR "${Qt5_DIR}/../../../bin" ABSOLUTE)
            endif()
            if(NOT EXISTS "${QT_BIN_DIR}/windeployqt.exe" AND Qt5_DIR)
                get_filename_component(QT_BIN_DIR "${Qt5_DIR}/../../../../bin" ABSOLUTE)
            endif()
            find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
            message("windeployqt: ${WINDEPLOYQT_EXECUTABLE}")

            if(WINDEPLOYQT_EXECUTABLE)
                    # Код внутри кавычек выполнится при инсталляции.
                    # Передаем абсолютные пути, чтобы полностью исключить баг с "Access is denied"
                    install(CODE "
                            execute_process(
                                    COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                                    \"\${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}.exe\"
                                    --dir \"\${CMAKE_INSTALL_PREFIX}\"
                                    --plugindir \"plugins\"
                                    --no-translations
                                    --force
                                    WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\"
                            )
                    ")

            get_filename_component(QT_BIN_DIR "${WINDEPLOYQT_EXECUTABLE}" DIRECTORY)
            set(QT_TRANSLATIONS_DIR "${QT_BIN_DIR}/../translations")

            install(FILES
                    "${QT_TRANSLATIONS_DIR}/qt_en.qm"
                    "${QT_TRANSLATIONS_DIR}/qt_ru.qm"
                    DESTINATION "translations")
            else()
                    message("windeployqt not found")
            endif()
    endif()

endfunction()
