/**********************************************************************************
// Bezier (C�digo Fonte)
//
// Cria��o:     12 Ago 2020
// Atualiza��o: 06 Ago 2023
// Compilador:  Visual C++ 2022
//
// Descri��o:   Base para gerar curvas usando Corner-Cutting
//
**********************************************************************************/

#include "DXUT.h"
#include <math.h>
#include <iostream>

// ------------------------------------------------------------------------------

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

// ------------------------------------------------------------------------------

class Bezier : public App
{
private:
    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineState;

    Mesh* geometry;
    Mesh* geometry1;
    Mesh* geometry2;
    Mesh* geometry3;

    static const uint MaxVertex = 10000;
    Vertex controle[MaxVertex];
    Vertex markingPoints[6*MaxVertex];
    uint count = 0;
    uint index = 0;
    
    Vertex controleBuffer[MaxVertex];
    Vertex markingPointsBuffer[6*MaxVertex];
    uint countBuffer = 0;
    uint indexBuffer = 0;
    
    static const int dots = 101;
    Vertex curva[dots*100];
    uint curveCount = 0;
    
    Vertex curvaBuffer[dots*100];
    uint curveCountBuffer = 0;

    Vertex auxiliares[MaxVertex];
    uint auxCount = 0;

    Vertex auxiliaresBuffer[MaxVertex];
    uint auxCountBuffer = 0;

    bool calcCurve = false;

public:
    void Init();
    void Update();
    void Display();
    void Finalize();

    void BuildRootSignature();
    void BuildPipelineState();

    void GenerateMarkingPoints();
    void CubicCurve();
};

// ------------------------------------------------------------------------------

void Bezier::Init()
{
    graphics->ResetCommands();

    // ---------[ Build Geometry ]------------
    
    // tamanho do buffer de v�rtices em bytes
    const uint vbSize = MaxVertex * sizeof(Vertex);

    // cria malha 3D
    geometry = new Mesh(vbSize, sizeof(Vertex));
    geometry1 = new Mesh(vbSize, sizeof(Vertex));
    geometry2 = new Mesh(vbSize, sizeof(Vertex));
    geometry3 = new Mesh(vbSize, sizeof(Vertex));

    // ---------------------------------------

    BuildRootSignature();
    BuildPipelineState();        
    
    // ---------------------------------------

    graphics->SubmitCommands();
}

// ------------------------------------------------------------------------------

void Bezier::Update()
{
    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    if (input->KeyPress(VK_DELETE)) {
        count = 0;
        index = 0;
        curveCount = 0;
        auxCount = 0;
        graphics->ResetCommands();
        Display();
    }

    if (input->KeyPress('S')) {
        for (int i = 0; i < count; i++) {
            controleBuffer[i] = controle[i];
        }
        countBuffer = count;
        indexBuffer = index;

        for (int i = 0; i < count*6; i++) {
            markingPointsBuffer[i] = markingPoints[i];
        }

        for (int i = 0; i < curveCount; i++) {
            curvaBuffer[i] = curva[i];
        }
        curveCountBuffer = curveCount;

        for (int i = 0; i < auxCount; i++) {
            auxiliaresBuffer[i] = auxiliares[i];
        }
        auxCountBuffer = auxCount;
    }

    if (input->KeyPress('L')) {
        for (int i = 0; i < countBuffer; i++) {
            controle[i] = controleBuffer[i];
        }
        count = countBuffer;
        index = indexBuffer;

        for (int i = 0; i < countBuffer * 6; i++) {
            markingPoints[i] = markingPointsBuffer[i];
        }

        for (int i = 0; i < curveCountBuffer; i++) {
            curva[i] = curvaBuffer[i];
        }
        curveCount = curveCountBuffer;

        for (int i = 0; i < auxCountBuffer; i++) {
            auxiliares[i] = auxiliaresBuffer[i];
        }
        auxCount = auxCountBuffer;

        graphics->ResetCommands();
        graphics->Copy(controle, geometry->vertexBufferSize, geometry->vertexBufferUpload, geometry->vertexBufferGPU);
        graphics->Copy(curva, geometry1->vertexBufferSize, geometry1->vertexBufferUpload, geometry1->vertexBufferGPU);
        graphics->Copy(auxiliares, geometry2->vertexBufferSize, geometry2->vertexBufferUpload, geometry2->vertexBufferGPU);
        graphics->Copy(markingPoints, geometry3->vertexBufferSize, geometry3->vertexBufferUpload, geometry3->vertexBufferGPU);
        graphics->SubmitCommands();
        Display();
    }

    // cria v�rtices com o bot�o do mouse
    if (input->KeyPress(VK_LBUTTON)) {
        float cx = float(window->CenterX());
        float cy = float(window->CenterY());
        float mx = float(input->MouseX());
        float my = float(input->MouseY());
        
        // converte as coordenadas da tela para a faixa -1.0 a 1.0
        // cy e my foram invertidos para levar em considera��o que 
        // o eixo y da tela cresce na dire��o oposta do cartesiano
        float x = (mx - cx) / cx;
        float y = (cy - my) / cy;

        if (count < MaxVertex)
            ++count;

        if (count == 4)
            calcCurve = true;
        else if (count > 4)
            calcCurve = !calcCurve;

        controle[index] = { XMFLOAT3(x, y, 0.0f), XMFLOAT4(Colors::Red) };
        index = (index + 1) % MaxVertex;

        GenerateMarkingPoints();

        // copia v�rtices para o buffer da GPU usando o buffer de Upload
        graphics->ResetCommands();
        graphics->Copy(controle, geometry->vertexBufferSize, geometry->vertexBufferUpload, geometry->vertexBufferGPU);
        graphics->Copy(markingPoints, geometry3->vertexBufferSize, geometry3->vertexBufferUpload, geometry3->vertexBufferGPU);
        
        if (calcCurve && count >= 4 && count % 2 == 0) {
            CubicCurve();
            curveCount++;
            
            graphics->Copy(curva, geometry1->vertexBufferSize, geometry1->vertexBufferUpload, geometry1->vertexBufferGPU);
            graphics->Copy(auxiliares, geometry2->vertexBufferSize, geometry2->vertexBufferUpload, geometry2->vertexBufferGPU);
        }

        //
       
        graphics->SubmitCommands();

        Display();
    }
}

