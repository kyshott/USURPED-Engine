#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include <fstream>
#include <iostream>
#include <cstdio>
#include "physics.hpp"
#include <string>
#include <print>
#include "scenebattle.hpp"

/**
 * Constructor for the Play Scene
 * 
 * @param gameEngine raw pointer to the game engine class
 * @param levelPath Path to level defintion file, relative to exe
 */
ScenePlay::ScenePlay(GameEngine* gameEngine, std::string levelPath):Scene(gameEngine){
    this->levelPath=levelPath;
    init(this->levelPath);
}

/**
 * Functions that initializes the scene:
 * 
 * 1. Loads Level information
 * 2. Spawns Players  
 * 3. Registers input action  
 * 4. Sets up camera  
 * 
 * @param levelPath Path to level defintion file, relative to exe
 */
void ScenePlay::init(const std::string& levelPath){
    loadLevel(levelPath);
    spawnPlayer();

    //TODO: Add actions for UP, DOWN, LEFT, RIGHT, and ATTACK
    registerAction(KEY_B,"BB");
    registerAction(KEY_G,"GRID");
    registerAction(KEY_T,"TEX");
    registerAction(KEY_H,"HEALTH");
    registerAction(KEY_V,"VISION");
    registerAction(KEY_ESCAPE,"QUIT");
    registerAction(KEY_R,"RELOAD");

    registerAction(KEY_W, "UP");
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_A, "LEFT");
    registerAction(KEY_D, "RIGHT");
    registerAction(KEY_SPACE, "ATTACK");
    registerAction(MOUSE_BUTTON_RIGHT, "SECONDARY");
    registerAction(KEY_E, "INTERACT");
    registerAction(KEY_L, "BATTLE_TEST");
    
    mainCamera=Camera2D({gameEngine->getWidth()/2.0f*GetWindowScaleDPI().x,gameEngine->getHeight()/2.0f*GetWindowScaleDPI().y},{gameEngine->getWidth()/2.0f,gameEngine->getHeight()/2.0f},0,GetWindowScaleDPI().x);

}

/**
 * Loads level information from level definition file.
 * 
 * Once loaded, the correct entity type is created and setup.
 * 
 * @param levelPath Path to level defintion file, relative to exe
 */
