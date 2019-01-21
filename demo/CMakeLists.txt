set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
create_simple_executable(${PROJECT_NAME} cryptoauth_demo.cpp atca_hal_prop.cpp)
target_link_libraries(${PROJECT_NAME} pwcryptoauth)
