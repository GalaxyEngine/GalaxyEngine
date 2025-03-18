#include "pch.h"
#include "Render/Command.h"

#include "Core/Application.h"
#include "Resource/Material.h"

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
        std::ranges::sort(commands, [](const auto& a, const auto& b)
        {
            return a->sortKey < b->sortKey;
        });
    }

    void Render::CommandBuffer::ExecuteCommands()
    {
        auto instance = Get();
        instance->SortCommands();
        uint64_t prevKey = UUID_NULL;
        RenderCommand* prevCommand = nullptr;
        for (size_t i = 0; i < instance->commands.size(); i++)
        {
            auto& cmd = instance->commands[i];
            uint64_t currentKey = cmd->sortKey;
            if (currentKey != prevKey)
            {
                if (prevCommand)
                {
                    prevCommand->AfterExecute();
                }
                prevKey = currentKey;
                cmd->BeforeExecute();
                
                cmd->Execute(nullptr);
            }
            else
            {
                cmd->Execute(prevCommand);
            }
            prevCommand = cmd.get();
        }
        instance->commands.clear();
    }
    
    Render::DrawCommand::DrawCommand(const DrawCommandData& _data)
    {
        data = _data;
        sortKey = _data.key;
    }

    void Render::DrawCommand::BeforeExecute()
    {
        Resource::Material* material = data.material;
        data.shader = material->SendValues(data.sceneID);
    }
    
    void Render::DrawCommand::Execute(RenderCommand* prevCommand)
    {
        using namespace Wrapper;
        using namespace Resource;
        Renderer* renderer = Renderer::GetInstance();
        if (prevCommand)
        {
            DrawCommand* drawCommand = dynamic_cast<DrawCommand*>(prevCommand);
            data.shader = drawCommand->data.shader;
        }
        auto shader = data.shader;

        if (!shader)
            return;

        renderer->BindVertexArray(data.vertexArrayID);

        shader->SendMat4("Model", data.modelMatrix);
        shader->SendMat4("MVP", data.MVP);
        shader->SendMat4("LSM", data.LSM);
        //todo : Handle picking

        renderer->DrawArrays(data.subMesh.startIndex, data.subMesh.count);
        renderer->UnbindVertexArray();
    }
}
