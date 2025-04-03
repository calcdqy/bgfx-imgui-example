cmake_minimum_required(VERSION 3.19)

if(POLICY CMP0130)
    cmake_policy(SET CMP0130 NEW)
endif()
if(POLICY CMP0012)
    cmake_policy(SET CMP0012 NEW)
endif()

find_program(GIT_EXECUTABLE git REQUIRED)

function(ask_user_confirm question default_ans alternate_ans)
    string(TOUPPER "${default_ans}" default_upper)
    string(TOLOWER "${alternate_ans}" alternate_lower)
    message("${question} [${default_upper}/${alternate_lower}] ")

    if(WIN32)
        execute_process(
            COMMAND cmd /c "set /p user_input= && call echo %user_input%"
            OUTPUT_VARIABLE user_input
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        execute_process(
            COMMAND bash -c "read user_input && echo $user_input"
            OUTPUT_VARIABLE user_input
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    if("${user_input}" STREQUAL "")
        set(user_input "${alternate_ans}")
    endif()

    string(TOLOWER "${user_input}" user_input_lower)
    string(TOLOWER "${default_ans}" default_ans_lower)

    if(user_input_lower STREQUAL default_ans_lower)
        set(USER_CONFIRMED TRUE PARENT_SCOPE)
    else()
        set(USER_CONFIRMED FALSE PARENT_SCOPE)
    endif()
endfunction()


function(get_github_latest_release repo_path)
    set(API_URL "https://api.github.com/repos/${repo_path}/releases")
    string(REGEX REPLACE ".*/([^/]+)$" "\\1" REPO_NAME "${repo_path}")
    set(JSON_FILE "${CMAKE_CURRENT_BINARY_DIR}/${REPO_NAME}_releases.json")

    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)

    message(STATUS "Fetching GitHub releases for ${repo_path}...")

    file(DOWNLOAD ${API_URL} ${JSON_FILE}
        STATUS DOWNLOAD_STATUS
        TIMEOUT 15
    )

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 STATUS_MESSAGE)

    if(STATUS_CODE EQUAL 35)
        message(STATUS "SSL verification failed, retrying without certificate verification...")
        file(DOWNLOAD ${API_URL} ${JSON_FILE}
            TLS_VERIFY OFF
            STATUS DOWNLOAD_STATUS
            TIMEOUT 15
        )
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 STATUS_MESSAGE)
    endif()
    
    if(NOT STATUS_CODE EQUAL 0)
        message(WARNING "Failed to download releases (HTTP ${STATUS_CODE}: ${STATUS_MESSAGE})")
        return()
    endif()

    file(READ ${JSON_FILE} RELEASES_JSON)

    if(RELEASES_JSON STREQUAL "" OR RELEASES_JSON MATCHES "Not Found")
        message(STATUS "No releases found or repository not accessible")
        file(REMOVE ${JSON_FILE})
        return()
    endif()
    
    string(JSON ARRAY_LENGTH LENGTH "${RELEASES_JSON}")
    if(ARRAY_LENGTH EQUAL 0)
        message(STATUS "No releases available in the repository")
        file(REMOVE ${JSON_FILE})
        return()
    endif()

    set(LATEST_RELEASE "")
    set(i 0)
    while(i LESS ARRAY_LENGTH AND LATEST_RELEASE STREQUAL "")
        string(JSON RELEASE_ITEM GET "${RELEASES_JSON}" ${i})
        string(JSON PRERELEASE GET "${RELEASE_ITEM}" "prerelease")
        string(JSON TAG_NAME GET "${RELEASE_ITEM}" "tag_name")
        
        if(NOT PRERELEASE AND NOT TAG_NAME MATCHES "error")
            set(LATEST_RELEASE ${TAG_NAME})
            message(STATUS "Found stable release: ${TAG_NAME}")
        endif()
        math(EXPR i "${i} + 1")
    endwhile()

    if(LATEST_RELEASE STREQUAL "" AND ARRAY_LENGTH GREATER 0)
        string(JSON FIRST_RELEASE GET "${RELEASES_JSON}" 0)
        string(JSON TAG_NAME GET "${FIRST_RELEASE}" "tag_name")
        if(NOT TAG_NAME MATCHES "error")
            set(LATEST_RELEASE ${TAG_NAME})
            message(STATUS "Using prerelease: ${TAG_NAME}")
        endif()
    endif()
    
    if(NOT LATEST_RELEASE STREQUAL "")
        set(LATEST_RELEASE ${LATEST_RELEASE} PARENT_SCOPE)
    endif()
    
    file(REMOVE ${JSON_FILE})
endfunction()

