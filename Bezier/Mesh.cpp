/**********************************************************************************
// Mesh (Código Fonte)
//
// Criação:     28 Abr 2016
// Atualização: 24 Jul 2023
// Compilador:  Visual C++ 2022
//
// Descrição:   Representa uma malha 3D em Direct3D 12
//
**********************************************************************************/

#include "Mesh.h"
#include "Engine.h"

// -------------------------------------------------------------------------------

Mesh::Mesh(uint vbSize, uint vbStride) : vertexBufferSize(vbSize), vertexBufferStride(vbStride)
{
    // inicializa buffers
    vertexBufferGPU = nullptr;
    vertexBufferUpload = nullptr;

    // aloca recursos para o vertex buffer
    Engine::graphics->Allocate(UPLOAD, vbSize, &vertexBufferUpload);
    Engine::graphics->Allocate(GPU, vbSize, &vertexBufferGPU);
}

// -------------------------------------------------------------------------------

Mesh::Mesh(const void* vb, uint vbSize, uint vbStride): vertexBufferSize(vbSize), vertexBufferStride(vbStride)
{
    // inicializa buffers
    vertexBufferGPU = nullptr;
    vertexBufferUpload = nullptr;

    // aloca recursos para o vertex buffer
    Engine::graphics->Allocate(UPLOAD, vbSize, &vertexBufferUpload);
    Engine::graphics->Allocate(GPU, vbSize, &vertexBufferGPU);

    // copia vértices para o buffer da GPU usando o buffer de Upload
    Engine::graphics->Copy(vb, vbSize, vertexBufferUpload, vertexBufferGPU);
}

// -------------------------------------------------------------------------------

Mesh::~Mesh()
{
    if (vertexBufferUpload) vertexBufferUpload->Release();
    if (vertexBufferGPU) vertexBufferGPU->Release();
}

// -------------------------------------------------------------------------------

D3D12_VERTEX_BUFFER_VIEW * Mesh::VertexBufferView()
{
    vertexBufferView.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = vertexBufferStride;
    vertexBufferView.SizeInBytes = vertexBufferSize;

    return &vertexBufferView;
}

// -------------------------------------------------------------------------------