// ------------------------------------------------------------------------------

void Bezier::Display()
{
    // limpa backbuffer
    graphics->Clear(pipelineState);


    // submete comandos de configura��o do pipeline
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);

    graphics->CommandList()->IASetVertexBuffers(0, 1, geometry->VertexBufferView());
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    graphics->CommandList()->DrawInstanced(count, 1, 0, 0);
    
    graphics->CommandList()->IASetVertexBuffers(0, 1, geometry2->VertexBufferView());
    graphics->CommandList()->DrawInstanced(auxCount, 1, 0, 0);

    graphics->CommandList()->IASetVertexBuffers(0, 1, geometry1->VertexBufferView());
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    graphics->CommandList()->DrawInstanced(dots * curveCount, curveCount, 0, dots);

    graphics->CommandList()->IASetVertexBuffers(0, 1, geometry3->VertexBufferView());
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    graphics->CommandList()->DrawInstanced(6 * count, count, 0, 6);

    // apresenta backbuffer
    graphics->Present();    
}

// ------------------------------------------------------------------------------

void Bezier::Finalize()
{
    rootSignature->Release();
    pipelineState->Release();
    delete geometry;
    delete geometry1;
    delete geometry2;
    delete geometry3;
}


// ------------------------------------------------------------------------------
//                                     D3D                                      
// ------------------------------------------------------------------------------

void Bezier::BuildRootSignature()
{
    // descri��o para uma assinatura vazia
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // serializa assinatura raiz
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &error));

    // cria uma assinatura raiz vazia
    ThrowIfFailed(graphics->Device()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));
}

// ------------------------------------------------------------------------------

