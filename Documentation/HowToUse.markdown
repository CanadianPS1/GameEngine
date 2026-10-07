**PROJECT START**

A project has on update methods (every frame) and on start methods
inorder for the engine to know what's what you need to declare a innitial method under its class and set your methods to there values

ie.
    #include "engine_code/EngineMain.hpp"
    void start(){std::cout<<"1"<<std::endl;}
    void update(){std::cout<<"2"<<std::endl;}
    engine::EngineMain::SetInitialMethods(){
        MakeMethodOnUpdate(update);
        MakeMethodOnStart(start);
    }

although the names of your methods dont matter what does is that they are void with no peramiters
you can have as many on start or update methods as you would like, if you decide you want no update AND start methods then you must still declare the SetInitialMethods method


**CREATING 3D OBJECTS**

Every obejct must be contained in a Sceen, a Sceen is just a collection of game objects that can be 2D or 3D Sceens, Sceens are used to load sets of game objects 
the Sceen constructer has 1 paramiter which is the name of the Sceen

this is how you create a Sceen

    void start(){
        engine::Sceen sceen("sceen");
    }
    void engine::EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }

once you have a Sceen you can create a 3D object 

    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObejct("../assets/scenes/Forest.obj", "Forest", glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f});
        engine::EngineMain::LoadGameObjects(sceen1);
    }
    void engine::EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }

Creating an object has many paramiters but the ones needed to create a 3D object are
    The path to the file
    The name of the object
    its position
    its scale
    its rotation

**ACTIONS WITH 3D OBJECTS**

