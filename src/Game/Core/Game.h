#pragma once

class Game
{
public:
    Game();
    ~Game();

public:
    void PreGameLoop();
    void GameLoopBegin();
    void GameLoopTickEarly();
    void GameLoopTickLate();
    void GameLoopPhysicsEarly();
    void GameLoopPhysicsLate();
    void GameLoopAudioEarly();
    void GameLoopAudioLate();
    void GameLoopWidgetEarly();
    void GameLoopDrawEarly();
    void GameLoopDrawJustBefore();
    void GameLoopDrawLate();
    void GameLoopFinish();


private:
    void InitGameSystems();
};