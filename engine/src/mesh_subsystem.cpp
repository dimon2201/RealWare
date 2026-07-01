// mesh_subsystem.cpp

#include "mesh_subsystem.hpp"

triton::XMeshSubsystem::XMeshSubsystem(cContext* context) : ISubsystem(context)
{
}

triton::XMeshSubsystem::~XMeshSubsystem()
{
}

void triton::XMeshSubsystem::CreateMesh(const std::string& filePath);
{
}