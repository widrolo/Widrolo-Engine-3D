#include "MaterialDefinition.h"

#include "Engine/Core/System/Iris.h"
#include "Engine/Util/Log.h"

using namespace WEngine;

void MaterialDefinition::Parse(const YAML::Node &root)
{
    if (!root["material"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Failed to load material definition!");
        return;
    }

    const YAML::Node material = root["material"];

    // we need to at least have the name to give a better error is something is missing later.
    if (!material["materialName"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Material definition name not present!");
        return;
    }

    name = material["materialName"].as<std::string>();

    if (!material["shader"])
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog(std::format("Material \"{}\" is missing one of the following fields:\n"
            "\t shader", name));
        return;
    }

    shaderName = material["shader"].as<std::string>();

    auto shaderN = Iris::GetShaderDef(shaderName);
    if (!shaderN.HasValue())
        return; // Iris already prints an error message

    const auto& fragInfo = shaderN.GetValue().fragInfo;

    bool disableStrictCheck = false;
    if (material["disableStrictCheck"])
        disableStrictCheck = material["disableStrictCheck"].as<bool>();

    // reading this unironically sounds like im praying on the materials' downfall.
    if (fragInfo.expectTextureCount != 0)
    {
        // since we have textures now, we have to ensure proper sanity checks.
        if (!material["texturesDevel"] || !material["texturesPackaging"] || !material["develTransform"])
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Material \"{}\" is missing one of the following fields:\n"
                "\t texturesDevel, texturesPackaging, develTransform.", name));
            return;
        }

        // this will later be vital for swizzling
        // first = temp name | second = file name
        wtl::vector<std::pair<std::string, std::string>> develTex;
        wtl::vector<std::pair<std::string, std::string>> packTex;

        for (const auto& tex : material["texturesDevel"])
            develTex.push_back({tex.first.as<std::string>(), tex.second.as<std::string>()});
        for (const auto& tex : material["texturesPackaging"])
            packTex.push_back({tex.first.as<std::string>(), tex.second.as<std::string>()});

        for (const auto& tex : develTex)
            texturesDevel.push_back(tex.second);
        for (const auto& tex : packTex)
            texturesPackaging.push_back(tex.second);

        if (packTex.size() != fragInfo.expectTextureCount)
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Material sanity test tripped in \"{}\":\n"
                "\t unexpected texture count!", name));
            return;
        }

        if (!disableStrictCheck)
        {
            if (!StrictChannelCheck(fragInfo.expectChannelNames, packTex))
                return;
            if (!StrictExtensionCheck(fragInfo, packTex))
                return;
        }

        SwizzleCompiler compiler;
        for (const auto& swz : material["develTransform"])
        {
            // yaml-cpp pass


        }

        if (!compiler.Compile())
            return;
    }

    if (!fragInfo.expectedParams.empty())
    {
        if (!material["params"])
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("Material \"{}\" is missing one of the following fields:\n"
                "\t texturesDevel", name));
            return;
        }

        ProcessParams(fragInfo, material);
    }

    valid = true;
}

bool MaterialDefinition::StrictChannelCheck(const wtl::vector<std::string> &channels,
    const wtl::vector<std::pair<std::string, std::string>> &develTextures)
{
    std::string missingExpected;
    bool found = true;
    for (const auto& expect : channels)
    {
        // i still dont know if im supposed to trust ranges
        if (!std::ranges::contains(develTextures, expect, [](const auto& p) { return p.first; }))
        {
            missingExpected = expect;
            found = false;
            break;
        }
    }
    if (!found)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog(std::format("Material sanity test tripped in \"{}\":\n"
            "\t expected channel \"{}\" not found!", name, missingExpected));
        return false;
    }
    return true;
}

bool MaterialDefinition::StrictExtensionCheck(const ShaderDefinition::FragmentInfo &fragInfo,
    const wtl::vector<std::pair<std::string, std::string>> &packTextures)
{
    wtl::vector<std::string> colorTex;
    wtl::vector<std::string> pbrTex;

    for (const auto& tex : packTextures)
    {
        // here we check for the file extension, png ends with g and pbr ends with r.
        // the spec specifically only allows these two to exist, it's your fault
        // if you show up with a jpg to a png convention.
        char last = tex.second.back();

        if (last == 'g')
            colorTex.push_back(tex.first);
        if (last == 'r')
            pbrTex.push_back(tex.first);
    }

    // Reddit purists are pounding on that door right now.
    // But I think it's much more elegant to do it with a goto rather than mangling
    // out of two loops into the same message or having the same error message twice.
    // Adding another supported extension is also trivial here.
    for (const auto& expect : fragInfo.colorTextures)
        if (!std::ranges::contains(colorTex, expect))
            goto checkFail;
    for (const auto& expect : fragInfo.pbrTextures)
        if (!std::ranges::contains(pbrTex, expect))
            goto checkFail;

    goto checkSuccess;

    checkFail:
    WLog::SetConsoleError();
    WLog::ConsoleLog(std::format("Material sanity test tripped in \"{}\":\n"
        "\t mismatch in expected texture types!", name));
    return false;

    checkSuccess:
    return true;
}