void Bezier::BuildPipelineState()
{
    // --------------------
    // --- Input Layout ---
    // --------------------
    
    D3D12_INPUT_ELEMENT_DESC inputLayout[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertexShader;
    ID3DBlob* pixelShader;

    D3DReadFileToBlob(L"Shaders/Vertex.cso", &vertexShader);
    D3DReadFileToBlob(L"Shaders/Pixel.cso", &pixelShader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

    D3D12_RASTERIZER_DESC rasterizer = {};
    rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ---------------------
    // --- Color Blender ---
    // ---------------------

    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    D3D12_DEPTH_STENCIL_DESC depthStencil = {};
    depthStencil.DepthEnable = TRUE;
    depthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencil.StencilEnable = FALSE;
    depthStencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depthStencil.FrontFace = defaultStencilOp;
    depthStencil.BackFace = defaultStencilOp;
    
    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = rootSignature;
    pso.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depthStencil;
    pso.InputLayout = { inputLayout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = graphics->Antialiasing();
    pso.SampleDesc.Quality = graphics->Quality();
    
    graphics->Device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipelineState));

    vertexShader->Release();
    pixelShader->Release();
}

void Bezier::CubicCurve() {
    // P1: index - 4 | P2: index - 3 | P3: index - 2 | P4: index - 1
    XMFLOAT3 pontos[4] = {
        controle[index - 4].Pos,
        controle[index - 3].Pos,
        controle[index - 2].Pos,
        controle[index - 1].Pos,
    };

    if (curveCount >= 1) {
        // troca pontos[0] e [1]

        XMFLOAT3 aux = pontos[0];
        pontos[0] = pontos[1];
        pontos[1] = aux;

        // espelha novo pontos[1] em rela��o a [0]
        float vectorX = pontos[1].x - pontos[0].x;
        float vectorY = pontos[1].y - pontos[0].y;

        vectorX *= -1;
        vectorY *= -1;

        pontos[1].x = pontos[0].x + vectorX;
        pontos[1].y = pontos[0].y + vectorY;

        // TO-DO: mostrar ponto auxiliar
        auxiliares[auxCount] = { pontos[0], XMFLOAT4(Colors::Red)};
        auxCount = (auxCount + 1) % MaxVertex;
        
        auxiliares[auxCount] = { pontos[1], XMFLOAT4(Colors::Black)};
        auxCount = (auxCount + 1) % MaxVertex;
    }

    int i = curveCount * dots;
    float d = (float) dots;
    float barrier = d / (d - 1.0);
    float step = barrier - 1.0;

    for (float t = 0.0f; t <= barrier; t += step, i++) {

        //X
        float xp1 = pow((1 - t), 3) * pontos[0].x;
        float xp2 = 3 * t * pow((1 - t), 2) * pontos[1].x;
        float xp3 = 3 * pow(t, 2) * (1 - t) * pontos[2].x;
        float xp4 = pow(t, 3) * pontos[3].x;
        
        float x = xp1 + xp2 + xp3 + xp4;

        //Y
        float yp1 = pow((1 - t), 3) * pontos[0].y;
        float yp2 = 3 * t * pow((1 - t), 2) * pontos[1].y;
        float yp3 = 3 * pow(t, 2) * (1 - t) * pontos[2].y;
        float yp4 = pow(t, 3) * pontos[3].y;

        float y = yp1 + yp2 + yp3 + yp4;

        curva[i] = { XMFLOAT3(x, y, 0.0f), XMFLOAT4(Colors::White) };
    }
}

void Bezier::GenerateMarkingPoints() {
    for (int i = 0, j = 0; i < count; i++) {
        float x = controle[i].Pos.x;
        float y = controle[i].Pos.y;

        float r = 0.005f;

        markingPoints[j++] = { XMFLOAT3(x - r , y + r, 0.0f), XMFLOAT4(Colors::Red) };
        markingPoints[j++] = { XMFLOAT3(x + r , y + r, 0.0f), XMFLOAT4(Colors::Red) };
        markingPoints[j++] = { XMFLOAT3(x + r , y - r, 0.0f), XMFLOAT4(Colors::Red) };
                                            
        markingPoints[j++] = { XMFLOAT3(x - r , y + r, 0.0f), XMFLOAT4(Colors::Red) };
        markingPoints[j++] = { XMFLOAT3(x + r , y - r, 0.0f), XMFLOAT4(Colors::Red) };
        markingPoints[j++] = { XMFLOAT3(x - r , y - r, 0.0f), XMFLOAT4(Colors::Red) };
    }
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance,    _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    try
    {
        // cria motor e configura a janela
        Engine* engine = new Engine();
        engine->window->Mode(WINDOWED);
        engine->window->Size(1024, 600);
        engine->window->ResizeMode(ASPECTRATIO);
        engine->window->Color(0, 0, 0);
        engine->window->Title("Bezier");
        engine->window->Icon(IDI_ICON);
        engine->window->LostFocus(Engine::Pause);
        engine->window->InFocus(Engine::Resume);

        // cria e executa a aplica��o
        engine->Start(new Bezier());

        // finaliza execu��o
        delete engine;
    }
    catch (Error & e)
    {
        // exibe mensagem em caso de erro
        MessageBox(nullptr, e.ToString().data(), "Bezier", MB_OK);
    }

    return 0;
}

// ----------------------------------------------------------------------------
