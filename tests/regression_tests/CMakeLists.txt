add_executable(regression_tests
    ../common/test_printers.hpp
    test_game_config.hpp
    TestClient.hpp
    cast_lightning_tests.cpp
    disconnect_tests.cpp
    move_tests.cpp
    spawn_tests.cpp
    spell_harm_tests.cpp
    spell_heal_tests.cpp
    regression_tests.cpp)

target_link_libraries(regression_tests ${Boost_LIBRARIES})

if(WIN32)
    target_link_libraries(regression_tests ws2_32 mswsock)
endif()
