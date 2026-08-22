#include "UnrealVoxelSim/Voxel/Solid/Commands/Queue.h"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <optional>
#include <thread>
#include <utility>
#include <variant>
#include <vector>
namespace UnrealVoxelSim::Voxel::Solid::Commands
{
class Queue::Impl final
{
  public:
    explicit Impl(Api::ICommands &commands) : Commands(commands) {}
    void AssertOwnerThread() const noexcept { assert(std::this_thread::get_id() == OwnerThread); }
    Api::ICommands &Commands;
    std::vector<Api::QueuedCommand> Pending;
    std::optional<Simulation::Api::TickIndex> LastProcessed;
    std::thread::id OwnerThread{std::this_thread::get_id()};
};
Queue::Queue(Api::ICommands &commands) : Impl_(std::make_unique<Impl>(commands)) {}
Queue::~Queue() = default;
std::expected<void, Api::QueueError> Queue::Submit(const std::span<const Api::QueuedCommand> commands)
{
    Impl_->AssertOwnerThread();
    auto combined = Impl_->Pending;
    combined.insert(combined.end(), commands.begin(), commands.end());
    for (std::size_t index = 0; index < commands.size(); ++index)
    {
        if (!commands[index].Stamp().IsValid()) return std::unexpected{Api::QueueError{Api::QueueErrorType::InvalidStamp, index}};
        if (Impl_->LastProcessed && commands[index].Stamp().TargetTick <= *Impl_->LastProcessed)
            return std::unexpected{Api::QueueError{Api::QueueErrorType::TickAlreadyProcessed, index}};
        const auto empty = std::visit([](const auto &value) {
            if constexpr (std::same_as<std::decay_t<decltype(value)>, Api::FillCommand>) return value.Placements.empty();
            else return value.Positions.empty();
        }, commands[index].Value());
        if (empty) return std::unexpected{Api::QueueError{Api::QueueErrorType::EmptyPayload, index}};
    }
    std::ranges::sort(combined, [](const auto &left, const auto &right) { return left.Stamp() < right.Stamp(); });
    if (std::ranges::adjacent_find(combined, [](const auto &left, const auto &right) { return left.Stamp() == right.Stamp(); }) != combined.end())
        return std::unexpected{Api::QueueError{Api::QueueErrorType::DuplicateStamp, 0}};
    Impl_->Pending = std::move(combined);
    return {};
}
void Queue::ProcessCommands(const Simulation::Api::StepContext context)
{
    Impl_->AssertOwnerThread();
    for (const auto &queued : Impl_->Pending)
    {
        if (queued.Stamp().TargetTick != context.Tick) continue;
        std::visit([&](const auto &value) {
            if constexpr (std::same_as<std::decay_t<decltype(value)>, Api::FillCommand>)
                static_cast<void>(Impl_->Commands.Place(value.Placements));
            else
                static_cast<void>(Impl_->Commands.Remove(value.Positions));
        }, queued.Value());
    }
    std::erase_if(Impl_->Pending, [&](const auto &queued) { return queued.Stamp().TargetTick <= context.Tick; });
    Impl_->LastProcessed = context.Tick;
}
} // namespace UnrealVoxelSim::Voxel::Solid::Commands
