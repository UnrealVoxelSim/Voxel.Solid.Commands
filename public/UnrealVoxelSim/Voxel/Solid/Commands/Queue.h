#pragma once
#include "UnrealVoxelSim/Voxel/Solid/Api/ICommandProcessor.h"
#include "UnrealVoxelSim/Voxel/Solid/Api/ICommandSink.h"
#include "UnrealVoxelSim/Voxel/Solid/Api/ICommands.h"
#include <memory>
namespace UnrealVoxelSim::Voxel::Solid::Commands
{
class Queue final : public Api::ICommandSink, public Api::ICommandProcessor
{
  public:
    explicit Queue(Api::ICommands &commands);
    ~Queue() override;
    [[nodiscard]] std::expected<void, Api::QueueError> Submit(std::span<const Api::QueuedCommand> commands) override;
    void ProcessCommands(Simulation::Api::StepContext context) override;
  private:
    class Impl;
    std::unique_ptr<Impl> Impl_;
};
} // namespace UnrealVoxelSim::Voxel::Solid::Commands
