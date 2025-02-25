/*
 *  Copyright 2019-2024 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include <random>
#include "Tutorial04_Instancing.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include "../../Common/src/TexturedCube.hpp"
#include "imgui.h"


namespace Diligent
{

SampleBase* CreateSample()
{
    return new Tutorial04_Instancing();
}

void Tutorial04_Instancing::CreatePipelineState()
{
    LayoutElement LayoutElems[] =
        {
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
            LayoutElement{2, 1, 4, VT_FLOAT32, False, INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
            LayoutElement{3, 1, 4, VT_FLOAT32, False, INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
            LayoutElement{4, 1, 4, VT_FLOAT32, False, INPUT_ELEMENT_FREQUENCY_PER_INSTANCE},
            LayoutElement{5, 1, 4, VT_FLOAT32, False, INPUT_ELEMENT_FREQUENCY_PER_INSTANCE}};

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);

    TexturedCube::CreatePSOInfo CubePsoCI;
    CubePsoCI.pDevice              = m_pDevice;
    CubePsoCI.RTVFormat            = m_pSwapChain->GetDesc().ColorBufferFormat;
    CubePsoCI.DSVFormat            = m_pSwapChain->GetDesc().DepthBufferFormat;
    CubePsoCI.pShaderSourceFactory = pShaderSourceFactory;

#ifdef DILIGENT_USE_OPENGL

    CubePsoCI.VSFilePath = "cube_inst_glsl.vert";
    CubePsoCI.PSFilePath = "cube_inst_glsl.frag";
#else
    CubePsoCI.VSFilePath = "cube_inst.vsh";
    CubePsoCI.PSFilePath = "cube_inst.psh";
#endif

    CubePsoCI.ExtraLayoutElements    = LayoutElems;
    CubePsoCI.NumExtraLayoutElements = _countof(LayoutElems);

    m_pPSO = TexturedCube::CreatePipelineState(CubePsoCI, m_ConvertPSOutputToGamma);

    CreateUniformBuffer(m_pDevice, sizeof(float4x4) * 2, "VS constants CB", &m_VSConstants);
    m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);
    m_pPSO->CreateShaderResourceBinding(&m_SRB, true);
}

void Tutorial04_Instancing::CreateInstanceBuffer()
{
    BufferDesc InstBuffDesc;
    InstBuffDesc.Name      = "Instance data buffer";
    InstBuffDesc.Usage     = USAGE_DEFAULT;
    InstBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    InstBuffDesc.Size      = sizeof(float4x4) * 22;
    m_pDevice->CreateBuffer(InstBuffDesc, nullptr, &m_InstanceBuffer);
    PopulateInstanceBuffer();
}

void Tutorial04_Instancing::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);
    CreatePipelineState();

    m_CubeVertexBuffer = TexturedCube::CreateVertexBuffer(m_pDevice, GEOMETRY_PRIMITIVE_VERTEX_FLAG_POS_TEX);
    m_CubeIndexBuffer  = TexturedCube::CreateIndexBuffer(m_pDevice);
    m_TextureSRV       = TexturedCube::LoadTexture(m_pDevice, "DGLogo.png")->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    m_SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);

    CreateInstanceBuffer();
}

static float angle = (PI_F / 1.0);

void Tutorial04_Instancing::PopulateInstanceBuffer()
{
    std::vector<float4x4> InstanceData(22);

    const float4x4 scaleHiloVertical = float4x4::Scale(0.01f, 1.0f, 0.01f);

    angle += 0;


    InstanceData[0]  = float4x4::Scale(5.0f, 0.1f, 0.01f) * float4x4::Translation(0.0f, 0.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[1]  = float4x4::Scale(0.01f, 0.1f, 5.0f) * float4x4::Translation(0.0f, 0.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[2]  = float4x4::Scale(0.1f, 1.0f, 0.01f) * float4x4::Translation(-5.0f, -1.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[3]  = float4x4::Scale(0.1f, 1.0f, 0.01f) * float4x4::Translation(5.0f, -1.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[4]  = float4x4::Scale(0.1f, 1.0f, 0.01f) * float4x4::Translation(0.0f, 1.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[5]  = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(0.0f, -1.0f, -5.0f) * float4x4::RotationY(angle);
    InstanceData[6]  = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(0.0f, -1.0f, 5.0f) * float4x4::RotationY(angle);
    InstanceData[7]  = float4x4::Scale(1, 1, 1) * float4x4::Translation(-5.0f, -2.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[8]  = float4x4::Scale(1, 1, 1) * float4x4::Translation(5.0f, -2.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[9]  = float4x4::Scale(1, 1, 1) * float4x4::Translation(0.0f, -2.0f, -5.0f) * float4x4::RotationY(angle);
    InstanceData[10] = float4x4::Scale(1, 1, 1) * float4x4::Translation(0.0f, -2.0f, 5.0f) * float4x4::RotationY(angle);
    InstanceData[11] = float4x4::Scale(3.0f, 0.05f, 0.01f) * float4x4::Translation(0.0f, -5.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[12] = float4x4::Scale(0.01f, 0.05f, 3.0f) * float4x4::Translation(0.0f, -5.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[13] = float4x4::Scale(0.05f, 4.0f, 0.01f) * float4x4::Translation(0.0f, -1.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[14] = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(-3.0f, -6.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[15] = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(3.0f, -6.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[16] = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(0.0f, -6.0f, 3.0f) * float4x4::RotationY(angle);
    InstanceData[17] = float4x4::Scale(0.05f, 1.0f, 0.01f) * float4x4::Translation(0.0f, -6.0f, -3.0f) * float4x4::RotationY(angle);
    InstanceData[18] = float4x4::Scale(1, 1, 1) * float4x4::Translation(-3.0f, -7.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[19] = float4x4::Scale(1, 1, 1) * float4x4::Translation(3.0f, -7.0f, 0.0f) * float4x4::RotationY(angle);
    InstanceData[20] = float4x4::Scale(1, 1, 1) * float4x4::Translation(0.0f, -7.0f, 3.0f) * float4x4::RotationY(angle);
    InstanceData[21] = float4x4::Scale(1, 1, 1) * float4x4::Translation(0.0f, -7.0f, -3.0f) * float4x4::RotationY(angle);



    Uint32 DataSize = static_cast<Uint32>(sizeof(InstanceData[0]) * InstanceData.size());
    m_pImmediateContext->UpdateBuffer(m_InstanceBuffer, 0, DataSize, InstanceData.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Tutorial04_Instancing::Render()
{
    auto*  pRTV       = m_pSwapChain->GetCurrentBackBufferRTV();
    auto*  pDSV       = m_pSwapChain->GetDepthBufferDSV();
    float4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};


    PopulateInstanceBuffer();

    if (m_ConvertPSOutputToGamma)
        ClearColor = LinearToSRGB(ClearColor);

    m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    {
        MapHelper<float4x4> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
        CBConstants[0] = m_ViewProjMatrix;
        CBConstants[1] = m_RotationMatrix;
    }

    const Uint64 offsets[] = {0, 0};
    IBuffer*     pBuffs[]  = {m_CubeVertexBuffer, m_InstanceBuffer};
    m_pImmediateContext->SetVertexBuffers(0, _countof(pBuffs), pBuffs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pImmediateContext->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_pImmediateContext->SetPipelineState(m_pPSO);
    m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawIndexedAttribs DrawAttrs;
    DrawAttrs.IndexType    = VT_UINT32;
    DrawAttrs.NumIndices   = 36;
    DrawAttrs.NumInstances = 22;
    DrawAttrs.Flags        = DRAW_FLAG_VERIFY_ALL;
    m_pImmediateContext->DrawIndexed(DrawAttrs);
}

void Tutorial04_Instancing::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    static float  yaw      = 0.0f;
    static float  pitch    = 0.0f;
    static float  distance = 20.0f;
    static float3 target(0.0f, -4.0f, 0.0f); 

    const float sensitivity = 0.005f;

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        yaw += dragDelta.x * sensitivity; 
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    }

    const float pitchLimit = 1.57f * 0.99f;
    if (pitch > pitchLimit)
        pitch = pitchLimit;
    if (pitch < -pitchLimit)
        pitch = -pitchLimit;

    float wheel = ImGui::GetIO().MouseWheel;
    if (fabs(wheel) > 0.0f)
    {
        distance -= wheel * 2.0f;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 100.0f) distance = 100.0f;
    }

    float3 offset;
    offset.x = distance * cosf(pitch) * sinf(yaw);
    offset.y = distance * sinf(pitch);
    offset.z = distance * cosf(pitch) * cosf(yaw);

    float3 cameraPos = target + offset;
    float3 forward = normalize(target - cameraPos);
    float3 right   = normalize(cross(float3(0.0f, 1.0f, 0.0f), forward));
    float3 camUp   = cross(forward, right);

    float panSpeed = 5.0f * static_cast<float>(ElapsedTime);
    if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
        target += camUp * panSpeed;
    if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
        target -= camUp * panSpeed;
    if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
        target += right * panSpeed;
    if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
        target -= right * panSpeed;

    cameraPos = target + offset;

    float4x4 View;
    View._11 = right.x;
    View._12 = camUp.x;
    View._13 = forward.x;
    View._14 = 0.0f;
    View._21 = right.y;
    View._22 = camUp.y;
    View._23 = forward.y;
    View._24 = 0.0f;
    View._31 = right.z;
    View._32 = camUp.z;
    View._33 = forward.z;
    View._34 = 0.0f;
    View._41 = -dot(right, cameraPos);
    View._42 = -dot(camUp, cameraPos);
    View._43 = -dot(forward, cameraPos);
    View._44 = 1.0f;

    auto SrfPreTransform = GetSurfacePretransformMatrix(float3{0, 0, 1});
    auto Proj            = GetAdjustedProjectionMatrix(PI_F / 4.0f, 0.1f, 100.f);

    m_ViewProjMatrix = View * SrfPreTransform * Proj;
    m_RotationMatrix = float4x4::Identity();

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 140, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_Always);
    ImGui::Begin("View Controls", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("View Orientation");

    ImGui::Separator();

    // Fila 1: Diagonales Superiores
    ImGui::Text("Top Diagonal Views");
    if (ImGui::Button("Front-Right"))
    {
        yaw   = PI_F / 4.0f;
        pitch = PI_F / 4.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Top-Right"))
    {
        yaw   = 0.0f;
        pitch = PI_F / 4.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Front-Left"))
    {
        yaw   = -PI_F / 4.0f;
        pitch = PI_F / 4.0f;
    }

    ImGui::Separator();

    // Fila 2: Vistas Principales
    ImGui::Text("Main Views");
    if (ImGui::Button("Right"))
    {
        yaw   = PI_F / 2.0f;
        pitch = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
        yaw   = 0.0f;
        pitch = PI_F / 2.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Front"))
    {
        yaw   = 0.0f;
        pitch = 0.0f;
    }

    if (ImGui::Button("Left"))
    {
        yaw   = -PI_F / 2.0f;
        pitch = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Down"))
    {
        yaw   = 0.0f;
        pitch = -PI_F / 2.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Back"))
    {
        yaw   = PI_F;
        pitch = 0.0f;
    }

    ImGui::Separator();

    // Fila 3: Diagonales Inferiores
    ImGui::Text("Bottom Diagonal Views");
    if (ImGui::Button("Right-Bottom"))
    {
        yaw   = PI_F / 4.0f;
        pitch = -PI_F / 4.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Down-Left"))
    {
        yaw   = 0.0f;
        pitch = -PI_F / 4.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Left-Bottom"))
    {
        yaw   = -PI_F / 4.0f;
        pitch = -PI_F / 4.0f;
    }

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Always);
    ImGui::Begin("Controles", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("Controles de la camara:");
    ImGui::Text("- Click derecho: Rotar");
    ImGui::Text("- Flechas: Desplazarse");
    ImGui::Text("- Rueda del mouse: Zoom");
    ImGui::End();

}

} // namespace Diligent