void ScenePlay::loadLevel(const std::string& levelPath){
    //TODO: Add the reading of the level file (Modify what you did from Assignment 3)

    std::ifstream file(levelPath);
    std::string str;
    std::string type;

    // enemy vars
    std::string ai;
    int roomX, roomY, x, y, np, speed, health, pairX, pairY;
    std::vector <Vec2> patrolPoints;

    //TODO: Add the reading of decorations (DEC) from the level definition file
    //Refer to the assignment PDF for the formatting of decorations
    while (file.good()) {
        file >> str;
        if (str == "TILE") {
            file >> type >> roomX >> roomY >> x >> y;
            auto e = entityManager.addEntity(str, type);
            e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
            float scaledX = gameEngine->getAssets().getAnimation(type).getScaledSize().x;
            float scaledY = gameEngine->getAssets().getAnimation(type).getScaledSize().y;
            float tileSizeX = gameEngine->getTileSizeX();
            float tileSizeY = gameEngine->getTileSizeY();
            float bboxSizeX = tileSizeX;
            float bboxSizeY = tileSizeY;
            if (scaledX > tileSizeX || scaledY > tileSizeY) {
                bboxSizeX = scaledX;
                bboxSizeY = scaledY;
            }
            e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
            if (type == "POND") e->getComponent<CBoundingBox>().blocksVision = false;
            Vec2 global = getPosition(roomX, roomY, x, y);
            Vec2 pos = gridToMidPixel(global.x, global.y, e);
            e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
            e->getComponent<CTransform>().prevPosition.x = pos.x;
            e->getComponent<CTransform>().prevPosition.y = pos.y;
        }
        if (str == "PLAYER") {
            file >> playerConfig.X >> playerConfig.Y >> playerConfig.BX >> playerConfig.BY >> playerConfig.SPEED >> playerConfig.HEALTH >> playerConfig.WEAPON;
        }
        if (str == "DEC") {
            file >> type >> roomX >> roomY >> x >> y;
            auto e = entityManager.addEntity(str, type);
            e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
            float scaledX = gameEngine->getAssets().getAnimation(type).getScaledSize().x;
            float scaledY = gameEngine->getAssets().getAnimation(type).getScaledSize().y;
            float tileSizeX = gameEngine->getTileSizeX();
            float tileSizeY = gameEngine->getTileSizeY();
            Vec2 global = getPosition(roomX, roomY, x, y);
            Vec2 pos = gridToMidPixel(global.x, global.y, e);
            e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
            e->getComponent<CTransform>().prevPosition.x = pos.x;
            e->getComponent<CTransform>().prevPosition.y = pos.y;
        }
        if (str == "ENEMY") {
            file >> type >> ai;

            if (ai == "PATROL") {
                file >> roomX >> roomY >> x >> y >> speed >> health >> np;
                patrolPoints.clear();
                auto e = entityManager.addEntity("DYNAMIC", str);
                e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
                for (int i = 0; i < np; ++i) {
                    file >> pairX >> pairY;
                    Vec2 global = getPosition(roomX, roomY, pairX, pairY);
                    patrolPoints.push_back(gridToMidPixel(global.x, global.y, e));
                }
                float scaledX = gameEngine->getAssets().getAnimation(type).getScaledSize().x;
                float scaledY = gameEngine->getAssets().getAnimation(type).getScaledSize().y;
                float tileSizeX = gameEngine->getTileSizeX();
                float tileSizeY = gameEngine->getTileSizeY();
                float bboxSizeX = tileSizeX;
                float bboxSizeY = tileSizeY;
                if (scaledX > tileSizeX || scaledY > tileSizeY) {
                    bboxSizeX = scaledX;
                    bboxSizeY = scaledY;
                }
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
                Vec2 global = getPosition(roomX, roomY, x, y);
                Vec2 pos = gridToMidPixel(global.x, global.y, e);
                e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
                e->getComponent<CTransform>().prevPosition.x = pos.x;
                e->getComponent<CTransform>().prevPosition.y = pos.y;
                e->addComponent<CDamage>(1);
                e->addComponent<CPatrol>(patrolPoints, speed);
                e->getComponent<CPatrol>().currentPosition = 0;
                e->addComponent<CHealth>(health, health);
                e->addComponent<CSpeed>(2);
                e->addComponent<CDefense>(1);
            }
            else if (ai == "FOLLOW") {
                file >> roomX >> roomY >> x >> y >> speed >> health;

                auto e = entityManager.addEntity("DYNAMIC", str);
                e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
                float scaledX = gameEngine->getAssets().getAnimation(type).getScaledSize().x;
                float scaledY = gameEngine->getAssets().getAnimation(type).getScaledSize().y;
                float tileSizeX = gameEngine->getTileSizeX();
                float tileSizeY = gameEngine->getTileSizeY();
                float bboxSizeX = tileSizeX;
                float bboxSizeY = tileSizeY;
                if (scaledX > tileSizeX || scaledY > tileSizeY) {
                    bboxSizeX = scaledX;
                    bboxSizeY = scaledY;
                }
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
                Vec2 global = getPosition(roomX, roomY, x, y);
                Vec2 pos = gridToMidPixel(global.x, global.y, e);
                e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
                e->getComponent<CTransform>().prevPosition.x = pos.x;
                e->getComponent<CTransform>().prevPosition.y = pos.y;
                e->addComponent<CDamage>(1);
                e->addComponent<CFollowPlayer>(Vec2(0.0f, 0.0f), speed);
                e->addComponent<CHealth>(health, health);
                e->addComponent<CSpeed>(10);
                e->addComponent<CDefense>(1);

            }
        }
    }
}

/**
 * Animation System
 * 
 * For the player: Sets the state and animation depending on user input.
 * For other entities: Updates the animation via the CAnimation component.
 *
 */
void ScenePlay::sAnimation() {
    CState& playerState = player->getComponent<CState>();
    CAnimation& playerAnim = player->getComponent<CAnimation>();
    CTransform& transf = player->getComponent<CTransform>();

    const bool isMoving =
        std::abs(transf.velocity.x) > 0.05f ||
        std::abs(transf.velocity.y) > 0.05f;

    std::string nextAnimation = playerAnim.animation.getName();

    if (playerState.isAttacking) {
        if (transf.facing.y == 1) {
            nextAnimation = "OLUSEU";
        }
        else if (transf.facing.y == -1) {
            nextAnimation = "OLUSED";
        }
        else if (transf.facing.x == -1 || transf.facing.x == 1) {
            nextAnimation = "OLUSER";
        }
    }
    else {
        if (transf.facing.y == 1) {
            nextAnimation = isMoving ? "OLWALKU" : "OLSTANDU";
        }
        else if (transf.facing.y == -1) {
            nextAnimation = isMoving ? "OLWALKD" : "OLSTANDD";
        }
        else if (transf.facing.x == -1 || transf.facing.x == 1) {
            nextAnimation = isMoving ? "OLWALKR" : "OLSTANDR";
        }
    }

    if (nextAnimation != playerAnim.animation.getName()) {
        playerAnim.animation = gameEngine->getAssets().getAnimation(nextAnimation);
    }

    for (auto& e : entityManager.getEntities()) {
        if (e->hasComponent<CAnimation>()) {
            e->getComponent<CAnimation>().animation.update();
        }
    }
}