bool MaterialDefinition::ProcessParams(const ShaderDefinition::FragmentInfo& fragInfo, const YAML::Node& matRoot)
{
    for (const auto& param : matRoot["params"])
    {
        bool found = false;
        ShaderSettingType expectedType;

        for (const auto& expected : fragInfo.expectedParams)
        {
            if (expected.first == param.first.as<std::string>())
            {
                found = true;
                expectedType = expected.second;
            }
        }
        if (!found)
            continue;
        if (!CheckParamType(expectedType, param.second))
            return false;

        ShaderSetting setting;
        setting.type = expectedType;
        setting.settingName = param.first.as<std::string>();
        switch (expectedType)
        {
            case ShaderSettingType::Float:
                setting.option = param.second.as<float32>();
            case ShaderSettingType::Int:
                setting.option = param.second.as<int32>();
            case ShaderSettingType::Vec2:
            {
                Vector2 vec;
                vec.x = param.second[0].as<float32>();
                vec.y = param.second[1].as<float32>();
                setting.option = vec;
            }
            case ShaderSettingType::Vec3:
            {
                Vector3 vec;
                vec.x = param.second[0].as<float32>();
                vec.y = param.second[1].as<float32>();
                vec.z = param.second[2].as<float32>();
                setting.option = vec;
            }
            case ShaderSettingType::Vec4:
            {
                Vector4 vec;
                vec.x = param.second[0].as<float32>();
                vec.y = param.second[1].as<float32>();
                vec.z = param.second[2].as<float32>();
                vec.w = param.second[3].as<float32>();
                setting.option = vec;
            }
            default: ;
        }
        params.push_back(setting);
    }
    return true;
}

bool MaterialDefinition::CheckParamType(ShaderSettingType expect, const YAML::Node& paramsRaw)
{
    switch (expect)
    {
        case ShaderSettingType::Float:
        case ShaderSettingType::Int:
            return paramsRaw.IsScalar();
        case ShaderSettingType::Vec2:
            return paramsRaw.IsSequence() && paramsRaw.size() == 2;
        case ShaderSettingType::Vec3:
            return paramsRaw.IsSequence() && paramsRaw.size() == 3;
        case ShaderSettingType::Vec4:
            return paramsRaw.IsSequence() && paramsRaw.size() == 4;

        default:
            return false;
    }
}

bool SwizzleCompiler::Compile()
{
    Lexer();
    if (!SyntaxAnalysis())
    {
        PrintUnexpectedToken();
        return false;
    }
    // check for validity of the string values gathered during lexical analysis
    if (!SemanticAnalysisTokenVals())
        return false;

    // This generates IR
    SwizzleIRGeneration();

    // TODO: Finish this later
    // semantic analysis 2, check for conflicts
    // Construct final Swizzle map.

    return true;
}

void SwizzleCompiler::AddSwizzleLine(const SwizzleRawLine &line)
{
    m_swizzles.push_back(line);
}

void SwizzleCompiler::AddDevelopmentTexture(const TextureIndex &tex)
{
    m_develTex.push_back(tex);
}

void SwizzleCompiler::AddPackagingTexture(const TextureIndex &tex)
{
    m_packTex.push_back(tex);
}

void SwizzleCompiler::Lexer()
{
    for (const auto& line : m_swizzles)
    {
        ParseExpression(line.first);
        m_tokens.push_back({SwizzleToken::Equal, 0});
        for (const auto& right : line.second)
        {
            ParseExpression(right);
            m_tokens.push_back({SwizzleToken::Separation, 0});
        }
        // little hack to account for the last separation token
        m_tokens[m_tokens.size() - 1].first = SwizzleToken::EndOfLine;
    }
    m_tokens.push_back({SwizzleToken::EndOfTokens, 0});
}

void SwizzleCompiler::ParseExpression(const std::string& expression)
{
    uint32 i = 0;
    uint32 strLen = expression.size();
    bool readingChannel = false;

    std::string left;

    while (i < strLen)
    {
        if (readingChannel)
        {
            m_channels.push_back(expression[i]);
            m_tokens.push_back({SwizzleToken::Channel, m_channels.size() - 1});
        }
        else
        {
            if (expression[i] == '.')
            {
                m_names.push_back(left);
                m_tokens.push_back({SwizzleToken::Name, m_names.size() - 1});
                readingChannel = true;
                m_tokens.push_back({SwizzleToken::Dot, 0});
                i++;
                continue;
            }
            left.push_back(expression[i]);
        }
        i++;
    }
}

