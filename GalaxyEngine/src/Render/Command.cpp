#include "pch.h"
#include "Render/Command.h"

#include "Core/Application.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"

namespace GALAXY 
{
    Render::CommandBuffer* Render::CommandBuffer::Get()
    {
        return Core::Application::GetInstance().GetCommandBuffer();
    }

    void Render::CommandBuffer::AddCommand(std::unique_ptr<RenderCommand> command)
    {
        Get()->commands.push_back(std::move(command));
    }

    void Render::CommandBuffer::SortCommands()
    {
        std::sort(commands.begin(), commands.end(),
            [](const Unique<RenderCommand>& a, const Unique<RenderCommand>& b)
            {
                if (a->sortKey.materialKey == b->sortKey.materialKey)
                    return a->sortKey.meshKey < b->sortKey.meshKey;
                return a->sortKey.materialKey < b->sortKey.materialKey;
            });
    }

    void Render::CommandBuffer::ExecuteCommands()
    {
        auto instance = Get();
        instance->SortCommands();
        SortKey prevKey;
        RenderCommand* prevCommand = nullptr;
        bool success = false;
        for (size_t i = 0; i < instance->commands.size(); i++)
        {
            auto& cmd = instance->commands[i];
            SortKey currentKey = cmd->sortKey;
            if (currentKey != prevKey)
            {
                if (prevCommand)
                {
                    prevCommand->AfterExecute();
                }
                prevKey = currentKey;
                success = cmd->BeforeExecute(prevCommand);

                if (success)
                    cmd->Execute(prevCommand);
            }
            else
            {
                if (success)
                    cmd->Execute(prevCommand);
            }
            prevCommand = cmd.get();
        }
        instance->commands.clear();
    }
    
    Render::DrawCommand::DrawCommand(const DrawCommandData& _data)
    {
        data = _data;
        sortKey = data.sortKey;
    }

    bool Render::DrawCommand::BeforeExecute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        Resource::Material* material = data.material;
        Shared<Resource::Shader> shader = material->GetShader();
        if (!shader || !shader->HasBeenSent())
            return false;
        material->SendForDefault(shader);
        if (!prevCommand || prevCommand->sortKey.meshKey != sortKey.meshKey)
        {
            Renderer* renderer = Renderer::GetInstance();
            renderer->BindVertexArray(data.vertexArrayID);
        }
        return true;
    }
    
    void Render::DrawCommand::Execute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        using namespace Resource;
        Renderer* renderer = Renderer::GetInstance();
        Shared<Shader> shader = data.material->GetShader();
        
        shader->SendVec3f("ViewPos", data.ViewPos);
        shader->SendVec3f("CamUp", data.CamUp);
        shader->SendVec3f("CamRight", data.CamRight);
        shader->SendMat4("Model", data.modelMatrix);
        shader->SendMat4("MVP", data.MVP);
        if (data.hasLSM)
            shader->SendMat4("LSM", data.LSM);
        //todo : Handle picking

        renderer->DrawArrays(data.subMesh.startIndex, data.subMesh.count);
    }

    void Render::DrawCommand::AfterExecute()
    {
        Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
        Shared<Resource::Shader> shader = data.material->GetShader();
        for (const auto& val : shader->GetUniforms() | std::views::values)
        {
            if (val.type == Resource::UniformType::Texture2D && val.bind.has_value())
            {
                renderer->UnbindTexture(val.bind.value());
            }
        }
        renderer->UnbindVertexArray();
    }

    bool Render::DrawPickingCommand::BeforeExecute(RenderCommand* prevCommand)
    {
        auto material = data.material;
        auto shader = material->GetShader()->GetPickingVariant().lock();
        if (!shader || !shader->HasBeenSent())
            return false;
        shader->Use();
        if (!prevCommand || prevCommand->sortKey.meshKey != sortKey.meshKey)
        {
            Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
            renderer->BindVertexArray(data.vertexArrayID);
        }
        return true;
    }

    void Render::DrawPickingCommand::Execute(RenderCommand* prevCommand)
    {
        auto material = data.material;
        auto shader = material->GetShader()->GetPickingVariant().lock();
        if (!shader || !shader->HasBeenSent())
            return;

        const int r = (data.sceneID & 0x000000FF) >> 0;
        const int g = (data.sceneID & 0x0000FF00) >> 8;
        const int b = (data.sceneID & 0x00FF0000) >> 16;

        shader->SendVec4f("idColor", Vec4f(r / 255.f, g / 255.f, b / 255.f, 1.f));
        DrawCommand::Execute(prevCommand);
    }

    bool Render::DrawOutlineCommand::BeforeExecute(RenderCommand* prevCommand)
    {
        using namespace Resource;
        auto unlitShader = ResourceManager::GetUnlitShader().lock();
        if (!unlitShader || !unlitShader->HasBeenSent())
            return false;
        unlitShader->Use();

        unlitShader->SendInt("material.hasAlbedo", false);
        unlitShader->SendVec4f("material.diffuse", Vec4f(1));
        return true;
    }

    void Render::DrawOutlineCommand::Execute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        using namespace Resource;
        Renderer* renderer = Renderer::GetInstance();
        Shared<Shader> shader = Resource::ResourceManager::GetUnlitShader().lock();
        
        shader->SendMat4("MVP", data.MVP);

        renderer->BindVertexArray(data.vertexArrayID);
        renderer->DrawArrays(data.subMesh.startIndex, data.subMesh.count);
        renderer->UnbindVertexArray();
    }

    bool Render::DrawPostProcessCommand::BeforeExecute(RenderCommand* prevCommand)
    {
        return DrawCommand::BeforeExecute(prevCommand);
    }

    void Render::DrawPostProcessCommand::Execute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        using namespace Resource;
        Renderer* renderer = Renderer::GetInstance();

        auto shader = data.material->GetShader();
        
        renderer->DrawArrays(data.subMesh.startIndex, data.subMesh.count);
    }

    bool Render::DrawShadowCommand::BeforeExecute(RenderCommand* prevCommand)
    {
       Shared<Resource::Shader> shadowShader = Resource::ResourceManager::GetInstance()->GetShadowShader().
            lock();
        if (!shadowShader || !shadowShader->HasBeenSent())
            return false;
        shadowShader->Use();
        
        if (!prevCommand || prevCommand->sortKey.meshKey != sortKey.meshKey)
        {
            Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
            renderer->BindVertexArray(data.vertexArrayID);
        }
        return true;
    }

    void Render::DrawShadowCommand::Execute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        using namespace Resource;
        Renderer* renderer = Renderer::GetInstance();
        std::shared_ptr<Shader> shader = Resource::ResourceManager::GetInstance()->GetShadowShader().lock();
        shader->SendMat4("MVP", data.MVP);
        
        renderer->DrawArrays(data.subMesh.startIndex, data.subMesh.count);
    }
}
