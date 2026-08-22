add_library(${PROJECT_NAME} UnrealVoxelSim/Voxel/Solid/Commands/Queue.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>" "$<INSTALL_INTERFACE:include>")
target_link_libraries(${PROJECT_NAME} PUBLIC UnrealVoxelSim.Voxel.Solid.Api)
UnrealVoxelSimConfigureTarget(${PROJECT_NAME})
