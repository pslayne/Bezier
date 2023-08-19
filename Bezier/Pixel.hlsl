/**********************************************************************************
// Pixel (Arquivo de Sombreamento)
//
// Criação:     22 Jul 2020
// Atualização: 22 Jul 2023
// Compilador:  FXC
//
// Descrição:   Um pixel shader simples que apenas passa a cor do pixel
//              para frente sem realizar nenhuma modificação.
//
**********************************************************************************/

struct pixelIn
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

float4 main(pixelIn pIn) : SV_TARGET
{
    return pIn.Color;
}