void ScenePlay::sWeapons() {
    for (auto& e : entityManager.getEntities("WEAPON")) {
        if (!e->hasComponent<CTransform>() || !e->hasComponent<CLifespan>()) {
            continue;
        }

        CTransform& playerTransform = player->getComponent<CTransform>();
        CTransform& weaponTransform = e->getComponent<CTransform>();
        CLifespan& life = e->getComponent<CLifespan>();

        float progress = 1.0f - (static_cast<float>(life.remaining) / static_cast<float>(life.total));
        float radius = static_cast<float>(gameEngine->getTileSizeX());

        float startAngle = 0.0f;
        float endAngle = 0.0f;

        if (playerTransform.facing.y == 1) {
            startAngle = 225.0f;
            endAngle = 315.0f;
        }
        else if (playerTransform.facing.x == 1) {
            startAngle = -45.0f;
            endAngle = 45.0f;
        }
        else if (playerTransform.facing.y == -1) {
            startAngle = 45.0f;
            endAngle = 135.0f;
        }
        else if (playerTransform.facing.x == -1) {
            startAngle = 135.0f;
            endAngle = 225.0f;
        }

        float angle = startAngle + (endAngle - startAngle) * progress;
        float radians = angle * DEG2RAD;

        weaponTransform.prevPosition = weaponTransform.position;
        weaponTransform.position.x = playerTransform.position.x + std::cos(radians) * radius;
        weaponTransform.position.y = playerTransform.position.y + std::sin(radians) * radius;
        weaponTransform.angle = angle + 90.0f;
    }

}

/**
 * Movement System
 * 
 * Moves all DYNAMIC entities
 * 
 */
void ScenePlay::sMovement() {
    for (auto& e : entityManager.getEntities("DYNAMIC")) {

        if (e->hasComponent<CInput>()) {
            CInput& inp = e->getComponent<CInput>();
            CTransform& transf = e->getComponent<CTransform>();
            CState& state = e->getComponent<CState>();

            const float ACCEL = 0.35f;
            const float DRAG = 0.84f;
            const float MAX_SPEED = playerConfig.SPEED;

            if (!state.isAttacking) {

                Vec2 inputDir(0.0f, 0.0f);
                if (inp.left)  inputDir.x -= 1.0f;
                if (inp.right) inputDir.x += 1.0f;
                if (inp.up)    inputDir.y -= 1.0f;
                if (inp.down)  inputDir.y += 1.0f;

                if (inputDir.length() > 0.0f) {
                    if (std::abs(inputDir.x) >= std::abs(inputDir.y)) {
                        transf.facing.x = (inputDir.x > 0.0f) ? 1.0f : -1.0f;
                        transf.facing.y = 0.0f;
                    }
                    else {
                        transf.facing.x = 0.0f;
                        transf.facing.y = (inputDir.y > 0.0f) ? -1.0f : 1.0f;
                    }
                }

                if (inputDir.length() > 0.0f) {
                    inputDir = inputDir.normalized();
                    Vec2 targetVelocity = inputDir * MAX_SPEED;

                    transf.velocity.x += (targetVelocity.x - transf.velocity.x) * ACCEL;
                    transf.velocity.y += (targetVelocity.y - transf.velocity.y) * ACCEL;
                }
                else {
                    transf.velocity.x *= DRAG;
                    transf.velocity.y *= DRAG;

                    if (std::abs(transf.velocity.x) < 0.05f) {
                        transf.velocity.x = 0.0f;
                    }
                    if (std::abs(transf.velocity.y) < 0.05f) {
                        transf.velocity.y = 0.0f;
                    }
                }
            }
            else {
                transf.velocity.x = 0.0f;
                transf.velocity.y = 0.0f;
            }

            room = Vec2(
                floor(player->getComponent<CTransform>().position.x / gameEngine->getWidth()),
                floor(player->getComponent<CTransform>().position.y / gameEngine->getHeight())
            );
        }

        if (e->hasComponent<CFollowPlayer>()) {
            CFollowPlayer& follow = e->getComponent<CFollowPlayer>();
            CTransform& transf = e->getComponent<CTransform>();
            CTransform& playerPos = player->getComponent<CTransform>();
            bool blocked = false;
            follow.home = playerPos.position;

            for (auto& e2 : entityManager.getEntities()) {
                if (e2->getComponent<CBoundingBox>().blocksVision) {
                    if (e == e2) continue;
                    if (e2->getID() == "PLAYER") continue;
                    if (e2->getID() == "ENEMY") continue;
                    if (e2->getID() == "SWORD") continue;

                    if (Physics::entityIntersect(transf.position, playerPos.position, e2)) {
                        blocked = true;
                        break;
                    }
                }
            }

            Vec2 delta = follow.home - transf.position;
            float dist = delta.length();

            if (dist > 0.0f && !blocked) {
                transf.velocity = delta.normalized() * follow.speed;
            }
            else {
                transf.velocity.x = 0.0f;
                transf.velocity.y = 0.0f;
            }
        }

        if (e->hasComponent<CPatrol>()) {
            CPatrol& patrol = e->getComponent<CPatrol>();
            CTransform& transf = e->getComponent<CTransform>();

            Vec2 target = patrol.positions[patrol.currentPosition];
            Vec2 delta = target - transf.position;
            float dist = delta.length();

            if (dist <= patrol.speed) {
                transf.position = target;
                transf.velocity.x = 0.0f;
                transf.velocity.y = 0.0f;
                patrol.currentPosition = (patrol.currentPosition + 1) % patrol.positions.size();
            }
            else {
                transf.velocity = delta.normalized() * patrol.speed;
            }
        }

        if (e->hasComponent<CTransform>()) {
            CTransform& transform = e->getComponent<CTransform>();
            transform.prevPosition = transform.position;
            transform.position += transform.velocity;
        }

        if (e->hasComponent<CInvincibility>()) {
            CInvincibility& inv = e->getComponent<CInvincibility>();
            if (inv.remaining > 0) {
                inv.remaining--;
            }
            else {
                e->removeComponent<CInvincibility>();
            }
        }
    }

    for (auto& e : entityManager.getEntities("WEAPON")) {
        //weaponSwing(e, player, entityManager, gameEngine);
	}
}


