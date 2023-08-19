/**********************************************************************************
// Mesh (Arquivo de Cabeçalho)
//
// Criação:     28 Abr 2016
// Atualização: 24 Jul 2023
// Compilador:  Visual C++ 2022
//
// Descrição:   Representa uma malha 3D em Direct3D 12
//
**********************************************************************************/

#ifndef DXUT_MESH_H_
#define DXUT_MESH_H_

// -------------------------------------------------------------------------------

#include "Graphics.h"
#include "Types.h"

// -------------------------------------------------------------------------------

struct Mesh
{
    ID3D12Resource* vertexBufferUpload;                 // buffer de Upload CPU -> GPU
    ID3D12Resource* vertexBufferGPU;                    // buffer na GPU
    
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;          // descritor do vertex buffer
    uint vertexBufferSize;                              // tamanho do vertex buffer
    uint vertexBufferStride;                            // tamanho de um vértice

    Mesh(uint vbSize, uint vbStride);                   // apenas aloca memória
    Mesh(const void * vb, uint vbSize, uint vbStride);  // aloca memória e copia vértices
    ~Mesh();                                            // destrutor
 
    D3D12_VERTEX_BUFFER_VIEW * VertexBufferView();      // retorna descritor do vertex buffer
};

// -------------------------------------------------------------------------------

#endif

