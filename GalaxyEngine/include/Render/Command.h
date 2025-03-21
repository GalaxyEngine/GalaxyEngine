#pragma once
#include "GalaxyAPI.h"
#include "Resource/Mesh.h"
#include "Utils/Type.h"

namespace GALAXY::Resource
{
    struct SubMesh;
}

namespace GALAXY::Resource
{
    class Material;
}

namespace GALAXY
{
    namespace Render
    {
        struct SortKey
        {
            uint64_t materialKey = UUID_NULL;
            uint64_t meshKey = UUID_NULL;
            bool operator!=(const SortKey& sortKey) const
            {
                return materialKey != sortKey.materialKey || meshKey != sortKey.meshKey;
            }
        };
        class GALAXY_API RenderCommand
        {
        public:
            virtual ~RenderCommand() = default;
            virtual bool BeforeExecute(RenderCommand* prevCommand) { return false; }
            virtual void AfterExecute() {}
            virtual void Execute(RenderCommand* prevCommand) = 0;

        public:
            SortKey sortKey;
        };

        class GALAXY_API CommandBuffer
        {
        public:
            CommandBuffer() = default;
            CommandBuffer(const CommandBuffer&) = delete;
            CommandBuffer& operator=(const CommandBuffer&) = delete;
            CommandBuffer(CommandBuffer&&) = default;
            CommandBuffer& operator=(CommandBuffer&&) = default;

            static CommandBuffer* Get();

            static void AddCommand(std::unique_ptr<RenderCommand> command);

            void SortCommands();

            static void ExecuteCommands();

        private:
            std::vector<Unique<RenderCommand>> commands;
        };

        
        struct DrawCommandData
        {
            SortKey sortKey;
            int vertexArrayID;
            Resource::Material* material = nullptr;
            Resource::SubMesh subMesh;
            Mat4 modelMatrix;
            Mat4 MVP;
            bool hasLSM = false;
            Mat4 LSM;
            Vec3f ViewPos;
            Vec3f CamUp;
            Vec3f CamRight;
            uint64_t sceneID;
        };
        
        class GALAXY_API DrawCommand : public RenderCommand
        {
        public:
            DrawCommand(const DrawCommandData& _data);

            bool BeforeExecute(RenderCommand* prevCommand) override;
            void Execute(RenderCommand* prevCommand) override;
            void AfterExecute() override;

            const DrawCommandData& GetData() const { return data; }
        protected:
            DrawCommandData data;
        };

        class GALAXY_API DrawPickingCommand : public DrawCommand
        {
        public:
            DrawPickingCommand(const DrawCommandData& _data) : DrawCommand(_data) {}

            bool BeforeExecute(RenderCommand* prevCommand) override;
            void Execute(RenderCommand* prevCommand) override;
        };

        class GALAXY_API DrawOutlineCommand : public DrawCommand
        {
        public:
            DrawOutlineCommand(const DrawCommandData& _data) : DrawCommand(_data) {}

            bool BeforeExecute(RenderCommand* prevCommand) override;
            void Execute(RenderCommand* prevCommand) override;
        };

        class GALAXY_API DrawPostProcessCommand : public DrawCommand
        {
        public:
            DrawPostProcessCommand(const DrawCommandData& _data) : DrawCommand(_data) {}

            bool BeforeExecute(RenderCommand* prevCommand) override;
            void Execute(RenderCommand* prevCommand) override;
        };

        class GALAXY_API DrawShadowCommand : public DrawCommand
        {
        public:
            DrawShadowCommand(const DrawCommandData& _data) : DrawCommand(_data) {}

            bool BeforeExecute(RenderCommand* prevCommand) override;
            void Execute(RenderCommand* prevCommand) override;
        };
    }
}