void ScenePlay::sCollision() {
    for (auto& de : entityManager.getEntities("DYNAMIC")) {
        int topYCollisions = 0;

        // deferred resolution variables
		bool skipPos = false; // skip position resolution if true
        float maxTopPush = 0.0f;    // push-up amount when landing on top of tiles
        float maxBottomPush = 0.0f; // push-down amount when hitting underside of tiles
		float maxLeftPush = 0.0f; // push-left amount when hitting right side of tiles
		float maxRightPush = 0.0f; // push-right amount when hitting left side of tiles
        std::vector<std::shared_ptr<Entity>> topColliders;
        std::vector<std::shared_ptr<Entity>> bottomColliders;
		std::vector<std::shared_ptr<Entity>> leftColliders;
		std::vector<std::shared_ptr<Entity>> rightColliders;

        for (auto& e : entityManager.getEntities()) {
            if (de == e) continue;
            if (de->getTag() == "WEAPON" || e->getTag() == "WEAPON") skipPos = true;
			if (de->getID() == "SWORD" || de->getID() == "HEART" || e->getID() == "SWORD" || e->getID() == "HEART") if (de->getID() != "PLAYER" && e->getID() != "PLAYER") skipPos = true;
            if (e->getTag() == "DEC") continue;

            Vec2 prevCol = Physics::getPreviousOverlap(de, e);
            Vec2 currCol = Physics::getOverlap(de, e);

            if (currCol.x > 0.0f && currCol.y > 0.0f) {

                // -------------------- INTERACTION RESOLUTIONS -------------------- 

                if (e->getID() == "HEART" && de->hasComponent<CHealth>() || de->getID() == "HEART" && e->hasComponent<CHealth>()) {
                    auto heart = (de->getID() == "HEART") ? de : e;
                    auto entity = (de->getID() == "ENTITY") ? de : e;
                    if (!(entity->getComponent<CHealth>().current == entity->getComponent<CHealth>().max)) {
                        entity->getComponent<CHealth>().current += 1;
                    }
                    gameEngine->playSound("HEART");
                    heart->destroy();
                    skipPos = true;
                    continue;
				}

                if (de->getID() == "PLAYER" && e->getID() == "BLACK") {
                    teleport(e);
                }

                if (de->getTag() == "WEAPON" && e->getID() == "ENEMY" || de->getID() == "ENEMY" && e->getTag() == "WEAPON") {
                    auto sword = (de->getTag() == "WEAPON") ? de : e;
                    auto entity = (de->getID() == "ENEMY") ? de : e;
                    if (!entity->hasComponent<CInvincibility>()) {
                        entity->getComponent<CHealth>().current -= de->getComponent<CDamage>().damage;
                        entity->addComponent<CInvincibility>(30);
                        entity->getComponent<CInvincibility>().remaining = 30;
                        gameEngine->playSound("ENEMYHIT");
                        if (entity->getComponent<CHealth>().current <= 0) {
                            gameEngine->playSound("ENEMYKILL");
                            spawnHeart(entity->getComponent<CTransform>().position);
                            entity->destroy();
                        }
                    }
                    skipPos = true;
                    continue;
                }

                if ((de->getID() == "PLAYER" && e->getID() == "ENEMY") ||
                    (de->getID() == "ENEMY" && e->getID() == "PLAYER")) {
                    auto playerEntity = (de->getID() == "PLAYER") ? de : e;
                    auto enemyEntity = (de->getID() == "ENEMY") ? de : e;

                    // Allow enemies to push player, not the other way around.
                    if (de->getID() == "ENEMY") {
                        continue;
                    }

                    if (battlecooldown <= 0) {
                        gameEngine->changeScene("BATTLE", std::make_shared<SceneBattle>(gameEngine, playerEntity, enemyEntity, shared_from_this()));
                    }
                    battlecooldown = 100;
                }

                // -------------------- POSITIONAL RESOLUTIONS --------------------

                bool prevSepX = (prevCol.x <= 0.0f);
                bool prevSepY = (prevCol.y <= 0.0f);

                if (!skipPos) {

                    // horizontal. previous overlap in y direction, but not in x direction
                    if (prevSepX && !prevSepY) {
                        if (de->getComponent<CTransform>().prevPosition.x < e->getComponent<CTransform>().prevPosition.x) {
                            // left
                            if (currCol.x > maxLeftPush) {
                                maxLeftPush = currCol.x;
                            }
                            leftColliders.push_back(e);
                        }
                        else {
                            // right
                            if (currCol.x > maxRightPush) {
                                maxRightPush = currCol.x;
                            }
                            rightColliders.push_back(e);
                        }
                    }
                    // vertical. previous overlap in x direction, but not in y direction
                    else if (prevSepY && !prevSepX) {
                        if (de->getComponent<CTransform>().prevPosition.y < e->getComponent<CTransform>().prevPosition.y) {
                            // on top 
                            if (currCol.y > maxTopPush) {
                                maxTopPush = currCol.y;
                            }
                            topColliders.push_back(e);
                        }
                        else {
                            // below 
                            if (currCol.y > maxBottomPush) {
                                maxBottomPush = currCol.y;
                            }
                            bottomColliders.push_back(e);
                        }
                    }
                    // horizontal and vertical (from corner). no previous overlap in x or y directions and dynamic entity is not grounded to prevent false positives for corner collisions when grounded
                    else if (prevSepX && prevSepY) {
                        if (currCol.x < currCol.y) {
                            if (de->getComponent<CTransform>().prevPosition.x < e->getComponent<CTransform>().prevPosition.x) {
                                if (currCol.x > maxLeftPush) {
                                    maxLeftPush = currCol.x;
                                }
                                leftColliders.push_back(e);
                            }
                            else {
                                if (currCol.x > maxRightPush) {
                                    maxRightPush = currCol.x;
                                }
                                rightColliders.push_back(e);
                            }
                        }
                        else {
                            if (de->getComponent<CTransform>().prevPosition.y < e->getComponent<CTransform>().prevPosition.y) {
                                if (currCol.y > maxTopPush) {
                                    maxTopPush = currCol.y;
                                }
                                topColliders.push_back(e);
                            }
                            else {
                                if (currCol.y > maxBottomPush) {
                                    maxBottomPush = currCol.y;
                                }
                                bottomColliders.push_back(e);
                            }
                        }
                    }
                    else {
                        if (currCol.x < currCol.y) {
                            if (de->getComponent<CTransform>().position.x < e->getComponent<CTransform>().position.x) {
                                if (currCol.x > maxLeftPush) {
                                    maxLeftPush = currCol.x;
                                }
                                leftColliders.push_back(e);
                            }
                            else {
                                if (currCol.x > maxRightPush) {
                                    maxRightPush = currCol.x;
                                }
                                rightColliders.push_back(e);
                            }
                        }
                        else {
                            if (de->getComponent<CTransform>().position.y < e->getComponent<CTransform>().position.y) {
                                if (currCol.y > maxTopPush) {
                                    maxTopPush = currCol.y;
                                }
                                topColliders.push_back(e);
                            }
                            else {
                                if (currCol.y > maxBottomPush) {
                                    maxBottomPush = currCol.y;
                                }
                                bottomColliders.push_back(e);
                            }
                        }
                    }

                }

            }
        }

        // vertical top resolution
        if (maxTopPush > 0.0f) {
            de->getComponent<CTransform>().position.y -= maxTopPush;
        }

        // vertical under/below resolution
        if (maxBottomPush > 0.0f) {
            de->getComponent<CTransform>().position.y += maxBottomPush;
        }

        // horizontal left resolution
        if (maxLeftPush > 0.0f) {
            de->getComponent<CTransform>().position.x -= maxLeftPush;
        }

        // horizontal right resolution
        if (maxRightPush > 0.0f) {
            de->getComponent<CTransform>().position.x += maxRightPush;
        }
    }
}

