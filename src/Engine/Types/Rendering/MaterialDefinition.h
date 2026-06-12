#pragma once
#include <array>
#include <string>

#include "ShaderDefinition.h"
#include "ShaderSettings.h"
#include "Engine/WTL/vector.h"
#include "yaml-cpp/yaml.h"

namespace WEngine
{
    struct MaterialDefinitionSwizzle
    {
        // index to texturesPackaging declaring which texture is going to get generated
        uint8 packedTexTarget;

        // look at me trying to be efficient and all.
        struct SwizzleOrigin
        {
            // index to texturesDevel declaring which texture is going to get sampled
            uint8 develTexOrigin : 6;
            // which channel of the texture; in order rgba
            uint8 channel : 2;
        };

        // a swizzle per channel
        std::array<SwizzleOrigin, 4> swizzle;
    };
    struct MaterialDefinition
    {
        bool valid = false;
        std::string name;
        std::string shaderName;

        wtl::vector<std::string> texturesDevel;
        wtl::vector<std::string> texturesPackaging;

        wtl::vector<MaterialDefinitionSwizzle> swizzles;
        ShaderSettings params;

        void Parse(const YAML::Node& root);

    private:
        bool StrictChannelCheck(const wtl::vector<std::string>& channels,
            const wtl::vector<std::pair<std::string, std::string>>& develTextures);
        bool StrictExtensionCheck(const ShaderDefinition::FragmentInfo& fragInfo,
            const wtl::vector<std::pair<std::string, std::string>>& packTextures);

        bool ProcessParams(const ShaderDefinition::FragmentInfo& fragInfo, const YAML::Node& matRoot);

        bool CheckParamType(ShaderSettingType expect, const YAML::Node& paramsRaw);
    };

    // overengineering final boss
    class SwizzleCompiler
    {
        enum class SwizzleToken
        {
            Invalid,
            Name,
            Channel,
            Dot,
            Equal,
            Separation,
            EndOfLine,
            EndOfTokens,
        };

        using SwizzleRawLine = std::pair<std::string, wtl::vector<std::string>>;
        using TextureIndex = std::pair<std::string, uint8>;
        // the uint8 is an index into the corresponding storage for the token.
        using Token = std::pair<SwizzleToken, uint8>;

        struct SwizzleGenerated
        {
            uint8 textureTarget;
            char channelTarget;
            uint8 textureOrigin;
            char channelOrigin;
        };

    public:
        bool Compile();
        void AddSwizzleLine(const SwizzleRawLine& line);
        void AddDevelopmentTexture(const TextureIndex& tex);
        void AddPackagingTexture(const TextureIndex& tex);

    private:
        void Lexer();
        void ParseExpression(const std::string& expression);

        bool SyntaxAnalysis();
        bool CheckSyntax_A();
        bool CheckSyntax_B();
        bool CheckSyntax_C();
        bool CheckSyntax_D();
        bool CheckSyntax_E();

        Token Peek();
        void Consume();

        void PrintUnexpectedToken();

        bool SemanticAnalysisTokenVals();

        void SwizzleIRGeneration();

    private:
        uint32 m_tokenCursor = 0;
        wtl::vector<Token> m_tokens;
        wtl::vector<std::string> m_names;
        wtl::vector<char> m_channels;
        wtl::vector<SwizzleGenerated> m_generated;

        // inputs
        wtl::vector<SwizzleRawLine> m_swizzles;
        wtl::vector<TextureIndex> m_develTex;
        wtl::vector<TextureIndex> m_packTex;
    };
}
