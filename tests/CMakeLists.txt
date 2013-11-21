add_executable(tests
    test_printers.hpp
    tests_main.cpp
    unit/math_tests.cpp
    unit/TestCanvas.hpp
    regression/cast_lightning_tests.cpp
    regression/disconnect_tests.cpp
    regression/move_tests.cpp
    regression/spawn_tests.cpp
    regression/spell_harm_tests.cpp
    regression/spell_heal_tests.cpp
    regression/test_game_config.hpp
    regression/TestClient.hpp
    )

target_link_libraries(tests ${Boost_LIBRARIES} ws2_32 mswsock)

