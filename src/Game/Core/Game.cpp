#include "Game.h"

#include <string>
#include <Engine/Core/System/Memory.h>
#include <Engine/Util/Log.h>


Game::Game()
{

}

Game::~Game()
{

}

void Game::PreGameLoop()
{
    InitGameSystems();
}

void Game::GameLoopBegin()
{

}

void Game::GameLoopTickEarly()
{

}

void Game::GameLoopTickLate()
{

}

void Game::GameLoopPhysicsEarly()
{

}

void Game::GameLoopPhysicsLate()
{

}

void Game::GameLoopAudioEarly()
{

}

void Game::GameLoopAudioLate()
{

}

void Game::GameLoopWidgetEarly()
{

}

void Game::GameLoopDrawEarly()
{

}

void Game::GameLoopDrawJustBefore()
{

}

void Game::GameLoopDrawLate()
{

}

void Game::GameLoopFinish()
{

}

template<class T>
void StartSystemSingle(T** container, std::string name)
{
    *container = (T*)WAllocator::Construct<T>();
    if (*container == nullptr)
    {
        WEngine::WLog::SetConsoleError();
        WEngine::WLog::ConsoleLog(std::format("{} creation failed", name));
        std::exit(-1);
    }
}

void Game::InitGameSystems()
{

}