/**
 * Collision System
 * 
 * Checks for any collisions between DYNAMIC objects and any other objects
 * 
 */


/**
 * Render System
 * 
 * Renders the textures, using the camera system
 * 
 */
void ScenePlay::sRender(){
    BeginDrawing();
    BeginMode2D(mainCamera);
    ClearBackground(Color(252,216,168,255));

    //********** Raylib Drawing Content **********
        if(renderTextures){
            renderTex();
            if(renderHealth)
                renderHealthBar();
        }
        if(renderBoundingBox)
            renderBB();
        if(renderGridLines)
            renderGrid();
        if(renderVisionDebug){
            renderAIDebug();
        }

    EndMode2D();
        //********** ImGUI Content *********
        sGUI();

    EndDrawing();
}

/**
 * ImGUI System
 * 
 * Renders the ImGUI
 * 
 */
void ScenePlay::sGUI(){
    rlImGuiBegin();
    ImGui::SetNextWindowSize(ImVec2(400, 350));
        ImGui::Begin("Debug",NULL,ImGuiWindowFlags_NoResize);
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Controls"))
                {
                    ImGui::SeparatorText("Rendering Controls");
                    ImGui::Checkbox("Textures",&renderTextures);
                    ImGui::Checkbox("Health Bar",&renderHealth);
                    ImGui::Checkbox("Bounding Boxes",&renderBoundingBox);
                    ImGui::Checkbox("Vision Debug",&renderVisionDebug);
                    ImGui::Checkbox("Grid",&renderGridLines);
                    ImGui::SeparatorText("Camera Controls");
                    ImGui::Checkbox("Follow Camera",&followCam);
                    /* Room position debug
                    ImGui::Text(std::to_string(room.x).c_str());
                    ImGui::Text(std::to_string(room.y).c_str());
                    */
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Entities"))
                {
                    int count=0;
                    if (ImGui::TreeNode("By Tag")) {
                        for (auto& tag : entityManager.getEntityMap()) {
                            if (ImGui::TreeNode(tag.first.c_str())) {
                                for (auto& e : tag.second) {
                                    std::string str = "D###" + std::to_string(reinterpret_cast<uintptr_t>(e.get()));
                                    if (ImGui::Button(str.c_str())) {
                                        e->destroy();
                                    }
                                    ImGui::SameLine();
                                    std::string text = tag.first + " " + e->getID() + " " + "(" + std::to_string((int)e->getComponent<CTransform>().position.x) + ", " + std::to_string((int)e->getComponent<CTransform>().position.y) + ")";
                                    ImGui::Text(text.c_str());
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("All")) {
                        for (auto e : entityManager.getEntities()) {
                            std::string str = "D###" + std::to_string(reinterpret_cast<uintptr_t>(e.get()));
                            if (ImGui::Button(str.c_str())) {
                                e->destroy();
                            }
                            ImGui::SameLine();
                            std::string text = e->getTag() + " " + e->getID() + " " + "(" + std::to_string((int)e->getComponent<CTransform>().position.x) + ", " + std::to_string((int)e->getComponent<CTransform>().position.y) + ")";
                            ImGui::Text(text.c_str());
                        }
                        ImGui::TreePop();
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        ImGui::End();
        rlImGuiEnd();
}

/**
 * Render entity health bars
 */
void ScenePlay::renderHealthBar(){
    for(auto& e : entityManager.getEntities("DYNAMIC")){
        if(e->hasComponent<CHealth>()){
            CHealth health=e->getComponent<CHealth>();
            Vec2 position=e->getComponent<CTransform>().position;
            Vec2 size=e->getComponent<CAnimation>().animation.getScaledSize();
            float yPos=position.y-size.y/2-15;
            float xPos=position.x-size.x/2;
            float width=size.x/health.max;
            float shift=0;
            Color c = RED;
            for(int i=0;i<health.max;i++){
                if(i>=health.current) c=BLACK;
                DrawRectangle(xPos+shift, yPos+1, width, 6, c);
                DrawRectangleLines(xPos+shift,yPos,width, 8,BLACK);
                shift+=width-1;
            }
        }
    }
}

/**
 * Render AI Debug information, including vision and patrol paths
 */
void ScenePlay::renderAIDebug(){
    for(auto& e : entityManager.getEntities("DYNAMIC")){
        if(e->hasComponent<CPatrol>()){
            auto patrol=e->getComponent<CPatrol>();
            Vec2 point;
            Vec2 nextPoint;
            for(int i=0;i<patrol.positions.size();i++){
                point=patrol.positions[i];
                int nextIndex=(i+1)%patrol.positions.size();
                nextPoint=patrol.positions[nextIndex];
                DrawLineEx(Vector2(point.x,point.y), Vector2(nextPoint.x,nextPoint.y),2, BLUE);
                DrawCircle(point.x,point.y,7,BLUE);
            }
            
        }
        if(e->hasComponent<CFollowPlayer>()){
            Vec2& home = e->getComponent<CFollowPlayer>().home;
            auto& position=e->getComponent<CTransform>().position;
            DrawLineEx(Vector2(position.x,position.y), Vector2(home.x,home.y),2, BLUE);
        }
    }
}

/**
 * Do Action System
 * 
 * Updates the player input and game state based on PRESS or RELEASE actions
 * 
 * @param action action sent from the gameEngine that contains type and name
 */
void ScenePlay::sDoAction(const Action& action) {
    CInput& input = player->getComponent<CInput>();
    CState& state = player->getComponent<CState>();

    if ((action.getType() == "PRESS")) {
        if (action.getName() == "BB") {
            renderBoundingBox = !renderBoundingBox;
        }
        if (action.getName() == "GRID") {
            renderGridLines = !renderGridLines;
        }
        if (action.getName() == "TEX") {
            renderTextures = !renderTextures;
        }
        if (action.getName() == "HEALTH") {
            renderHealth = !renderHealth;
        }
        if (action.getName() == "VISION") {
            renderVisionDebug = !renderVisionDebug;
        }
        if (action.getName() == "QUIT") {
            gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine));
        }
        if (action.getName() == "RELOAD") {
            reload = true;
        }

        if (action.getName() == "UP") {
            input.up = true;
        }
        if (action.getName() == "DOWN") {
            input.down = true;
        }
        if (action.getName() == "LEFT") {
            input.left = true;
        }
        if (action.getName() == "RIGHT") {
            input.right = true;
        }
        if (action.getName() == "ATTACK" || action.getName() == "MOUSE_LEFT") {
            if (!state.isAttacking) {           
                /*
                input.attack = true;
                state.isAttacking = true;
                usePrimaryWeapon(player, entityManager, gameEngine);
                */
            }
        }
        if (action.getName() == "BATTLE_TEST") {
			gameEngine->changeScene("BATTLE", std::make_shared<SceneBattle>(gameEngine, player, player, shared_from_this()));
            //gameEngine->stopMusic("TITLEMUSIC");
        }
    }

    if ((action.getType() == "RELEASE")) {
        if (action.getName() == "UP") {
            input.up = false;
        }
        if (action.getName() == "DOWN") {
            input.down = false;
        }
        if (action.getName() == "LEFT") {
            input.left = false;
        }
        if (action.getName() == "RIGHT") {
            input.right = false;
        }
        if (action.getName() == "ATTACK") {
            input.attack = false;
        }
    }

    if (action.getType() == "MOUSE_MOVE") {
        input.mouseLocation = action.getPosition();
    }
}

/**
 * Camera System
 * 
 * Updates camera
 * 
 */
void ScenePlay::sCamera(){
    CTransform& transf = player->getComponent<CTransform>();

    if (followCam) {
        mainCamera.target.x = transf.position.x;
        mainCamera.target.y = transf.position.y;
    }
    else {
        int w = gameEngine->getWidth();
        int h = gameEngine->getHeight();
        mainCamera.target.x = room.x * w + w / 2;
        mainCamera.target.y = room.y * h + h / 2;
    }
}

void ScenePlay::battleReturn(std::shared_ptr<Entity> e) {
    e->destroy();
	gameEngine->playMusic("TITLEMUSIC");
    player->getComponent<CTransform>().velocity = Vec2(0.0f, 0.0f);
	player->addComponent<CInvincibility>(100);
    player->getComponent<CInvincibility>().remaining = 100;
}

/**
 * Spawns Player
 * 
 * Example of spawning a simple player
 * 
 */
void ScenePlay::spawnPlayer(){
    player=entityManager.addEntity("DYNAMIC", "PLAYER");
    player->addComponent<CState>("DOWN");
    player->addComponent<CInput>();
    player->addComponent<CHealth>(playerConfig.HEALTH, playerConfig.HEALTH);
    player->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("OLSTANDD"),true);
    int scaledHeight=player->getComponent<CAnimation>().animation.getScaledSize().y;
    int scaledWidth=player->getComponent<CAnimation>().animation.getScaledSize().x - 30;
    player->addComponent<CBoundingBox>(Vec2(gameEngine->getTileSizeX(), gameEngine->getTileSizeY()));
    Vec2 pos = gridToMidPixel(playerConfig.X,playerConfig.Y,player);
    player->addComponent<CTransform>(Vec2(pos.x,pos.y), Vec2(0.0f,0.0f), 0.0f);
    player->addComponent<CEquipment>();
    player->getComponent<CEquipment>().weapons.push_back(gameEngine->getAssets().getWeapon("ANCIENTBLADE"));
	player->getComponent<CEquipment>().currentWeapon = player->getComponent<CEquipment>().weapons[0];
    player->addComponent<CDefense>(3);
    player->addComponent<CSpeed>(5);
	player->getComponent<CEquipment>().items.push_back(gameEngine->getAssets().getItem("SHEAL"));
    player->getComponent<CEquipment>().items.push_back(gameEngine->getAssets().getItem("SHEAL"));
    player->getComponent<CEquipment>().items.push_back(gameEngine->getAssets().getItem("SMANA"));
    player->addComponent<CName>("Player");
}

/**
 * Spawns a sword at the player's location
 */
void ScenePlay::spawnSword() {
    gameEngine->playSound("LINKSWING");
    gameEngine->playSound("LINKYELP");
    auto e = entityManager.addEntity("DYNAMIC", "SWORD");
    e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("SWORD"), true);
    e->addComponent<CLifespan>(10);
    e->getComponent<CLifespan>().remaining = 10;
    e->addComponent<CDamage>(1);
    e->addComponent<CState>("NOHIT");
    float bboxSizeX = gameEngine->getAssets().getAnimation("SWORD").getScaledSize().x;
    float bboxSizeY = gameEngine->getAssets().getAnimation("SWORD").getScaledSize().y;

    CTransform& ptsf = player->getComponent<CTransform>();
    float px = ptsf.position.x;
    float py = ptsf.position.y;
    e->addComponent<CTransform>();
    CTransform& transf = e->getComponent<CTransform>();
    if (ptsf.facing.y == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py - gameEngine->getTileSizeY();
        transf.prevPosition.y = py - gameEngine->getTileSizeY();
        transf.angle = 0.0f;
    }
    else if (ptsf.facing.y == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py + gameEngine->getTileSizeY() - 4;
        transf.prevPosition.y = py + gameEngine->getTileSizeY();
        transf.angle = 180.0f;
    }
    else if (ptsf.facing.x == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px + gameEngine->getTileSizeX() - 4;
        transf.prevPosition.x = px + gameEngine->getTileSizeX() - 4;
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 90.0f;
    }
    else if (ptsf.facing.x == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px - gameEngine->getTileSizeX() + 4;
        transf.prevPosition.x = px - gameEngine->getTileSizeX() + 4;
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 270.0f;
    }
}

/**
* Spawns a heart at a defeated enemy's location.
*/
void ScenePlay::spawnHeart(Vec2& position) {
	auto e = entityManager.addEntity("DYNAMIC", "HEART");
	e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("HEART"), true);
    e->addComponent<CBoundingBox>(Vec2(gameEngine->getAssets().getAnimation("HEART").getScaledSize().x, gameEngine->getAssets().getAnimation("HEART").getScaledSize().y));
	e->addComponent<CTransform>(Vec2(position.x, position.y), Vec2(0.0f, 0.0f), 0.0f);
    e->getComponent<CBoundingBox>().blocksVision = false;
}

/**
* Teleports the player to a random cave entrance location on the map
*/
void ScenePlay::teleport(std::shared_ptr<Entity> e) {
	std::vector<std::shared_ptr<Entity>> caveEntrances;
    for(auto& entity : entityManager.getEntities("TILE")) {
        if (entity->getID() == "BLACK" && entity->getComponent<CTransform>().position != e->getComponent<CTransform>().position) {
			caveEntrances.push_back(entity);
        }
	}
    if (caveEntrances.empty()) {
        return;
	}

    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, caveEntrances.size() - 1);
    auto choice = caveEntrances[dist(gen)];

	player->getComponent<CTransform>().position = Vec2(choice->getComponent<CTransform>().position.x, choice->getComponent<CTransform>().position.y + gameEngine->getTileSizeY() * 2);
}

