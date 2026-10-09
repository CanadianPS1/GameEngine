**PROJECT START**

A project has on update methods (every frame) and on start methods
inorder for the engine to know what's what you need to declare a innitial method under its class and set your methods to there values

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

    #include "engine_code/EngineGameObject.hpp"
    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObejct("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
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

If you want to change every one of an objects locations in one blow then youll need to provide a glm::vec3 with the x y z as floats

    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        engine::EngineMain::LoadGameObjects(sceen1);
        forest->transform.translation = glm::vec3{2.0f, 2.0f, 2.0f};
        forest->transform.scale = glm::vec3{2.0f, 2.0f, 2.0f};
        forest->transform.rotation = glm::vec3{2.0f, 2.0f, 2.0f};
    }

you can also change just the x y or z of an object

    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        engine::EngineMain::LoadGameObjects(sceen1);
        forest->transform.translation.x = 9.f;
        forest->transform.scale.y = 9.f;
        forest->transform.rotation.z = 9.f;
    }

**ACTIONS WITH 3D CAMERAS**

you can have 2D and 3D cameras but only one active at a time this will be the main camera inorder to create a camera the CreateCamera method you need to provide
    if its 2D
    its position
    its target
    its rotation
    its feald of view
    its near clipping plain
    its far clipping plain
then after words youll need to set the camera as the main camera if you want to see out of it

    #include "engine_code/EngineCamera.hpp"
    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        engine::EngineMain::LoadGameObjects(sceen1);
        engine::EngineCamera* camera = engine::EngineCamera::CreateCamera(0, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 2.5f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        engine::EngineCamera::SetMainCamera(camera);
    }
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }

you can change its locations attributes the same as any other game object but you call the viewer object inside the camera

    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        engine::EngineMain::LoadGameObjects(sceen1);
        engine::EngineCamera* camera = engine::EngineCamera::CreateCamera(0, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 2.5f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        engine::EngineCamera::SetMainCamera(camera);
        camera->viewerObject->transform.translation.x = 9.f;
    }
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }

but if you want to change the clipping planes or fov you just call it on the camera its self

    void start(){
        engine::Sceen sceen1("sceen1");
        engine::EngineGameObject* forest = sceen1.CreateObject("../assets/scenes/Forest.obj", "Forest", 
            glm::vec3{0.0f, 7.0f, 13.f}, glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec3{0.f,0.f,0.f});
        engine::EngineMain::LoadGameObjects(sceen1);
        engine::EngineCamera* camera = engine::EngineCamera::CreateCamera(0, glm::vec3(-1.f, -2.f, 2.f), 
        glm::vec3{0.f, 0.f, 2.5f}, glm::vec3{-0.5f, 0.f, 0.f}, 20, 8, 30);
        engine::EngineCamera::SetMainCamera(camera);
        camera->near = 6;
        camera->far = 25;
        camera->fov = 50;
    }
    void EngineMain::SetInitialMethods(){
        MakeMethodOnStart(start);
    }

**INPUT**

With input you can detect when any key is pressed and get its keycode or you can detect when a spesific key is pressed or releaced and bind it to a method
inorder to get any pressed key you would call the GetEvent() method on InputController

    #include "engine_code/EngineInputController.hpp"
    void update(){
        std::cout<<engine::InputController::GetEvent()<<std::endl;
    }

You can bind methods to key presses and releases by calling the SetUpInput or SetDownInput method which take a int (the keycode) and a method name

    void wPressed(){std::cout<<"w pressed"<<std::endl;}
    void sPressed(){std::cout<<"s pressed"<<std::endl;}
    void EngineMain::SetInitialMethods(){
        engine::InputController::SetDownInput(GLFW_KEY_W, wPressed);
        engine::InputController::SetUpInput(GLFW_KEY_S, sPressed);
    }

the method will get called every frame that the key is up or down so make sure to take that into account when writing your methods