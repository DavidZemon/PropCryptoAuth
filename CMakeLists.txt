cmake_minimum_required(VERSION 3.3)

find_package(PropWare REQUIRED)
project(PropCrypto)

set(MODEL cmm)
set(BOARD activityboard)

add_subdirectory(cryptoauthlib/lib EXCLUDE_FROM_ALL)
get_target_property(CRYPTOAUTH_RELATIVE_SRCS cryptoauth SOURCES)
foreach(src IN LISTS CRYPTOAUTH_RELATIVE_SRCS)
    list(APPEND CRYPTOAUTH_SRCS "${PROJECT_SOURCE_DIR}/cryptoauthlib/lib/${src}")
endforeach()
create_library(pwcryptoauth ${CRYPTOAUTH_SRCS})
target_compile_options(pwcryptoauth PRIVATE ${CRYPTO_AUTH_OPTS} -w)
target_compile_definitions(pwcryptoauth PRIVATE -DATCA_HAL_I2C)
target_include_directories(pwcryptoauth SYSTEM PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cryptoauthlib/lib>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cryptoauthlib/lib/hal>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cryptoauthlib/lib/basic>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cryptoauthlib/lib/crypto>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/hal>
    $<INSTALL_INTERFACE:include/basic>
    $<INSTALL_INTERFACE:include/crypto>)

add_subdirectory(demo)
install(TARGETS cryptoauth EXPORT ${PROJECT_NAME}Config DESTINATION lib)
install(DIRECTORY cryptoauthlib/lib/ DESTINATION include
    FILES_MATCHING PATTERN *.h)

install(EXPORT ${PROJECT_NAME}Config
    DESTINATION lib/cmake/${PROJECT_NAME}
    NAMESPACE PW::
)