/**
 * Reloads this play scene
 */
void ScenePlay::reloadScene(){
    gameEngine->changeScene("PLAY",std::make_shared<ScenePlay>(gameEngine, levelPath));
}

/**
 * Gets the world position from room x,y and tile x,y
 * @param rx room x
 * @param ry room y
 * @param tx tile x
 * @param ty tile y
 * return Vec2 with the world position
 */
Vec2 ScenePlay::getPosition(int rx, int ry, int tx, int ty) {
    int w = gameEngine->getTilesX();
    int h = gameEngine->getTilesY();
    return Vec2(rx * w + tx, ry * h + ty);
}

/**
* Converts mouse position in window coordinates to world coordinates
*/
Vec2 ScenePlay::mouseToWorld(Vec2 windowPos) {
    //convert to world
    Vector2 worldMouse = GetScreenToWorld2D(Vector2(windowPos.x, windowPos.y), mainCamera);
    return { worldMouse.x,worldMouse.y };
}

/**
* Lifespan system 
* 
* Checks entities' lifespans and removes them if needed.
* 
*/
void ScenePlay::sLifespan() {
    for (auto& e : entityManager.getEntities()) {
        if (e->hasComponent<CLifespan>()) {
            e->getComponent<CLifespan>().remaining--;
            if (e->getComponent<CLifespan>().remaining <= 0) {
                if (e->getID() == "ANCIENTBLADE") {
                    player->getComponent<CState>().isAttacking = false;
                }
                e->destroy();
            }
        }
    }
}

/**
 * Updates all systems and entityManager
 */
void ScenePlay::update(){
    entityManager.update();

    if (battlecooldown > 0) {
        if (battlecooldown == 0) {
            battlecooldown = 0;
        }
        else {
            battlecooldown--;
        }
    }

    sMovement();
    //sWeapons();
    sAnimation();
    sCollision();
    sLifespan();
    sMusic();
    sCamera();
    sRender();

    if(reload==true){
        reloadScene();
    }
    
}