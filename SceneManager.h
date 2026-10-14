#pragma once
#include "Scene.h"

class SceneManager {
public:
    static Scene& GetActiveScene() {
        static Scene s_activeScene;
        return s_activeScene;
    }
};