# UnrealVoxelSim.Voxel.Solid.Commands

Thread-affine, solid-domain-owned queue for canonical tick-stamped fill and erase commands. Submission and processing
are segregated capabilities: presentation receives only the sink, while a composition root invokes the processor and
the queue calls the authoritative synchronous solid command API.