bool SwizzleCompiler::SyntaxAnalysis()
{
    /* grammar up for analysis:
     * Alphabet:
     * n = Name | c = Channel | d = Dot | e = Equal | s = Separation | l = EndOfLine | t = EndOfTokens
     *
     * Grammar:
     * S -> AS | At     // full code
     * A -> BeDl        // full swizzle line
     * B -> ndC         // single expression
     * C -> cC | c      // channel collection
     * D -> BE          // right side of swizzle line
     * E -> BsE | eps   // right side of swizzle line
     */

    // non-terminal S check
    while (m_tokenCursor < m_tokens.size())
    {
        if (!CheckSyntax_A())
            return false;
    }

    return true;
}

bool SwizzleCompiler::CheckSyntax_A()
{
    if (!CheckSyntax_B())
        return false;

    if (Peek().first != SwizzleToken::Equal)
        return false;
    Consume();

    if (!CheckSyntax_D())
        return false;

    if (Peek().first != SwizzleToken::EndOfLine)
        return false;
    Consume();

    return true;
}

bool SwizzleCompiler::CheckSyntax_B()
{
    if (Peek().first != SwizzleToken::Name)
        return false;
    Consume();

    if (Peek().first != SwizzleToken::Dot)
        return false;
    Consume();

    if (!CheckSyntax_C())
        return false;

    return true;
}

bool SwizzleCompiler::CheckSyntax_C()
{
    // satisfies case c
    if (Peek().first != SwizzleToken::Channel)
        return false;
    Consume();

    // satisfies case cC
    while (Peek().first == SwizzleToken::Channel)
        Consume();

    return true;
}

bool SwizzleCompiler::CheckSyntax_D()
{
    if (!CheckSyntax_B())
        return false;

    if (!CheckSyntax_E())
        return false;

    return true;
}

bool SwizzleCompiler::CheckSyntax_E()
{
    // satisfies case eps
    if (Peek().first != SwizzleToken::Separation)
        return true;

    Consume();

    if (!CheckSyntax_B())
        return false;

    // no need to check for separation

    return CheckSyntax_E();
}

SwizzleCompiler::Token SwizzleCompiler::Peek()
{
    return m_tokens[m_tokenCursor];
}

void SwizzleCompiler::Consume()
{
    m_tokenCursor++;
}

void SwizzleCompiler::PrintUnexpectedToken()
{
    std::string error = "[SwizzleCompiler] Compilation Error! Syntax Error, unexpected Token \"";
    switch (Peek().first)
    {
        case SwizzleToken::Invalid:
            error += "Invalid";
            break;
        case SwizzleToken::Name:
            error += "Name";
            break;
        case SwizzleToken::Channel:
            error += "Channel";
            break;
        case SwizzleToken::Dot:
            error += "Dot";
            break;
        case SwizzleToken::Equal:
            error += "Equal";
            break;
        case SwizzleToken::Separation:
            error += "Separation";
            break;
        case SwizzleToken::EndOfLine:
            error += "EndOfLine";
            break;
        case SwizzleToken::EndOfTokens:
            error += "EndOfTokens";
            break;
    }

    error += "\"!";

    WLog::SetConsoleError();
    WLog::ConsoleLog(error);
}

