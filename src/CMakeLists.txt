project(KeroseneLamp )

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( KeroseneLamp KeroseneLamp KeroseneLamp.cpp )
target_include_directories( KeroseneLamp PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( KeroseneLamp ${CMAKE_SOURCE_DIR}/../ricardian )