function(update_submodule submodule_path submodule_url)
    string(REGEX REPLACE ".*github\\.com[/:](.*)\\.git$" "\\1" GITHUB_REPO_PATH "${submodule_url}")

    if(NOT EXISTS "${submodule_path}/.git")
        message(WARNING "Submodule ${submodule_path} is not initialized, skipping")
        return()
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${submodule_path}
        OUTPUT_VARIABLE CURRENT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    message(STATUS "Current version of ${submodule_path}: ${CURRENT_VERSION}")

    set(LATEST_RELEASE "")
    if(GITHUB_REPO_PATH)
        get_github_latest_release(${GITHUB_REPO_PATH})
    endif()

    if(LATEST_RELEASE)
        message(STATUS "Checking out release ${LATEST_RELEASE}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} checkout ${LATEST_RELEASE}
            WORKING_DIRECTORY ${submodule_path}
            RESULT_VARIABLE CHECKOUT_RESULT
        )
        
        if(NOT CHECKOUT_RESULT EQUAL "0")
            message(WARNING "Failed to checkout ${LATEST_RELEASE}")
            set(LATEST_RELEASE "")
        endif()
    endif()
    
    if(NOT LATEST_RELEASE)
        message(STATUS "Using default branch")
        set(DEFAULT_BRANCHES main master)
        foreach(BRANCH IN LISTS DEFAULT_BRANCHES)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} checkout ${BRANCH}
                WORKING_DIRECTORY ${submodule_path}
                RESULT_VARIABLE CHECKOUT_RESULT
                ERROR_QUIET
            )
            
            if(CHECKOUT_RESULT EQUAL "0")
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} pull
                    WORKING_DIRECTORY ${submodule_path}
                    RESULT_VARIABLE PULL_RESULT
                )
                if(PULL_RESULT EQUAL "0")
                    message(STATUS "Successfully updated to ${BRANCH} branch")
                    break()
                endif()
            endif()
        endforeach()
    endif()
endfunction()

function(main)
    ask_user_confirm("Update all submodules to latest GitHub release (or default branch)?" "Y" "n")
    if(NOT USER_CONFIRMED)
        message(STATUS "Skipping submodule update.")
    else()
        if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/.gitmodules")
            message(STATUS "No .gitmodules file found - nothing to update")
            return()
        endif()

        file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/.gitmodules" SUBMODULE_PATHS REGEX "path = .*")
        file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/.gitmodules" SUBMODULE_URLS REGEX "url = .*")

        if(NOT SUBMODULE_PATHS)
            message(STATUS "No submodules found in .gitmodules")
            return()
        endif()

        set(SUCCESS_COUNT 0)
        set(TOTAL_COUNT 0)

        foreach(SUBMODULE_ENTRY ${SUBMODULE_PATHS})
            string(REGEX REPLACE ".*path = (.*)" "\\1" SUBMODULE_PATH "${SUBMODULE_ENTRY}")
            list(FIND SUBMODULE_PATHS "${SUBMODULE_ENTRY}" INDEX)
            list(GET SUBMODULE_URLS ${INDEX} SUBMODULE_URL_ENTRY)
            string(REGEX REPLACE ".*url = (.*)" "\\1" SUBMODULE_URL "${SUBMODULE_URL_ENTRY}")

            math(EXPR TOTAL_COUNT "${TOTAL_COUNT} + 1")
            message(STATUS "\nProcessing submodule ${TOTAL_COUNT}: ${SUBMODULE_PATH}")
            update_submodule(
                "${CMAKE_CURRENT_LIST_DIR}/${SUBMODULE_PATH}"
                "${SUBMODULE_URL}"
            )
        endforeach()
    endif()

    if(USER_CONFIRMED)
        ask_user_confirm("About to UPDATE LOCAL FILES to ${LATEST_RELEASE}. Confirm?" "Y" "n")
        if(NOT USER_CONFIRMED)
            message(STATUS "Skipping local file update for ${SUBMODULE_PATH}")
            continue()
        endif()

        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --remote --merge
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            RESULT_VARIABLE UPDATE_RESULT
        )
    endif()


    ask_user_confirm("\nBuild project now?" "Y" "n")
    if(USER_CONFIRMED)
        message(STATUS "Configuring and building...")

        file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/build")

        execute_process(
            COMMAND ${CMAKE_COMMAND} -S . -B build
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )

        execute_process(
            COMMAND ${CMAKE_COMMAND} --build build
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
    endif()

    message(STATUS "Done!")
    if(WIN32)
        execute_process(COMMAND cmd /c "pause")
    else()
        message("")
        execute_process(
            COMMAND bash -c "read -p 'Press [Enter] to continue...'"
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
    endif()
endfunction()

main()