bool SwizzleCompiler::SemanticAnalysisTokenVals()
{
    // Rules:
    // 1) Check all channels, and see if they all 'r', 'g', 'b' or 'a'.
    // 2) Check whether all name tokens after EOL:
    //      -> Exist in Packaged Texture temp names
    //      -> All Packaged Texture temp names are present
    // 3) Check whether all name tokens after either Equal or Separator:
    //      -> Exist in Devel Texture temp names
    // 4) Check if per line, the number of channel tokens on the left matches the right, max 4 channels

    // Rule 1
    for (const auto& c : m_channels)
    {
        if (c != 'r' && c != 'g' && c != 'b' && c != 'a')
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Unexpected channel \"{}\"!", c));
            return false;
        }
    }

    m_tokenCursor = 0;

    while (Peek().first != SwizzleToken::EndOfTokens)
    {
        const auto t = Peek().first;
        // Rule 2
        if (t == SwizzleToken::EndOfLine)
        {
            Consume();
            std::string name = m_names[Peek().second];
            if (!std::ranges::contains(m_packTex, name, [&](const auto& tex){ return tex.first; }))
            {
                WLog::SetConsoleError();
                WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Unexpected left texture \"{}\"!", name));
                return false;
            }
        }

        // Rule 3
        if (t == SwizzleToken::Equal || t == SwizzleToken::Separation)
        {
            Consume();
            std::string name = m_names[Peek().second];
            if (!std::ranges::contains(m_develTex, name, [&](const auto& tex){ return tex.first; }))
            {
                WLog::SetConsoleError();
                WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Unexpected right texture \"{}\"!", name));
                return false;
            }
        }

        Consume();
    }

    m_tokenCursor = 0;

    // Rule 4
    uint8 leftChannelCount = 0;
    uint8 rightChannelCount = 0;
    bool countingRight = false;
    while (Peek().first != SwizzleToken::EndOfTokens)
    {
        if (countingRight)
        {
            if (Peek().first == SwizzleToken::Channel)
            {
                rightChannelCount++;
                if (rightChannelCount > 4)
                {
                    WLog::SetConsoleError();
                    WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Too many channels on right side!"));
                    return false;
                }
            }
            if (Peek().first == SwizzleToken::EndOfLine)
            {
                if (leftChannelCount != rightChannelCount)
                {
                    WLog::SetConsoleError();
                    WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Channel count not equal!"));
                    return false;
                }

                countingRight = false;
                leftChannelCount = 0;
                rightChannelCount = 0;
            }
        }
        else
        {
            if (Peek().first == SwizzleToken::Channel)
            {
                leftChannelCount++;
                if (leftChannelCount > 4)
                {
                    WLog::SetConsoleError();
                    WLog::ConsoleLog(std::format("[SwizzleCompiler] Compilation Error! Too many channels on left side!"));
                    return false;
                }
            }
            if (Peek().first == SwizzleToken::Equal)
                countingRight = true;
        }
        Consume();
    }

    return true;
}

void SwizzleCompiler::SwizzleIRGeneration()
{
    // Given tokens:
    // n = Name | c = Channel | d = Dot | e = Equal | s = Separation | l = EndOfLine | t = EndOfTokens
    // Assume example token series:
    // ... l n d c c c c e n d c c c s n d c l ...
    //
    // We need to generate four swizzle structures for the packed texture. For each, we will have two cursor
    // searching for the next channel on either side to link them together.
    // This function should never fail, Semantic Analysis 1 is responsible for clearing the way for us.
    //
    // Assumptions we will need:
    // 1) first token and first token after EOL is always name of left texture
    // 2) first token after Equal or Separator is always name of right texture
    // 3) if the cursor is on Name, advancing by 2 will give its first channel
    // 4) if right cursor hits a Separator, the next Token will be the name of the next right texture
    // 5) if left cursor is on equal, right cursor is on end of line

    m_tokenCursor = 0;

    uint32 leftChannelCursor = 0;
    uint32 rightChannelCursor = 0;

    Token leftNameTok;
    Token rightNameTok;

    while (Peek().first != SwizzleToken::EndOfTokens)
    {
        leftNameTok = Peek();
        leftChannelCursor = m_tokenCursor + 2;

        while (Peek().first != SwizzleToken::Equal)
            Consume();
        Consume(); // to get to the name

        rightNameTok = Peek();
        rightChannelCursor = m_tokenCursor + 2;

        while (m_tokens[leftChannelCursor].first != SwizzleToken::Equal)
        {
            // if the channel of the right side ends, we need to advance to the next right texture
            if (m_tokens[rightChannelCursor].first == SwizzleToken::Separation)
            {
                rightChannelCursor++;
                rightNameTok = m_tokens[rightChannelCursor];
                rightChannelCursor += 2;
            }

            // while we need the index, its easier to do this first.
            std::string leftName = m_names[leftNameTok.second];
            std::string rightName = m_names[rightNameTok.second];

            uint8 leftTexture;
            uint8 rightTexture;

            for (const auto& tex : m_packTex)
                if (tex.first == leftName)
                    leftTexture = tex.second;
            for (const auto& tex : m_develTex)
                if (tex.first == rightName)
                    rightTexture = tex.second;

            SwizzleGenerated swizzle;
            swizzle.textureTarget = leftTexture;
            swizzle.textureOrigin = rightTexture;
            swizzle.channelTarget = m_channels[m_tokens[leftChannelCursor].second];
            swizzle.channelOrigin = m_channels[m_tokens[rightChannelCursor].second];

            m_generated.push_back(swizzle);

            leftChannelCursor++;
            rightChannelCursor++;
        }

        // assumption 5 go brr
        m_tokenCursor = rightChannelCursor + 1;
    }

}

